#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

int main(int argc, char* argv[]) {
  char buf[512]; // ������
  char* p = buf; // ָ�򻺳�����ָ��
  char* args[MAXARG]; // ��������
  int i, n;

  // �����������С��2��������ӡ��ȷ�÷����˳�
  if (argc < 2) {
    fprintf(2, "Error: xargs requires 1 or more arguments\nRight Usage: xargs command ...\n");
    exit(1);
  }

  // �������в������Ƶ�����������
  for (i = 1; i < argc; i++)
    args[i - 1] = argv[i];

  // ��ȡ��׼����
  while ((n = read(0, p, 1)) > 0) {
    // �����ȡ�����з�
    if (*p == '\n') {
      *p = 0; // �����з��滻Ϊ�ַ���������
      args[argc - 1] = buf; // ���������е��ַ�����Ϊ������ӵ�����������
      args[argc] = 0; // ���ò�������Ľ�����־

      // �����ӽ���ִ������
      if (fork() == 0) {
        exec(args[0], args);
        exit(0);
      }
      else {
        wait(0); // �����̵ȴ��ӽ������
      }
      p = buf; // ����ָ��ָ�򻺳�����ͷ
    }
    else {
      p++; // ָ�����
    }
  }

  exit(0); // �˳�����
}
