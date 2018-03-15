/* This file is part of ESDM.
 *
 * This program is is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief Debug adds functionality for logging and inspection of ESDM datatypes
 *        during development.
 *
 */

#include <stdio.h>
#include <stdarg.h>

#include <glib.h>

#include <esdm-internal.h>


int read_file(char *filepath, char **buf)
{


	if (*buf != NULL)
	{
		ESDM_ERROR("read_file(): Potential memory leak. Overwriting existing pointer with value != NULL.");
		exit(1);
	}

	FILE *fp = fopen(filepath, "rb");

	if( fp == NULL ) {
		printf("Could not open or find: %s\n", filepath);
	}


	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);  //same as rewind(f);

	char *string = malloc(fsize + 1);
	fread(string, fsize, 1, fp);
	fclose(fp);

	string[fsize] = 0;


	*buf = string;

	printf("read_file(): %s\n", string);
}

