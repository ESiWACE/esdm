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
 * @brief The site configuration describes the data center or subcomponents.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <glib.h>

#include <esdm.h>
#include <esdm-internal.h>


json_t* esdm_config_gather(int argc, char const *argv[]);


/**
 * Initializes the site configuration module.
 *
 * @param	[in] esdm   Pointer to esdm instance.
 * @return	Pointer to newly created configuration instance.
 */
esdm_config_t* esdm_config_init(esdm_instance_t *esdm)
{
	ESDM_DEBUG(__func__);	

	esdm_config_t* config = NULL;
	config = (esdm_config_t*) malloc(sizeof(esdm_config_t));

	config->json = esdm_config_gather(0, NULL);

	return config;
}



esdm_status_t esdm_config_finalize(esdm_instance_t *esdm)
{
	ESDM_DEBUG(__func__);	

	json_decref(esdm->config->json);
	free(esdm->config);

	return ESDM_SUCCESS;
}


/**
 * Gathers ESDM configuration settings from multiple locations:
 *
 *
 * TODO:
 *	/etc/esdm/esdm.conf
 *	~/.config/esdm/esdm.conf
 *	~/.esdm.conf
 *  ./esdm.conf
 *  environment variable
 *  arguments
 *
 */
json_t* esdm_config_gather(int argc, char const* argv[]) 
{
	ESDM_DEBUG(__func__);	

	char* config_json = NULL;


	read_file("_esdm.conf", &config_json);


	// parse text into JSON structure
	json_t *root = load_json(config_json);

	return root;
}


esdm_config_backend_t* esdm_config_get_metadata_coordinator(esdm_instance_t* esdm)
{
	ESDM_DEBUG(__func__);	

	json_t *root = (json_t*) esdm->config->json;

	json_t *elem = json_path_get(root, "$.esdm.metadata.type");
	printf("json_path_get (metadata backend) => %p -> %s\n",  elem, json_string_value(elem));
	print_json(elem);
	printf("\n\n");



	esdm_config_backend_t *config_backend = (esdm_config_backend_t*) malloc(sizeof(esdm_config_backend_t));
	config_backend->type = json_string_value(elem);
	config_backend->esdm = root;
	config_backend->backend = json_path_get(root, "$.esdm.metadata");


	return config_backend;
}


/**
 *	Fetches backends
 *
 *
 */
esdm_config_backends_t* esdm_config_get_backends(esdm_instance_t* esdm)
{
	ESDM_DEBUG(__func__);	

	json_t *root = (json_t*) esdm->config->json;

	// fetch configured backends
	json_t *element = json_path_get(root, "$.esdm.backends");

	esdm_config_backends_t* config_backends = (esdm_config_backends_t*) malloc(sizeof(esdm_config_backends_t));

	if (element)
	{
		if (json_typeof(element) == JSON_ARRAY)
		{
			// Element is array, therefor may contain valid backend configurations
			size_t size = json_array_size(element);

			esdm_config_backend_t* backends;
			backends = (esdm_config_backend_t*) malloc(sizeof(esdm_config_backend_t)*size);

			//printf("JSON Array of %ld element%s:\n", size, json_plural(size));

			for (size_t i = 0; i < size; i++) {
				//print_json_aux(json_array_get(element, i), 0);

				json_t *backend = json_array_get(element, i);
				json_t *elem = NULL;
				
				elem = json_object_get(backend, "type");
				backends[i].type = json_string_value(elem);

				elem = json_object_get(backend, "name");
				backends[i].name = json_string_value(elem);

				elem = json_object_get(backend, "target");
				backends[i].target = json_string_value(elem);


				backends[i].esdm = root;
				backends[i].backend = backend;
			}	

			config_backends->count = size;
			config_backends->backends = backends;

		}
	} else {
		ESDM_ERROR("Invalid configuration! /esdm/backends is not an array.");
	}


	return config_backends;
}



















































