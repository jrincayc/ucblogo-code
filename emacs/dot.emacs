;;; LOGO-MODE CHANGES *BEGIN*
;;;
;;; Do *NOT* insert your own forms between lines "LOGO-MODE CHANGES *BEGIN*" 
;;; and "LOGO-MODE CHANGES *END*" . Everything between these two lines 
;;; can/will be deleted during subsequent installation of logo-mode!
;;
;; This is a must!
(setq auto-mode-alist
      (cons '("\\.lgo?\\'" . logo-mode)
	    auto-mode-alist))
(autoload 'logo-mode "logo")
;;
;; Forcing `xterm' as terminal definition for Logo. If you prefer to use
;; `vt100' , then reverse the commenting of next two lines. To use the
;; system default terminal definition -- comment out both:
(setq logo-system-type 'xterm)
;(setq logo-system-type 'vt100)
;;
;; If you don't want syntax highlighting uncomment next line.
;(setq logo-syntax-highlight nil)
;;
;; If you are not a novice (meaning emacs - mostly), uncomment next line
;(setq logo-novice-management nil) 
;;
;; Next 5 are automatically inserted by MAKE - in case you used
;; make LIBLOC=some-unusual-place, or changed $(prefix) in toplevel
;; ucblogo makefile
;;
(setq load-path (cons "/usr/local/lib/logo/emacs" load-path))
(setq logo-binary-name "/usr/local/bin/logo")
(setq logo-info-file "/usr/local/info/ucblogo.info")
(setq logo-help-path "/usr/local/lib/logo/helpfiles/")
(setq logo-tutorial-path "/usr/local/lib/logo/emacs/")
;;
;; Uncomment next 2 if you want to change the default size of Logo window.
;; Default is 80 x 24.
;(setq process-term-columns 86)
;(setq process-term-rows 32)
;;
;; Next 2 are not important, but I like it this way.
(setq logo-setcursor-overwrite nil)
(setq logo-unbalanced-distance 4096)
;;
;; I assume you are not planning to install translated ucblogo info-manual.
;; If you are, (this goes for Croatians) then comment out next line.
(setq logo-info-trans nil)
;;
;; If you have no intention to use .logo initialization file then
;; uncomment next line:
;(setq logo-load-language nil)
;;
;; If you don't want me messing with your default emacs colors, then
;; uncomment next line:
;(setq dont-mess-with-logo-colors t)
;;
;; If you don't want parens matching on cursor movement, then
;; uncomment next line:
;(setq logo-flash-on-movement nil)
;;
;;; LOGO-MODE CHANGES *END*

