#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(int argc, char* argv[])
{
    // 参数错误
    if (argc != 1)
    {
        fprintf(2, "Error: sysinfo requires 0 argument\nRight Usage:%s sysinfo \n", argv[0]);
        exit(1);
    }

    struct sysinfo info;
    sysinfo(&info);
    // 打印系统信息
    printf("Available space:%d\nUsed Process:%d\n", info.freemem, info.nproc);
    exit(0);
}
