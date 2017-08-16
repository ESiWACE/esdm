/*
 * =====================================================================================
 *
 *       Filename:  base.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/25/2017 03:42:29 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "base.h"

char* create_path(SQO_t* sqo) {
	static const char* sep = "/";
	char* res = NULL;

	if (NULL != sqo->location && NULL != sqo->name) {
		res = (char*) malloc(strlen(sqo->location) + strlen(sep) + strlen(sqo->name) + 1);
		strcpy(res, sqo->location);
		strcat(res, sep);
		strcat(res, sqo->name);
	} 
	else {
		ERRORMSG("");
	}
	return res;
}

void destroy_path(char* path) {
	free(path);
	path = NULL;
}

