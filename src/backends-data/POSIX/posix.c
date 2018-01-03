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
 * @brief A data backend to provide POSIX compatibility.
 */


#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include<esdm.h>


void log(uint32_t loglevel, const char* format, ...)
{
	uint32_t active_loglevel = 99;

	if ( loglevel <= active_loglevel ) {
		va_list args;
		va_start(args,format);
		vprintf(format,args);
		va_end(args);
	}
}
#define DEBUG(loglevel, msg) log(loglevel, "[POSIX] %-30s %s:%d\n", msg, __FILE__, __LINE__)


///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int mkfs() 
{
	struct stat st = {0};

	if (stat("_esdm-fs", &st) == -1)
	{
		mkdir("_esdm-fs", 0700);
		mkdir("_esdm-fs/containers", 0700);
		mkdir("_esdm-fs/datasets-shared", 0700);
		mkdir("_esdm-fs/fragments-shared", 0700);
		
	}
}





/**
 * Similar to the command line counterpart fsck for ESDM plugins is responsible
 * to check and potentially repair the "filesystem".
 *
 *
 *
 *
 */
int fsck()
{


	return 0;
}






///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int posix_backend_performance_estimate() 
{
	DEBUG(0, "Calculating performance estimate.");

	return 0;
}



int posix_create() 
{
	DEBUG(0, "Create");


	// check if container already exists

	struct stat st = {0};
	if (stat("_esdm-fs", &st) == -1)
	{
		mkdir("_esdm-fs/containers", 0700);
	}


    //#include <unistd.h>
    //ssize_t pread(int fd, void *buf, size_t count, off_t offset);
    //ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

	return 0;
}

int posix_open() 
{
	DEBUG(0, "Open");
	return 0;
}

int posix_write() 
{
	DEBUG(0, "Write");
	return 0;
}

int posix_read() 
{
	DEBUG(0, "Read");
	return 0;
}

int posix_close() 
{
	DEBUG(0, "Close");
	return 0;
}



int posix_allocate() 
{
	DEBUG(0, "Allocate");
	return 0;
}


int posix_update() 
{
	DEBUG(0, "Update");
	return 0;
}


int posix_lookup() 
{
	DEBUG(0, "Lookup");
	return 0;
}



///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend_t backend = {
	.name = "POSIX",
	.type = ESDM_TYPE_DATA,
	.version = "0.0.1",
	.data = NULL,
	.callbacks = {
		NULL, // finalize
		posix_backend_performance_estimate, // performance_estimate

		posix_create, // create
		posix_open, // open
		posix_write, // write
		posix_read, // read
		posix_close, // close

		NULL, // allocate
		NULL, // update
		NULL, // lookup
	},
};

/**
* Initializes the POSIX plugin. In particular this involves:
*
*	* Load configuration of this backend
*	* Load and potenitally calibrate performance model
*
*	* Connect with support services e.g. for technical metadata
*	* Setup directory structures used by this POSIX specific backend
*
*	* Poopulate esdm_backend_t struct and callbacks required for registration
*
* @return pointer to backend struct
*/
esdm_backend_t* posix_backend_init() {
	
	DEBUG(0, "Initializing POSIX backend.");

	
	// todo check posix style persitency structure available?
	mkfs();	






	return &backend;

}

/**
* Initializes the POSIX plugin. In particular this involves:
*
*/
int posix_finalize()
{

	return 0;
}


