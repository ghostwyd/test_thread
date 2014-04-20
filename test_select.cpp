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
#define BUF_SIZE 256 

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
    char  buffer[256];
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

    int                fd = -1;
    int                max_fd = -1;
    fd_set             r_set;
    fd_set             w_set;
    fd_set             r_tmp_set;
    fd_set             w_tmp_set;
    char               buffer[BUF_SIZE];
    struct timeval     timeout;
    int                fd_list[FD_SETSIZE];

    for (int i =0; i <  FD_SETSIZE; i++) {
        fd_list[i] = -1; 
    }
    timeout.tv_sec = 0;
    timeout.tv_usec = 500;

    for (int i = 0; i < 1; ++i) {
        fd = conn_server();
        if (fd == -1) {
           return -1;
        }
        fd_list[fd] = 1;
        if (fd > max_fd) {
            max_fd = fd;
        }
    }

    for ( ; ; ) {
        FD_ZERO(&r_set);
        FD_ZERO(&w_set);
        for (int i = 0; i <= max_fd; i++) {
            if (fd_list[i] != 1) {
                continue;
            }
            FD_SET(i, &r_set);
            FD_SET(i, &w_set);
        }
        //n =  select(max_fd + 1, &r_set, &w_set, NULL,  &timeout);
        select(max_fd + 1, &r_set, &w_set, NULL,  &timeout);

        for (int i = 0; i < max_fd + 1; i++) { 
            if (fd_list[i] == -1) {
                continue;
            }
            fd = i;
            if (FD_ISSET(fd, &r_set)) {
                printf("enter read fd:%d\n", fd);
                int r_len = read(fd,buffer, BUF_SIZE);
                if (r_len == 0) {
                    fprintf(stderr, "close socket\n");
                    FD_CLR(fd, &r_tmp_set);
                    close(fd);
                    return -1; 
                } else if (r_len < 0) {
                    if (! (errno == EAGAIN || errno == EINTR)) {
                        fprintf(stderr, "write error:%s\n", strerror(errno));
                        FD_CLR(fd, &r_tmp_set);
                        close(fd);
                        return -1;
                    }
                }
                buffer[r_len]  = '\0';
                fprintf(stderr, "fd %d receive len:%d\n", fd, r_len);
                fprintf(stderr, "fd %d receive from server:%s\n",fd,buffer);
            } else {
                fprintf(stderr, "fd :%d, not readable\n", fd);
            }

            srand(time(NULL));
            if (FD_ISSET(fd, &w_set)) {
                for (int j = 0; j < BUF_SIZE; j++) {
                    buffer[j] = rand() % 26 + 'A';
                }
                buffer[BUF_SIZE-1]= '\0';
                int w_len = write(fd, buffer, BUF_SIZE);
                fprintf(stderr, "send to server:%s\n", buffer);
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
            } else {
                fprintf(stderr, "not ready for write !\n");
            }
        }
        //break;
    }
    return EXIT_SUCCESS;
}
