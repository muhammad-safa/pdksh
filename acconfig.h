/*
 * This file, acconfig.h, which is a part of pdksh (the public domain ksh),
 * is placed in the public domain.  It comes with no licence, warranty
 * or guarantee of any kind (i.e., at your own risk).
 */

#ifndef CONFIG_H
#define CONFIG_H

@TOP@

/* Define if your kernal doesn't handle scripts starting with #! */
#undef SHARPBANG

/* Define as the return value of signal handlers (0 or ).  */
#undef RETSIGVAL

/* Define if signals don't interrupt read() */
#undef SIGNALS_DONT_INTERRUPT

/* Define to nothing if compiler doesn't like the volatile keyword */
#undef volatile

/* Define if C compiler groks __attribute__((...)) (const, noreturn, format) */
#undef HAVE_GCC_FUNC_ATTR

/* Define to 32-bit signed integer type if <sys/types.h> doesn't define */
#undef clock_t

/* Define to the type of struct rlimit fields if the rlim_t type is missing */
#undef rlim_t

/* Define if time() is declared in <time.h> */
#undef TIME_DECLARED

/* Define to `unsigned' if <signal.h> doesn't define */
#undef sigset_t

/* Define if sys_errlist[] and sys_nerr are in the C library */
#undef HAVE_SYS_ERRLIST

/* Define if sys_errlist[] and sys_nerr are defined in <errno.h> */
#undef SYS_ERRLIST_DECLARED

/* Define if sys_siglist[] is in the C library */
#undef HAVE_SYS_SIGLIST

/* Define if you have a sane <termios.h> header file */
#undef HAVE_TERMIOS_H

/* Define if you can include <sys/ioctl.h> with <termios.h> */
#undef SYS_IOCTL_WITH_TERMIOS

/* Define if you can include <sys/ioctl.h> with <termio.h> */
#undef SYS_IOCTL_WITH_TERMIO

/* Define if you have a sane <termio.h> header file */
#undef HAVE_TERMIO_H

/* Define if your OS maps references to /dev/fd/n to file descriptor n */
#undef HAVE_DEV_FD

/* Define if your C library's getwd/getcwd function dumps core in unreadable
 * directories.  */
#undef HPUX_GETWD_BUG

/*   ------- ------ ----- ---- --- -- - - -- --- ---- ----- ------ -------   */
/*   Defines from here on down are enable/disable options to configure */

/* Default PATH (see comments in configure.in for more details) */
#undef DEFAULT_PATH

/* Include ksh features? (see comments in configure.in for more details) */
#undef KSH

/* Include emacs editing? (see comments in configure.in for more details) */
#undef EMACS

/* Include vi editing? (see comments in configure.in for more details) */
#undef VI

/* Include brace-expansion? (see comments in configure.in for more details) */
#undef BRACE_EXPAND

/* Include any history? (see comments in configure.in for more details) */
#undef HISTORY

/* Include complex history? (see comments in configure.in for more details) */
#undef COMPLEX_HISTORY

/* Strict POSIX behaviour? (see comments in configure.in for more details) */
#undef POSIXLY_CORRECT

/* Specify default $ENV? (see comments in configure.in for more details) */
#undef DEFAULT_ENV

/* Include shl(1) support? (see comments in configure.in for more details) */
#undef SWTCH

/* Include game-of-life? (see comments in configure.in for more details) */
#undef SILLY

@BOTTOM@

/* Need to use a separate file to keep the configure script from commenting
 * out the undefs....
 */
#include "conf-end.h"

#endif /* CONFIG_H */
