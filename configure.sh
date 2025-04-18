#! /bin/sh

# local options:  ac_help is the help message that describes them
# and LOCAL_AC_OPTIONS is the script that interprets them.  LOCAL_AC_OPTIONS
# is a script that's processed with eval, so you need to be very careful to
# make certain that what you quote is what you want to quote.

# load in the configuration file
#
ac_help='
--disable-vi		Disable vi line editing
--disable-emacs		Disable emacs line editing
--disable-jobs		Disable job control
--disable-braces	Don'\''t compile in brace expansion (a{b,c} -> ab ac)
--path=PATH		Default path if not defined in <paths.h>
--{sh,ksh}		Specify whether to build a ksh or a bourne sh
--env=ENV		Default environment
--history={no,simple,complex}	History support
--posix			Posix behavior by default
--silly			[A silly option]
--swtch			Shell layer (shl(1)) support.  Obsolete?'

LOCAL_AC_OPTIONS='
set=`locals $*`;
if [ "$set" ]; then
    eval $set
    shift 1
else
    ac_error=T;
fi'

locals() {
    local history
    local K=`echo $1 | $AC_UPPERCASE`
    case "$K" in
    --DISABLE-*)disable=`echo $K | sed -e 's/--DISABLE-//' | tr '-' '_'`
		echo NO_${disable}=T ;;
    --HISTORY=*)history=`echo $K | sed -e 's/^--HISTORY=//'`
		if [ "$history" = "NO" ]; then
		    echo HISTORY=$history
		elif [ "$history" = "SIMPLE" ]; then
		    echo HISTORY=$history
		elif [ "$history" = "COMPLEX" ]; then
		    echo HISTORY=$history
		fi ;;
    --PATH=*)   echo "$1" | sed -e's/^--..../DEFAULT_PATH/' ;;
    --ENV=*)	echo "$1" | sed -e's/^--.../DEFAULT_ENV/' ;;
    --SH)	echo SHELL=SH ;;
    --KSH)	echo SHELL=KSH ;;
    --POSIX)	echo POSIX=T ;;
    --SILLY)	echo SILLY=T ;;
    --SWTCH)	echo SWTCH=T ;;
    esac
}

VERSION=`grep -i VERSION: IAFA-PACKAGE | awk '{print $2}'`
TARGET=pdksh
SHELL=KSH
HISTORY=COMPLEX

. ./configure.inc

AC_INIT $TARGET
unset _MK_LIBRARIAN

AC_CHECK_FIELD stat st_rdev sys/types.h sys/stat.h unistd.h

# check for __attribute__(noreturn), which means that __attribute__ works
AC_CHECK_NORETURN && AC_DEFINE 'HAVE_GCC_FUNC_ATTR' '1'

AC_CHECK_FUNCS flock
AC_CHECK_FUNCS strlcpy
AC_CHECK_FUNCS strlcat

test -d /dev/fd/0 && AC_DEFINE 'HAVE_DEV_FD' '1'

LOGN "checking whether #! script headers work "
PGM_FALSE=`acLookFor false`

if [ ! "$PGM_FALSE" ] ;then
    echo "int main() { return 1; }" >> ngc$$.c
    if $AC_CC -o ngc$$ ngc$$.c; then
	PGM_FALSE=`pwd`/ngc$$
    else
	rm -f ngc$$ ngc$$.c
	AC_FAIL "can't compile a falsifier?"
    fi
fi
cat > ngc$$.sh << EOF
#! $PGM_FALSE
exit 0
EOF
chmod +x ngc$$.sh

if ./ngc$$.sh; then
    LOG "(no)"
else
    AC_DEFINE 'SHARPBANG' '1'
    LOG "(yes)"
fi
rm -f ngc$$ ngc$$.sh ngc$$.c

# add in the various flags that can be passed on in

AC_DEFINE 'RETSIGTYPE' 'void'
AC_DEFINE 'RETSIGVAL' '/**/'

AC_SUB 'ac_exe_suffix' ''

test "$SILLY" && AC_DEFINE 'SILLY' '1'
test "$NO_JOBS" || AC_DEFINE 'JOBS' '1'
test "$POSIX" && AC_DEFINE 'POSIXLY_CORRECT' '1'
test "$NO_BRACES" || AC_DEFINE 'BRACE_EXPAND' '1'

if [ "$SHELL" = "SH" ]; then
    AC_DEFINE 'SH' '1'
    AC_SUB 'SHELL_PROG' 'sh'
else
    AC_DEFINE 'KSH' '1'
    AC_SUB 'SHELL_PROG' 'ksh'
    test "$NO_VI" || AC_DEFINE 'VI' '1'
    test "$NO_EMACS" || AC_DEFINE 'EMACS' '1'
fi

case "$DEFAULT_ENV" in
"")	;;
0|NULL)	AC_DEFINE 'DEFAULT_ENV' $DEFAULT_ENV ;;
*)	AC_DEFINE 'DEFAULT_ENV' \""${DEFAULT_ENV}"\" ;;
esac

AC_DEFINE 'DEFAULT_PATH' '"'${DEFAULT_PATH:-/bin:/usr/bin:/usr/local/bin}'"'


if [ "$HISTORY" != "NO" ]; then
    AC_DEFINE 'HISTORY' '1'
    if [ "$HISTORY" = "COMPLEX" ] ;then
	AC_DEFINE 'COMPLEX_HISTORY' '1'
    else
	AC_DEFINE 'EASY_HISTORY' '1'
    fi
fi

AC_TEXT '#include "conf-end.h"'

AC_OUTPUT Makefile

LOG
LOGN "shell type:     "; test "$SHELL" = "KSH" && LOG "ksh" || LOG "sh"
LOGN "vi-mode:        "; test "$NO_VI" && LOG "no" || LOG "yes"
LOGN "emacs-mode:     "; test "$NO_EMACS" && LOG "no" || LOG "yes"
LOGN "job control:    "; test "$NO_JOBS" && LOG "no" || LOG "yes"
LOGN "brace expansion:"; test "$NO_BRACES" && LOG "no" || LOG "yes"
LOGN "posixly_correct:"; test "$POSIX" && LOG "yes" || LOG "no"
LOGN "shell layers:   "; test "$SWTCH" && LOG "yes" || LOG "no"
test "$SILLY" && LOG "silly:          quite so!"
test "$DEFAULT_PATH" && LOG "default_path:   $DEFAULT_PATH"
test "$DEFAULT_ENV"  && LOG "default_env:    $DEFAULT_ENV"
case "$HISTORY" in
NO)      LOG "history:        disabled" ;;
COMPLEX) LOG "history:        complex" ;;
*)       LOG "history:        simple" ;;
esac
LOG
