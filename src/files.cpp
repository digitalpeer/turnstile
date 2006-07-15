/* $Id: files.cpp,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#include "files.h"
#include "crypto.h"
#include "utils.h"
#include "typeconv.h"

#include <wx/wx.h>
#include <wx/textfile.h>
#include <wx/utils.h>

using namespace std;

unsigned int ShareFile::idCounter_ = 0;

const wxString ShareFile::STATUSES[STATUS_COUNT] =
	{
		"In Progress ...",
		"Failed",
		"Stopped/Paused",
		"Complete",
		"Cancelled"
	};

ShareFile::ShareFile()
	: completedSize_(0), size_(0), status_(STATUS_PAUSED),
		crit_(new wxCriticalSection),kbs_(0),
		id_(wxString().Format("id%d",idCounter_++))
{;}

ShareFile::ShareFile(const ShareFile& file)
	: crit_(new wxCriticalSection)
{
	node_ = file.node_;
	key_ = file.key_;
	hash_ = file.hash_;
	size_ = file.size_;
	file_ = file.file_;
	totalSize_ = file.totalSize_;
	completedSize_ = file.completedSize_;
	md5_ = file.md5_;
	status_ = file.status_;
	kbs_ = file.kbs_;
	id_ = file.id_;
}

ShareFile::ShareFile(const wxString& path)
	: crit_(new wxCriticalSection),completedSize_(0),size_(0)
{
	file_.Assign(path);
}

wxString
ShareFile::Path() const
{
	wxCriticalSectionLocker locker(*crit_);

	return file_.GetFullPath();
}

wxString
ShareFile::MD5() const
{
	wxCriticalSectionLocker locker(*crit_);

	return md5_;
}

void
ShareFile::MD5(const wxString& md5)
{
	wxCriticalSectionLocker locker(*crit_);

	md5_ = md5;
}

void
ShareFile::Path(const wxString& path)
{
	wxCriticalSectionLocker locker(*crit_);

	file_.Assign(path);
}

wxString
ShareFile::Key() const
{
	wxCriticalSectionLocker locker(*crit_);

	return key_;
}

void
ShareFile::Key(const wxString& key)
{
	wxCriticalSectionLocker locker(*crit_);

	hash_ = crypto::Sha1Hash(key);
	key_ = key;
}

wxString
ShareFile::Hash() const
{
	wxCriticalSectionLocker locker(*crit_);

	return hash_;
}

void
ShareFile::Hash(const wxString& hash)
{
	wxCriticalSectionLocker locker(*crit_);

	hash_=hash;
}

wxString ShareFile::Name() const
{
	wxCriticalSectionLocker locker(*crit_);

	return file_.GetFullName();
}

void
ShareFile::Size(unsigned int size)
{
	wxCriticalSectionLocker locker(*crit_);

	size_ = size;
}

unsigned int
ShareFile::Size() const
{
	wxCriticalSectionLocker locker(*crit_);

	return size_;
}

wxString
ShareFile::Node() const
{
	wxCriticalSectionLocker locker(*crit_);

	return node_;
}

void
ShareFile::Node(const wxString& node)
{
	wxCriticalSectionLocker locker(*crit_);

	node_ = node;
}

unsigned int
ShareFile::CompletedSize() const
{
	wxCriticalSectionLocker locker(*crit_);

	return completedSize_;
}

void
ShareFile::CompletedSize(unsigned int completed)
{
	wxCriticalSectionLocker locker(*crit_);

	completedSize_ = completed;
}

int
ShareFile::Status() const
{
	wxCriticalSectionLocker locker(*crit_);

	return status_;
}

void
ShareFile::Status(int status)
{
	wxCriticalSectionLocker locker(*crit_);

	status_ = status;

	if (status != STATUS_INPROGRESS)
		kbs_ = 0;
}

void
ShareFile::SaveInfoFile()
{
	wxTextFile out;

	// remove any existing file
	RemoveInfoFile();

	if (!out.Create(wxPathOnly(Path()) + "/." + id_ + ".info"))
	{
		LogDebug(wxString().Format("Error: Could not save info file: %s", (Path() + ".info").c_str()));
		return;
	}

	out.AddLine(Path());
	out.AddLine(MD5());
	out.AddLine(toString(Size()));
	out.AddLine(toString(CompletedSize()));
	out.AddLine(Node());

	out.Write();
}

void
ShareFile::RemoveInfoFile()
{
	wxRemoveFile(wxPathOnly(Path()) + "/." + id_ + ".info");
}

wxString
ShareFile::PartPath()
{
	return wxPathOnly(Path()) + "/" + id_ + ".part";
}

void ShareFile::PartToReal()
{
	RemoveInfoFile();

	bool exists = true;
	int counter = 0;
	while (exists)
	{
		counter++;
		if (!wxFileExists(Path()))
		{
			wxRenameFile(wxPathOnly(Path()) + "/" + id_ + ".part",Path());
			exists = false;
		}
		else
		{
			Path(Path() + wxString().Format(" (%d)",counter));
		}

	}
}

bool
ShareFile::LoadInfoFile()
{
	wxTextFile in;

	if (!in.Open(wxPathOnly(Path()) + "/." + id_ + ".info"))
	{
		//LogDebug(wxString().Format("Error: Could not open info file: %s", (Path() + ".info").c_str()));
		return TRUE;
	}

	const unsigned int size = in.GetLineCount();

	if (size != 5)
	{
		LogError(wxString().Format("Invalid part file: %s", (Path() + "/." + id_ + ".info").c_str()));
		return FALSE;
	}

	int x = 0;

	Path(in[x++]);
	MD5(in[x++]);
	Size(atoi(in[x++].c_str()));
	CompletedSize(atoi(in[x++].c_str()));
	Node(in[x++]);

	return TRUE;
}

void
ShareFile::Kbs(double kbs)
{
	wxCriticalSectionLocker locker(*crit_);

	kbs_ = kbs;
}

double
ShareFile::Kbs() const
{
	wxCriticalSectionLocker locker(*crit_);

	return kbs_;
}

ShareFile::~ShareFile()
{
	delete crit_;
}




