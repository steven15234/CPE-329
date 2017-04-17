#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define VEC_UINT 0x35123 //Not yet supported
#define VEC_INT 0x35123
#define VEC_FLOAT 0x35124
#define VEC_DOUBLE 0x35125
#define VEC_CHAR 0x35126
#define VEC_STRING 0x35127

typedef struct dynamic_vector{
   void** data;
   size_t size;
   size_t len;
   int type;
   void (*push_back)(void*);
}vector;

typedef vector* Vector;

Vector GLOBAL_VEC;

Vector new_vector(size_t size, int type);
void delete_vector(Vector vec);
void vector_set_global(Vector vec);
void vector_clear_global();
void vector_push_back(void* item);
void* vector_at(size_t index);
void vector_remove(size_t index);
void vector_print(Vector vec);