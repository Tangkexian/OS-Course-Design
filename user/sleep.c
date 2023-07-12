#include "kernel/types.h" // 包含内核类型定义
#include "user.h" // 包含用户库函数

int main(int argc, char* argv[]) {
  // 检查传递给程序的参数个数是否为 2（包括程序名称本身）
  if (argc != 2) {
    // 如果不是，则打印一条错误信息和一条使用信息
    fprintf(2, "Error: sleep requires 1 argument\nRight Usage: sleep ticks\n");
    // 然后以非零状态码退出
    exit(1);
  }
  // 使用 atoi 函数将传递给程序的第一个参数（argv[1]）转换为整数
  int ticks = atoi(argv[1]);
  // 调用 sleep 系统调用，使程序休眠 ticks 个时钟周期
  sleep(ticks);
  // 调用 exit 函数以零状态码退出程序
  exit(0);
}
