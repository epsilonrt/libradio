# Copyright (c) 2014 Pascal JEAN <pascal.jean@btssn.net>
###############################################################################
# This program is free software: you can redistribute it and/or modif         #
#    it under the terms of the GNU Lesser General Public License as published #
#    by the Free Software Foundation, either version 3 of the License, or     #
#    (at your option) any later version.                                      #
#                                                                             #
#    This program is distributed in the hope that it will be useful,          #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of           #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
#    GNU Lesser General Public License for more details.                      #
#                                                                             #
#    You should have received a copy of the GNU Lesser General Public License #
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.    #
###############################################################################
# $Id$

SUBDIRS = libradio test

lib: libradio
install: libradio
uninstall: libradio
all: $(SUBDIRS)
rebuild: $(SUBDIRS)
clean: $(SUBDIRS)
distclean: $(SUBDIRS)


$(SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS)

.PHONY: all rebuild clean distclean install uninstall $(SUBDIRS)

