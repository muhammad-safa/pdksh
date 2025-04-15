/* A simple program to print information about process groups */

#include "config.h"
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int	usage();

char	*progname =	"check-pgrp";

int main(int argc, char **argv)
{
    extern int	optind;
    extern char	*optarg;

    int		opt;
    int		ttyfd = 0;
    int		my_pgrp, my_pid, my_ppid, tty_pgrp;
    char	*tty_name;
    char	*e;

    if (argc > 0 && argv[0] && *argv[0])
	progname = argv[0];

    while ((opt = getopt(argc, argv, "u:")) != EOF) {
	switch (opt) {
	case 'u':
	    ttyfd = atoi(optarg);
	    break;

	default:
	    usage(1);
	}
    }

    if (optind != argc) {
	    fprintf(stderr, "%s: too many arguments\n", progname);
	    usage(0);
    }

    my_pid = getpid();
    my_ppid = getppid();
    my_pgrp = getpgrp();
    tty_pgrp = tcgetpgrp(ttyfd);
    tty_name = ttyname(ttyfd);

    printf("in %s pgrp\ntty %s (%s)\npid %d, ppid %d, pgrp %d, tty_pgrp %d\n",
	my_pgrp == my_pid ? "my own"
	    : (my_pgrp == my_ppid ? "parent's" : "unknown"),
	tty_name ? tty_name : "(none)",
	tty_pgrp == my_pgrp ? "mine"
	    : (tty_pgrp == my_ppid ? "parent's" : "unknown"),
	my_pid, my_ppid, my_pgrp, tty_pgrp);

    return 0;
}

int
usage(verbose)
    int verbose;
{
    fprintf(stderr, "Usage: %s [-?] [-u fd]\n", progname);
    if (verbose)
	fprintf(stderr, "\
    -u fd	use fd as tty fd (default is 0)\n\
");

    exit(1);
    return 0;
}
