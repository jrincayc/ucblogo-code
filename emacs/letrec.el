;;; letrec.el

;; Copyright (C) 2001 by Hrvoje Blazevic (hrvoje.blazevic@ri.tel.hr)

;; Logo.el is free software distributed under the terms
;; of the GNU General Public License, version 2, or (at your option)
;; any later version.

;; Logo.el is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;;; Commentary:

;; This file defines extensions to emacs lisp:
;; filter, letrec ...

;; We need to increase `funcall' depth for recursive calls here.
(or (> max-lisp-eval-depth 1200)
    (setq max-lisp-eval-depth 1200))
(or (> max-specpdl-size 3600)
    (setq max-specpdl-size 3600))

(eval-and-compile
  (defun letrec-transformer (proc-text)
    "Insert `funcall' before all local function calls."
    (if (null proc-text)
	nil
      (if (consp (car proc-text))
	  (cond
	   ((memq (caar proc-text) p-names)
	    (cons (cons 'funcall (letrec-transformer (car proc-text)))
		  (letrec-transformer (cdr proc-text))))
	   ((and (eq (caar proc-text) 'quote)
		 (not (functionp (car (cdar proc-text)))))
	    (cons (car proc-text) (letrec-transformer (cdr proc-text))))
	   (t (cons (letrec-transformer (car proc-text))
		    (letrec-transformer (cdr proc-text)))))
	(cons (car proc-text) (letrec-transformer (cdr proc-text)))))))

(defmacro letrec (procs &rest body-forms)
    "Defining local functions in e-lisp -- similar to Scheme letrec.
Uses `let' syntax. First argument, PROCS is a list of name to lambda 
bindings. Binding names to value expressions is supported -- however,
see the note below.

Scope in letrec is true Scheme letrec scope -- local functions defined
this way can be both -- recursive, and mutually recursive.

Note:
 
Just like in Scheme, you can not have the variable and the local 
function of the same name.
If you use the local function as an argument to a functional like
`mapcar', you must *not* quote the local function name.

(defun letrec-test (num-list)
  (letrec
      ((add100 (lambda (x) (+ x 100))))
    (mapcar add100 num-list)))

Unlike Scheme, a value binding can refer to lexically previously 
defined bindings in its value expression -- ergo; scope in this case 
behaves like `let*' scope."
  (let* ((p-names
	  (mapcar
	   (lambda (x) (if (functionp (cadr x)) (car x) '(x))) procs))
 	 (funcall-procs
 	  (mapcar
 	   (lambda (p) (list (car p) (letrec-transformer (cadr p))))
	   procs)))
    `(let ,(mapcar (lambda (x) (list (car x))) procs)
       ,@(mapcar (lambda (x) `(setq ,(car x)
				    ,(if (functionp (cadr x))
					 `(function ,(cadr x))
				       `(identity ,(cadr x)))))
 		 funcall-procs)
       ,@(letrec-transformer body-forms))))

(defun filter (pred lst)
  "Returns a filtered list of all elements of list LST, for which the
predicate PRED returned t."
  (letrec
      ((f (lambda (lst)
	    (if (null lst)
		nil
	      (if (funcall pred (car lst))
		  (cons (car lst) (f (cdr lst)))
		(f (cdr lst)))))))
    (f lst)))

(provide 'letrec)

;; letrec.el ends here
