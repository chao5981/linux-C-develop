#include "sys/stat.h"
#include "fcntl.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/mman.h"
#include "string.h"

int main(void)
{
    int fd1,fd2;
    struct stat stat_buff;
    void *srcaddr;
    void *desaddr;
    fd1=open("src_file",O_RDWR,S_IRWXU);
    if(fd1==-1)
    {
        perror("open src_file error");
        exit(-1);
    }
    fd2=open("des_file",O_CREAT|O_EXCL|O_RDWR,S_IRWXU);
    if(fd2==-1)
    {
        perror("create des_file error");
        close(fd1);
        exit(-1);
    }
    //获取文件属性
    if(fstat(fd1,&stat_buff)==-1)
    {
        perror("fstat error");
        goto out1;
    }
    if(ftruncate(fd2,stat_buff.st_size)==-1)
    {
        perror("ftruncate error");
        goto out1;
    }

    /* 将文件映射到内存 */
    //addr：参数 addr 用于指定映射到内存区域的起始地址。通常将其设置为 NULL
    //length：参数 length 指定映射长度，表示将文件中的多大部分映射到内存区域中，以字节为单位
    //prot：参数 prot 指定了映射区的保护要求，可取值如下：
        // ⚫ PROT_EXEC：映射区可执行；
        // ⚫ PROT_READ：映射区可读；
        // ⚫ PROT_WRITE：映射区可写；
        // ⚫ PROT_NONE：映射区不可访问。
    //flags：参数 flags 可影响映射区的多种属性，参数 flags 必须要指定以下两种标志之一：
        // ⚫ MAP_SHARED：此标志指定当对映射区写入数据时，数据会写入到文件中，也就是会将写入到映射区中的数据更新到文件中，并且允许其它进程共享。
        // ⚫ MAP_PRIVATE：此标志指定当对映射区写入数据时，会创建映射文件的一个私人副本（copy-on-write），对映射区的任何操作都不会更新到文件中，仅仅只是对文件副本进行读写。
    //offset：文件映射的偏移量，通常将其设置为 0，表示从文件头部开始映射；


    //如果自定义的话，参数 addr 和 offset 在不为 NULL 和 0 的情况下，addr 和 offset 的值通常被要求是系统页大小的整数倍
    srcaddr=mmap(NULL,stat_buff.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd1,0);
    if(srcaddr==MAP_FAILED)
    {
        perror("src mmap error");
        goto out1;
    }

    desaddr=mmap(NULL,stat_buff.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd2,0);
    if(desaddr==MAP_FAILED)
    {
        perror("des mmap error");
        goto out2;
    }

    /* 更改映射区的保护请求 */
    if(mprotect(desaddr,stat_buff.st_size,PROT_READ|PROT_WRITE|PROT_EXEC)==-1)
    {
        perror("mprotect error");
        goto out3;
    }

    memcpy(desaddr,srcaddr,stat_buff.st_size);

    /* 数据同步到磁盘 */
    //参数 flags 应指定为 MS_ASYNC 和 MS_SYNC 两个标志之一，除此之外，还可以根据需求选择是否指
    // 定 MS_INVALIDATE 标志，作为一个可选标志。
    // ⚫ MS_ASYNC：以异步方式进行同步操作。调用 msync()函数之后，并不会等待数据完全写入磁盘之
    // 后才返回。
    // ⚫ MS_SYNC：以同步方式进行同步操作。调用 msync()函数之后，需等待数据全部写入磁盘之后才
    // 返回。
    // ⚫ MS_INVALIDATE：是一个可选标志，请求使同一文件的其它映射无效（以便可以用刚写入的新
    // 值更新它们）。
    if(msync(desaddr,stat_buff.st_size,MS_ASYNC)==-1)
    {
        perror("msync error");
        goto out3;
    }
    goto out4;




out1:
    close(fd1);
    close(fd2);
    exit(-1);

out2:
    close(fd1);
    close(fd2);

    /* 解除映射 */
    munmap(srcaddr,stat_buff.st_size);
    exit(-1);

out3:
    close(fd1);
    close(fd2);
    munmap(srcaddr,stat_buff.st_size);
    munmap(desaddr,stat_buff.st_size);
    exit(-1);

out4:
    close(fd1);
    close(fd2);
    munmap(srcaddr,stat_buff.st_size);
    munmap(desaddr,stat_buff.st_size);
    exit(0);
}