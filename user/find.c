#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 定义 find 函数，用于在指定目录及其子目录中查找文件
void find(char* path, char* filename) {
  int fd;
  struct stat st;
  struct dirent de;
  char buf[512], * p;

  // 打开目录
  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  // 获取文件状态
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // 判断是否为目录
  switch (st.type) {
  case T_FILE: // 如果文件类型是普通文件，不做任何处理
    break;

  case T_DIR: // 如果文件类型是目录，继续处理
    // 如果路径名太长，超过了缓冲区的大小，打印错误信息并跳出
    
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) { 
      printf("find: path too long\n");
      break;
    }
    // 把路径名复制到缓冲区中
    strcpy(buf, path); 
    // 把指针 p 指向缓冲区的末尾
    p = buf + strlen(buf); 
    // 在末尾添加一个斜杠，作为目录的分隔符
    *p++ = '/'; 
    // 循环读取目录中的每个条目，直到读完或出错
    while (read(fd, &de, sizeof(de)) == sizeof(de)) { 
      // 如果条目是空的或者是当前目录或上级目录，跳过不处理
      if (de.inum == 0 || !strcmp(de.name, ".") || !strcmp(de.name, "..")) 
        continue;
      // 把条目的文件名复制到 p 指向的位置，形成完整的路径名
      memmove(p, de.name, DIRSIZ); 
      // 在文件名后面添加一个空字符，作为字符串的结束符
      p[DIRSIZ] = 0; 
      // 获取条目的文件状态，如果失败，打印错误信息并继续下一个条目
      if (stat(buf, &st) < 0) { 
        printf("find: cannot stat %s\n", buf);
        continue;
      }
      // 如果找到了匹配的文件名，打印路径
      if (!strcmp(de.name, filename))
        printf("%s\n", buf);
      // 如果是目录，递归查找
      if (st.type == T_DIR)
        find(buf, filename);
    }
    break;
  }
  close(fd);
}

int main(int argc, char* argv[]) {
  // 检查命令行参数个数
  if (argc != 3) {
    fprintf(2, "Error: find requires 2 arguments\nRight Usage: find [path] [filename]\n");
    exit(1);
  }
  // 调用 find 函数进行查找
  find(argv[1], argv[2]);
  exit(0);
}
