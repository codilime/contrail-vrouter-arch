/*
POSIX getopt for Windows

AT&T Public License

Code given out at the 1985 UNIFORUM conference in Dallas.
*/

#ifdef __GNUC__
#include <getopt.h>
#endif
#ifndef __GNUC__

#ifndef _WINGETOPT_H_
#define _WINGETOPT_H_

#define no_argument		0
#define required_argument	1
#define optional_argument	2

struct option
{
	const char *name;
	/* has_arg can't be an enum because some compilers complain about
	type mismatches in all the code that assumes it is an int.  */
	int has_arg;
	int *flag;
	int val;
};

#ifdef __cplusplus
extern "C" {
#endif

	extern int opterr;
	extern int optind;
	extern int optopt;
	extern char *optarg;
	extern int getopt(int argc, char **argv, char *opts);

#ifdef __cplusplus
}
#endif

#endif  /* _GETOPT_H_ */
#endif  /* __GNUC__ */