#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  setup.py
#
#  Copyright 2014 Pascal JEAN aka epsilonrt <pjean@btssn.net>
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
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#
#

from distutils.core import setup, Extension

module_swig = Extension('_radio',
                        ['radio.i'],
                        include_dirs = ['/usr/local/include'],
                        libraries = ['radio++','stdc++','ssdv'],
                        library_dirs = ['/usr/local/lib'],
                        swig_opts=['-c++'],
                        language='c++')
setup (name = 'radio',
       version = '1.1',
       description = 'Simple AX25 data link layer package',
       author = 'Pascal JEAN aka epsilonrt',
       author_email = 'pjean@btssn.net',
       url = 'http://www.btssn.net',
       license = 'GNU Lesser General Public License version 3',
       ext_modules = [module_swig],
       py_modules = ['radio'],)

