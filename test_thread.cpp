#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <queue>

using namespace std;

#define BUFFER_SIZE       128
#define THREAD_COUNT      8

typedef struct master_info {
    unsigned     ip;
    int          port;
    char         main_business[BUFFER_SIZE];
    char         second_business[BUFFER_SIZE];
};


int main(void)
{
    std::queue
    return EXIT_SUCCESS;
}
