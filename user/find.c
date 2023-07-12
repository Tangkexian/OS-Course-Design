#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// ���� find ������������ָ��Ŀ¼������Ŀ¼�в����ļ�
void find(char* path, char* filename) {
  int fd;
  struct stat st;
  struct dirent de;
  char buf[512], * p;

  // ��Ŀ¼
  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  // ��ȡ�ļ�״̬
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // �ж��Ƿ�ΪĿ¼
  switch (st.type) {
  case T_FILE: // ����ļ���������ͨ�ļ��������κδ���
    break;

  case T_DIR: // ����ļ�������Ŀ¼����������
    // ���·����̫���������˻������Ĵ�С����ӡ������Ϣ������
    
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) { 
      printf("find: path too long\n");
      break;
    }
    // ��·�������Ƶ���������
    strcpy(buf, path); 
    // ��ָ�� p ָ�򻺳�����ĩβ
    p = buf + strlen(buf); 
    // ��ĩβ���һ��б�ܣ���ΪĿ¼�ķָ���
    *p++ = '/'; 
    // ѭ����ȡĿ¼�е�ÿ����Ŀ��ֱ����������
    while (read(fd, &de, sizeof(de)) == sizeof(de)) { 
      // �����Ŀ�ǿյĻ����ǵ�ǰĿ¼���ϼ�Ŀ¼������������
      if (de.inum == 0 || !strcmp(de.name, ".") || !strcmp(de.name, "..")) 
        continue;
      // ����Ŀ���ļ������Ƶ� p ָ���λ�ã��γ�������·����
      memmove(p, de.name, DIRSIZ); 
      // ���ļ����������һ�����ַ�����Ϊ�ַ����Ľ�����
      p[DIRSIZ] = 0; 
      // ��ȡ��Ŀ���ļ�״̬�����ʧ�ܣ���ӡ������Ϣ��������һ����Ŀ
      if (stat(buf, &st) < 0) { 
        printf("find: cannot stat %s\n", buf);
        continue;
      }
      // ����ҵ���ƥ����ļ�������ӡ·��
      if (!strcmp(de.name, filename))
        printf("%s\n", buf);
      // �����Ŀ¼���ݹ����
      if (st.type == T_DIR)
        find(buf, filename);
    }
    break;
  }
  close(fd);
}

int main(int argc, char* argv[]) {
  // ��������в�������
  if (argc != 3) {
    fprintf(2, "Error: find requires 2 arguments\nRight Usage: find [path] [filename]\n");
    exit(1);
  }
  // ���� find �������в���
  find(argv[1], argv[2]);
  exit(0);
}
