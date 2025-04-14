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

/* XXX tnn note: The below copyright applies to strlcpy and strlcat */
/* taken from libnbcompat */

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND TODD C. MILLER DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL TODD C. MILLER BE LIABLE
 * FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef HAVE_STRLCPY
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
        char *d = dst;
        const char *s = src;
        size_t n = siz;

        /* Copy as many bytes as will fit */
        if (n != 0 && --n != 0) {
                do {
                        if ((*d++ = *s++) == 0)
                                break;
                } while (--n != 0);
        }

        /* Not enough room in dst, add NUL and traverse rest of src */
        if (n == 0) {
                if (siz != 0)
                        *d = '\0';              /* NUL-terminate dst */
                while (*s++)
                        ;
        }

        return(s - src - 1);    /* count does not include NUL */
}
#endif

#ifndef HAVE_STRLCAT
size_t
strlcat(char *dst, const char *src, size_t siz) {
        char *d = dst;
        const char *s = src;
        size_t n = siz;
        size_t dlen;

        /* Find the end of dst and adjust bytes left but don't go past end */
        while (n-- != 0 && *d != '\0')
                d++;
        dlen = d - dst;
        n = siz - dlen;

        if (n == 0)
                return(dlen + strlen(s));
        while (*s != '\0') {
                if (n != 1) {
                        *d++ = *s;
                        n--;
                }
                s++;
        }
        *d = '\0';

        return(dlen + (s - src));       /* count does not include NUL */
}
#endif
