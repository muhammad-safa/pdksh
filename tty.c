#include <sys/stat.h>

#include "sh.h"
#define EXTERN
#include "tty.h"
#undef EXTERN

int get_tty(int fd, TTY_state *ts)
{
	int ret;
	ret = tcgetattr(fd, ts);
	return ret;
}

int set_tty(int fd, TTY_state *ts, int flags)
{
	int ret = 0;
	ret = tcsetattr(fd, TCSADRAIN, ts);
	return ret;
}


/* Initialize tty_fd.  Used for saving/reseting tty modes upon
 * foreground job completion and for setting up tty process group.
 */
void
tty_init(init_ttystate)
	int init_ttystate;
{
	int	do_close = 1;
	int	tfd;

	if (tty_fd >= 0) {
		close(tty_fd);
		tty_fd = -1;
	}
	tty_devtty = 1;

	/* SCO can't job control on /dev/tty, so don't try... */
#if !defined(__SCO__)
	if ((tfd = open("/dev/tty", O_RDWR, 0)) < 0) {
#ifdef __NeXT
		/* rlogin on NeXT boxes does not set up the controlling tty,
		 * so force it to be done here...
		 */
		{
			extern char *ttyname(int);
			char *s = ttyname(isatty(2) ? 2 : 0);
			int fd;

			if (s && (fd = open(s, O_RDWR, 0)) >= 0) {
				close(fd);
				tfd = open("/dev/tty", O_RDWR, 0);
			}
		}
#endif /* __NeXT */

/* X11R5 xterm on mips doesn't set controlling tty properly - temporary hack */
# if !defined(__mips) || !(defined(_SYSTYPE_BSD43) || defined(__SYSTYPE_BSD43))
		if (tfd < 0) {
			tty_devtty = 0;
			warningf(FALSE,
				"No controlling tty (open /dev/tty: %s)",
				strerror(errno));
		}
# endif /* __mips  */
	}
#else /* !__SCO__ */
	tfd = -1;
#endif /* __SCO__ */

	if (tfd < 0) {
		do_close = 0;
		if (isatty(0))
			tfd = 0;
		else if (isatty(2))
			tfd = 2;
		else {
			warningf(FALSE, "Can't find tty file descriptor");
			return;
		}
	}
	if ((tty_fd = ksh_dupbase(tfd, FDBASE)) < 0) {
		warningf(FALSE, "j_ttyinit: dup of tty fd failed: %s",
			strerror(errno));
	} else if (fcntl(tty_fd, F_SETFD, 1)) {
		warningf(FALSE, "j_ttyinit: can't set close-on-exec flag: %s",
			strerror(errno));
		close(tty_fd);
		tty_fd = -1;
	} else if (init_ttystate)
		get_tty(tty_fd, &tty_state);
	if (do_close)
		close(tfd);
}

void
tty_close()
{
	if (tty_fd >= 0) {
		close(tty_fd);
		tty_fd = -1;
	}
}
