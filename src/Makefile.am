#  $Id: Makefile.am,v 1.3 2004/07/22 18:53:07 dp Exp $
#
#  Turnstile
#
#  Copyright (C) 2004, J.D. Henderson <www.digitalpeer.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

bin_PROGRAMS = turnstile

turnstile_SOURCES = 	aboutdialog.cpp \
			aboutdialog.h \
			app.cpp \
			app.h \
			buddy.cpp \
			buddy.h \
			buddytreectrl.cpp \
			buddytreectrl.h \
			config.h \
			connection.cpp \
			connection.h \
			connectionthread.cpp \
			connectionthread.h \
			crypto.cpp \
			crypto.h \
			debugframe.cpp \
			debugframe.h \
			dhtutils.cpp \
			dhtutils.h \
			eraser.cpp \
			eraser.h \
			fileframe.cpp \
			fileframe.h \
			files.cpp \
			files.h \
			filetransferthread.cpp \
			filetransferthread.h \
			findbuddyframe.cpp \
			findbuddyframe.h \
			finger.cpp \
			finger.h \
			frameutils.cpp \
			frameutils.h \
			hashtable.cpp \
			hashtable.h \
			hostcache.cpp \
			hostcache.h \
			imeventqueue.cpp \
			imeventqueue.h \
			imevent.h \
			imframe.cpp \
			imframe.h \
			lookupresponse.cpp \
			lookupresponse.h \
			mainframe.cpp \
			mainframe.h \
			network.cpp \
			network.h \
			packet.cpp \
			packet.h \
			packetgen.cpp \
			packetgen.h \
			prefframe.cpp \
			prefframe.h \
			prefs.cpp \
			prefs.h \
			profiledialog.cpp \
			profiledialog.h \
			server.cpp \
			server.h \
			typeconv.cpp \
			typeconv.h \
			utils.cpp \
			utils.h

AM_CPPFLAGS=@WX_CPPFLAGS@
AM_CXXFLAGS=@WX_CXXFLAGS_ONLY@ -Wno-write-strings -fpermissive
AM_CFLAGS=@WX_CFLAGS_ONLY@

AM_LDFLAGS=@WX_LIBS@

SUBDIRS = libcrypto libjdhsocket

turnstile_LDADD = libcrypto/libcrypto.a libjdhsocket/libjdhsocket.a

