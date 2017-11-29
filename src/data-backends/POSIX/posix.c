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



#include <stdio.h>
#include <stdarg.h>

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


int posix_finalize() {}

int posix_backend_performance_estimate() 
{
	DEBUG(0, "Calculating performance estimate.");

	return 0;
}


int posix_create() 
{
	DEBUG(0, "Create");
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




static esdm_backend_t backend = {
	.name = "POSIX",
	.type = ESDM_TYPE_DATA,
	.callbacks = {
		NULL, // finalize
		posix_backend_performance_estimate, // performance_estimate

		NULL, // create
		NULL, // open
		NULL, // write
		NULL, // read
		NULL, // close

		NULL, // allocate
		NULL, // update
		NULL, // lookup
	}
};





esdm_backend_t* posix_backend_init() {
	
	DEBUG(0, "Initializing POSIX backend.");
	
	return &backend;

}
