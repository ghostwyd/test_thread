#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <queue>

using namespace std;

#define BUFFER_SIZE       128
#define THD_COUNT         8

bool      stop;
typedef struct master_info {
    unsigned     ip;
    int          port;
    char         main_business[BUFFER_SIZE];
    char         second_business[BUFFER_SIZE];
};

void* work_thread(void *ptr)
{

}


int main(void)
{
    std::queue<master_info>   info_list[THD_COUNT];
    pthread_t                 thread_list[THD_COUNT];
    stop = false;
    srand(time(NULL));
    int                       index = 0;

    for (int i = 0; i < THD_COUNT; ++i) {
         pthread_create(&thread_list[i], NULL, work_thread, &info_list[i]);
    }
    for (int i = 0; i < 160; ++i) {
       index = random() % THD_COUNT;
       info_list[i].;
    }
    return EXIT_SUCCESS;
}
