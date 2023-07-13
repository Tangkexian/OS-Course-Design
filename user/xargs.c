#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

int main(int argc, char* argv[]) {
  char buf[512]; // 缓冲区
  char* p = buf; // 指向缓冲区的指针
  char* args[MAXARG]; // 参数数组
  int i, n;

  // 如果参数数量小于2，报错，打印正确用法并退出
  if (argc < 2) {
    fprintf(2, "Error: xargs requires 1 or more arguments\nRight Usage: xargs command ...\n");
    exit(1);
  }

  // 将命令行参数复制到参数数组中
  for (i = 1; i < argc; i++)
    args[i - 1] = argv[i];

  // 读取标准输入
  while ((n = read(0, p, 1)) > 0) {
    // 如果读取到换行符
    if (*p == '\n') {
      *p = 0; // 将换行符替换为字符串结束符
      args[argc - 1] = buf; // 将缓冲区中的字符串作为参数添加到参数数组中
      args[argc] = 0; // 设置参数数组的结束标志

      // 创建子进程执行命令
      if (fork() == 0) {
        exec(args[0], args);
        exit(0);
      }
      else {
        wait(0); // 父进程等待子进程完成
      }
      p = buf; // 重置指针指向缓冲区开头
    }
    else {
      p++; // 指针后移
    }
  }

  exit(0); // 退出程序
}
