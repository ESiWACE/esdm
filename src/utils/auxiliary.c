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

#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <glib.h>

#include <esdm.h>


int read_file(char *filepath, char **buf)
{
	if (*buf != NULL)
	{
		printf("read_file(): Potential memory leak. Overwriting existing pointer with value != NULL.");
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




void mkdir_recursive(const char *path) {
        char tmp[PATH_MAX];
        char *p = NULL;
        size_t len;

		// copy provided path, as we modify it
        snprintf(tmp, sizeof(tmp),"%s",path);

		// check if last char of string is a /
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
			// if it is, set to 0
			tmp[len - 1] = 0;

		// traverse string from start to end
		for(p = tmp + 1; *p; p++)
		{
			if(*p == '/') {
				// if current char is a /
				// temporaly set character at address p to 0
				// create dir from start of string 
				// reset char at address back to /
				*p = 0;
				mkdir(tmp, S_IRWXU);
				*p = '/';
			}
			// continue with next position in string
		}
		mkdir(tmp, S_IRWXU);
}




void print_stat(struct stat sb)
{
	printf("\n");
	printf("File type:                ");
	switch (sb.st_mode & S_IFMT) {
		case S_IFBLK:  printf("block device\n");      break;
		case S_IFCHR:  printf("character device\n");  break;
		case S_IFDIR:  printf("directory\n");         break;
		case S_IFIFO:  printf("FIFO/pipe\n");         break;
		case S_IFLNK:  printf("symlink\n");           break;
		case S_IFREG:  printf("regular file\n");      break;
		case S_IFSOCK: printf("socket\n");            break;
		default:       printf("unknown?\n");          break;
	}
	printf("I-node number:            %ld\n", (long) sb.st_ino);
	printf("Mode:                     %lo (octal)\n", (unsigned long) sb.st_mode);
	printf("Link count:               %ld\n", (long) sb.st_nlink);
	printf("Ownership:                UID=%ld   GID=%ld\n", (long) sb.st_uid, (long) sb.st_gid);
	printf("Preferred I/O block size: %ld bytes\n", (long) sb.st_blksize);
	printf("File size:                %lld bytes\n", (long long) sb.st_size);
	printf("Blocks allocated:         %lld\n", (long long) sb.st_blocks);
	printf("Last status change:       %s", ctime(&sb.st_ctime));
	printf("Last file access:         %s", ctime(&sb.st_atime));
	printf("Last file modification:   %s", ctime(&sb.st_mtime));
	printf("\n");
}




