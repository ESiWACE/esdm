/*
 * =====================================================================================
 *
 *       Filename:  base.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/25/2017 03:44:39 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */


#ifndef  base_INC
#define  base_INC

#include "debug.h"
#include "h5_sqlite_plugin.h"

#include <stdlib.h>
#include <string.h>


char* create_path(SQO_t* sqo);
void destroy_path(char* path);

#endif   /* ----- #ifndef base_INC  ----- */
