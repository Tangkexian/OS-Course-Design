#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READEND 0
#define WRITEEND 1

// �ӽ��̺���
void child(int* fd) {
    int p, n;
    // �ر�д��
    close(fd[WRITEEND]);
    // �ӹܵ��ж�ȡһ������
    if (read(fd[READEND], &p, sizeof(p)) == 0) {
        // �����ȡʧ�ܣ��رն��˲��˳�
        close(fd[READEND]);
        exit(0);
    }
    // ��ӡ����
    printf("prime %d\n", p);
    // �����µĹܵ�
    int newfd[2];
    pipe(newfd);
    // �����µ��ӽ���
    if (fork() == 0) {
        child(newfd);
    }
    // �ر��¹ܵ��Ķ���
    close(newfd[READEND]);
    // ѭ����ȡ����
    while (read(fd[READEND], &n, sizeof(n)) != 0) {
        // ������ֲ��������ı�����д���¹ܵ�
        if (n % p != 0) {
            write(newfd[WRITEEND], &n, sizeof(n));
        }
    }
    // �رվɹܵ��Ķ��˺��¹ܵ���д��
    close(fd[READEND]);
    close(newfd[WRITEEND]);
    // �ȴ��ӽ����˳�
    wait(0);
    exit(0);
}

int main(int argc, char* argv[]) {
    int fd[2];
    // �����ܵ�
    pipe(fd);
    // �����ӽ���
    if (fork() == 0) {
        child(fd);
    }
    // �رն���
    close(fd[READEND]);
    // ��ܵ���д��2��35������
    for (int i = 2; i <= 35; i++) {
        write(fd[WRITEEND], &i, sizeof(i));
    }
    // �ر�д��
    close(fd[WRITEEND]);
    // �ȴ��ӽ����˳�
    wait(0);
    exit(0);
}
