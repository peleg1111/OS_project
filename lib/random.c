#include "stdio.h"

#define RAND_MAX_CUSTOM 2147483647

static unsigned long seed = 1;


int my_rand()
{
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % RAND_MAX_CUSTOM;
}

int rnd(int min, int max){
    
    if(min >= max){
        max ^= min;
        min ^= max;
        max ^= min;
    }
    
    return min + (int)( my_rand() % (max - min + 1));
}