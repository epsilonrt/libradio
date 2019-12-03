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
# LD_LIBRARY_PATH=. python test/unit/test_unt.py

MODULE=radio

CPPFLAGS=$(shell python-config --cflags | sed 's/-Wstrict-prototypes//g')
TARGET=_$(MODULE).so

all:$(TARGET)
clean:
	rm -f $(TARGET) $(MODULE)_wrap.* $(MODULE).py*
	rm -fr build

$(MODULE)_wrap.cpp: $(MODULE).i
	swig -python -c++ -o $@ $<

$(MODULE)_wrap.o: $(MODULE)_wrap.cpp
	gcc -fPIC -c $< $(CPPFLAGS)

_$(MODULE).so: $(MODULE)_wrap.o
	gcc -shared $< -lradio++ -lstdc++ -o $@

