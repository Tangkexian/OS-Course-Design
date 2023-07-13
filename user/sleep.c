#include "kernel/types.h" // �����ں����Ͷ���
#include "user.h" // �����û��⺯��

int main(int argc, char* argv[]) {
  // ��鴫�ݸ�����Ĳ��������Ƿ�Ϊ 2�������������Ʊ���
  if (argc != 2) {
    // ������ǣ����ӡһ��������Ϣ��һ��ʹ����Ϣ
    fprintf(2, "Error: sleep requires 1 argument\nRight Usage: sleep ticks\n");
    // Ȼ���Է���״̬���˳�
    exit(1);
  }
  // ʹ�� atoi ���������ݸ�����ĵ�һ��������argv[1]��ת��Ϊ����
  int ticks = atoi(argv[1]);
  // ���� sleep ϵͳ���ã�ʹ�������� ticks ��ʱ������
  sleep(ticks);
  // ���� exit ��������״̬���˳�����
  exit(0);
}
