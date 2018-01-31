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
 * @brief The layout component fragments and reconstructs logical data.
 *
 * This file contains the layout implementation.
 *
 *
 *
 *
 * TODO:
 *
 *
 *	JSON: helper
 *		required fields, and mapper to struct
 *		carry along json?
 *
 *
 *
 *	mapper:
 *		1d in => (reorder?) (single/multiple) sequence
 *			index,   blocksize[dim], filling curve
 *
 *
 *		2d in => (reorder?) (single/multiple) sequence
 *			decoupling of indexes?
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>

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



// layout component may like to have following capabilites:
//
// domain decomposition
//
// adaptive mesh refinements
//		e.g.  refine and coarsen operations on regular grid
//
//
// minimization surface vs volume


// space filling curves
//	hilber curve
//	z-ordering?



// also have a look at device data environments from openmp
// openmp:  map clause





esdm_layout_t* esdm_layout_init(esdm_instance_t* esdm) {
	ESDM_DEBUG(__func__);	

	esdm_layout_t* layout = NULL;
	layout = (esdm_layout_t*) malloc(sizeof(esdm_layout_t));
	return layout;
}


esdm_status_t esdm_layout_finalize() {
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}



esdm_status_t esdm_layout_stat(char* desc) {
	ESDM_DEBUG(__func__);	
	ESDM_DEBUG("received metadata lookup request");


	// parse text into JSON structure
	json_t *root = load_json(desc);

	if (root) {
		// print and release the JSON structure
		print_json(root);
		json_decref(root);
	}



}










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



























