/*
 * =====================================================================================
 *
 *       Filename:  constants.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/17/2016 11:41:22 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Eugen Betke (betke@dkrz.de), Julian Kunkel (juliankunkel@googlemail.com)
 *   Organization:  DKRZ (Deutsches Klimarechenzentrum)
 *
 * =====================================================================================
 */

#ifndef  constants_INC
#define  constants_INC

#define NDIMS 4

#define DT 0
#define DX 1
#define DY 2
#define DZ 3

typedef enum io_mode_t {IO_MODE_WRITE, IO_MODE_READ} io_mode_t;

#endif   /* ----- #ifndef constants_INC  ----- */
