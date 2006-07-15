/* $Id: utils.h,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
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

#ifndef TURNSTILE_UTILS_H
#define TURNSTILE_UTILS_H

class wxString;

/** Get the index of a string in an array. */
int GetIndex(const wxString* array, int size, wxString value);

/** Chop off the next token and incriment position to after that token+del
in the string. */
wxString extract(const char* raw, unsigned int& len, unsigned int& position, char del);
bool crawl(const char* raw, unsigned int& len, unsigned int& position, char del);

wxString ParseHost(wxString value);
int ParsePort(wxString value);

void LogDebug(wxString text);
void LogError(wxString text);
void LogMsg(wxString text);

unsigned int FileSize(const wxString& path);

wxString UnAliasEscape(const wxString& alias);

wxString AliasEscape(const wxString& alias);

wxString HumanFileSize(unsigned int bytes);

bool MYlte(const wxString& a, const wxString& b);
bool MYlt(const wxString& a, const wxString& b);
bool MYne(const wxString& a, const wxString& b);
bool MYe(const wxString& a, const wxString& b);
bool MYgt(const wxString& a, const wxString& b);


#endif
