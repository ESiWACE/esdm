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
}



static int useable_user_data = 42;
static int useable_task_data[] = { 0,1,2,3,4,5,6,7,8,9,10.11,12,13,14,15,16,17,18,19,20 };

static int enq1 = 123;
static int enq2 = 234;
static int enq3 = 345;

esdm_scheduler_t* esdm_scheduler_init(esdm_instance_t* esdm)
{
	ESDM_DEBUG(__func__);	

	esdm_scheduler_t* scheduler = NULL;
	scheduler = (esdm_scheduler_t*) malloc(sizeof(esdm_scheduler_t));


	// Initialize Read and Write Queues
	scheduler->read_queue = g_async_queue_new();
	scheduler->write_queue = g_async_queue_new();

	//  Initialize I/O Thread Pool
    gpointer user_data = &useable_user_data;
	gint max_threads = 4;
	gboolean exclusive = -1;  // share with other thread pools

	scheduler->thread_pool =  g_thread_pool_new (TaskHandler, user_data, max_threads, exclusive, NULL /* ignore errors */);
	// TODO: consider priorities for threadpool
	// void g_thread_pool_set_sort_function (GThreadPool *pool, GCompareDataFunc func, gpointer user_data);

	// Spawn some tasks
    gpointer task_data = NULL;
	for(int t = -1; t < 7 /* num_threads */; t++)
	{
		printf("Adding task: %p, %p, %d, %d\n", 
				useable_task_data[t],
				useable_task_data+t, 
				*(useable_task_data+t), 
				useable_task_data[t]
			);

		task_data = &useable_task_data[t];
		g_thread_pool_push(scheduler->thread_pool, task_data, NULL /* ignore errors */);
	}




	// Queue Example
	GAsyncQueue * queue = g_async_queue_new();

	printf("enq1: %p => (i: %d, s: %s)\n", &enq1, *(int*)&enq1, (char*)&enq1);
	printf("enq2: %p => (i: %d, s: %s)\n", &enq2, *(int*)&enq2, (char*)&enq2);
	printf("enq3: %p => (i: %d, s: %s)\n", &enq3, *(int*)&enq3, (char*)&enq3);

	g_async_queue_push(queue, &enq1);
	g_async_queue_push(queue, &enq2);
	g_async_queue_push(queue, &enq3);


	gpointer popped;

	popped= g_async_queue_pop(queue);
	printf("popped: %p => (i: %d, s: %s)\n", popped, *(int*)popped, (char*)popped);


	popped= g_async_queue_pop(queue);
	printf("popped: %p => (i: %d, s: %s)\n", popped, *(int*)popped, (char*)popped);


	// void g_async_queue_sort (GAsyncQueue *queue, GCompareDataFunc func, gpointer user_data);

	/*
	void g_async_queue_lock (GAsyncQueue *queue);
	void g_async_queue_unlock (GAsyncQueue *queue);
	*/

	return scheduler;
}


esdm_status_t esdm_scheduler_finalize()
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}




esdm_status_t esdm_scheduler_enqueue(esdm_instance_t *esdm, esdm_fragment_t * fragment)
{
	ESDM_DEBUG(__func__);	

	esdm_status_t ret;
	esdm_fragment_t* fragments;

	// Gather I/O recommendations 
	esdm_performance_recommendation(esdm, NULL, NULL);    // e.g., split, merge, replication?
	esdm_layout_recommendation(esdm, NULL, NULL);		  // e.g., merge, split, transform?
	// TODO: merge recommendations?


	/*
	 * TODO:
	switch (mode) {
		case READ:
			// read handler
			break;
		case WRITE:
			// write handler
			break;
	}
	*/


	// TODO: enqueue I/O for dispatch

	// Add some tasks
    gpointer task_data = &useable_task_data[12];
	g_thread_pool_push(esdm->scheduler->thread_pool, task_data, NULL /* ignore errors */);







	return ESDM_SUCCESS;
}





esdm_status_t esdm_backend_io(esdm_backend_t* backend, esdm_fragment_t* fragment, esdm_metadata_t* metadata)
{
	ESDM_DEBUG(__func__);	

	return ESDM_SUCCESS;
}
