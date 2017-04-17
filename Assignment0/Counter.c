#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    
    int i = 0;
    printf("D5 D4 D3 D2 D1 D0\n");
    printf("-----------------\n");

    while(i < 64){

        printf("%x  %x  %x  %x  %x  %x\n",
            (i & 32) >> 5, (i & 16) >> 4, (i & 8) >> 3,
            (i & 4) >> 2, (i & 2) >> 1, i & 1);
        i++;
    }
    
    return 0;
}
