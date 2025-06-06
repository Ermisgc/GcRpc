#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <liburing.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

#define BUFFER_SIZE 1024
#define ENTITY_SIZE 1024
#define MAX_CQES 10

#define USER_EVENT_ACCEPT 0
#define USER_EVENT_READ 1
#define USER_EVENT_WRITE 2
#define USER_EVENT_CLOSE 3

struct io_uring_user_info{
    int fd;
    int event = USER_EVENT_ACCEPT;
    char buf[BUFFER_SIZE];
};

int main(int argc, char *argv[]) {
    int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == -1) return false;
    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(1214);

    if(bind(sockfd, (const sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        close(sockfd);
        return false;
    } 
    if(listen(sockfd, 10) == -1){
        close(sockfd);
        return false;
    } 

    struct io_uring ring;
    struct io_uring_params params;
    memset(&params, 0, sizeof(params));
    if(io_uring_queue_init_params(ENTITY_SIZE, &ring, &params) < 0){
        close(sockfd);
        return false;
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char * buffer[BUFFER_SIZE];
    for(int i = 0;i < ENTITY_SIZE; ++i){
        struct io_uring_sqe * sqe = io_uring_get_sqe(&ring);
        struct io_uring_user_info * info = new io_uring_user_info;
        info->fd = 0;
        info->event = USER_EVENT_ACCEPT;
        sqe->user_data = (unsigned long long)info;

        io_uring_prep_accept(sqe, sockfd, (sockaddr*)&client_addr, &client_addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    }
    //开始接收
    while(1){

        io_uring_submit(&ring);

        struct io_uring_cqe *cqes[MAX_CQES];
        uint32_t ret = io_uring_peek_batch_cqe(&ring, cqes, MAX_CQES);
        for(unsigned i = 0;i < ret; ++i){
            struct io_uring_cqe * cqe_now = cqes[i];
            if(cqe_now->res < 0) {
                perror("I/O operation failed");
            }

            io_uring_user_info * info = (io_uring_user_info *)cqe_now->user_data;
            struct io_uring_sqe * another_sqe = io_uring_get_sqe(&ring);
            another_sqe->user_data = (unsigned long long)info;

            switch (info->event) {
            case USER_EVENT_ACCEPT:
                info->fd = cqe_now->res;
                std::cout << info->fd << " is Connected" << std::endl;
                info->event = USER_EVENT_READ;
                io_uring_prep_read(another_sqe, info->fd, info->buf, BUFFER_SIZE, 0);
                break;
            
            case USER_EVENT_READ:
                std::cout << "Recv:" << info->buf << std::endl;
                info->event = USER_EVENT_WRITE;
                io_uring_prep_write(another_sqe, info->fd, info->buf, cqe_now->res, 0);
                break;

            case USER_EVENT_WRITE:
                info->event = USER_EVENT_CLOSE;
                io_uring_prep_close(another_sqe, info->fd);
                break;

            case USER_EVENT_CLOSE:
                info->event = USER_EVENT_ACCEPT;
                io_uring_prep_accept(another_sqe, sockfd, (sockaddr*)&client_addr, &client_addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
                break;
                
            default:
                break;
            }

            io_uring_cqe_seen(&ring, cqe_now);
        }
    }
    close(sockfd);
    return 0;
}