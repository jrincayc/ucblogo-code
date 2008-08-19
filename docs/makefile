BUILDIR	      = `pwd`
INFODIR	      = $(prefix)/info
DOCSDIR	      = $(LIBLOC)/docs
HTMLDIR	      = $(DOCSDIR)/html

all: usermanual.ps usermanual.pdf html/usermanual_1.html ucblogo.info

ucblogo.info: usermanual.texi
	makeinfo usermanual.texi

usermanual.dvi: usermanual.texi
	tex --interaction batchmode usermanual.texi
	tex --interaction batchmode usermanual.texi
	texindex usermanual.cp
	tex --interaction batchmode usermanual.texi

usermanual.ps: usermanual.dvi
	dvips -t letter -o usermanual.ps usermanual.dvi

usermanual.pdf: usermanual.ps usermanual.dvi
	ps2pdf usermanual.ps
	dvipdf usermanual.dvi

html/usermanual_1.html: usermanual.texi
	for d in html; do [ -d $$d ] || mkdir -p $$d || exit 1; done
	texi2html --expand tex --split chapter --no-iftex usermanual.texi
	mv usermanual/*.html html
	rmdir usermanual

ship:
	-rm -f *.{aux,cp,cps,dvi,fn,ky,log,pg,toc,tp,vr}

install: all
	for d in $(INFODIR) $(DOCSDIR) $(HTMLDIR); do [ -d $$d ] || mkdir -p $$d || exit 1; done
	-cp -f *.info* $(INFODIR)/.
	-cp -f html/*.html $(HTMLDIR)/.
	-cp -f usermanual.ps $(DOCSDIR)/.
	-cp -f usermanual.pdf $(DOCSDIR)/.
	-cp -f usermanual.texi $(DOCSDIR)/.
	-cp -f ../usermanual $(DOCSDIR)/.
