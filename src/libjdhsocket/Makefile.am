#
# $Id: Makefile.am,v 1.3 2004/08/29 23:16:30 dp Exp $
#
# JdhSocket Library - Portable, Thread Safe Sockets Library
#
# Copyright (C) 2004, J.D. Henderson <www.digitalpeer.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

noinst_LIBRARIES = libjdhsocket.a

libjdhsocket_a_SOURCES= 	clientsocket.cpp \
				clientsocket.h \
				serversocket.cpp \
				serversocket.h \
				socketbase.cpp \
				socketbase.h

#The three numbers stand for CURRENT:REVISION:AGE, or C:R:A for short. The libtool script typically tacks these three numbers onto the end of the name of the .so file it creates. The formula for calculating the file numbers on Linux and Solaris is (C - A).(A).(R), so the example given here would create the file libgrump.so.3.2.1. Other operating systems might use a different library file name convention; libtool takes care of the details.
#As you release new versions of your library, you will update the library's C:R:A. Although the rules for changing these version numbers can quickly become confusing, a few simple tips should help keep you on track. The libtool documentation goes into greater depth.
#In essence, every time you make a change to the library and release it, the C:R:A should change. A new library should start with 0:0:0. Each time you change the public interface (i.e., your installed header files), you should increment the CURRENT number. This is called your interface number. The main use of this interface number is to tag successive revisions of your API.
#The AGE number is how many consecutive versions of the API the current implementation supports. Thus if the CURRENT library API is the sixth published version of the interface and it is also binary compatible with the fourth and fifth versions (i.e., the last two), the C:R:A might be 6:0:2. When you break binary compatibility, you need to set AGE to 0 and of course increment CURRENT.
#The REVISION marks a change in the source code of the library that doesn't affect the interface-for example, a minor bug fix. Anytime you increment CURRENT, you should set REVISION back to 0.

#libjdhsocket_a_LDFLAGS = -version-info 0:0:0

EXTRA_DIST= COPYING ChangeLog README Doxyfile
