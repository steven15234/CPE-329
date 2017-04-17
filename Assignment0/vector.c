#include "vector.h"

void vector_set_global(Vector vec){
   GLOBAL_VEC = vec;
}

void vector_clear_global(){
   GLOBAL_VEC = NULL;
}

void vector_resize(Vector vec){
   if(vec->len == vec->size){
      vec->size = 1 + (vec->size*2);
      if(NULL == (vec->data = realloc(vec->data, vec->size * sizeof(*(vec->data))))){
         perror("Failed to resize Vector");
         exit(1);
      }
   }
}

void check_vec(){
   if(GLOBAL_VEC == NULL){
      perror("Failed to assign a global vector");
      exit(1);
   }
}

int set_size(Vector vec){
   check_vec(vec);
   if(vec->type == VEC_INT)
      return sizeof(int);
   else if(vec->type == VEC_FLOAT)
      return sizeof(float);
   else if(vec->type == VEC_DOUBLE)
      return sizeof(double);
   else if(vec->type == VEC_CHAR)
      return sizeof(char);
   else if(vec->type == VEC_STRING)
      return -1;
}

Vector new_vector(size_t size, int type){
   Vector res = NULL;
   if(NULL == (res = malloc(sizeof(vector)))){
      perror("Failed to malloc Vector");
      exit(1);
   }
   res->size = size;
   res->len = 0;
   res->data = malloc(sizeof(*(res->data)) * size);
   res->type = type;
   res->push_back = &vector_push_back;
   return res;
}

void delete_vector(Vector vec){
   int i, len;
   for(i = 0; i < vec->len; i++){
      free(vec->data[i]);
   }
   free(vec->data);
   free(vec);
}

void vector_push_back(void* item){
   check_vec();
   vector_resize(GLOBAL_VEC);
   int size = set_size(GLOBAL_VEC);
   if(size == -1)
      size = strlen((char*)item);
   GLOBAL_VEC->data[GLOBAL_VEC->len] = malloc(size);
   memcpy(GLOBAL_VEC->data[GLOBAL_VEC->len++], item, size);
   
}

void* vector_at(size_t index){
   check_vec();
   return GLOBAL_VEC->data[index];
}

void vector_remove(size_t index){
   int i;
   check_vec();
   for(i = index; i < GLOBAL_VEC->len - 1; i++){
      free(GLOBAL_VEC->data[i]);
      GLOBAL_VEC->data[i] = GLOBAL_VEC->data[i+1];
   }
   GLOBAL_VEC->len--;
}

void vector_print(Vector vec){
   int i;
   printf("{ ");
   if(vec->type == VEC_INT){
      for(i = 0; i < vec->len - 1; i++){
         printf("%d, ", *((int*)vector_at(i)));
      }
      printf("%d }\n", *((int*)vector_at(i)));
   }
   else if(vec->type == VEC_FLOAT || vec->type == VEC_DOUBLE){
       for(i = 0; i < vec->len - 1; i++){
         printf("%f, ", *((double*)vector_at(i)));
      }
      printf("%f }\n", *((double*)vector_at(i)));
   }
   else if(vec->type == VEC_CHAR){
       for(i = 0; i < vec->len - 1; i++){
         printf("\'%c\', ", *((char*)vector_at(i)));
      }
      printf("\'%c\' }\n", *((char*)vector_at(i)));
   }
   else if(vec->type == VEC_STRING){
       for(i = 0; i < vec->len - 1; i++){
         printf("\"%s\", ", (char*)vector_at(i));
      }
      printf("\"%s\" }\n", (char*)vector_at(i));
   }
   
}