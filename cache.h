#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifndef __CACHE__
#define __CACHE__

#define CACHE_SIZE  (1<<10) 

struct cache {
    char buffer[CACHE_SIZE];
    int  head;
    int  tail;
};
typedef struct cache cache_t;

int cache_append(cache_t *c, char *buffer, int len) 
{
    assert(c!= NULL); 
    int left_len = 0;

    if (c->head > c->tail)  {
        left_len = c->head - c->tail - 1;
    } else {
        left_len = CACHE_SIZE + c->head - c->tail -1;
    }
    fprintf(stderr, "left len :%d\n", left_len); 

    if (left_len < len) {
        fprintf(stderr, "cache space is not enough!\n");
        fprintf(stderr, "head:%d, tail:%d, len:%d, left_len:%d\n", c->head, c->tail, len, left_len);
        return -1;
    }

    if (c->tail + len < CACHE_SIZE) {
        memcpy(c->tail + c-> buffer, buffer, len);
        c->tail += len;
        return 0;
    }
    memcpy(c->tail + c->buffer, buffer, CACHE_SIZE - c->tail);
    memcpy(c->buffer, buffer + (CACHE_SIZE - c->tail), len + c->tail - CACHE_SIZE);
    c->tail = len + c->tail - CACHE_SIZE; 
    return 0;
}

void init_cache(cache_t *c)
{
    memset(c->buffer, CACHE_SIZE, 0);
    c->head = c->tail = 0;
}
    

#endif
#endif
