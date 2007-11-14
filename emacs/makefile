BUILDIR	      = `pwd`
INFODIR	      = $(prefix)/info
EMACSDIR      = $(LIBLOC)/emacs
DOCSDIR	      = $(LIBLOC)/docs
HTMLDIR	      = $(DOCSDIR)/html
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

docs: docs/usermanual.ps docs/usermanual.pdf docs/usermanual_1.html docs/ucblogo.info

docs/ucblogo.info: docs/usermanual.texi
	-cd docs; makeinfo usermanual.texi

docs/usermanual.dvi: docs/usermanual.texi
	-cd docs; tex --interaction batchmode usermanual.texi
	-cd docs; tex --interaction batchmode usermanual.texi
	-cd docs; texindex usermanual.cp
	-cd docs; tex --interaction batchmode usermanual.texi

docs/usermanual.ps: docs/usermanual.dvi
	-cd docs; dvips -t letter -o usermanual.ps usermanual.dvi

docs/usermanual.pdf: docs/usermanual.ps docs/usermanual.dvi
	-cd docs; ps2pdf usermanual.ps
	-cd docs; dvipdf usermanual.dvi

docs/usermanual_1.html: docs/usermanual.texi
	-cd docs; texi2html -expand tex -split chapter usermanual.texi

clean:
	-rm -f e.*
	-rm -f add.user

ship:
	-rm -f e.*
	-rm -f add.user install-logo-mode dot.emacs dot.loops *.elc
	-rm -f docs/*.{aux,cp,cps,dvi,fn,ky,log,pg,toc,tp,vr}

install: all
	for d in $(INFODIR) $(EMACSDIR) $(DOCSDIR) $(HTMLDIR); do [ -d $$d ] || mkdir -p $$d || exit 1; done
	cp -f info/* $(INFODIR)/.
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
	-cp -f docs/*.info* $(INFODIR)/.
	-cp -f docs/*.html $(HTMLDIR)/.
	-cp -f docs/usermanual.ps $(DOCSDIR)/.
	-cp -f docs/usermanual.pdf $(DOCSDIR)/.
	-cp -f docs/usermanual.texi $(DOCSDIR)/.


