;; Deleting from .emacs previous logo-mode installation
(defun edfunc ()
  (goto-char (point-max))
  (let ((start (re-search-backward
		"^[ \t]*;+[ \t]*LOGO-MODE[ \t]+CHANGES[ \t]+\\*BEGIN\\*" nil t))
	(end (re-search-forward
	      "^[ \t]*;+[ \t]*LOGO-MODE[ \t]+CHANGES[ \t]+\\*END\\*" nil t)))
    (and end start (not (> (- end start) 2500)) (delete-region start end)))
  (goto-char (point-max))
  (delete-blank-lines)
  (or (bobp)
      (insert "\n\n")))

;; temporary

(defun compile-load-path ()
  (setq load-path (cons "./" load-path)))