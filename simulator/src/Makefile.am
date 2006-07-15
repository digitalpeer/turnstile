#  $Id: Makefile.am,v 1.1.1.1 2004/07/02 23:01:16 dp Exp $
#
#  yawpsim
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

bin_PROGRAMS = yawpsim

yawpsim_SOURCES = 	main.cpp \
			node.cpp \
			node.h \
			simutils.cpp \
			simutils.h \
			statistics.cpp \
			statistics.h \
			network.cpp \
			network.h \
			../../src/crypto.cpp \
			../../src/crypto.h \
			../../src/utils.cpp \
			../../src/utils.h


AM_CPPFLAGS=@WX_CPPFLAGS@
AM_CXXFLAGS=@WX_CXXFLAGS_ONLY@
AM_CFLAGS=@WX_CFLAGS_ONLY@

AM_LDFLAGS=@WX_LIBS@

yawpsim_LDADD = ../../src/crypto/libcrypto.a
