/*
 * Routines which may be missing on some machines
 */

#include "sh.h"
#include "ksh_stat.h"
#include "ksh_dir.h"


#ifdef TIMES_BROKEN
# include "ksh_time.h"
# include "ksh_times.h"
# ifdef HAVE_GETRUSAGE
#  include <sys/resource.h>
# else /* HAVE_GETRUSAGE */
#  include <sys/timeb.h>
# endif /* HAVE_GETRUSAGE */

clock_t
ksh_times(tms)
	struct tms *tms;
{
	static clock_t base_sec;
	clock_t rv;

# ifdef HAVE_GETRUSAGE
	{
		struct timeval tv;
		struct rusage ru;

		getrusage(RUSAGE_SELF, &ru);
		tms->tms_utime = ru.ru_utime.tv_sec * CLK_TCK
			+ ru.ru_utime.tv_usec * CLK_TCK / 1000000;
		tms->tms_stime = ru.ru_stime.tv_sec * CLK_TCK
			+ ru.ru_stime.tv_usec * CLK_TCK / 1000000;

		getrusage(RUSAGE_CHILDREN, &ru);
		tms->tms_cutime = ru.ru_utime.tv_sec * CLK_TCK
			+ ru.ru_utime.tv_usec * CLK_TCK / 1000000;
		tms->tms_cstime = ru.ru_stime.tv_sec * CLK_TCK
			+ ru.ru_stime.tv_usec * CLK_TCK / 1000000;

		gettimeofday(&tv, (struct timezone *) 0);
		if (base_sec == 0)
			base_sec = tv.tv_sec;
		rv = (tv.tv_sec - base_sec) * CLK_TCK;
		rv += tv.tv_usec * CLK_TCK / 1000000;
	}
# else /* HAVE_GETRUSAGE */
	/* Assume times() available, but always returns 0
	 * (also assumes ftime() available)
	 */
	{
		struct timeb tb;

		if (times(tms) == (clock_t) -1)
			return (clock_t) -1;
		ftime(&tb);
		if (base_sec == 0)
			base_sec = tb.time;
		rv = (tb.time - base_sec) * CLK_TCK;
		rv += tb.millitm * CLK_TCK / 1000;
	}
# endif /* HAVE_GETRUSAGE */
	return rv;
}
#endif /* TIMES_BROKEN */

#ifdef OPENDIR_DOES_NONDIR
/* Prevent opendir() from attempting to open non-directories.  Such
 * behavior can cause problems if it attempts to open special devices...
 */
DIR *
ksh_opendir(d)
	const char *d;
{
	struct stat statb;

	if (stat(d, &statb) != 0)
		return (DIR *) 0;
	if (!S_ISDIR(statb.st_mode)) {
		errno = ENOTDIR;
		return (DIR *) 0;
	}
	return opendir(d);
}
#endif /* OPENDIR_DOES_NONDIR */
