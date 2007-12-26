/*
 *      math.c          logo math functions module              dvb
 *
 *	Copyright (C) 1993 by the Regents of the University of California
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *  
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *  
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "logo.h"
#include "globals.h"
#include <signal.h>
#include <setjmp.h>
#include <math.h>

#define isdigit(dig)    (dig >= '0' && dig <= '9')

int numberp(NODE *snd) {
    int dl,dr, pcnt, plen;
    char *p;

    if (is_number(snd)) return(1);

    snd = cnv_node_to_strnode(snd);
    if (snd == UNBOUND) return(0);

    p = getstrptr(snd); plen = getstrlen(snd); pcnt = dl = dr = 0;
    if (plen >= MAX_NUMBER) {
	return(0);
    }

    if (pcnt < plen && *p == '-')
	p++, pcnt++;

    while (pcnt < plen && isdigit(*p))
	p++, pcnt++, dl++;

    if (pcnt < plen && *p == '.') {
	p++, pcnt++;
	while (pcnt < plen && isdigit(*p))
	    p++, pcnt++, dr++;
    }

    if (pcnt < plen && (dl || dr) && (*p == 'E' || *p == 'e')) {
	p++, pcnt++;

	if (pcnt < plen && (*p == '+' || *p == '-'))
	    p++, pcnt++;

	while (pcnt < plen && isdigit(*p))
	    p++, pcnt++, dr++;
    }

    if ((dl == 0 && dr == 0) || pcnt != plen)
	return (0);
    else
	return (dr + 1);
}

NODE *lrandom(NODE *arg) {
	NODE *val;
	unsigned long r, base, range;

	val = pos_int_arg(arg);
	if (NOT_THROWING) {
	    if (cdr(arg)==0) {	/* (random 10) => (0, 10) */
		base = 0;
		range = getint(val);
	    } else {		/* (random 3 10) => (3, 8) */
		base = getint(val);
		val = pos_int_arg(arg);
		if (NOT_THROWING) { /* (random 0 9) <=> (random 10) */
		    range = getint(val);
		    range = range + 1 - base;
		}
	    }
	}
	if (NOT_THROWING) {
#ifdef HAVE_SRANDOM
	    r = (range <= 0 ? 0 : random() % range);
#else
	    r = (((long)rand()) << 15) | rand();
	    r = (range <= 0 ? 0 : r % range);
#endif
	    r += base;
	    val = newnode(INT);
	    setint(val, (FIXNUM)r);
	    return(val);
	} else return(UNBOUND);
}

NODE *lrerandom(NODE *arg) {
	int seed=1;

	if (arg != NIL) {
		seed = int_arg(arg);
	}
	if (NOT_THROWING) {
#ifdef HAVE_SRANDOM
		srandom((int)seed);
#else
		srand((int)seed);
#endif
	}
	return(UNBOUND);
}

jmp_buf oflo_buf;
BOOLEAN handling_oflo = FALSE;

#ifdef SIG_TAKES_ARG
#define sig_arg 0
RETSIGTYPE handle_oflo(int sig) {
#else
#define sig_arg 
RETSIGTYPE handle_oflo() {
#endif
    signal(SIGFPE, handle_oflo);
    if (handling_oflo) longjmp(oflo_buf,1);
    SIGRET
}

void math_init() {
    signal(SIGFPE, handle_oflo);
}

#ifdef HAVE_MATHERR
int matherr(struct exception *x) {
    if (x->type == UNDERFLOW) return(1);
    longjmp(oflo_buf,1);
}
#endif

#ifdef mac
FLONUM degrad = 0.017453292520;
#else
FLONUM degrad = 3.141592653589793227020265931059839203954/180.0;
#endif

#if defined(mac)||defined(ibm)
#define errchk(x) {errno = 0; x; if (errno) err_logo(BAD_DATA_UNREC,arg);}
#include <errno.h>
#else
#define errchk(x) x
#endif

NODE *binary(NODE *args, char fcn) {
    NODE *arg, *val;
    BOOLEAN imode;
    FIXNUM iarg = 0, ival = 0, oval, nval;
    FLONUM farg = 0.0, fval = 0.0;
    int sign, wantint=0;

    /* Force imode, arg and fval into the stack because otherwise they may be
       clobbered during setjmp/longjmp. Especially on Sparc. */
    (void)&imode; (void)&arg; (void)&fval;

/*
    if (fcn == '%' || fcn == 'm')
	arg = integer_arg(args);
    else
 */
	arg = numeric_arg(args);
    args = cdr(args);
    if (stopping_flag == THROWING) return UNBOUND;
    if (nodetype(arg) == INT) {
	imode = TRUE;
	ival = getint(arg);
    } else {
	imode = FALSE;
	fval = getfloat(arg);
    }
    if (args == NIL) {    /* one argument supplied */
      if (imode)
	switch(fcn) {
	  case '-': ival = -ival; break;
	  case '~': ival = ~ival; break;
	  case 's':
	  case 'c':
	  case 't':
	  case 'S':
	  case 'C':
	  case 'T':
	  case 'q':
	  case 'e':
	  case 'g':
	  case 'n':
	  case '/':
	    imode = FALSE;
	    fval = (FLONUM)ival;
	    break;
	}
      if (imode == FALSE) {
       if (!setjmp(oflo_buf)) {
	switch(fcn) {
	  case '-': fval = -fval; break;
	  case '/':
	    if (fval == 0.0)
		err_logo(BAD_DATA_UNREC,arg);
	    else
		fval = 1/fval;
	    break;
	  case '~': err_logo(BAD_DATA_UNREC,arg); break;
	  case 'c':
	    fval = 90.0 - fval;
	  case 's':
	    /* Kahan sez we can't just multiply any old
	     * angle by degrad, but have to get into the
	     * range 0-45 first */
	    sign = (fval < 0.0);
	    if (sign) fval = -fval;
#ifndef HAVE_DREM
	    fval = fmod(fval,360.0);
#else
	    fval = drem(fval,360.0);
#endif
	    if (fval > 180.0) {
		fval -= 180.0;
		sign = !sign;
	    }
	    if (fval > 90.0) fval = 180.0 - fval;
	    if (fval > 45.0)
		fval = cos((90.0-fval)*degrad);
	    else
		fval = sin(fval*degrad);
	    if (sign) fval = -fval;
	    break;
	  case 't': fval = atan(fval)/degrad; break;
	  case 'S': fval = sin(fval); break;
	  case 'C': fval = cos(fval); break;
	  case 'T': fval = atan(fval); break;
	  case 'q': errchk(fval = sqrt(fval)); break;
	  case 'e': errchk(fval = exp(fval)); break;
	  case 'g': errchk(fval = log10(fval)); break;
	  case 'n': errchk(fval = log(fval)); break;
	  case 'r':
	    fval += (fval < 0 ? -0.5 : 0.5);
	  case 'i':
	    handling_oflo = TRUE;
	    if (fval > (FLONUM)MAXLOGOINT ||
		    fval < -(FLONUM)MAXLOGOINT)
		handle_oflo(sig_arg);
	    ival = (FIXNUM)fval;
	    imode = TRUE;
	    handling_oflo = FALSE;
	    break;
	}
       } else {	/* overflow */
	    if (fcn == 'r' || fcn == 'i') {
	      if (fval < 0.0)
		fval = ceil(fval);
	      else
		fval = floor(fval);
	    } else
		err_logo(BAD_DATA_UNREC,arg);
       }
      }	    /* end float case */
    }	    /* end monadic */
    while (args != NIL && NOT_THROWING) {
/*
	if (fcn == '%' || fcn == 'm')
	    arg = integer_arg(args);
	else
 */
	    arg = numeric_arg(args);
	args = cdr(args);
	if (stopping_flag == THROWING) return UNBOUND;

	if (nodetype(arg) == INT) {
	    if (imode) iarg = getint(arg);
	    else farg = (FLONUM)getint(arg);
	} else {
	    if (imode) {
		fval = (FLONUM)ival;
		imode = FALSE;
	    }
	    farg = getfloat(arg);
	}

	if (imode) {
	    oval = ival;
	    handling_oflo = TRUE;
	    if (setjmp(oflo_buf) == 0) {
	     switch(fcn) {
	      case '-': iarg = -iarg;
	      case '+':
		if (iarg < 0) {
		    nval = ival + iarg;
		    if (nval >= ival) {
			imode = FALSE;
			fcn = '+';
			fval = (FLONUM)ival;
			farg = (FLONUM)iarg;
		    } else ival = nval;
		} else {
		    nval = ival + iarg;
		    if (nval < ival) {
			imode = FALSE;
			fcn = '+';
			fval = (FLONUM)ival;
			farg = (FLONUM)iarg;
		    } else ival = nval;
		}
		break;
	      case '/':
		if (iarg == 0)
		  err_logo(BAD_DATA_UNREC,arg);
		else
		  if (ival % iarg != 0) {
		    imode = FALSE;
		    fval = (FLONUM)ival;
		    farg = (FLONUM)iarg;
		  }
		  else ival /= iarg;
		  break;
	      case '%':
		if (iarg == 0)
		  err_logo(BAD_DATA_UNREC,arg);
		else
		  ival %= iarg;
		break;
	      case 'm':
		if (iarg == 0)
		  err_logo(BAD_DATA_UNREC,arg);
		else
		  ival %= iarg;
		if ((ival < 0) != (iarg < 0))
		    ival += iarg;
		break;
	      case '&': ival &= iarg; break;
	      case '|': ival |= iarg; break;
	      case '^': ival ^= iarg; break;
	      case 'a':
	      case 'l':
		if (iarg < 0) {
		  if (fcn == 'a')
		    ival >>= -iarg;
		  else
		    ival = (unsigned)ival
			>> -iarg;
		} else
		  ival <<= iarg;
		break;
	      case '*':
		if (ival < SAFEINT && ival > -SAFEINT &&
		    iarg < SAFEINT && iarg > -SAFEINT) {
		    ival *= iarg;
		    break;
		}
		wantint++;
	      default: /* math library */
		imode = FALSE;
		fval = (FLONUM)ival;
		farg = (FLONUM)iarg;
	     }
	    } else {    /* integer overflow detected */
		imode = FALSE;
		fval = (FLONUM)oval;
		farg = (FLONUM)iarg;
	    }
	    handling_oflo = FALSE;
	}
	if (imode == FALSE) {
	  handling_oflo = TRUE;
	  if (setjmp(oflo_buf) == 0) {
	    switch(fcn) {
	      case '+': fval += farg; break;
	      case '-': fval -= farg; break;
	      case '*':
		fval *= farg;
		if (wantint) {
		    wantint = 0;
		    if (fval <= MAXLOGOINT && fval >= -MAXLOGOINT) {
			imode = TRUE;
			ival = (FIXNUM)fval;
		    }
		}
		break;
	      case '/':
		    if (farg == 0.0)
		      err_logo(BAD_DATA_UNREC,arg);
		    else
		      fval /= farg;
		    break;
	      case 't':
		errchk(fval = atan2(farg,fval)/degrad);
		break;
	      case 'T':
		errchk(fval = atan2(farg,fval));
		break;
	      case 'p':
		errchk(fval = pow(fval,farg));
		break;
	      case '%':
		if (farg == 0.0)
		    err_logo(BAD_DATA_UNREC,arg);
		else
		    errchk(fval = fmod(fval,farg));
		break;
	      case 'm':
		if (farg == 0.0)
		    err_logo(BAD_DATA_UNREC,arg);
		else
		    errchk(fval = fmod(fval,farg));
		if ((fval < 0.0) != (farg < 0.0))
		    fval += farg;
		break;
	      default: /* logical op */
		if (nodetype(arg) == INT)
		  err_logo(BAD_DATA_UNREC, make_floatnode(fval));
		else
		  err_logo(BAD_DATA_UNREC,arg);
	    }
	  } else {    /* floating overflow detected */
	    err_logo(BAD_DATA_UNREC,arg);
	  }
	  handling_oflo = FALSE;
	}    /* end floating point */
    }	/* end dyadic */
    if (NOT_THROWING) {
	if (imode) {
	    val = newnode(INT);
	    setint(val, ival);
	} else {
	    val = newnode(FLOATT);
	    setfloat(val, fval);
	}
	return(val);
    }
    return(UNBOUND);
}

NODE *ladd(NODE *args) {
    if (args == NIL) return make_intnode(0L);
    return(binary(args, '+'));
}

NODE *lsub(NODE *args) {
    return(binary(args, '-'));
}

NODE *lmul(NODE *args) {
    if (args == NIL) return make_intnode(1L);
    return(binary(args, '*'));
}

NODE *ldivide(NODE *args) {
    return(binary(args, '/'));
}

NODE *lremainder(NODE *args) {
    return(binary(args, '%'));
}

NODE *lmodulo(NODE *args) {
    return(binary(args, 'm'));
}

NODE *lbitand(NODE *args) {
    if (args == NIL) return make_intnode(-1);
    return(binary(args, '&'));
}

NODE *lbitor(NODE *args) {
    if (args == NIL) return make_intnode(0);
    return(binary(args, '|'));
}

NODE *lbitxor(NODE *args) {
    if (args == NIL) return make_intnode(0);
    return(binary(args, '^'));
}

NODE *lashift(NODE *args) {
    return(binary(args, 'a'));
}

NODE *llshift(NODE *args) {
    return(binary(args, 'l'));
}

NODE *lbitnot(NODE *args) {
    return(binary(args, '~'));
}

NODE *lsin(NODE *args) {
    return(binary(args, 's'));
}

NODE *lcos(NODE *args) {
    return(binary(args, 'c'));
}

NODE *latan(NODE *args) {
    return(binary(args, 't'));
}

NODE *lradsin(NODE *args) {
    return(binary(args, 'S'));
}

NODE *lradcos(NODE *args) {
    return(binary(args, 'C'));
}

NODE *lradatan(NODE *args) {
    return(binary(args, 'T'));
}

NODE *lsqrt(NODE *args) {
    return(binary(args, 'q'));
}

NODE *linteg(NODE *args) {
    return(binary(args, 'i'));
}

NODE *lroundx(NODE *args) { /* There's an lround in <math.h> */
    return(binary(args, 'r'));
}

NODE *lexp(NODE *args) {
    return(binary(args, 'e'));
}

NODE *llog10(NODE *args) {
    return(binary(args, 'g'));
}

NODE *lln(NODE *args) {
    return(binary(args, 'n'));
}

NODE *lpower(NODE *args) {
    return(binary(args, 'p'));
}

int compare_numnodes(NODE *n1, NODE *n2) {
    FLONUM f;
    FIXNUM i;

    if (nodetype(n1) == INT) {
	if (nodetype(n2) == INT) {
	    i = getint(n1) - getint(n2);
	    return (i == 0L ? 0 : (i > 0L ? 1 : -1));
	} else {
	    f = (FLONUM)getint(n1) - getfloat(n2);
	    return(f == 0.0 ? 0 : (f > 0.0 ? 1 : -1));
	}
    }
    else {
	if (nodetype(n2) == INT) {
	    f = getfloat(n1) - (FLONUM)getint(n2);
	    return(f == 0.0 ? 0 : (f > 0.0 ? 1 : -1));
	}
	else {
	    f = getfloat(n1) - getfloat(n2);
	    return(f == 0.0 ? 0 : (f > 0.0 ? 1 : -1));
	}
    }
}

NODE *torf(BOOLEAN tf) {
    return (tf ? TrueName() : FalseName());
}

NODE *llessp(NODE *args) {
    NODE *n1, *n2;

    n1 = numeric_arg(args);
    n2 = numeric_arg(cdr(args));

    if (NOT_THROWING) {
	return torf(compare_numnodes(n1, n2) < 0);
    }
    return(UNBOUND);
}

NODE *llessequalp(NODE *args) {
    NODE *n1, *n2;

    n1 = numeric_arg(args);
    n2 = numeric_arg(cdr(args));

    if (NOT_THROWING) {
	return torf(compare_numnodes(n1, n2) <= 0);
    }
    return(UNBOUND);
}

NODE *lgreaterp(NODE *args) {
    NODE *n1, *n2;

    n1 = numeric_arg(args);
    n2 = numeric_arg(cdr(args));

    if (NOT_THROWING) {
	return torf(compare_numnodes(n1, n2) > 0);
    }
    return(UNBOUND);
}

NODE *lgreaterequalp(NODE *args) {
    NODE *n1, *n2;

    n1 = numeric_arg(args);
    n2 = numeric_arg(cdr(args));

    if (NOT_THROWING) {
	return torf(compare_numnodes(n1, n2) >= 0);
    }
    return(UNBOUND);
}

int compare_node(NODE *n1, NODE *n2, BOOLEAN ignorecase) {
    NODE *a1 = NIL, *a2 = NIL, *nn1 = NIL, *nn2 = NIL;
    int icmp = 0, cmp_len;
    NODETYPES nt1, nt2;

    if (n1 == n2) return 0;

    nt1 = nodetype(n1);
    nt2 = nodetype(n2);

    if (!(nt1 & NT_WORD) || !(nt2 & NT_WORD)) return -9999;

    if (nt1==QUOTE && is_list(node__quote(n1))) return -9999;
    if (nt2==QUOTE && is_list(node__quote(n2))) return -9999;

    if (nt1 == CASEOBJ && nt2 == CASEOBJ && ignorecase &&
	 (object__caseobj(n1) == object__caseobj(n2))) return 0;

    if ((nt1 & NT_NUMBER) && (nt2 & NT_NUMBER))
	return compare_numnodes(n1, n2);

    if (nt1 & NT_NUMBER) {
	nn2 = cnv_node_to_numnode(n2);
	if (nn2 != UNBOUND) {
	    icmp = compare_numnodes(n1, nn2);
	    return icmp;
	}
    }

    if (nt2 & NT_NUMBER) {
	nn1 = cnv_node_to_numnode(n1);
	if (nn1 != UNBOUND) {
	    icmp = compare_numnodes(nn1, n2);
	    return icmp;
	}
    }

    a1 = cnv_node_to_strnode(n1);
    a2 = cnv_node_to_strnode(n2);
    nt1 = nodetype(a1);
    nt2 = nodetype(a2);
    if (nt1 == STRING && nt2 == STRING) {
	if (getstrptr(a1) == getstrptr(a2))
	    icmp = getstrlen(a1) - getstrlen(a2);
	else {
	    cmp_len = (getstrlen(a1) > getstrlen(a2)) ?
		    getstrlen(a2) : getstrlen(a1);

	    if (ignorecase)
		icmp = low_strncmp(getstrptr(a1), getstrptr(a2), cmp_len);
	    else
		icmp = strncmp(getstrptr(a1), getstrptr(a2), cmp_len);
	    if (icmp == 0)
		icmp = getstrlen(a1) - getstrlen(a2);
	}
    }
    else if (nt1 & NT_BACKSL || nt2 & NT_BACKSL) {
	if (getstrptr(a1) == getstrptr(a2))
	    icmp = getstrlen(a1) - getstrlen(a2);
	else {
	    cmp_len = (getstrlen(a1) > getstrlen(a2)) ?
			getstrlen(a2) : getstrlen(a1);

	    if (ignorecase)
		icmp = noparitylow_strncmp(getstrptr(a1),
					   getstrptr(a2), cmp_len);
	    else
		icmp = noparity_strncmp(getstrptr(a1), getstrptr(a2), cmp_len);
	    if (icmp == 0)
		icmp = getstrlen(a1) - getstrlen(a2);
	}
    }
    else err_logo(FATAL, NIL);
 
    return(icmp);
}

BOOLEAN equalp_help(NODE *arg1, NODE *arg2, BOOLEAN ignc) {
    if (is_list(arg1)) {
	if (!is_list(arg2)) return FALSE;
	while (arg1 != NIL && arg2 != NIL) {
	    if (!equalp_help(car(arg1), car(arg2), ignc))
		return FALSE;
	    arg1 = cdr(arg1);
	    arg2 = cdr(arg2);
	    if (check_throwing) break;
	}
	return (arg1 == NIL && arg2 == NIL);
    } else if (is_list(arg2))
	return FALSE;
    else if (nodetype(arg1) == ARRAY) {
	if (nodetype(arg2) != ARRAY) return FALSE;
	return (arg1 == arg2);
    } else if (nodetype(arg2) == ARRAY)
	return FALSE;
    else return (!compare_node(arg1, arg2, ignc));
}

NODE *lequalp(NODE *args) {
    NODE *arg1, *arg2;
    BOOLEAN val;

    arg1 = car(args);
    arg2 = cadr(args);

    if (varTrue(Caseignoredp))
	val = equalp_help(arg1, arg2, TRUE);
    else
	val = equalp_help(arg1, arg2, FALSE);

    return(torf(val));
}

NODE *lnotequalp(NODE *args) {
    NODE *arg1, *arg2;
    BOOLEAN val;

    arg1 = car(args);
    arg2 = cadr(args);

    if (varTrue(Caseignoredp))
	val = equalp_help(arg1, arg2, TRUE);
    else
	val = equalp_help(arg1, arg2, FALSE);

    return(torf(!val));
}

NODE *l_eq(NODE *args) {
    return torf(car(args) == cadr(args));
}

NODE *lbeforep(NODE *args) {
    NODE *arg1, *arg2;
    int val;

    arg1 = string_arg(args);
    arg2 = string_arg(cdr(args));

    if (varTrue(Caseignoredp))
	val = compare_node(arg1, arg2, TRUE);
    else
	val = compare_node(arg1, arg2, FALSE);

    return (val < 0 ? TrueName() : FalseName());
}
