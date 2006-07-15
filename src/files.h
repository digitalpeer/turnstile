/* $Id: files.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file files.h
@brief Instance of a shared file.
*/
#ifndef TURNSTILE__FILES_H
#define TURNSTILE__FILES_H

#include "dhtutils.h"

#include <wx/filename.h>

#include <vector>

class wxString;
class wxCriticalSection;

class ShareFile
{
public:
	enum	{
			STATUS_INPROGRESS,
			STATUS_FAILED,
			STATUS_PAUSED,
			STATUS_FINISHED,
			STATUS_CANCELLED,
			//
			STATUS_COUNT
		};

	static const wxString STATUSES[STATUS_COUNT];

	// TODO speed up file searching by generating the hash and md5 on construction once
	ShareFile();
	ShareFile(const ShareFile& rhs);
	ShareFile(const wxString& path);

	wxString Path() const;
	wxString Key() const;
	void Path(const wxString& path);
	void Key(const wxString& key);
	void Size(unsigned int size);
	unsigned int Size() const;
	wxString Hash() const;
	void Hash(const wxString& hash);
	wxString MD5() const;
	void MD5(const wxString& md5);
	wxString Name() const;
	wxString Node() const;
	void Node(const wxString& node);
	unsigned int CompletedSize() const;
	void CompletedSize(unsigned int completed);
	int Status() const;
	void Status(int status);
	void Kbs(double kbs);
	double Kbs() const;

	wxString PartPath();
	void PartToReal();

	void SaveInfoFile();
	bool LoadInfoFile();
	void RemoveInfoFile();

	~ShareFile();

private:

	static unsigned int idCounter_;

	//unsigned int id_;

	wxString id_;

	double kbs_;

	wxString node_;

	/** The key for the file (the token). */
	wxString key_;

	/** SHA-1 generated hash for the key. */
	wxString hash_;

	/** The size of the file. */
	unsigned int size_;

	/** The file info if it's a local file. */
	wxFileName file_;

	/** Number of bytes that need to be completed. */
	unsigned int totalSize_;

	/** Number of bytes completed */
	unsigned int completedSize_;

	/** The MD5 of the file so we can validate it when it is finished. */
	wxString md5_;

	int status_;

	wxCriticalSection* crit_;

};

#endif
