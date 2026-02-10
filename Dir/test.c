#include "sys/stat.h"
#include "sys/types.h"
#include "stdio.h"
#include "errno.h"
#include "dirent.h"
#include "unistd.h"


int main(void)
{
    //文件夹的操作和文件类似，毕竟在linux中，文件夹也是一种文件，第二个参数权限和文件的权限一样
    if(mkdir("Dir_test",S_IRWXU)==-1)
    {
        perror("create dir error");
    }
    if(mkdir("Dir1_test",S_IRWXU)==-1)
    {
        perror("create dir1 error");
    }
    DIR *dir=opendir("Dir_test");
    if(dir==NULL)
    {
        perror("open dir error");
    }
    if(rmdir("Dir1_test")==-1)
    {
        perror("remove dir1_test error");
    }

    //一个区别：传入指针时需要有内存，不然就是野指针，所以传入指针时一般都是取地址；
    //相比之下，函数返回一个指针时，即函数已经有开辟了一片内存了，不需要自己创建，只需要一个指针去接收即可。
    struct dirent *dirp;
    //struct dirent结构体内容如下所示:
    // struct dirent {
    // ino_t d_ino; /* inode 编号 */
    // off_t d_off; /* not an offset; see NOTES */
    // unsigned short d_reclen; /* length of this record */
    // unsigned char d_type; /* type of file; not supported by all filesystem types */
    // char d_name[256]; /* 文件名 */
    // };

    //当使用 opendir()打开目录时，目录流将指向了目录列表的头部（0），使用 readdir()读取一条目录条目之后，目录流将会向后移动、指向下一个目录条目。
    dirp=readdir(dir);
    if(dirp==NULL)
    {
        perror("read dir error");
    }
    printf("innode:%ld,dir name:%s\n",dirp->d_ino,dirp->d_name);

    //类似与lseek(),但是无返回值
    rewinddir(dir);

    char buffer[50];
    //获取进程的当前工作目录，返回值为指向 buf 的指针
    if(getcwd(buffer,sizeof(buffer))==NULL)
    {
        perror("get current working dir error");
    }
    printf("current working directory is :%s\n",buffer);
    
    //改变当前工作目录
    if(chdir("Dir_test")==-1)       //或者用 fchdir(int fd);将进程的当前工作目录更改为 fd 文件描述符所指定的目录（譬如使用 open 函数打开一个目录）
    {
        perror("change current working dir error");
    }

    if(getcwd(buffer,sizeof(buffer))==NULL)
    {
        perror("get now current working dir error");
    }
    printf("now current working directory is :%s\n",buffer);


    closedir(dir);
    return 0;
}