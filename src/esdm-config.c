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

#include <jansson.h>

#include <esdm.h>
#include <esdm-internal.h>


// forward declarations for parsing helpers
static void print_json(json_t *root);
static void print_json_aux(json_t *element, int indent);
static void print_json_indent(int indent);
static const char *json_plural(int count);
static void print_json_object(json_t *element, int indent);
static void print_json_array(json_t *element, int indent);
static void print_json_string(json_t *element, int indent);
static void print_json_integer(json_t *element, int indent);
static void print_json_real(json_t *element, int indent);
static void print_json_true(json_t *element, int indent);
static void print_json_false(json_t *element, int indent);
static void print_json_null(json_t *element, int indent);
static json_t *load_json(const char *text);
static char *read_line(char *line, int max_chars);




json_t* esdm_config_gather(int argc, char const *argv[]);



esdm_config_t* esdm_config_init(esdm_instance_t *esdm)
{
	ESDM_DEBUG(__func__);	

	esdm_config_t* config = NULL;
	config = (esdm_config_t*) malloc(sizeof(esdm_config_t));

	config->json = esdm_config_gather(0, NULL);


	return config;
}


esdm_status_t esdm_config_finalize(esdm_instance_t *esdm) {


	json_decref(esdm->config->json);
		
	free(esdm->config);

	return ESDM_SUCCESS;
}





// TODO: move to utils?
int read_file(char *filepath, char **buf)
{
	if (*buf != NULL)
	{
		ESDM_ERROR("read_file(): Potential memory leak. Overwriting existing pointer with value != NULL.");
		exit(1);
	}

	FILE *f = fopen(filepath, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);

	char *string = malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	string[fsize] = 0;


	*buf = string;

	printf("read_file(): %s\n", string);
}




/**
 * Gathers ESDM configuration settings from multiple locations:
 *
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






esdm_config_backends_t* esdm_config_get_backends(esdm_instance_t* esdm)
{
	ESDM_DEBUG(__func__);	

	json_t *root = (json_t*) esdm->config->json;

	// fetch configured backends
	json_t *element = NULL;
	element = json_object_get(root, "esdm");
	element = json_object_get(element, "backends");

	size_t i;
	size_t size;;

	esdm_config_backends_t* config_backends = (esdm_config_backends_t*) malloc(sizeof(esdm_config_backends_t));

	if (element)
	{
		if (json_typeof(element) == JSON_ARRAY)
		{
			// Element is array, therefor may contain valid backend configurations
			size = json_array_size(element);

			esdm_config_backend_t* backends;
			backends = (esdm_config_backend_t*) malloc(sizeof(esdm_config_backend_t)*size);


			printf("JSON Array of %ld element%s:\n", size, json_plural(size));

			for (i = 0; i < size; i++) {
				print_json_aux(json_array_get(element, i), 0);

				json_t *backend = json_array_get(element, i);
				json_t *elem = NULL;
				
				elem = json_object_get(backend, "type");
				backends[i].type = json_string_value(elem);

				elem = json_object_get(backend, "name");
				backends[i].name = json_string_value(elem);

				elem = json_object_get(backend, "target");
				backends[i].target = json_string_value(elem);
			}	

			config_backends->count = size;
			config_backends->backends = backends;

		}
	} else {
				ESDM_ERROR("Invalid configuration! /esdm/backends is not an array.");
	}


	return config_backends;
}



















/**
 * Json parser helper for site configuration.
 *
 *
 *
 */
void print_json(json_t *root) {
    print_json_aux(root, 0);
}

void print_json_aux(json_t *element, int indent) {
    switch (json_typeof(element)) {
    case JSON_OBJECT:
        print_json_object(element, indent);
        break;
    case JSON_ARRAY:
        print_json_array(element, indent);
        break;
    case JSON_STRING:
        print_json_string(element, indent);
        break;
    case JSON_INTEGER:
        print_json_integer(element, indent);
        break;
    case JSON_REAL:
        print_json_real(element, indent);
        break;
    case JSON_TRUE:
        print_json_true(element, indent);
        break;
    case JSON_FALSE:
        print_json_false(element, indent);
        break;
    case JSON_NULL:
        print_json_null(element, indent);
        break;
    default:
        fprintf(stderr, "unrecognized JSON type %d\n", json_typeof(element));
    }
}

void print_json_indent(int indent) {
    int i;
    for (i = 0; i < indent; i++) { putchar(' '); }
}

const char *json_plural(int count) {
    return count == 1 ? "" : "s";
}

void print_json_object(json_t *element, int indent) {
    size_t size;
    const char *key;
    json_t *value;

    print_json_indent(indent);
    size = json_object_size(element);

    printf("JSON Object of %ld pair%s:\n", size, json_plural(size));
    json_object_foreach(element, key, value) {
        print_json_indent(indent + 2);
        printf("JSON Key: \"%s\"\n", key);
        print_json_aux(value, indent + 2);
    }

}

void print_json_array(json_t *element, int indent) {
    size_t i;
    size_t size = json_array_size(element);
    print_json_indent(indent);

    printf("JSON Array of %ld element%s:\n", size, json_plural(size));
    for (i = 0; i < size; i++) {
        print_json_aux(json_array_get(element, i), indent + 2);
    }
}

void print_json_string(json_t *element, int indent) {
    print_json_indent(indent);
    printf("JSON String: \"%s\"\n", json_string_value(element));
}

void print_json_integer(json_t *element, int indent) {
    print_json_indent(indent);
    printf("JSON Integer: \"%" JSON_INTEGER_FORMAT "\"\n", json_integer_value(element));
}

void print_json_real(json_t *element, int indent) {
    print_json_indent(indent);
    printf("JSON Real: %f\n", json_real_value(element));
}

void print_json_true(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    printf("JSON True\n");
}

void print_json_false(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    printf("JSON False\n");
}

void print_json_null(json_t *element, int indent) {
    (void)element;
    print_json_indent(indent);
    printf("JSON Null\n");
}

/*
 * Parse text into a JSON object. If text is valid JSON, returns a
 * json_t structure, otherwise prints and error and returns null.
 */
json_t *load_json(const char *text) {
    json_t *root;
    json_error_t error;

    root = json_loads(text, 0, &error);

    if (root) {
        return root;
    } else {
        fprintf(stderr, "json error on line %d: %s\n", error.line, error.text);
        return (json_t *)0;
    }
}

/*
 * Print a prompt and return (by reference) a null-terminated line of
 * text.  Returns NULL on eof or some error.
 */
char *read_line(char *line, int max_chars) {
    printf("Type some JSON > ");
    fflush(stdout);
    return fgets(line, max_chars, stdin);
}



































