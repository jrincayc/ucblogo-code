logolibdir=$(pkgdatadir)/logolib

logolib_DATA = RENAME-GRAVE-ACCENT RENAME-NUMBER-SIGN backslashedp	\
 backslashedQ buryall buryname cascade cascade.2 case closeall		\
 combine cond crossmap dequeue do.until do.while edall edn edns edpl	\
 edpls edps emacs.debug ern erpl filep fileQ filter find for foreach	\
 gensym ignore invoke iseq localmake macroexpand map map.se mdarray	\
 mditem mdsetitem Messages Messages.fr Messages.sp Messages.zh_TW	\
 name namelist pen pick pllist poall pon pons pop popl popls pops	\
 pots push Qrest queue quoted reduce remdup remove reverse rseq savel	\
 setpen transfer unburyall unburyname until while xcor ycor

install-data-hook:
	(cd "$(DESTDIR)$(logolibdir)" && \
	mv --verbose RENAME-GRAVE-ACCENT '`' && \
	mv --verbose RENAME-NUMBER-SIGN '#')

uninstall-hook:
	(cd "$(DESTDIR)$(logolibdir)" && \
	rm -f '`' && \
	rm -f '#')
