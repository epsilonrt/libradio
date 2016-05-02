###############################################################################
# Copyright © 2015 epsilonRT, All rights reserved.                            #
# This software is governed by the CeCILL license <http://www.cecill.info>    #
###############################################################################

SUBDIRS = ax25 tnc

all: $(SUBDIRS)
rebuild: $(SUBDIRS)
static: $(SUBDIRS)
clean: $(SUBDIRS)
distclean: $(SUBDIRS)
install: $(SUBDIRS)
install-static: $(SUBDIRS)
uninstall: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS)

.PHONY: all rebuild static clean distclean install install-static uninstall $(SUBDIRS)
