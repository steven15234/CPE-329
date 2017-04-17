#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

int main(int argc, char** argv){
    
    
    Vector testVec1 = new_vector(10, VEC_CHAR);
    Vector testVec2 = new_vector(10, VEC_STRING);
    vector_set_global(testVec1);
    for(size_t i = 72; i < 100; i++){
        vector_push_back(&i);
    }
    vector_print(testVec1);
    vector_clear_global();
    vector_set_global(testVec2);
    testVec2->push_back("aa");
    testVec2->push_back("bb");
    vector_print(testVec2);
    return 0;
}
