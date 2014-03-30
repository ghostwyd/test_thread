#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#ifndef __CACHE__
#define __CACHE__

#define CACHE_SIZE  (1<<10) 

typedef unsigned (*conn_fun)(void *ptr);

typedef struct cache cache_t;
struct cache {
    char      *buffer;
    unsigned  head;
    unsigned  buf_size;
    unsigned  len;
};

typedef struct conn conn_t;
struct  conn {
    unsigned        ip;
    unsigned        port;
    int             fd;
    conn_fun        *func;
    cache_t         r_cache;
    cache_t         w_cache;
    list_head_t     next; 
    
};


int cache_append(cache_t *c, char *buffer, unsigned len)
{
     assert(c!=NULL && c->buffer!=NULL);

     if (c->buf_size - c->len < len) {

         fprintf(stderr, "resize cache_t\n");
         unsigned  new_size = c->buf_size;
         char      *tmp;

         while(new_size < (c->buf_size + len)) {
             new_size += CACHE_SIZE;
         }

         tmp = (char*)realloc(c->buffer, new_size);
         if (!tmp) {
             fprintf(stderr, "realloc fail  fro cache_t.\n");
             return -1;
         }
         c->buffer = tmp;
         c->buf_size = new_size;
     }
     
     if (c->head + c->len + len > c->buf_size) {
         memmove(c->buffer, c->buffer + c->head, c->len);   
         c->head = 0;
     }

     memcpy(c->buffer + c->head + c->len, buffer, len);       
     c->len += len;
     return 0;
}

int init_cache(cache_t *c)
{
    c->buffer = NULL;
    c->head = c->len = 0;
    c->buf_size = 0;
    c->buffer = (char*)malloc(CACHE_SIZE);
    if (!c->buffer) {
        fprintf(stderr, "malloc fail for cache!\n");             
        return -1;
    }
    c->buf_size = CACHE_SIZE;
    memset(c->buffer, c->buf_size, 0);
    return 0;
}
    

int  cache_read(cache_t *c, char *buf, unsigned len)
{
     assert(c!=NULL && c->buf_size != 0); 
     int read_len = len < c->len ? len:c->len;
     if (c->len == 0) {
         return 0;
     }
     memcpy(buf, c->buffer + c->head, read_len);
     c->head = (c->head + read_len) % c->buf_size;
     c->len -= read_len;
     return read_len;

}

void cache_print(cache_t *c)
{
    assert(c!=NULL && c->buffer != NULL);
    write(1, c->buffer + c->head, c->len); 
}

void free_conn(conn_t *c)
{
   assert(c != NULL);  
   if (c->r_cache.buffer) {
       free(c->r_cache.buffer); 
   }
   if (c->w_cache.buffer) {
       free(c->w_cache.buffer); 
   }
   free(c);
}

#endif
