#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READEND 0
#define WRITEEND 1

// 子进程函数
void child(int* fd) {
    int p, n;
    // 关闭写端
    close(fd[WRITEEND]);
    // 从管道中读取一个素数
    if (read(fd[READEND], &p, sizeof(p)) == 0) {
        // 如果读取失败，关闭读端并退出
        close(fd[READEND]);
        exit(0);
    }
    // 打印素数
    printf("prime %d\n", p);
    // 创建新的管道
    int newfd[2];
    pipe(newfd);
    // 创建新的子进程
    if (fork() == 0) {
        child(newfd);
    }
    // 关闭新管道的读端
    close(newfd[READEND]);
    // 循环读取数字
    while (read(fd[READEND], &n, sizeof(n)) != 0) {
        // 如果数字不是素数的倍数，写入新管道
        if (n % p != 0) {
            write(newfd[WRITEEND], &n, sizeof(n));
        }
    }
    // 关闭旧管道的读端和新管道的写端
    close(fd[READEND]);
    close(newfd[WRITEEND]);
    // 等待子进程退出
    wait(0);
    exit(0);
}

int main(int argc, char* argv[]) {
    int fd[2];
    // 创建管道
    pipe(fd);
    // 创建子进程
    if (fork() == 0) {
        child(fd);
    }
    // 关闭读端
    close(fd[READEND]);
    // 向管道中写入2到35的数字
    for (int i = 2; i <= 35; i++) {
        write(fd[WRITEEND], &i, sizeof(i));
    }
    // 关闭写端
    close(fd[WRITEEND]);
    // 等待子进程退出
    wait(0);
    exit(0);
}
