#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

#define  THREAD_COUNT    8
#define  QUEUE_SIZE      32

typedef struct {
   int  index;
}task;

pthread_mutex_t       mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t        cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t        cond_writeable = PTHREAD_COND_INITIALIZER;
int                   read_index = 0;
int                   write_index = 0;
task                  task_list[QUEUE_SIZE];

void* work_thread(void *ptr)
{
    while(1) {
        pthread_mutex_lock(&mutex); 
        while (read_index == write_index) {
            pthread_cond_wait(&cond, &mutex);
        }
        fprintf(stderr, "%d\n", task_list[read_index].index);
        //if (read_index == (write_index + 1) % QUEUE_SIZE){
            pthread_cond_signal(&cond_writeable);
        //}
        if (++read_index == QUEUE_SIZE) {
            read_index = 0;
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}


int main(void)
{
    pthread_t thread_list[THREAD_COUNT];

    for (int i  = 0 ; i < THREAD_COUNT; ++i) {
        pthread_create(&thread_list[i], NULL, work_thread, NULL);
    }

   for (int i = 0; i < 160000; ++i) {
       pthread_mutex_lock(&mutex);
       /*if (read_index == (write_index + 1) % QUEUE_SIZE) {
            fprintf(stderr, "%d enqueue fail! Queue is full!\n", i);
            i--;
            pthread_mutex_unlock(&mutex);
            continue;
       }*/
       while (read_index == (write_index + 1) % QUEUE_SIZE) {
            pthread_cond_wait(&cond_writeable, &mutex);
       }
       task_list[write_index++].index = i;
       if (write_index == QUEUE_SIZE) {
            write_index = 0;
       }
       pthread_cond_signal(&cond);
       pthread_mutex_unlock(&mutex);
   }
}
