/*
 * =====================================================================================
 *
 *       Filename:  types.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/15/2016 08:44:58 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Eugen Betke (betke@dkrz.de), Julian Kunkel (juliankunkel@googlemail.com)
 *   Organization:  DKRZ (Deutsches Klimarechenzentrum)
 *
 * =====================================================================================
 */


#ifndef  types_INC
#define  types_INC

#include <stdlib.h>
#include "timer.h"
#include "constants.h"

typedef struct duration_t {
	double open;
	double io;
	double close;
} duration_t; 

typedef struct procs_t {
	size_t nn;
	size_t ppn;
} procs_t;

typedef struct point3d_t {
	size_t x;
	size_t y;
	size_t z;
} point3d_t;

typedef struct time_layout_t {
	size_t slice;
	size_t range;
} time_layout_t;

typedef struct measurement_t {
	size_t time_offset;
	double duration;
} measurement_t;




#endif   /* ----- #ifndef types_INC  ----- */
