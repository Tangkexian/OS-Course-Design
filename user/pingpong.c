#include "kernel/types.h" // �����ں˶�������ͣ��� int, char ��
#include "kernel/stat.h"  // �����ں˶����״̬�룬�� T_DIR, T_FILE ��
#include "user/user.h"    // �����û�������õĺ������� printf, fork, pipe ��

int
main(int argc, char* argv[])
{
  int p2c[2], c2p[2]; // ���������������飬�ֱ��ʾ�����̵��ӽ��̺��ӽ��̵������̵Ĺܵ�
  char buf;           // ����һ���ַ����������ڴ洢�ӹܵ��ж�ȡ��д�������

  // ��������в���������
  if (argc != 1) {
    fprintf(2, "Error: pingpong requires 0 argument\nRight Usage: %s\n", argv[0]);
    exit(1);
  }

  // ���������ܵ���ÿ���ܵ��������ļ����������ֱ��ʾ���˺�д��
  pipe(p2c); // p2c[0] �Ƕ��ˣ�p2c[1] ��д��
  pipe(c2p); // c2p[0] �Ƕ��ˣ�c2p[1] ��д��

  if (fork() == 0) { // ����һ���ӽ��̣����ж��Ƿ����ӽ���
    // �ӽ���
    read(p2c[0], &buf, 1); // �Ӹ����̵��ӽ��̵Ĺܵ��Ķ��˶�ȡһ���ֽڵ����ݣ����洢�� buf ��
    printf("%d: received ping\n", getpid()); // ��ӡ��ǰ���̵� ID ���յ�����Ϣ
    write(c2p[1], &buf, 1); // �� buf �е�����д�뵽�ӽ��̵������̵Ĺܵ���д��
    exit(0); // �����ӽ��̣������� 0 ��ʾ�����˳�
  }
  else {
    // ������
    write(p2c[1], "p", 1); // ���ַ� 'p' д�뵽�����̵��ӽ��̵Ĺܵ���д��
    read(c2p[0], &buf, 1); // ���ӽ��̵������̵Ĺܵ��Ķ��˶�ȡһ���ֽڵ����ݣ����洢�� buf ��
    printf("%d: received pong\n", getpid()); // ��ӡ��ǰ���̵� ID ���յ�����Ϣ
    exit(0); // ���������̣������� 0 ��ʾ�����˳�
  }
}
