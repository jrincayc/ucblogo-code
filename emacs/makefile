BUILDIR	      = `pwd`
EMACSDIR      = $(LIBLOC)/emacs
INFODIR	      = $(prefix)/info
LOADPATH      = $(subst X,$(EMACSDIR),(setq load-path (cons "\"X\"" load-path)))
LOGOBIN	      = $(subst X,$(BINDIR),(setq logo-binary-name \"X/logo\"))
INFOPATH      = $(subst X,$(INFODIR),(setq logo-info-file \"X/ucblogo.info\"))
HELPPATH      = $(subst X,$(LIBLOC),(setq logo-help-path \"X/helpfiles/\"))
TUTORPATH     = $(subst X,$(EMACSDIR),(setq logo-tutorial-path \"X/\"))

all: logo.elc dot.emacs install-logo-mode 

logo.elc: comint-logo.el letrec.el logo.el
	emacs -batch -l $(BUILDIR)/edfunc.el -f compile-load-path -f batch-byte-compile comint-logo.el letrec.el logo.el

dot.emacs:
	@echo "$(LOADPATH)" > e.path
	@echo "$(LOGOBIN)" > e.bin
	@echo "$(INFOPATH)" > e.info
	@echo "$(HELPPATH)" > e.help
	@echo "$(TUTORPATH)" > e.tutor
	@echo ";;" > e.safe
	cat dot.emacs1 e.safe e.path e.bin e.info e.help e.tutor dot.emacs2 > dot.emacs
	@echo ";;;   -*- logo -*-" > e.loops1
	@echo "load \"$(EMACSDIR)/.logo load \"$(EMACSDIR)/.LOOPS" > e.loops2
	cat e.loops1 e.loops2 > dot.loops 

install-logo-mode:
	@echo "emacs -batch ~/.emacs -l $(EMACSDIR)/edfunc.el -f edfunc -insert $(EMACSDIR)/dot.emacs -f save-buffer -kill" > e.user
	@echo "exec emacs $(EMACSDIR)/check.lg &" > e.check
	@echo "#!/bin/sh" > add.user
	cat add.user e.user e.check > install-logo-mode
	chmod u=rwx,go=rx install-logo-mode

clean:
	-rm -f e.*
	-rm -f add.user

ship:
	-rm -f e.*
	-rm -f add.user install-logo-mode dot.emacs dot.loops *.elc

install: all
	for d in $(EMACSDIR); do [ -d $$d ] || mkdir -p $$d || exit 1; done
	cp -f logo.* $(EMACSDIR)/.
	cp -f letrec.* $(EMACSDIR)/.
	cp -f comint*.* $(EMACSDIR)/.
	cp -f tutor* $(EMACSDIR)/.
	cp -f dot.loops $(EMACSDIR)/.
	cp -f dot.logo $(EMACSDIR)/.logo
	cp -f dot.LOOPS $(EMACSDIR)/.LOOPS
	cp -f dot.emacs $(EMACSDIR)/.
	cp -f README $(EMACSDIR)/.
	cp -p install-logo-mode $(BINDIR)/.
	cp -f edfunc.el $(EMACSDIR)/.
	cp -f check.lg $(EMACSDIR)/.
