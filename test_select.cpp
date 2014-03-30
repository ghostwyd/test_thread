#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>

#define THD_NUM  1200

int conn_server() 
{
    int                 fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in  addr;
    int                 alive = 100;

    addr.sin_port = htons(45777);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);


    if (fd == -1) {
        fprintf(stderr, "create socket fail!\n");
        return -1;
    }
    //  set_noblock(fd);
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &alive, sizeof(int));

    if (connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
        fprintf(stderr, "bind error:%s\n", strerror(errno));
        fprintf(stderr, "fd :%d bind fail!\n", fd);
        return -1;
    }
    return fd;
}

void* do_work(void *ptr)
{
    char  buffer[64];
    int fd = conn_server();
    if (fd < 0) {
        return NULL;
    }
    srand(time(NULL));
    while(1) {
        for (int i = 0; i <  64 ; i++) {
            buffer[i] = rand() % 26 + 'A';
        }
        int w_len = write(fd, buffer, 64);
        if (w_len < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            fprintf(stderr, "write error:%s\n", strerror(errno));
            close(fd);
            return NULL;
        }
        int r_len = read(fd,buffer, 64);
        if (r_len == 0) {
            fprintf(stderr, "close socket\n");
            close(fd);
            return NULL;
        } else if (r_len < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            fprintf(stderr, "write error:%s\n", strerror(errno));
            close(fd);
            return NULL;

        }
        fprintf(stderr, "fd %d read_len %d : %s\n", fd, r_len, buffer);
    }
    close(fd); 
    return NULL;
}
int main(void)
{
    /*pthread_t thd_list[THD_NUM];
    for (int i = 0; i < THD_NUM; i++) {
        pthread_create(&thd_list[i], NULL,  do_work, NULL);
    }

    for (int i = 0; i < THD_NUM; i++) {
        pthread_join(thd_list[i], NULL);
    }*/

    int     fd = conn_server();
    int     max_fd = -1;
    fd_set  r_set;
    fd_set  w_set;
    fd_set  r_tmp_set;
    fd_set  w_tmp_set;
    int      n = 0;
    char     buffer[64];
    struct timeval   timeout;
    int              fd_list[FD_SETSIZE];

    for (int i =0; i <  FD_SETSIZE; i++) {
        fd_list[i] = -1; 
    }
    FD_ZERO(&r_set);
    FD_ZERO(&w_set);
    FD_SET(fd, &r_set);
    FD_SET(fd, &w_set);
    timeout.tv_sec = 0;
    timeout.tv_usec = 500;

    fd_list[fd] = 1;
    if (fd > max_fd) {
        max_fd = fd;
    }

    printf("fd:%d\n", fd);
    int spec_fd = fd;
    for ( ; ; ) {
        r_tmp_set =  r_set;
        w_tmp_set = w_set;
        FD_SET(spec_fd, &r_set);
        FD_SET(spec_fd, &w_set);
        n =  select(max_fd + 1, &r_set, &w_set, NULL,  &timeout);

        for (int i = 0; i < max_fd + 1; i++) { 
            if (fd_list[i] == -1) {
                continue;
            }
            fd = i;
            if (FD_ISSET(fd, &r_set)) {
                printf(" enter read fd:%d\n", fd);
                int r_len = read(fd,buffer, 64);
                if (r_len == 0) {
                    fprintf(stderr, "close socket\n");
                    FD_CLR(fd, &r_tmp_set);
                    close(fd);
                    return -1; 
                } else if (r_len < 0) {
                    if (errno == EAGAIN || errno == EINTR) {
                        continue;
                    }
                    fprintf(stderr, "write error:%s\n", strerror(errno));
                    FD_CLR(fd, &r_tmp_set);
                    close(fd);
                    return -1;
                }
            } else {
                fprintf(stderr, "not readable\n");
            }

            srand(time(NULL));
            if (FD_ISSET(fd, &w_set)) {
                for (int j = 0; j <  64 ; j++) {
                    buffer[j] = rand() % 26 + 'A';
                }
                int w_len = write(fd, buffer, 64);
                if (w_len < 0) {
                    if (errno == EAGAIN || errno == EINTR) {
                        continue;
                    }
                    fprintf(stderr, "write error:%s\n", strerror(errno));
                    FD_CLR(fd, &w_tmp_set);
                    close(fd);
                    return -1;
                }
                fprintf(stderr, "write len:%d\n", w_len);
                //-------------------------------
                int r_len = read(fd,buffer, 64);
                if (r_len == 0) {
                    fprintf(stderr, "close socket\n");
                    FD_CLR(fd, &r_tmp_set);
                    close(fd);
                    return -1; 
                } else if (r_len < 0) {
                    if (errno == EAGAIN || errno == EINTR) {
                        continue;
                    }
                    fprintf(stderr, "write error:%s\n", strerror(errno));
                    FD_CLR(fd, &r_tmp_set);
                    close(fd);
                    return -1;
                }

            }
        }
        r_set = r_tmp_set;
        w_set = w_tmp_set;
    }

    return EXIT_SUCCESS;
}

