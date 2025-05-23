#   Copyright (c) 1999-2017 David Parsons. All rights reserved.
#   
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#  3. My name may not be used to endorse or promote products derived
#     from this software without specific prior written permission.
#     
#  THIS SOFTWARE IS PROVIDED BY DAVID PARSONS ``AS IS'' AND ANY
#  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVID
#  PARSONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
#  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGE.
#


#
# this preamble code is executed when this file is sourced and it picks
# interesting things off the command line.
#
ac_default_path="/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin"

ac_standard="--src=DIR		where the source lives (.)
--prefix=DIR		where to install the final product (/usr/local)
--execdir=DIR		where to put executables (prefix/bin)
--sbindir=DIR		where to put static executables (prefix/sbin)
--confdir=DIR		where to put configuration information (/etc)
--libdir=DIR		where to put libraries (prefix/lib)
--libexecdir=DIR	where to put private executables
--mandir=DIR		where to put manpages"


# remove files created by configure on abend
#
AC_EXIT() {
    rm -f $__config_files
    exit ${1:-1}
}

__fail=AC_EXIT


if dirname B/A 2>/dev/null >/dev/null; then
__ac_dirname() {
    dirname "$1"
}
else
__ac_dirname() {
    echo "$1" | sed -e 's:/[^/]*$::'
}
fi

__remove() {
    if [ \( -x "$1" \) -a "$__MACOS_DSYM" ]; then
	rm -rf "$1".dSYM
    fi
    rm -f "$@"
}

ac_progname=$0
ac_configure_command=
Q=\'
for x in "$@"; do
    ac_configure_command="$ac_configure_command $Q$x$Q"
done
# ac_configure_command="$*"

__d=`__ac_dirname "$ac_progname"`
if [ "$__d" = "$ac_progname" ]; then
    AC_SRCDIR=`pwd`
else
    AC_SRCDIR=`cd $__d;pwd`
fi

__ac_dir() {
    if test -d "$1"; then
	(cd "$1";pwd)
    else
	echo "$1";
    fi
}

#
# echo w/o newline
#
echononl()
{
    ${ac_echo:-echo} "${@}$ac_echo_nonl"
}

#
# log something to the terminal and to a logfile.
#
LOG () {
    echo "$@"
    echo "$@" 1>&5
}

#
# log something to the terminal without a newline, and to a logfile with
# a newline
#
LOGN () {
    echononl "$@" 1>&5
    echo "$@"
}

#
# log something to the terminal
#
TLOG () {
    echo "$@" 1>&5
}

#
# log something to the terminal, no newline
#
TLOGN () {
    echononl "$@" 1>&5
}


#
# AC_CONTINUE tells configure not to bomb if something fails, but to
# continue blithely along
#
AC_CONTINUE () {
    __fail="return"
}


#
# generate a .o file from sources
#
__MAKEDOTO() {
    AC_PROG_CC

    if $AC_CC -c -o ngc$$.o "$@" $LIBS 2>ngc$$.err; then
	__remove ngc$$.o ngc$$.err
	TLOG " (found)"
	return 0
    fi
    __remove ngc$$.o
    TLOG " (not found)"
    echo "test failed:  command was $AC_CC -c -o ngc$$.o" "$@" $LIBS
    echo "output:"
    cat ngc$$.err
    __remove ngc$$.err
    echo "offending sources:"
    for x in "$@"; do
	echo "$x:"
	cat $x
    done
    return 1
}


#
# Emulate gnu autoconf's AC_CHECK_HEADERS() function
#
AC_CHECK_HEADERS () {

    AC_PROG_CPP

    for __hdr in "$@"; do
	(   echo "/* AC_CHECK_HEADERS */"
	    echo "#include <${__hdr}>" ) > ngc$$.c

	LOGN "looking for header $__hdr"

	if $AC_CPP_FILTER ngc$$.c >/dev/null; then
	    AC_DEFINE 'HAVE_'`echo $__hdr | $AC_UPPERCASE | tr './' '_'` 1
	    rc=0
	    LOG " (found)"
	else
	    rc=1
	    LOG " (not found)"
	fi
	__remove ngc$$.c
    done
    # return code is only meaningful if one header is checked
    return $rc
}


#
# emulate GNU autoconf's AC_CHECK_FUNCS function
#
AC_CHECK_FUNCS () {
    AC_PROG_CC

    B=`echo "$1" | sed -e 's/(.*)//'`

    case "$B" in
    "$1") F="$1()"; need_proto=1 ;;
    *)    F="$1"  ; unset need_proto ;;
    esac

    shift
    __remove ngc$$.c

    while [ "$1" ]; do
	echo "#include <$1>" >> ngc$$.c
	shift
    done

    if [ "$need_proto" ]; then
	echo "void $F;" >> ngc$$.c
    fi

    cat >> ngc$$.c << EOF
int main()
{

    $F;
}
EOF

    LOGN "looking for the $B function"

    if $AC_CC $AC_CFLAGS $AC_LDFLAGS -o ngc$$ ngc$$.c $LIBS; then
	AC_DEFINE `echo ${2:-HAVE_$B} | $AC_UPPERCASE` 1
	TLOG " (found)"
	rc=0
    else
	echo "offending command was:"
	cat ngc$$.c
	echo "$AC_CC $AC_CFLAGS $AC_LDFLAGS -o ngc$$ ngc$$.c $LIBS"
	TLOG " (not found)"
	rc=1
    fi
    __remove ngc$$ ngc$$.c
    return $rc
}


#
# check to see if some structure exists
#
# usage: AC_CHECK_STRUCT structure {include ...}
#
AC_CHECK_STRUCT () {
    struct=$1
    shift

    __remove ngc$$.c

    for include in $*; do
	echo "#include <$include>" >> ngc$$.c
    done

    cat >> ngc$$.c << EOF
int main()
{
    struct $struct foo;
}
EOF

    LOGN "looking for struct $struct"

    if __MAKEDOTO ngc$$.c; then
	AC_DEFINE HAVE_STRUCT_`echo ${struct} | $AC_UPPERCASE`
	rc=0
    else
	rc=1
    fi
    __remove ngc$$.c
    return $rc
}


#
# check to see if some type exists
#
# usage: AC_CHECK_TYPE type {include ...}
#
AC_CHECK_TYPE () {
    type=$1
    shift

    __remove ngc$$.c

    for include in $*; do
	echo "#include <$include>" >> ngc$$.c
    done

    cat >> ngc$$.c << EOF
int main()
{
    $type foo;
}
EOF

    LOGN "looking for $type type"

    if __MAKEDOTO ngc$$.c; then
	test "$UNDEFINED" || AC_DEFINE HAVE_`echo ${type} | $AC_UPPERCASE`
	rc=0
    else
	rc=1
    fi
    __remove ngc$$.c
    return $rc
}


#
# check to see if some #define exists
#
# usage: AC_CHECK_DEF define {include ...}
#
AC_CHECK_DEF () {
    define=$1
    shift

    __remove ngc$$.c

    for include in $*; do
	echo "#include <$include>" >> ngc$$.c
    done

    cat >> ngc$$.c << EOF
int main()
{
#ifndef $define
#error $define not defined
#endif
}
EOF

    LOGN "looking for #define $define"

    __MAKEDOTO ngc$$.c
    rc=$?
    
    __remove ngc$$.c
    return $rc
}


#
# check to see if some structure contains a field
#
# usage: AC_CHECK_FIELD structure field {include ...}
#
AC_CHECK_FIELD () {

    struct=$1
    field=$2
    shift 2

    __remove ngc$$.c

    for include in $*;do
	echo "#include <$include>" >> ngc$$.c
    done

    cat >> ngc$$.c << EOF
int main()
{
    struct $struct foo;

    foo.$field;
}
EOF

    LOGN "checking that struct $struct has a $field field"

    if __MAKEDOTO ngc$$.c; then
	# HAVE_STRUCT_ is for Gnu configure compatability
	AC_DEFINE HAVE_STRUCT_`echo ${struct}_$field | $AC_UPPERCASE`
	rc=0
    else
	rc=1
    fi
    __remove ngc$$.c
    return $rc
}


#
# check that the C compiler works
#
AC_PROG_CC () {
    test "$AC_CC" && return 0

    cat > ngc$$.c << \EOF
#include <stdio.h>
int main()
{
    puts("hello, sailor");
}
EOF

    TLOGN "checking the C compiler"

    unset AC_CFLAGS AC_LDFLAGS __MACOS_DSYM

    if [ "$CC" ] ; then
	AC_CC="$CC"
    elif [ "$WITH_PATH" ]; then
	AC_CC=`acLookFor cc`
    elif [ "`acLookFor cc`" ]; then
	# don't specify the full path if the user is looking in their $PATH
	# for a C compiler.
	AC_CC=cc
    fi

    # finally check for POSIX c89
    test "$AC_CC" || AC_CC=`acLookFor c89`

    if [ ! "$AC_CC" ]; then
	TLOG " (no C compiler found)"
	$__fail 1
    fi
    echo "checking out the C compiler"

    $AC_CC -o ngc$$ ngc$$.c
    status=$?

    TLOGN " ($AC_CC)"

    if [ $status -eq 0 ]; then
	if $AC_CC -x c /dev/null -dM -E 2>&1 | grep '__clang__' >/dev/null; then
	    TLOG " yuck, you're using clang"
	    IS_CLANG=T
	    IS_BROKEN_CC=T
	elif $AC_CC -v 2>&1 | grep 'gcc version' >/dev/null; then
	    TLOG " oh ick, it looks like gcc"
	    IS_GCC=T
	    IS_BROKEN_CC=T
	else
	    TLOG " ok"
	fi

	# check that the CFLAGS and LDFLAGS aren't bogus

	unset AC_CFLAGS AC_LDFLAGS

	if [ "$CFLAGS" ]; then
	    test "$CFLAGS" && echo "validating CFLAGS=${CFLAGS}"
	    if $AC_CC $CFLAGS -c -o ngc$$.o ngc$$.c ; then
		AC_CFLAGS=${CFLAGS:-"-g"}
		test "$CFLAGS" && TLOG "CFLAGS=\"${CFLAGS}\" are okay"
	    elif [ "$CFLAGS" ]; then
		TLOG "ignoring bogus CFLAGS=\"${CFLAGS}\""
	    fi
	else
	    AC_CFLAGS=-g
	fi
	if [ "$LDFLAGS" ]; then
	    test "$LDFLAGS" && echo "validating LDFLAGS=${LDFLAGS}"
	    if $AC_CC $CFLAGS $LDFLAGS -o ngc$$ ngc$$.c; then
		AC_LDFLAGS=${LDFLAGS:-"-g"}
		test "$LDFLAGS" && TLOG "LDFLAGS=\"${LDFLAGS}\" are okay"
	    elif [ "$LDFLAGS" ]; then
		TLOG "ignoring bogus LDFLAGS=\"${LDFLAGS}\""
	    fi
	else
	    AC_LDFLAGS=${CFLAGS:-"-g"}
	fi
	
	# macos-specific(?) test for .dSYM resource directories
	$AC_CC $AC_CFLAGS $AC_LDFLAGS -o ngc$$ ngc$$.c
	ls -dl ngc$$*
	test -d ngc$$.dSYM && __MACOS_DSYM=1
	
    else
	__remove ngc$$ ngc$$.c ngc$$.o
	AC_FAIL " does not compile code properly"
    fi

    __remove ngc$$ ngc$$.c ngc$$.o

    return $status
}


#
# acLookFor actually looks for a program, without setting anything.
#
acLookFor () {
    path=${AC_PATH:-$ac_default_path}
    case "X$1" in
    X-[rx]) __mode=$1
	    shift
	    ;;
    *)	    __mode=-x
	    ;;
    esac
    oldifs="$IFS"
    for program in $*; do
	IFS=":"
	for x in $path; do
	    if [ $__mode $x/$program -a -f $x/$program ]; then
		echo $x/$program
		break 2
	    fi
	done
    done
    IFS="$oldifs"
    unset __mode
}


#
# check that a program exists and set its path
#
MF_PATH_INCLUDE () {
    SYM=$1; shift

    case X$1 in
    X-[rx]) __mode=$1
	    shift
	    ;;
    *)      unset __mode
	    ;;
    esac

    TLOGN "looking for $1"

    DEST=`acLookFor $__mode $*`

    __sym=`echo "$SYM" | $AC_UPPERCASE`
    if [ "$DEST" ]; then
	TLOG " ($DEST)"
	echo "$1 is $DEST"
	AC_MAK $SYM
	AC_DEFINE PATH_$__sym \""$DEST"\"
	AC_SUB $__sym "$DEST"
	eval CF_$SYM=$DEST
	return 0
    else
	#AC_SUB $__sym ''
	echo "$1 is not found"
	TLOG " (not found)"
	return 1
    fi
}

#
# AC_INIT starts the ball rolling
#
# After AC_INIT, fd's 1 and 2 point to config.log
# and fd 5 points to what used to be fd 1
#
AC_INIT () {
    __config_files="config.cmd config.sub config.h config.mak"
    rm -f $__config_files
    __cwd=`pwd`
    exec 5>&1 1>"$__cwd"/config.log 2>&1
    AC_CONFIGURE_FOR=__AC_`echo $1 | sed -e 's/\..$//' | $AC_UPPERCASE  | tr ' ' '_'`_D

    # check to see whether to use echo -n or echo ...\c
    #
    echo -n hello > $$
    echo world >> $$
    if grep "helloworld" $$ >/dev/null; then
	ac_echo="echo -n"
	echo "[echo -n] works"
    else
	ac_echo="echo"
	echo 'hello\c' > $$
	echo 'world' >> $$
	if grep "helloworld" $$ >/dev/null; then
	    ac_echo_nonl='\c'
	    echo "[echo ...\\c] works"
	fi
    fi
    rm -f $$

    LOG "Configuring for [$1]"
    _MK_LIBRARIAN=Y

    cat > "$__cwd"/config.h << EOF
/*
 * configuration for $1${2:+" ($2)"}, generated `date`
 * by ${LOGNAME:-`whoami`}@`hostname`
 */
#ifndef $AC_CONFIGURE_FOR
#define $AC_CONFIGURE_FOR 1


EOF

    unset __share
    if [ -d $AC_PREFIX/share/man ]; then
	for t in 1 2 3 4 5 6 7 8 9; do
	    if [ -d $AC_PREFIX/share/man/man$t ]; then
		__share=/share
	    elif [ -d $AC_PREFIX/share/man/cat$t ]; then
		__share=/share
	    fi
	done
    else
	__share=
    fi

    if [ -d $AC_PREFIX/libexec ]; then
	__libexec=libexec
    else
	__libexec=lib
    fi


    AC_PREFIX=${AC_PREFIX:-/usr/local}
    AC_EXECDIR=${AC_EXECDIR:-$AC_PREFIX/bin}
    AC_SBINDIR=${AC_SBINDIR:-$AC_PREFIX/sbin}
    AC_LIBDIR=${AC_LIBDIR:-$AC_PREFIX/lib}
    AC_MANDIR=${AC_MANDIR:-$AC_PREFIX$__share/man}
    AC_LIBEXEC=${AC_LIBEXEC:-$AC_PREFIX/$__libexec}
    AC_CONFDIR=${AC_CONFDIR:-/etc}

    AC_PATH=${WITH_PATH:-$PATH}
    AC_PROG_CPP
    AC_PROG_INSTALL

    ac_os=`uname -s`
    _os=`echo $ac_os | $AC_UPPERCASE | sed -e 's/[^A-Z0-9_].*$//'`
    AC_DEFINE OS_$_os	1
    eval OS_${_os}=1
    unset _os
}


#
# AC_LIBRARY checks to see if a given library exists and contains the
# given function.
# usage: AC_LIBRARY function library [alternate ...]
#
AC_LIBRARY() {
    local SRC=$1
    shift

    # first see if the function can be found in any of the
    # current libraries
    LOGN "Looking for the ${SRC} function"
    if AC_QUIET AC_CHECK_FUNCS $SRC; then
	AC_DEFINE HAVE_LIB`echo $1 | sed -e 's/-l//' | $AC_UPPERCASE`
	LOG "(found)"
	return 0
    fi

    # then search through the supplied list of libraries
    local __libs="$LIBS"
    for x in "$@"; do
	LIBS="$__libs $x"
	if AC_QUIET AC_CHECK_FUNCS $SRC; then
	    AC_DEFINE HAVE_LIB`echo $1 | sed -e 's/-l//' | $AC_UPPERCASE`
	    LOG " (in $x)"
	    return 0
	fi
    done
    LOG " (not found)"
    LIBS="$__libs"	# reset LIBS if we couldn't find anything
    return 1
}


#
# AC_PROG_LEX checks to see if LEX exists, and if it's lex or flex.
#
AC_PROG_LEX() {
    TLOGN "looking for lex "

    DEST=`acLookFor lex`
    if [ "$DEST" ]; then
	AC_MAK LEX
	AC_DEFINE PATH_LEX \"$DEST\"
	AC_SUB 'LEX' "$DEST"
	echo "lex is $DEST"
    else
	DEST=`acLookFor flex`
	if [ "$DEST" ]; then
	    AC_MAK FLEX
	    AC_DEFINE 'LEX' \"$DEST\"
	    AC_SUB 'LEX', "$DEST"
	    echo "lex is $DEST"
	else
	    AC_SUB LEX ''
	    echo "neither lex or flex found"
	    TLOG " (not found)"
	    return 1
	fi
    fi
    TLOG "($DEST)"

    if AC_LIBRARY yywrap -ll -lfl; then
	return 0
    else
	TLOG "(no lex library found)"
	return 1
    fi
}


#
# AC_PROG_YACC checks to see if YACC exists, and if it's bison or
# not.
#
AC_PROG_YACC () {

    TLOGN "looking for yacc "

    DEST=`acLookFor yacc`
    if [ "$DEST" ]; then
	AC_MAK YACC
	AC_DEFINE PATH_YACC \"$DEST\"
	AC_SUB 'YACC' "$DEST"
	TLOG "($DEST)"
	echo "yacc is $DEST"
    else
	DEST=`acLookFor bison`
	if [ "$DEST" ]; then
	    AC_MAK BISON
	    AC_DEFINE 'YACC' \"$DEST\"
	    AC_SUB 'YACC' "$DEST -y"
	    echo "yacc is $DEST -y"
	    TLOG "($DEST -y)"
	else
	    AC_SUB 'YACC' ''
	    echo "neither yacc or bison found"
	    TLOG " (not found)"
	    return 1
	fi
    fi
    return 0
}


#
# AC_PROG looks for a program
#
AC_PROG () {
    PN=`basename $1 | $AC_UPPERCASE | tr -dc $AC_UPPER_PAT`

    if set | grep -v PROG_$PN >/dev/null; then
	TLOGN "looking for $1"
	DEST=`acLookFor $1`
	if [ "$DEST" ]; then
	    eval PROG_$PN="$DEST"
	    AC_SUB $PN $DEST
	    TLOG " ($DEST)"
	    return 0
	fi
	AC_SUB $PN true
	TLOG " (not found)"
	return 1
    fi
}


#
# AC_PROG_LN_S checks to see if ln exists, and, if so, if ln -s works
#
AC_PROG_LN_S () {
    test "$PROG_FIND" || AC_PROG_FIND

    test "$PROG_LN_S" && return 0
    
    TLOGN "looking for \"ln -s\""
    DEST=`acLookFor ln`

    if [ "$DEST" ]; then
	rm -f /tmp/b$$
	$DEST -s /tmp/a$$ /tmp/b$$
	if [ "`$PROG_FIND /tmp/b$$ -type l -print`" ]; then
	    TLOG " ($DEST)"
	    echo "$DEST exists, and ln -s works"
	    PROG_LN_S="$DEST -s"
	    AC_SUB 'LN_S' "$DEST -s"
	    rm -f /tmp/b$$
	else
	    AC_SUB 'LN_S' ''
	    TLOG " ($DEST exists, but -s does not seem to work)"
	    echo "$DEST exists, but ln -s doesn't seem to work"
	    rm -f /tmp/b$$
	    return 1
	fi
    else
	AC_SUB 'LN_S' ''
	echo "ln not found"
	TLOG " (not found)"
	return 1
    fi
}


#
# AC_PROG_FIND looks for the find program and sets the FIND environment
# variable
#
AC_PROG_FIND () {
    if test -z "$PROG_FIND"; then
	MF_PATH_INCLUDE FIND find
	rc=$?
	PROG_FIND=$CF_FIND
	return $rc
    fi
    return 0
}


#
# AC_PROG_AWK looks for the awk program and sets the AWK environment
# variable
#
AC_PROG_AWK () {
    if test -z "$AC_AWK_PROG"; then
	MF_PATH_INCLUDE AWK awk
	rc=$?
	AC_AWK_PROG=$DEST
	return $rc
    fi
    return 0
}


#
# AC_PROG_SED looks for the sed program and sets the SED environment
# variable
#
AC_PROG_SED () {
    if test -z "$AC_SED_PROG"; then
	MF_PATH_INCLUDE SED sed
	rc=$?
	AC_SED_PROG=$DEST
	return $rc
    fi
    return 0
}


#
# AC_HEADER_SYS_WAIT looks for sys/wait.h
#
AC_HEADER_SYS_WAIT () {
    AC_CHECK_HEADERS sys/wait.h || return 1
}

#
# AC_TYPE_PID_T checks to see if the pid_t type exists
#
AC_TYPE_PID_T () {

    AC_CHECK_TYPE pid_t sys/types.h
    return $?
}


#
# check for the existence of  __attribute__((__noreturn__))
#
AC_CHECK_NORETURN() {
    AC_PROG_CC

    echo "extern int noreturn  __attribute__((__noreturn__));" > ngc$$.c

    TLOGN "Checking __attribute__((__noreturn__)) "
    if $AC_CC -c ngc$$.c; then
	TLOG "(yes)"
	AC_DEFINE 'NORETURN' ' __attribute__((__noreturn__))'
    else
	TLOG "(no)"
	AC_DEFINE 'NORETURN' ''
    fi
    rm -f ngc$$.o ngc$$.c
}


#
# AC_C_INLINE checks to see if compiler supports the inline keyword
#
AC_C_INLINE() {
    echo 'inline int foo() { return 1; }' > ngc$$.c
    LOGN 'Checking for "inline" keyword'
    if __MAKEDOTO ngc$$.c; then
	rc=0
    else
	AC_DEFINE inline '/**/'
	rc=1
    fi
    __remove ngc$$.c
    return $rc
}


#
# AC_WHATIS tries to print out the value of a macro
#
AC_WHATIS() {
    MODE=$1	# what it should be (string,int,char)
    shift
    MACRO=$1	# the macro name
    shift

    case "$MODE" in
    string)
	__fmt='%s' ;;
    int)
	__fmt='%d' ;;
    char)
	__fmt='%c' ;;
    *)  LOG "AC_WHATIS $MODE $MACRO -- mode isn't string, int, or char"
	return 1 ;;
    esac

    (   echo '#include <stdio.h>'

	for x in "$@"; do
	    echo "#include <${x}>"
	done

	echo "main() { printf(\"${MACRO}=\\\"${__fmt}\\\"\\n\", ${MACRO}); }" )  > _ngc$$.c

    if $AC_CC $AC_CFLAGS -o _ngc$$ _ngc$$.c; then
	./_ngc$$
	rc=0
    else
	rc=1
    fi
    rm -f _ngc$$ _ngc$$.c
    return $rc
}


#
# AC_SCALAR_TYPES checks to see if the compiler can generate 2 and 4 byte ints.
#
AC_SCALAR_TYPES () {

    rc=1
    LOGN "defining WORD & DWORD scalar types"
    
#    if AC_QUIET AC_CHECK_HEADERS WinDef.h; then
#	# windows machine; BYTE, WORD, DWORD already
#	# defined
#	echo "#include <WinDef.h>" >> "$__cwd"/config.h
#	TLOG " (defined in WinDef.h)"
#	return 0
#    fi
	
    cat > ngc$$.c << EOF
#include <stdio.h>
#include <string.h>

int pound_define = 1;

void
say(char *w, char *v)
{
    printf(pound_define ? "#define %s %s\n"
			: "s:@%s@:%s:g\n", w, v);
}

int
main(argc, argv)
char **argv;
{
    unsigned long v_long;
    unsigned int v_int;
    unsigned short v_short;

    if ( argc > 1 && strcmp(argv[1], "sub") == 0 )
	pound_define = 0;
	
    if (sizeof v_long == 4)
	say("DWORD", "unsigned long");
    else if (sizeof v_int == 4)
	say("DWORD", "unsigned int");
    else
	return 1;

    if (sizeof v_int == 2)
	say("WORD", "unsigned int");
    else if (sizeof v_short == 2)
	say("WORD", "unsigned short");
    else
	return 2;
	
    say("BYTE", "unsigned char");
    fprintf(stderr, "OK!");
    return 0;
}
EOF

    if $AC_CC ngc$$.c -o ngc$$; then
	if [ $# -gt 0 ]; then
	    while [ "$1" ]; do
		case "$1" in
		sub)if ./ngc$$ sub >> "$__cwd"/config.sub; then
			rc=0
		    fi;;
		*)  if ./ngc$$ >> "$__cwd"/config.h; then
			rc=0
		    fi ;;
		esac
		shift
	    done
	elif ./ngc$$ >> "$__cwd"/config.h; then
	    rc=0
	fi
	if [ "$rc" != 0 ]; then
	    if ./ngc$$ >> "$__cwd"/config.h; then
		rc=0
	    fi
	fi
    fi
    __remove ngc$$ ngc$$.c
    case "$rc" in
    0) TLOG "" ;;
    *) AC_FAIL " ** FAILED **" ;;
    esac
    return $rc
}


#
# AC_OUTPUT generates makefiles from makefile.in's
#
AC_OUTPUT () {
    
    cd "$__cwd"
    AC_SUB 'LIBS'    "$LIBS"
    
    if test "$__MACOS_DSYM"; then
	# deal with extra OSX droppings, if they exist
	AC_SUB 'DISTCLEAN' 'rm -fr'
	AC_SUB 'GENERATED_FILES' "*.dSYM $*"
    else
	AC_SUB 'DISTCLEAN' 'rm -f'
	AC_SUB 'GENERATED_FILES' "$*"
    fi
    
    AC_SUB 'CC'       "$AC_CC"
    AC_SUB 'CFLAGS'   "$AC_CFLAGS"
    AC_SUB 'LDFLAGS'  "$AC_LDFLAGS"
    AC_SUB 'CPPFLAGS' "$CPPFLAGS"
    AC_SUB 'srcdir'   "$AC_SRCDIR"
    AC_SUB 'prefix'   "$AC_PREFIX"
    AC_SUB 'exedir'   "$AC_EXECDIR"
    AC_SUB 'bindir'   "$AC_EXECDIR"
    AC_SUB 'sbindir'  "$AC_SBINDIR"
    AC_SUB 'libdir'   "$AC_LIBDIR"
    AC_SUB 'libexec'  "$AC_LIBEXEC"
    AC_SUB 'confdir'  "$AC_CONFDIR"
    AC_SUB 'mandir'   "$AC_MANDIR"

    if [ "$_MK_LIBRARIAN" ] && echo "$__config_files" | grep -v librarian.sh >/dev/null; then
	# write a librarian that works with static libraries
	if AC_PROG_LN_S ; then
	    __dolink=$PROG_LN_S
	elif AC_PROG ln; then
	    __dolink=$PROG_LN
	elif AC_PROG cp; then
	    __dolink=$PROG_CP
	else
	    __dolink=:
	fi
	AC_PROG ar
	AC_PROG ranlib
	AC_SUB LD_LIBRARY_PATH HERE
	__config_files="$__config_files librarian.sh"
	cat > librarian.sh << EOF
#! /bin/sh
#
#  Build static libraries, hiding (some) ickiness from the makefile

ACTION=\$1; shift
LIBRARY=\$1; shift
VERSION=\$1; shift

case "\$ACTION" in
make)   # first strip out any libraries that might
	# be passed in on the object line
	objs=
	for x in "\$@"; do
	    case "\$x" in
	    -*) ;;
	    *) objs="\$objs \$x" ;;
	    esac
	done
	${PROG_AR} crv \$LIBRARY.a \$objs
	${PROG_RANLIB} \$LIBRARY.a
	rm -f \$LIBRARY
	${__dolink} \$LIBRARY.a \$LIBRARY
	;;
files)  echo "\${LIBRARY}.a"
	;;
install)$PROG_INSTALL -m 644 \${LIBRARY}.a \$1
	;;
esac
EOF
	chmod +x librarian.sh
    fi

    AC_SUB 'CONFIGURE_FILES' "$__config_files config.log"

    if [ -r config.sub ]; then
	test "$AC_SED_PROG" || AC_PROG_SED
	test "$AC_SED_PROG" || return 1

	echo                                   >> config.h
	echo "#endif/* ${AC_CONFIGURE_FOR} */" >> config.h

	rm -f config.cmd
	Q=\'
	cat - > config.cmd << EOF
#! /bin/sh
${CC:+CC=${Q}${CC}${Q}} ${CFLAGS:+CFLAGS=${Q}${CFLAGS}${Q}} ${LDFLAGS:+LDFLAGS=${Q}${LDFLAGS}${Q}} $ac_progname $ac_configure_command
EOF
	chmod +x config.cmd

	__d=$AC_SRCDIR
	for makefile in $*;do
	    if test -r "$__d/${makefile}.in"; then
		LOG "generating $makefile"
		./config.md `__ac_dirname ./$makefile` 2>/dev/null
		$AC_SED_PROG -f config.sub < "$__d/${makefile}.in" > $makefile
		__config_files="$__config_files $makefile"
	    else
		LOG "WARNING: ${makefile}.in does not exist!"
	    fi
	done
	unset __d

    else
	echo 
    fi
    rm -f config.sed
}

#
# AC_CHECK_FLOCK checks to see if flock() exists and if the LOCK_NB argument
# works properly.
#
AC_CHECK_FLOCK() {

    AC_CHECK_HEADERS sys/types.h sys/file.h fcntl.h

    cat << EOF > ngc$$.c
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>

int main()
{
    int x = open("ngc$$.c", O_RDWR, 0666);
    int y = open("ngc$$.c", O_RDWR, 0666);

    alarm(1);
    if (flock(x, LOCK_EX) != 0)
	exit(1);
    if (flock(y, LOCK_EX|LOCK_NB) == 0)
	exit(1);
    exit(0);
}
EOF

    LOGN "checking flock() sanity"
    HAS_FLOCK=0
    if $AC_CC -o ngc$$ ngc$$.c ; then
	if ./ngc$$ ; then
	    LOG " (good)"
	    HAS_FLOCK=1
	    AC_DEFINE HAS_FLOCK
	else
	    LOG " (bad)"
	fi
    else
	LOG " (not found)"
    fi

    __remove ngc$$ ngc$$.c

    case "$HAS_FLOCK" in
    0) return 1 ;;
    *) return 0 ;;
    esac
}


#
# AC_CHECK_RESOLVER finds out whether the berkeley resolver is
# present on this system.
#
AC_CHECK_RESOLVER () {
    AC_PROG_CC

    TLOGN "looking for the Berkeley resolver library"

    __ACR_rc=0

    cat > ngc$$.c << EOF
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

int main()
{
    char bfr[256];

    res_init();
    res_query("hello", C_IN, T_A, bfr, sizeof bfr);
}
EOF

    if $AC_CC -o ngc$$ ngc$$.c; then
	TLOG " (found)"
    elif $AC_CC -o ngc$$ ngc$$.c -lresolv; then
	TLOG " (yes, with -lresolv)"
	LIBS="$LIBS -lresolv"
    elif $AC_CC -DBIND_8_COMPAT -o ngc$$ ngc$$.c; then
	TLOG " (yes, with BIND_8_COMPAT)"
	AC_DEFINE BIND_8_COMPAT 1
    elif $AC_CC -DBIND_8_COMPAT -o ngc$$ ngc$$.c -lresolv; then
	TLOG " (yes, with BIND_8_COMPAT & -lresolv)"
	LIBS="$LIBS -lresolv"
	AC_DEFINE BIND_8_COMPAT 1
    else
	TLOG " (not found)"
	__ACR_rc=1
    fi
    __remove ngc$$ ngc$$.c
    return $__ACR_rc
}


#
# AC_CHECK_ALLOCA looks for alloca
#
AC_CHECK_ALLOCA () {

    AC_PROG_CC
    AC_CHECK_HEADERS stdlib.h

    cat - > ngc$$.c << EOF
#if T
# include <alloca.h>
#else
# include <stdlib.h>
#endif
int main()
{
	alloca(10);
}
EOF

    LOGN "looking for the alloca function"
    if $AC_CC -DT ngc$$.c -o ngc$$; then
	AC_DEFINE 'HAVE_ALLOCA_H' 1
	status=0
	TLOG " (found in alloca.h)"
    elif $AC_CC ngc$$.c -o ngc$$; then
	TLOG " (found)"
	status=0
    else
	TLOG " (not found)"
	status=1
    fi
    __remove ngc$$ ngc$$.c
    return $status

}


#
# AC_CHECK_BASENAME looks for a copy of basename that does NOT use
# a local static buffer to hold results in.
#
AC_CHECK_BASENAME() {
    TLOGN "looking for a reentrant basename "

    cat > ngc$$.c << EOF
#include <string.h>

extern char *basename(char*);

int main()
{
    char *a = basename("/a/test");
    char *b = basename("/a/nother");

    return (strcmp(a,b) != 0) ? 0 : 1;

}
EOF

    if $AC_CC -o ngc$$ ngc$$.c $LIBS; then
	if ./ngc$$; then
	    TLOG "(found)"
	    AC_DEFINE 'HAVE_BASENAME' 1
	    AC_CHECK_HEADERS libgen.h
	else
	    TLOG "(broken)"
	fi
    else
	TLOG "(not found)"
    fi
    __remove ngc$$ ngc$$.c
}

#
# AC_COMPILER_PIC checks for the compiler option to produce position independent
# code.  At the moment we assume gcc semantics.
#
AC_COMPILER_PIC () {
    AC_PROG_CC

    LOGN "checking for C compiler option to produce PIC "
    echo "int some_variable = 0;" > ngc$$.c 

    if $AC_CC -c -fPIC -o ngc$$ ngc$$.c $LIBS; then
	AC_CFLAGS="$AC_CFLAGS -fPIC"
        LOG "(-fPIC)"
	__rc=0
    else
        LOG "(none)"
	__rc=1
    fi
    __remove ngc$$ ngc$$.c
    return $__rc
}


# generate a macosX librarian
#
__AC_MACOS_LIBRARIAN() {
    AC_SUB LD_LIBRARY_PATH DYLD_LIBRARY_PATH
    __config_files="$__config_files librarian.sh"
    cat > librarian.sh << EOF
#! /bin/sh
#
#  Build MacOS shared libraries, hiding (some) ickiness from the makefile

ACTION=\$1; shift
LIBRARY=\$1; shift
	
eval \`awk -F. '{ printf "MAJOR=%d\n", \$1;
		  printf "VERSION=%d.%d.%d\n", \$1, \$2, \$3; }' \$1\`
shift

LIBNAME=\$LIBRARY.dylib
FULLNAME=\$LIBNAME

case "\$ACTION" in
make)   FLAGS="$AC_CFLAGS -dynamiclib"
	VFLAGS="-current_version \$VERSION -compatibility_version \$MAJOR"

	rm -f \$LIBRARY
	if $AC_CC \$FLAGS \$VFLAGS -o \$FULLNAME "\$@"; then
	    $PROG_LN_S \$FULLNAME \$LIBRARY
	fi
	;;
files)  echo "\$FULLNAME"
	;;
install)$PROG_INSTALL -c \$FULLNAME "\$1"
	;;
esac
EOF
    chmod +x librarian.sh
}


# Generate an ELF librarian (for Linux, freebsd)
#
__AC_ELF_LIBRARIAN() {
    AC_SUB LD_LIBRARY_PATH LD_LIBRARY_PATH
    # -Wl option probably works, but be paranoid anyway
    _VFLAGS="$AC_PICFLAG -shared -Wl,-soname,ngc$$.so.1"
    if $AC_CC $_VFLAGS -o ngc$$.so ngc$$.c; then
	USE_SONAME=T
    fi
    LDCONFIG=`AC_PATH=/sbin:/usr/sbin:/usr/local/sbin acLookFor ldconfig`

    if [ "$LDCONFIG" ]; then
	case `uname -s 2>/dev/null | $AC_UPPERCASE` in
	*BSD)	# *BSD ldconfig, when passed a directory, blows away the
		# ld.so hints file and replaces it with one that's just
		# the files in the library.   It needs a `-m` flag to
		# tell it to merge the new entries with the old
		LDCONFIG="$LDCONFIG -m" ;;
	esac
    fi
    
    __config_files="$__config_files librarian.sh"
    cat > librarian.sh << EOF
#! /bin/sh
#
#  Build ELF shared libraries, hiding (some) ickiness from the makefile

ACTION=\$1; shift
LIBRARY=\$1; shift
	
eval \`awk -F. '{ printf "MAJOR=%d\n", \$1;
		  printf "VERSION=%d.%d.%d\n", \$1, \$2, \$3; }' \$1\`
shift

LIBNAME=\$LIBRARY.so
FULLNAME=\$LIBNAME.\$VERSION

case "\$ACTION" in
make)   FLAGS="$AC_CFLAGS -shared"
	unset VFLAGS
	test "$USE_SONAME" && VFLAGS="-Wl,-soname,\$LIBNAME.\$MAJOR"

	rm -f \$LIBRARY \$LIBNAME \$LIBNAME.\$MAJOR
	if $AC_CC \$FLAGS \$VFLAGS -o \$FULLNAME "\$@"; then
	    $PROG_LN_S \$FULLNAME \$LIBRARY
	    $PROG_LN_S \$FULLNAME \$LIBNAME
	    $PROG_LN_S \$FULLNAME \$LIBNAME.\$MAJOR
	fi
	;;
files)  echo "\$FULLNAME" "\$LIBNAME" "\$LIBNAME.\$MAJOR"
	;;
install)$PROG_INSTALL -c \$FULLNAME "\$1"
	$PROG_LN_S -f \$FULLNAME \$1/\$LIBNAME.\$MAJOR
	$PROG_LN_S -f \$FULLNAME \$1/\$LIBNAME
EOF
    test "$LDCONFIG" && echo '	'$LDCONFIG '"$1"' >> librarian.sh
    cat >> librarian.sh << EOF
	;;
esac
EOF
    chmod +x librarian.sh
}


#
# AC_CC_SHLIBS checks if the C compiler can produce shared libraries
# and if it can writes a librarian that handles those libraries for us.
#
AC_CC_SHLIBS () {
    AC_PROG_CC || AC_FAIL "Need a C compiler to build shared libraries"
    AC_PROG_LN_S || AC_FAIL "Need to be able to make symbolic links for shared libraries"
    AC_PROG_INSTALL || AC_FAIL "Need an install program to install shared libraries"
    
    LOGN "checking whether the C compiler can build shared libraries "

    echo "int some_variable = 0;" > ngc$$.c 

    _MK_LIBRARIAN=
    if uname -a | grep Darwin >/dev/null; then
	# Claims to be macos?
	if $AC_CC $AC_PICFLAG  -dynamiclib -o ngc$$.so ngc$$.c; then
	    __AC_MACOS_LIBRARIAN

	    LOG "(yes; macos dylib)"
	    __rc=0
	else
	    LOG "(no)"
	    __rc=1
	fi
    elif $AC_CC $AC_PICFLAG -shared -o ngc$$.so ngc$$.c; then
	__AC_ELF_LIBRARIAN
        LOG "(yes; -shared)"
	__rc=0
    else
	_MK_LIBRARIAN=Y
        LOG "(no)"
	__rc=1
    fi

    __remove ngc$$.so ngc$$.c

    return $__rc
}


#
# AC_PROG_INSTALL finds the install program and guesses whether it's a 
# Berkeley or GNU install program
#
AC_PROG_INSTALL () {

    DEST=`acLookFor install`

    LOGN "looking for install"
    unset IS_BSD
    if [ "$DEST" ]; then
	# BSD install or GNU install?  Let's find out...
	touch /tmp/a$$

	$DEST /tmp/a$$ /tmp/b$$

	if test -r /tmp/a$$; then
	    LOG " ($DEST)"
	else
	    IS_BSD=1
	    LOG " ($DEST) bsd install"
	fi
	rm -f /tmp/a$$ /tmp/b$$
    else
	DEST=`acLookFor ginstall`
	if [ "$DEST" ]; then
	    LOG " ($DEST)"
	else
	    DEST="false"
	    LOG " (not found)"
	fi
    fi

    if [ "$IS_BSD" ]; then
	PROG_INSTALL="$DEST -c"
    else
	PROG_INSTALL="$DEST"
    fi

    AC_SUB 'INSTALL' "$PROG_INSTALL"
    AC_SUB 'INSTALL_PROGRAM' "$PROG_INSTALL -s -m 755"
    AC_SUB 'INSTALL_DATA' "$PROG_INSTALL -m 444"

    # finally build a little directory installer
    # if mkdir -p works, use that, otherwise use install -d,
    # otherwise build a script to do it by hand.
    # in every case, test to see if the directory exists before
    # making it.

    if mkdir -p $$a/b; then
	# I like this method best.
	__mkdir="mkdir -p"
	rmdir $$a/b
	rmdir $$a
    elif $PROG_INSTALL -d $$a/b; then
	__mkdir="$PROG_INSTALL -d"
	rmdir $$a/b
	rmdir $$a
    fi

    __config_files="$__config_files config.md"
    AC_SUB 'INSTALL_DIR' "$__cwd/config.md"
    echo "#! /bin/sh"                                   > ""$__cwd"/config.md"
    echo "# script generated" `date` "by configure.sh" >> ""$__cwd"/config.md"
    echo                                               >> ""$__cwd"/config.md"
    if [ "$__mkdir" ]; then
	echo "test -d \"\$1\" || $__mkdir \"\$1\""     >> ""$__cwd"/config.md"
	echo "exit $?"                                 >> ""$__cwd"/config.md"
    else
	cat - >> ""$__cwd"/config.md" << \EOD
pieces=`IFS=/; for x in $1; do echo $x; done`
dir=
for x in $pieces; do
    dir="$dir$x"
    mkdir $dir || exit 1
    dir="$dir/"
done
exit 0
EOD
    fi
    chmod +x "$__cwd"/config.md
}

#
# acCheckCPP is a local that runs a C preprocessor with a given set of
# compiler options
#
acCheckCPP () {
	cat > ngc$$.c << EOF
#define FOO BAR

FOO
EOF

    good=
    use_cflags=
    if $1 $2 $AC_CFLAGS ngc$$.c > ngc$$.o; then
	good=1
	use_cflags=1
    elif $1 $2 ngc$$.c > ngc$$.o; then
	good=1
    fi

    if [ "$good" ]; then
	if grep -v '#define' ngc$$.o | grep -s BAR >/dev/null; then
	    echo "CPP=[$1], CPP_PIPE=[$2${use_cflags:+ $AC_CFLAGS}]"
	    AC_SUB 'CPP' "$1 $2${use_cflags:+ $AC_CFLAGS}"
	    AC_CPP_FILTER="$1 $2${use_cflags:+ $AC_CFLAGS}"
	    rm ngc$$.c ngc$$.o
	    return 0
	fi
    fi
    rm ngc$$.c ngc$$.o
    return 1
}

#
# AC_PROG_CPP checks for cpp, then checks to see which CPPFLAGS are needed
# to run it as a filter.
#
AC_PROG_CPP () {
    test "$AC_CPP_FILTER" && return

    AC_PROG_CC

    if [ "$AC_CPP_PROG" ]; then
	DEST=$AC_CPP_PROG
    else
	__ac_path="$AC_PATH"
	AC_PATH="/lib:/usr/lib:${__ac_path:-$ac_default_path}"
	DEST=`acLookFor cpp`
	AC_PATH="$__ac_path"
    fi

    unset fail
    LOGN "Looking for cpp"
    
    if acCheckCPP "$AC_CC" -E; then
	TLOG " (using \$CC -E as a cpp pipeline)"
	return 0
    fi
    
    if [ "$DEST" ]; then
	TLOGN " ($DEST)"
	acCheckCPP $DEST "$CPPFLAGS" || \
		 acCheckCPP $DEST -traditional-cpp -E || \
		 acCheckCPP $DEST -E || \
		 acCheckCPP $DEST -traditional-cpp -pipe || \
	         acCheckCPP $DEST -pipe || fail=1

	if [ "$fail" ]; then
	    AC_FAIL " (can't run cpp as a pipeline)"
	else
	    TLOG " ok"
	    return 0
	fi
    fi
    AC_FAIL " (not found)"
}

#
# AC_FAIL spits out an error message, then __fail's 
AC_FAIL() {
    LOG "$*" 
    $__fail 1
}

#
# __ac_config_sed; a C program to do escaping for AC_SUB
__ac_config_sed() {


    test -x config.sed && return

    echo "generating config.sed"

    AC_PROG_CC
    
cat > ngc$$.c << \EOF
#include <stdio.h>

int
main(argc, argv)
int argc;
char **argv;
{
    char *p;
    
    if (argc != 3)
	return 1;

    printf("s;@%s@;", argv[1]);

    for (p=argv[2]; *p; ++p) {
	if ( *p == ';' )
	    putchar('\\');
	putchar(*p);
    }

    puts(";g");
    return 0;
}
EOF

    if $AC_CC -o config.sed ngc$$.c; then
	rm -f ngc$$.c
	__config_files="$__config_files config.sed"
    else
	rm -f ngc$$.c
	AC_FAIL "Cannot generate config.sed helper program"
    fi
}

#
# AC_SUB writes a substitution into config.sub
AC_SUB() {
    
    _target="$1"
    shift

    echo "target=$_target, rest=$*"
    
    __ac_config_sed
    ./config.sed "$_target" "$*" >> "$__cwd"/config.sub
}
#
# AC_TEXT writes arbitrary text into config.h
AC_TEXT() {
    echo "$@" >> "$__cwd"/config.h
}

#
# AC_MAK writes a define into config.mak
AC_MAK() {
    echo "HAVE_$1 = 1" >> "$__cwd"/config.mak
}

#
# AC_DEFINE adds a #define to config.h
AC_DEFINE() {
    echo "#define $1 ${2:-1}" >> "$__cwd"/config.h
}

#
# AC_INCLUDE adds a #include to config.h
AC_INCLUDE() {
    echo "#include \"$1\"" >> "$__cwd"/config.h
}

#
# AC_CONFIG adds a configuration setting to all the config files
AC_CONFIG() {
    AC_DEFINE "PATH_$1" \""$2"\"
    AC_MAK "$1"
    AC_SUB "$1" "$2"
}

#
# AC_QUIET does something quietly
AC_QUIET() {
    eval $* 5>/dev/null
}
    

AC_TR=`acLookFor tr`
if [ "$AC_TR" ]; then
    # try posix-style tr
    ABC=`echo abc | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ`
    if [ "$ABC" = "ABC" ]; then
	AC_UPPERCASE="$AC_TR abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	AC_UPPER_PAT="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    else
	ABC=`echo abc | tr a-z A-Z`
	if [ "$ABC" = "ABC" ]; then
	    AC_UPPERCASE="$AC_TR a-z A-Z"
	    AC_UPPER_PAT="A-Z"
	else
	    ABC=`echo abc | tr '[a-z]' '[A-Z]'`
	    if [ "$ABC" = "ABC" ]; then
		AC_UPPERCASE="$AC_TR '[a-z]' '[A-Z]'"
		AC_UPPER_PAT="'[A-Z]'"
	    else
		AC_FAIL "$AC_TR cannot translate lowercase to uppercase"
		return 0
	    fi
	fi
    fi
else
    AC_FAIL "configure requires a functional version of tr"
fi

while [ $# -gt 0 ]; do
    unset matched

    case X"$1" in
    X--src|X--srcdir)
	AC_SRCDIR=`__ac_dir "$2"`
	_set_srcdir=1
	shift 2;;

    X--src=*|X--srcdir=*)
	__d=`echo "$1" | sed -e 's/^[^=]*=//'`
	AC_SRCDIR=`__ac_dir "$__d"`
	_set_srcdir=1
	shift 1 ;;

    X--prefix)
	AC_PREFIX=`__ac_dir "$2"`
	_set_prefix=1
	shift 2;;

    X--prefix=*)
	__d=`echo "$1"| sed -e 's/^[^=]*=//'`
	AC_PREFIX=`__ac_dir "$__d"`
	_set_prefix=1
	shift 1;;

    X--confdir)
	AC_CONFDIR=`__ac_dir "$2"`
	_set_confdir=1
	shift 2;;

    X--confdir=*)
	__d=`echo "$1" | sed -e 's/^[^=]*=//'`
	AC_CONFDIR=`__ac_dir "$__d"`
	_set_confdir=1
	shift 1;;

    X--libexec|X--libexecdir)
	AC_LIBEXEC=`__ac_dir "$2"`
	_set_libexec=1
	shift 2;;

    X--libexec=*|X--libexecdir=*)
	__d=`echo "$1" | sed -e 's/^[^=]*=//'`
	AC_LIBEXEC=`__ac_dir "$__d"`
	_set_libexec=1
	shift 1;;

    X--lib|X--libdir)
	AC_LIBDIR=`__ac_dir "$2"`
	_set_libdir=1
	shift 2;;

    X--lib=*|X--libdir=*)
	__d=`echo "$1" | sed -e 's/^[^=]*=//'`
	AC_LIBDIR=`__ac_dir "$__d"`
	_set_libdir=1
	shift 1;;

    X--exec|X--execdir)
	AC_EXECDIR=`__ac_dir "$2"`
	_set_execdir=1
	shift 2;;

    X--exec=*|X--execdir=*)
	__d=`echo "$1" | sed -e 's/^[^=]*=//'`
	AC_EXECDIR=`__ac_dir "$__d"`
	_set_execdir=1
	shift 1;;

    X--sbin|X--sbindir)
	AC_SBINDIR=`__ac_dir "$2"`
	_set_sbindir=1
	shift 2;;

    X--sbin=*|X--sbindir=*)
	__d=`echo "$1" | sed -e 's/^[^=]*=//'`
	AC_SBINDIR=`__ac_dir "$__d"`
	_set_sbindir=1
	shift 1;;

    X--man|X--mandir)
	AC_MANDIR=`__ac_dir "$2"`
	_set_mandir=1
	shift 2;;

    X--man=*|X--mandir=*)
	__d=`echo "$1" | sed -e 's/^[^=]*=//'`
	AC_MANDIR=`__ac_dir "$__d"`
	_set_mandir=1
	shift 1;;

    X--use-*=*)
	_var=`echo "$1"| sed -n 's/^--use-\([A-Za-z][-A-Za-z0-9_]*\)=.*$/\1/p'`
	if [ "$_var" ]; then
	    _val=`echo "$1" | sed -e 's/^--use-[^=]*=\(.*\)$/\1/'`
	    _v=`echo $_var | $AC_UPPERCASE | tr '-' '_'`
	    case X"$_val" in
	    X[Yy][Ee][Ss]|X[Tt][Rr][Uu][Ee]) eval USE_${_v}=T ;;
	    X[Nn][Oo]|X[Ff][Aa][Ll][Ss][Ee]) eval unset USE_${_v} ;;
	    *) echo "Bad value for --use-$_var ; must be yes or no"
	       exit 1 ;;
	    esac
	else
	    echo "Bad option $1.   Use --help to show options" 1>&2
	    exit 1
	fi
	shift 1 ;;

    X--use-*)
        _var=`echo "$1"|sed -n 's/^--use-\([A-Za-z][-A-Za-z0-9_]*\)$/\1/p'`
	_v=`echo $_var | $AC_UPPERCASE | tr '-' '_'`
	eval USE_${_v}=T
	shift 1;;

    X--with-*=*)
	_var=`echo "$1"| sed -n 's/^--with-\([A-Za-z][-A-Za-z0-9_]*\)=.*$/\1/p'`
	if [ "$_var" ]; then
	    _val=`echo "$1" | sed -e 's/^--with-[^=]*=\(.*\)$/\1/'`
	    _v=`echo $_var | $AC_UPPERCASE | tr '-' '_'`
	    eval WITH_${_v}=\"$_val\"
	else
	    echo "Bad option $1.   Use --help to show options" 1>&2
	    exit 1
	fi
	shift 1 ;;

    X--with-*)
	_var=`echo "$1" | sed -n 's/^--with-\([A-Za-z][A-Za-z0-9_-]*\)$/\1/p'`
	if [ "$_var" ]; then
	    _v=`echo $_var | $AC_UPPERCASE | tr '-' '_'`
	    eval WITH_${_v}=1
	else
	    echo "Bad option $1.   Use --help to show options" 1>&2
	    exit 1
	fi
	shift 1 ;;

    X--help)
	echo "$ac_standard"
	test "$ac_help" && echo "$ac_help"
	exit 0;;

    *)	if [ "$LOCAL_AC_OPTIONS" ]; then
	    eval "$LOCAL_AC_OPTIONS"
	else
	    ac_error=T
	fi
	if [ "$ac_error" ]; then
	    echo "Bad option $1.   Use --help to show options" 1>&2
	    exit 1
	fi ;;
    esac
done

