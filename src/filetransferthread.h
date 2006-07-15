/* $Id: filetransferthread.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

/** @file filetransferthread.h
@brief File Transfer Thread Definition
*/
#ifndef TURNSTILE_FILETRANSFERTHREAD_H
#define TURNSTILE_FILETRANSFERTHREAD_H

#include "connection.h"

#include <wx/thread.h>

class wxString;
class ShareFile;


/** @class FileTransferThread
*/
class FileTransferThread : public wxThread,Connection
{
public:
	FileTransferThread(ShareFile* file);

	void* Entry();
	void OnExit();

private:
	/* This should never be called.
	*/
	FileTransferThread()
	{;}

	/** Download a file.
	*/
	void Get(ShareFile& file);

	/** The current file we are downloading.
	*/
	ShareFile* file_;
};

#endif

