#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <vector>

using namespace std;

#define BUFFER_SIZE       128
#define THD_COUNT         8

bool      stop;
pthread_rwlock_t  rw_lock;

typedef struct master_info {
    unsigned          ip;
    int               port;
    char              main_business[BUFFER_SIZE];
    char              second_business[BUFFER_SIZE];
};

typedef struct worker_head {
    pthread_mutex_t     p_mutex;
    std::vector<int>    vec_task; 
} worker_head_t;

worker_head_t             task_list[THD_COUNT];
pthread_t                 thread_list[THD_COUNT];

void* work_thread(void *ptr)
{
    worker_head_t  *head = (worker_head_t*)ptr;

    while (1)  {
        pthread_mutex_lock(&head->p_mutex);
        if (head->vec_task.size() == 0) {
            pthread_rwlock_rdlock(&rw_lock);
            if (stop) {
                printf("thread %d, finish\n", (int)pthread_self());
                pthread_rwlock_unlock(&rw_lock);
                pthread_mutex_unlock(&head->p_mutex);
                break;
            } else {
                pthread_rwlock_unlock(&rw_lock);
            }
        } else {
            int value = head->vec_task[0];
            head->vec_task.erase(head->vec_task.begin());
            //printf("in thread %d: %d\n", pthread_self(), value);
        }
        pthread_mutex_unlock(&head->p_mutex);
    }

    return NULL;
}


int main(void)
{
    int                       index = 0;
    stop = false;
    srand(time(NULL));


    for (int i = 0; i < THD_COUNT; ++i) {
        pthread_mutex_init(&task_list[i].p_mutex, NULL); 
        pthread_create(&thread_list[i], NULL, work_thread, &task_list[i]);
    }
    pthread_rwlock_init(&rw_lock, NULL);

    for (int i = 0; i < 1600; ++i) {
        index = random() % THD_COUNT;
        pthread_mutex_lock(&task_list[index].p_mutex);
        task_list[index].vec_task.push_back(i);
        pthread_mutex_unlock(&task_list[index].p_mutex);
    }

    pthread_rwlock_wrlock(&rw_lock);
    stop = true;
    pthread_rwlock_unlock(&rw_lock);
    
    printf("ready to wait join\n");
    for(int i = 0; i < THD_COUNT; ++i) {
        pthread_join(thread_list[i], NULL);
        printf("wait to join %d\n", i);
        pthread_mutex_destroy(&task_list[i].p_mutex);
    }

    pthread_rwlock_destroy(&rw_lock);
    return EXIT_SUCCESS;
}
