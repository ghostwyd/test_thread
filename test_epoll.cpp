#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "list.h"
#include "cache.h"

#define MAX_NUM   1024
#define HEAD_NUM  777 

typedef unsigned (*conn_fun)(void *ptr);

int set_noblock(int fd);

char buffer[1<<12];
char  *rsp = "hello kitty, welcome to the jungle";
list_head_t   heads[HEAD_NUM];

struct  conn {
    unsigned        ip;
    unsigned        port;
    int             fd;
    conn_fun        *func;
    cache_t         r_cache;
    cache_t         w_cache;
    list_head_t     next; 
    
};
typedef struct conn conn_t;

int bind() 
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in  addr;
    addr.sin_port = htons(45777);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("10.153.140.88");


    if (fd == -1) {
        fprintf(stderr, "create socket fail!\n");
        return -1;
    }
    set_noblock(fd);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
        fprintf(stderr, "bind error:%s\n", strerror(errno));
        fprintf(stderr, "fd :%d bind fail!\n", fd);
        return -1;
    }
    listen(fd, 1024); 
    return fd;
}

int set_noblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0); 
    if (flags < 0) {
        fprintf(stderr, "fd :%d get fl fail.\n", fd);
        return -1;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        fprintf(stderr, "fd :%d set fl fail.\n",fd);
        return -1;
    }
    return 0;
}

int handle_read(int epoll_fd, struct epoll_event *ev) {

    conn_t *c = (conn_t*) ev->data.ptr;
    int     conn_fd = c->fd;
    int     read_len = 0;

    fprintf(stderr, "read from %d:", conn_fd);
    while (1) {

        read_len =  read(conn_fd, buffer, sizeof(buffer));
        fprintf(stderr, "read length:%d\n", read_len);

        if (read_len == 0) {
            fprintf(stderr, "disconnect! close fd :%d\n", conn_fd);
            close(conn_fd);
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn_fd, NULL);
            //todo free c and remove from list
            return -1;
        } else if (read_len == -1) {
            if (errno == EAGAIN) {
                break;
            }
            fprintf(stderr, "fd :%d, read error!\n", conn_fd);
            close(conn_fd);
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn_fd, NULL);
            return -1;
        }
        cache_append(&c->r_cache, buffer, read_len);
    }//end of while

    return 0;
}

int handle_write(int epoll_fd, struct epoll_event *ev)
{
    conn_t  *c = (conn_t*)ev->data.ptr;
    int     conn_fd = c->fd;
    int     left = strlen((c->w_cache).buffer);
    char    *pos = c->w_cache.buffer;
    int     wr_len = 0;

    while(1) {
        if (left == 0) {
            break;
        }
        wr_len = write(conn_fd, pos, left);
        if (wr_len == -1) {
            if (errno == EAGAIN) {
                break;
            } else {
                fprintf(stderr, "fd:%d delete from epoll_event.\n", conn_fd);
                close(conn_fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn_fd, NULL);
                break;
            }

        } else {
            pos += wr_len;
            left -= wr_len;
        }
    }

    return 0;
}

int accept_conn(int epoll_fd, int sock_fd)
{
    struct sockaddr_in      in_addr;
    socklen_t               sock_len = sizeof(struct sockaddr_in);
    int                     conn_fd = -1;  
    struct epoll_event      event;
    conn_t                  *c = NULL; 

    while(1) {
        if ((conn_fd = accept(sock_fd, (struct sockaddr*)&in_addr, &sock_len)) == -1) {
            //fprintf(stderr, "accept fail! error:%s\n", strerror(errno));
            return -1;
        }
        c = (conn_t*)calloc(1, sizeof(conn_t));
        if (!c) {
            fprintf(stderr, "malloc fail.\n");
            close(conn_fd);
            return -1;
        }

        init_cache(&c->r_cache);
        init_cache(&c->w_cache);
        c->ip = in_addr.sin_addr.s_addr;
        c->port = in_addr.sin_port;
        c->fd = conn_fd;
        strncpy(c->w_cache.buffer, rsp, strlen(rsp));

        INIT_LIST_HEAD(&(c->next));
        list_add_tail(&(c->next), &heads[conn_fd % HEAD_NUM]);

        event.events = EPOLLIN | EPOLLOUT | EPOLLET;
        event.data.ptr = c;
        set_noblock(conn_fd);
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event)) {
            fprintf(stderr, "epoll_ctl add fail!\n");
            return -1;
        }
        fprintf(stderr, "accept conn from %s:%d\n", inet_ntoa(in_addr.sin_addr), in_addr.sin_port);

    }
    return 0;
}

int create_epoll()
{
    int                     epoll_fd = epoll_create(MAX_NUM);
    struct epoll_event      events[MAX_NUM];
    struct epoll_event      event;
    int                     sock_fd = bind();

    if (sock_fd == -1) {
        return -1;
    }

    if (epoll_fd == -1) {
        fprintf(stderr, "create epoll fail!\n");
        return -1;
    }

    for (int i = 0; i < HEAD_NUM; i++ ) {
        INIT_LIST_HEAD(&heads[i]);
    }

    event.events = EPOLLIN;
    event.data.fd = sock_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event); 
    
    for ( ; ;) {

        int fds = epoll_wait(epoll_fd, events, MAX_NUM, 500);
        printf("active num :%d\n",fds);
        if (fds == - 1) {
            if (errno == EINTR) {
                continue;
            } else {
                fprintf(stderr, "epoll_wait error:%s\n", strerror(errno));
                break;
            }
        }

        for (int i = 0; i < fds; i++) {
            if (events[i].data.fd == sock_fd)  {
                accept_conn(epoll_fd, sock_fd);
                continue;
            }

            if (events[i].events && EPOLLIN) {
                handle_read(epoll_fd, &events[i]);
            } 
            if (events[i].events && EPOLLOUT) {
                handle_write(epoll_fd, &events[i]);
            }
        }
    }
    close(epoll_fd);
    return 0;
}

int main()
{
     
    create_epoll();
    return EXIT_SUCCESS;
}
