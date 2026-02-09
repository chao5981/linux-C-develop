/** 使用宏定义 O_DIRECT 需要在程序中定义宏_GNU_SOURCE
** 不然提示 O_DIRECT 找不到 **/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
/** 定义一个用于存放数据的 buf，起始地址以 4096 字节进行对其 **/
static char buf[8192] __attribute((aligned (4096)));

int main(void)
{
    int fd;
    int count;
    /* 打开文件 */
    fd = open("./test_file",O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT,0664);
    if (0 > fd) {
    perror("open error");
    exit(-1);
    }
    /* 写文件 */
    count = 10000;
    while(count--) {
        if (4096 != write(fd, buf, 4096)) {
        perror("write error");
        exit(-1);
        }
    }
    /* 关闭文件退出程序 */
    close(fd);
    exit(0);
}


//直接IO的三个要求
//应用程序中用于存放数据的缓冲区，其内存起始地址必须以块大小的整数倍进行对齐；
//写文件时，文件的位置偏移量必须是块大小的整数倍；
//写入到文件的数据大小必须是块大小的整数倍。

//直接IO一般用于是测试磁盘设备的读写速率，那么在这种应用需要下，我们就需要保证 read/write 操作是直接访问磁盘设备，而不经过内核缓冲，