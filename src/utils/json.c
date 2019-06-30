/*
 * Copyright (c) 2012 Rogerz Zhang <rogerz.zhang@gmail.com>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jansson.h>

#include <esdm-internal.h>


///////////////////////////////////////////////////////////////////////////////
// JSON - Load and try to parse
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
// JSON - Print helper to display json object contents.
///////////////////////////////////////////////////////////////////////////////

static void print_json_indent(int indent);
static void print_json_object(const json_t *element, int indent);
static void print_json_array(const json_t *element, int indent);
static void print_json_string(const json_t *element, int indent);
static void print_json_integer(const json_t *element, int indent);
static void print_json_real(const json_t *element, int indent);
static void print_json_true(const json_t *element, int indent);
static void print_json_false(const json_t *element, int indent);
static void print_json_null(const json_t *element, int indent);


void print_json(const json_t *root) {
  print_json_aux(root, 0);
}


void print_json_aux(const json_t *element, int indent) {
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
  for (i = 0; i < indent; i++) {
    putchar(' ');
  }
}


const char *json_plural(int count) {
  return count == 1 ? "" : "s";
}


void print_json_object(const json_t *element, int indent) {
  size_t size;
  const char *key;
  json_t *value;

  print_json_indent(indent);
  size = json_object_size(element);

  printf("JSON Object of %ld pair%s:\n", size, json_plural(size));
  json_object_foreach((json_t *)element, key, value) {
    print_json_indent(indent + 2);
    printf("JSON Key: \"%s\"\n", key);
    print_json_aux(value, indent + 2);
  }
}


void print_json_array(const json_t *element, int indent) {
  size_t i;
  size_t size = json_array_size(element);
  print_json_indent(indent);

  printf("JSON Array of %ld element%s:\n", size, json_plural(size));
  for (i = 0; i < size; i++) {
    print_json_aux(json_array_get(element, i), indent + 2);
  }
}


void print_json_string(const json_t *element, int indent) {
  print_json_indent(indent);
  printf("JSON String: \"%s\"\n", json_string_value(element));
}


void print_json_integer(const json_t *element, int indent) {
  print_json_indent(indent);
  printf("JSON Integer: \"%" JSON_INTEGER_FORMAT "\"\n", json_integer_value(element));
}


void print_json_real(const json_t *element, int indent) {
  print_json_indent(indent);
  printf("JSON Real: %f\n", json_real_value(element));
}


void print_json_true(const json_t *element, int indent) {
  (void)element;
  print_json_indent(indent);
  printf("JSON True\n");
}


void print_json_false(const json_t *element, int indent) {
  (void)element;
  print_json_indent(indent);
  printf("JSON False\n");
}


void print_json_null(const json_t *element, int indent) {
  (void)element;
  print_json_indent(indent);
  printf("JSON Null\n");
}


///////////////////////////////////////////////////////////////////////////////
// JSON PATH - for convienient access of json members
///////////////////////////////////////////////////////////////////////////////

/*
 * Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 * Copyright (c) 2012 Rogerz Zhang <rogerz.zhang@gmail.com>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license.
 *
 * https://github.com/rogerz/jansson/blob/json_path/src/path.c
 *
 * Also see disccusion on why this is not build into Jansson:
 * https://groups.google.com/forum/#!topic/jansson-users/oN7Wfo3lVPk
 *
 * Spec proposal for JSONPath:
 * http://goessner.net/articles/JsonPath/
 *
 */

/* jansson private helper functions */
static void *jsonp_malloc(size_t size);
static void jsonp_free(void *ptr);
static char *jsonp_strdup(const char *str);

static json_malloc_t do_malloc = malloc;
static json_free_t do_free     = free;


json_t *json_path_get(const json_t *json, const char *path) {
  static const char root_chr = '$', array_open = '[';
  static const char *path_delims = ".[", *array_close = "]";
  const json_t *cursor;
  char *token, *buf, *peek, *endptr, delim = '\0';
  const char *expect;

  if (!json || !path || path[0] != root_chr)
    return NULL;
  else
    buf = jsonp_strdup(path);

  peek   = buf + 1;
  cursor = json;
  token  = NULL;
  expect = path_delims;

  while (peek && *peek && cursor) {
    char *last_peek = peek;
    peek            = strpbrk(peek, expect);
    if (peek) {
      if (!token && peek != last_peek)
        goto fail;
      delim   = *peek;
      *peek++ = '\0';
    } else if (expect != path_delims || !token) {
      goto fail;
    }

    if (expect == path_delims) {
      if (token) {
        cursor = json_object_get(cursor, token);
      }
      expect = (delim == array_open ? array_close : path_delims);
      token  = peek;
    } else if (expect == array_close) {
      size_t index = strtol(token, &endptr, 0);
      if (*endptr)
        goto fail;
      cursor = json_array_get(cursor, index);
      token  = NULL;
      expect = path_delims;
    } else {
      goto fail;
    }
  }

  jsonp_free(buf);
  return (json_t *)cursor;
fail:
  jsonp_free(buf);
  return NULL;
}


int json_path_set_new(json_t *json, const char *path, json_t *value, size_t flags, json_error_t *error) {
  static const char root_chr = '$', array_open = '[', object_delim = '.';
  static const char *const path_delims = ".[", *array_close = "]";

  json_t *cursor, *parent = NULL;
  char *token, *buf = NULL, *peek, delim = '\0';
  const char *expect;
  int index_saved = -1;

  //jsonp_error_init(error, "<path>");

  if (!json || !path || flags || !value) {
    //jsonp_error_set(error, -1, -1, 0, "invalid argument");
    printf("invalid argument");

    goto fail;
  } else {
    buf = jsonp_strdup(path);
  }

  if (buf[0] != root_chr) {
    //jsonp_error_set(error, -1, -1, 0, "path should start with $");
    printf("path should start with $");
    goto fail;
  }

  peek   = buf + 1;
  cursor = json;
  token  = NULL;
  expect = path_delims;

  while (peek && *peek && cursor) {
    char *last_peek = peek;
    peek            = strpbrk(last_peek, expect);

    if (peek) {
      if (!token && peek != last_peek) {
        //jsonp_error_set(error, -1, -1, last_peek - buf, "unexpected trailing chars");
        printf("unexpected trailing chars");
        goto fail;
      }
      delim   = *peek;
      *peek++ = '\0';
    } else { // end of path
      if (expect == path_delims) {
        break;
      } else {
        //jsonp_error_set(error, -1, -1, peek - buf, "missing ']'?");
        printf("missing ']'?");
        goto fail;
      }
    }

    if (expect == path_delims) {
      if (token) {
        if (token[0] == '\0') {
          //jsonp_error_set(error, -1, -1, peek - buf, "empty token");
          printf("empty token");
          goto fail;
        }

        parent = cursor;
        cursor = json_object_get(parent, token);

        if (!cursor) {
          if (!json_is_object(parent)) {
            //jsonp_error_set(error, -1, -1, peek - buf, "object expected");
            printf("object expected");
            goto fail;
          }
          if (delim == object_delim) {
            cursor = json_object();
            json_object_set_new(parent, token, cursor);
          } else {
            //jsonp_error_set(error, -1, -1, peek - buf, "new array is not allowed");
            printf("new array is not allowed");
            goto fail;
          }
        }
      }
      expect = (delim == array_open ? array_close : path_delims);
      token  = peek;
    } else if (expect == array_close) {
      char *endptr;
      size_t index;

      parent = cursor;
      if (!json_is_array(parent)) {
        ////jsonp_error_set(error, -1, -1, peek - buf, "array expected");
        printf("array expected");
        goto fail;
      }
      index = strtol(token, &endptr, 0);
      if (*endptr) {
        //jsonp_error_set(error, -1, -1, peek - buf, "invalid array index");
        printf("invalid array index");
        goto fail;
      }
      cursor = json_array_get(parent, index);
      if (!cursor) {
        //jsonp_error_set(error, -1, -1, peek - buf, "array index out of bound");
        printf("array index out of bound");
        goto fail;
      }
      index_saved = index;
      token       = NULL;
      expect      = path_delims;
    } else {
      assert(1);
      //jsonp_error_set(error, -1, -1, peek - buf, "kidding me");
      printf("kidding me");
      goto fail;
    }
  }

  if (token) {
    if (json_is_object(cursor)) {
      json_object_set(cursor, token, value);
    } else {
      //jsonp_error_set(error, -1, -1, peek - buf, "object expected");
      printf("object expected");
      goto fail;
    }
    cursor = json_object_get(cursor, token);
  } else if (index_saved != -1 && json_is_array(parent)) {
    json_array_set(parent, index_saved, value);
    cursor = json_array_get(parent, index_saved);
  } else {
    //jsonp_error_set(error, -1, -1, peek - buf, "invalid path");
    printf("invalid path");
    goto fail;
  }

  json_decref(value);
  jsonp_free(buf);
  return 0;

fail:
  json_decref(value);
  jsonp_free(buf);
  return -1;
}


/* jansson private helper functions */
static void *jsonp_malloc(size_t size) {
  if (!size)
    return NULL;

  return (*do_malloc)(size);
}


static void jsonp_free(void *ptr) {
  if (!ptr)
    return;

  (*do_free)(ptr);
}


static char *jsonp_strdup(const char *str) {
  char *new_str;

  new_str = jsonp_malloc(strlen(str) + 1);
  if (!new_str)
    return NULL;

  strcpy(new_str, str);
  return new_str;
}
/* end jansson private helpers */
