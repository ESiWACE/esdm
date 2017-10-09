/*
 * =====================================================================================
 *
 *       Filename:  timer.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/15/2016 02:20:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */


#ifndef  timer_INC
#define  timer_INC

#include <time.h>

typedef struct timespec timespec_t;

void start_timer(timespec_t *t1);
void stop_timer(timespec_t *t1);

timespec_t time_diff (const timespec_t end, const timespec_t start);
double time_to_double (const timespec_t t);

#endif   /* ----- #ifndef timer_INC  ----- */
