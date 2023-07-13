#include "kernel/types.h" // 引入内核定义的类型，如 int, char 等
#include "kernel/stat.h"  // 引入内核定义的状态码，如 T_DIR, T_FILE 等
#include "user/user.h"    // 引入用户程序可用的函数，如 printf, fork, pipe 等

int
main(int argc, char* argv[])
{
  int p2c[2], c2p[2]; // 定义两个整数数组，分别表示父进程到子进程和子进程到父进程的管道
  char buf;           // 定义一个字符变量，用于存储从管道中读取或写入的数据

  // 检查命令行参数的数量
  if (argc != 1) {
    fprintf(2, "Error: pingpong requires 0 argument\nRight Usage: %s\n", argv[0]);
    exit(1);
  }

  // 创建两个管道，每个管道有两个文件描述符，分别表示读端和写端
  pipe(p2c); // p2c[0] 是读端，p2c[1] 是写端
  pipe(c2p); // c2p[0] 是读端，c2p[1] 是写端

  if (fork() == 0) { // 创建一个子进程，并判断是否是子进程
    // 子进程
    read(p2c[0], &buf, 1); // 从父进程到子进程的管道的读端读取一个字节的数据，并存储到 buf 中
    printf("%d: received ping\n", getpid()); // 打印当前进程的 ID 和收到的消息
    write(c2p[1], &buf, 1); // 将 buf 中的数据写入到子进程到父进程的管道的写端
    exit(0); // 结束子进程，并返回 0 表示正常退出
  }
  else {
    // 父进程
    write(p2c[1], "p", 1); // 将字符 'p' 写入到父进程到子进程的管道的写端
    read(c2p[0], &buf, 1); // 从子进程到父进程的管道的读端读取一个字节的数据，并存储到 buf 中
    printf("%d: received pong\n", getpid()); // 打印当前进程的 ID 和收到的消息
    exit(0); // 结束父进程，并返回 0 表示正常退出
  }
}
