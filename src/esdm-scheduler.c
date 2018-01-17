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
 * @brief The scheduler receives application requests and schedules subsequent
 *        I/O requests as a are necessary for metadata lookups and data
 *        reconstructions.
 *
 */



#include <esdm.h>
#include <esdm-internal.h>

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>


void TaskHandler (gpointer data, gpointer user_data)
{
   printf("Hello from TaskHandler gthread! data = %p => %d,  user_data = %p => %d!\n", data, *(int*)(data), user_data, *(int*)(user_data));
   pthread_exit(NULL);
}



static int useable_user_data = 42;
static int useable_task_data[] = { 0,1,2,3,4,5,6,7,8,9,10.11,12,13,14,15,16,17,18,19,20 };

esdm_scheduler_t* esdm_scheduler_init(esdm_instance_t* esdm)
{
	ESDM_DEBUG(__func__);	

	esdm_scheduler_t* scheduler = NULL;
	scheduler = (esdm_scheduler_t*) malloc(sizeof(esdm_scheduler_t));

	// init thread pool
    gpointer user_data = &useable_user_data;
	gint max_threads = 5;
	gboolean exclusive = 0;
	GThreadPool* pool =  g_thread_pool_new (TaskHandler, user_data, max_threads, exclusive, NULL /* ignore errors */);

	// spawn a few tasks
    gpointer task_data = NULL;
	for(int t = 0; t < 7 /* num_threads */; t++)
	{
		printf("Adding task: %p, %p, %d, %d\n", useable_task_data[t], useable_task_data+t, *(useable_task_data+t), useable_task_data[t]);
		task_data = &useable_task_data[t];
		g_thread_pool_push(pool, task_data, NULL /* ignore errors */);
	}



	return scheduler;
}


esdm_status_t esdm_scheduler_finalize()
{

	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}




esdm_status_t esdm_scheduler_submit(esdm_pending_fragment_t * io)
{

	ESDM_DEBUG(__func__);	

	esdm_init();


	esdm_pending_fragments_t* pending_fragments;
	esdm_fragment_t* fragments;


	esdm_perf_model_split_io(pending_fragments, fragments);

	// no threads here
	esdm_status_t ret;
	
	//esdm_metadata_t * metadata = esdm_metadata_t_alloc();
	esdm_metadata_t * metadata;


	/*
	for(int i=0 ; i < fragments; i++){
		ret = esdm_backend_io(b_ios[i]->backend, b_ios[i]->io, metadata);
	}
	esdm_metata_backend_update(metadata);
	free(metadata);

	return ESDM_SUCCESS;
	*/
}





esdm_status_t esdm_backend_io(esdm_backend_t* backend, esdm_fragment_t* fragment, esdm_metadata_t* metadata)
{
	ESDM_DEBUG(__func__);	


	return ESDM_SUCCESS;
}
