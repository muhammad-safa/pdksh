/* Wrapper around the ugly dir includes/ifdefs */
/* $Id$ */

#include <dirent.h>
#define NLENGTH(dirent)	(strlen(dirent->d_name))
