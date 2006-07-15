/* $Id: filetransferthread.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
 *
 * Turnstile
 *
 * Copyright (C) 2004, J.D. Henderson <www.digitalpeer.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "filetransferthread.h"
#include "files.h"
#include "network.h"
#include "utils.h"
#include "packet.h"
#include "packetgen.h"
#include "crypto.h"
#include "prefs.h"

#include <string>
using namespace std;

#include <wx/file.h>
#include <wx/timer.h>

FileTransferThread::FileTransferThread(ShareFile* file)
	: wxThread(),Connection(0,-1, TYPE_DOWNLOAD, file->Node()), file_(file)
{;}

void*
FileTransferThread::Entry()
{
	Get(*file_);
	KillSocket(-1);

	return 0;
}

void
FileTransferThread::Get(ShareFile& file)
{
	// save the info file (recovery file) every this count of packets
	const int SAVE_INFO_FILE_EVERY = 10;
	// counter for when to save info file
	int saveInfoFile = 0;
	// the total bytes read in THIS session
	unsigned int totalBytesTransfered = 0;

	LogMsg(wxString().Format("Downloading file %s ...",file.Path().c_str()));

	if (!Connect())
	{
		LogError(wxString().Format("Handshaking failed when starting file transfer in con id %d",id_));
		file.Status(ShareFile::STATUS_FAILED);
		return;
	}

	/*
	Send the request for the file segment.
	*/
	Packet snd = GenPacket_Request(file.Name(),file.MD5());
	if (!Send(snd))
	{
		LogError(wxString().Format("Failed when requesting file on con id %d",id_));
		file.Status(ShareFile::STATUS_FAILED);
		return;
	}

	wxFile data(file.PartPath(), wxFile::write);

	if (!data.IsOpened())
	{
		LogMsg(wxString().Format("Failed to open download file %s for writing",file.Path().c_str()));
		file.Status(ShareFile::STATUS_FAILED);
		data.Close();
		return;
	}

	if (!file.LoadInfoFile())
	{
		file.Status(ShareFile::STATUS_FAILED);
		data.Close();
		LogError("Corrupt info file.");
		return;
	}

	// seek to where we left off
	if (data.Seek(file.CompletedSize(), wxFromStart) != file.CompletedSize())
	{
		file.Status(ShareFile::STATUS_FAILED);
		data.Close();
		LogError("Info file and part file do not match.");
		return;
	}

	file.Status(ShareFile::STATUS_INPROGRESS);

	sw_->Start(0);
	bool first = true;

	while (true)
	{
		if (Network::Instance()->KillAll() ||
			file.Status() == ShareFile::STATUS_CANCELLED ||
			file.Status() == ShareFile::STATUS_PAUSED)
		{
			//LogDebug("File transfer thread got signal to exit");
			file.Status(ShareFile::STATUS_PAUSED);
			data.Close();
			return;
		}
		else if (file.Size() == file.CompletedSize())
		{
			LogMsg(wxString().Format("Download of file %s completed.",file.Path().c_str()));
			file.Status(ShareFile::STATUS_FINISHED);

			file.PartToReal();

			if (crypto::Md5Hash(file.Path()) != file.MD5())
			{
				LogError("MD5's don't match!  File is corrupt.");
				file.MD5(file.MD5() + " (Downloaded file does not match MD5)");
				file.Status(ShareFile::STATUS_FAILED);
			}

			data.Close();
			return;
		}
		else if (file.Size() < file.CompletedSize())
		{
			LogError("Got too many bytes in file!");
			file.Status(ShareFile::STATUS_FAILED);
			data.Close();
			return;
		}

		unsigned int end;

		if (first)
		{
			first = false;
			/*
			Request the next file segment we need with a start and ending byte.
			*/
			end = file.CompletedSize() + FILE_SEG_BUFFER_SIZE;
			if (end > file.Size())
				end = file.Size();

			Packet snd = GenPacket_Cont(file.CompletedSize(),end);
			if (!Send(snd))
			{
				LogError(wxString().Format("Failed when sending cont on con id %d",id_));
				file.Status(ShareFile::STATUS_FAILED);
				return;
			}
		}

		/*
		Get the data.
		*/
		Packet* rcv;
		if (!Receive(rcv) || rcv == 0)
		{
			LogError(wxString().Format("Receive failed when trying to get file packet on con id %d", id_));
			file.Status(ShareFile::STATUS_FAILED);
			data.Close();
			delete rcv;
			return;
		}

		if (!rcv->Valid())
		{
			LogError(wxString().Format("Receive invalid packet when trying to get file packet on con id %d", id_));
			file.Status(ShareFile::STATUS_FAILED);
			data.Close();
			delete rcv;
			return;
		}

		/*
		Request the next file segment we need with a start and ending byte.
		*/
		if (file.Size() > end)
		{
			unsigned int start = end;
			end += FILE_SEG_BUFFER_SIZE;
			if (end > file.Size())
				end = file.Size();

			Packet snd = GenPacket_Cont(start,end);
			if (!Send(snd))
			{
				LogError(wxString().Format("Failed when sending cont on con id %d",id_));
				file.Status(ShareFile::STATUS_FAILED);
				return;
			}
		}

		switch(rcv->Type())
		{
		case Packet::TYPE_FILESEG:
		{
			string raw = crypto::HexDecode(rcv->Get(Packet::HEADER_PAYLOAD).c_str());
			unsigned int count = data.Write(raw.c_str(), raw.size());

			//LogDebug(wxString().Format("Received %d (%d) bytes ...",rcv->Get(Packet::HEADER_PAYLOAD).size(),count));

			// make sure the write was successful
			if (count != raw.size())
			{
				LogError(wxString().Format("Failed to write to file (disk may be full) on con id %d", id_));
				file.Status(ShareFile::STATUS_FAILED);
				data.Close();
				delete rcv;
				return;
			}

			file.CompletedSize(file.CompletedSize()+count);
			totalBytesTransfered += count;
			file.Kbs((double)(totalBytesTransfered/1024)/(double)(sw_->Time()/1000));

			++saveInfoFile;

			if (saveInfoFile == SAVE_INFO_FILE_EVERY)
			{
				saveInfoFile = 0;
				//file.SaveInfoFile();
			}

			break;
		}
		default:
		{
			delete rcv;
			data.Close();
			LogError(wxString().Format("Receive bad packet type when trying to get file packet on con id %d", id_));
			file.Status(ShareFile::STATUS_FAILED);
			return;
		}
		}

		delete rcv;
	}
}

void FileTransferThread::OnExit()
{;}
