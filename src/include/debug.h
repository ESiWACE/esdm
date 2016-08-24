#include <libgen.h>
#include <stdio.h>

#ifdef DEBUG
#define DEBUG_MESSAGE(...) \
{ \
	fprintf(stdout, "DEBUG %*s:%-*d ", 15, basename(__FILE__), 5, __LINE__); \
	fprintf(stdout, __VA_ARGS__); \
}
#else
#define DEBUG_MESSAGE(...)
#endif

#ifndef FATAL_ERR
#define FATAL_ERR(...) \
	fprintf(stderr, "Error in %s:%d:%s: ", __FILE__, __LINE__, __PRETTY_FUNCTION__); \
	fprintf(stderr, __VA_ARGS__); \
	exit(-1)
#endif

