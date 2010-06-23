#
# Makefile for the xGNOKII tool suite.
#

TOPDIR=..
include $(TOPDIR)/Makefile.global

HELP1_DIR = en_US
HELP11_DIR = pl_PL
HELP2_DIR = examples
HELP3_DIR = default

GNOKII1_MAN1 = "man/gnokii.1"
GNOKII1_MAN2 = "man/todologo.1"
GNOKII8_MAN1 = "man/gnokiid.8"
GNOKII8_MAN2 = "man/mgnokiidev.8"
XGNOKII_MAN = "man/xgnokii.1x"

all: 
	@echo

makelib:
	@echo
	
install:
	$(INSTALL) -d $(docdir)

	( cd $(HELP1_DIR); \
	  $(FIND) . -type d \! -path "*CVS*" \
	       -exec $(INSTALL) -d $(docdir)/en_US/{} \; ; \
	  $(FIND) . -type f \! -path "*CVS*" \
	       -exec $(INSTALL) -m 0444 {} $(docdir)/en_US/{} \; \
	)
	( cd $(HELP11_DIR); \
	  $(FIND) . -type d \! -path "*CVS*" \
	       -exec $(INSTALL) -d $(docdir)/pl_PL/{} \; ; \
	  $(FIND) . -type f \! -path "*CVS*" \
	       -exec $(INSTALL) -m 0444 {} $(docdir)/pl_PL/{} \; \
	)

	( cd $(HELP2_DIR); \
	  $(FIND) . -type d \! -path "*CVS*" \
	       -exec $(INSTALL) -d $(docdir)/examples/{} \; ; \
	  $(FIND) . -type f \! -path "*CVS*" \
	       -exec $(INSTALL) -m 0444 {} $(docdir)/examples/{} \; \
	)
	( cd $(HELP3_DIR); \
	  $(FIND) . -type d \! -path "*CVS*" \
	       -exec $(INSTALL) -d $(docdir)/default/{} \; ; \
	  $(FIND) . -type f \! -path "*CVS*" \
	       -exec $(INSTALL) -m 0444 {} $(docdir)/default/{} \; \
	)
	$(INSTALL) -m 0444 CREDITS $(docdir)
	$(INSTALL) -m 0444 ../COPYING $(docdir)
	$(INSTALL) $(GNOKII1_MAN1) $(man1dir)
	$(INSTALL) $(GNOKII1_MAN2) $(man1dir)
	$(INSTALL) $(GNOKII8_MAN1) $(man8dir)
	$(INSTALL) $(GNOKII8_MAN2) $(man8dir)
	if [ "x$(HAVE_XGNOKII)" = xyes ]; then \
		($(INSTALL) $(XGNOKII_MAN) $(xmandir)) \
	fi
	@echo "done"

clean:
	$(RM) *~

depend dep:
	@echo

.PHONY: all install clean dep depend
