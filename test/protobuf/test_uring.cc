#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <liburing.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    struct io_uring ring;
    int fd;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    char buffer[BUFFER_SIZE];
    int ret;

    // 初始化 io_uring
    ret = io_uring_queue_init(1, &ring, 0);
    if (ret < 0) {
        perror("io_uring_queue_init");
        return 1;
    }

    // 打开文件
    fd = open("test.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // 获取一个提交队列项
    sqe = io_uring_get_sqe(&ring);
    if (!sqe) {
        fprintf(stderr, "Failed to get SQE\n");
        return 1;
    }

    // 准备异步读取请求
    io_uring_prep_read(sqe, fd, buffer, BUFFER_SIZE, 0);
    io_uring_sqe_set_data(sqe, buffer);

    // 提交请求
    ret = io_uring_submit(&ring);
    if (ret <= 0) {
        fprintf(stderr, "io_uring_submit failed: %d\n", ret);
        return 1;
    }

    // 等待完成的请求
    ret = io_uring_wait_cqe(&ring, &cqe);
    if (ret < 0) {
        perror("io_uring_wait_cqe");
        return 1;
    }

    // 处理完成的请求
    if (cqe->res < 0) {
        fprintf(stderr, "Async read failed: %d\n", cqe->res);
    } else {
        buffer[cqe->res] = '\0';
        printf("Read %d bytes: %s\n", cqe->res, buffer);
    }

    // 标记完成的请求已处理
    io_uring_cqe_seen(&ring, cqe);

    // 关闭文件
    close(fd);

    // 清理 io_uring
    io_uring_queue_exit(&ring);

    return 0;
}