#include "sys/types.h"
#include "sys/stat.h"
#include "unistd.h"
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "time.h"
#include "sys/time.h"
#include "fcntl.h"

int main(void)
{

    //补充一个低级错误：struct stat *file_stat;只定义了一个指针变量（4/8 字节），但指针本身指向随机的非法地址（野指针）
    //这样只是创建了地址，但没有创建内存，必然报bad address错误


    struct stat file_stat;
    /* stat结构体 */
//     struct stat {
//     dev_t     st_dev;         // 设备ID（文件所在的设备），类型：dev_t（无符号整数，通常是32位）
//     ino_t     st_ino;         // inode号，类型：ino_t（无符号整数，通常是64位）
//     mode_t    st_mode;        // 文件类型和权限，类型：mode_t（无符号整数，通常是16/32位）
//     nlink_t   st_nlink;       // 硬链接数，类型：nlink_t（无符号整数，通常是32位）
//     uid_t     st_uid;         // 文件所有者的用户ID，类型：uid_t（无符号整数，通常是32位）
//     gid_t     st_gid;         // 文件所有者的组ID，类型：gid_t（无符号整数，通常是32位）
//     dev_t     st_rdev;        // 特殊文件（如设备文件）的设备ID，类型：dev_t
//     off_t     st_size;        // 文件大小（字节），类型：off_t（有符号整数，通常是64位）
//     blksize_t st_blksize;     // 文件系统I/O的块大小，类型：blksize_t（无符号整数）
//     blkcnt_t  st_blocks;      // 占用的磁盘块数（每块通常512字节），类型：blkcnt_t（无符号整数）
//     time_t    st_atime;       // 最后访问时间，类型：time_t（长整型，存储Unix时间戳），也就是被读取的时间
//     time_t    st_mtime;       // 最后修改时间，类型：time_t，文件内容发生改变的时间
//     time_t    st_ctime;       // 最后状态改变时间（如权限/所有者修改），类型：time_t，文件的inode节点最后一次被修改的时间
// };


    //fstat和stat类似，而 fstat 函数则是从文件描述符出发得到文件属性信息，所以使用 fstat 函数之前需要先打开文件得到文件描述符
    //lstat()与 stat、fstat 的区别在于，对于符号链接文件，stat、fstat 查阅的是符号链接文件所指向的文件对应的文件属性信息，而 lstat 查阅的是符号链接文件本身的属性信息。
    if(stat("file.txt",&file_stat)==-1)
    {
        perror("find file.txt stat error");
        return 0;
    }
    printf("inode:%ld,size:%ld\n",file_stat.st_ino,file_stat.st_size);

    //检查文件是否有有权限用与操作
    if(file_stat.st_mode&S_IWOTH)
    {
        printf("其它用户具有可写权限\n");
    }
    else
    {
        printf("其它用户不具有可写权限\n");
    }
    if(file_stat.st_mode&S_IROTH)
    {
        printf("其它用户具有可读权限\n");
    }
    else
    {
        printf("其它用户不具有可读权限\n");
    }
    //打印时间戳,用localtime()/localtime_r()和 strftime()来得到更利于我们查看的时间表达方式
    struct tm* local_time=localtime(&file_stat.st_atime);
    char time_str[100];
    strftime(time_str,sizeof(time_str),"%Y-%m-%d %H:%M:%S",local_time);
    printf("最后访问时间：%s\n",time_str);
    local_time=localtime(&file_stat.st_mtime);
    strftime(time_str,sizeof(time_str),"%Y-%m-%d %H:%M:%S",local_time);
    printf("最后修改时间：%s\n",time_str);
    local_time=localtime(&file_stat.st_ctime);
    strftime(time_str,sizeof(time_str),"%Y-%m-%d %H:%M:%S",local_time);
    printf("最后状态改变时间：%s\n",time_str);

    //getuid/getgid：获取当前运行进程的 “实际用户 ID”（Real UID）/“用户组ID”，也就是启动这个进程的用户的 ID/用户组ID。
    printf("uid:%d, gid:%d\n",getuid(),getgid());

    //修改文件的权限为root
    //fchown()、lchown()这两个函数与 chown()的区别就像是 fstat()、lstat()与 stat 的区别
    // if(chown("file.txt",0,0)==-1)
    if(chown("file.txt",1000,1000)==-1)
    {
        perror("chown error");
    }

    /* 文件类型和权限 */
    //文件类型宏定义为：
    // S_IFSOCK 0140000 socket（套接字文件）
    // S_IFLNK 0120000 symbolic link（链接文件）
    // S_IFREG 0100000 regular file（普通文件）
    // S_IFBLK 0060000 block device（块设备文件）
    // S_IFDIR 0040000 directory（目录）
    // S_IFCHR 0020000 character device（字符设备文件）
    // S_IFIFO 0010000 FIFO（管道文件）

    //文件权限宏定义为：
    // S_IRWXU 00700 owner has read, write, and execute permission
    // S_IRUSR 00400 owner has read permission
    // S_IWUSR 00200 owner has write permission
    // S_IXUSR 00100 owner has execute permission
    // S_IRWXG 00070 group has read, write, and execute permission
    // S_IRGRP 00040 group has read permission
    // S_IWGRP 00020 group has write permission
    // S_IXGRP 00010 group has execute permission
    // S_IRWXO 00007 others (not in group) have read, write, and execute permission
    // S_IROTH 00004 others have read permission
    // S_IWOTH 00002 others have write permission
    // S_IXOTH 00001 others have execute permission

    //文件还有特殊权限，宏定义如下：
    // S_ISUID 04000 set-user-ID bit        当一个文件设置了 SUID，谁运行它，运行时的进程就会临时变成文件的所有者。
    // S_ISGID 02000 set-group-ID bit (see below)       当文件设置了 SGID，谁运行它，运行时的进程就会临时变成文件的所属组。
    // S_ISVTX 01000 sticky bit (see below)         用的很少

    /* 目录权限 */
    //读写和执行权限

    //access:检查当前进程（实际用户 ID）是否有权限访问某个文件 / 设备」的函数，它直接对接你之前学的文件权限规则，但判断依据是「实际 UID/GID」而非「有效 UID/GID」,检查项通过则返回 0
    if(access("file.txt",F_OK)==0)
    {
        printf("file exist\n");
    }
    else
    {
        printf("file don`t exist\n");
    }
    
    /* chmod修改文件权限 */
    //修改文件权限，和终端命令一样
    if(chmod("file.txt",0666)==-1)
    {
        perror("chmod file.txt error");
    }

    //umask命令用于查看/设置权限掩码，权限掩码主要用于对新建文件的权限进行屏蔽

    /* 文件的时间属性 */
    //只能显式修改文件的最后一次访问时间和文件内容最后被修改的时间，不能显式修改文件状态最后被改变的时间，当你修改任意前俩者时，后者会发生改变

    //utime和utimes:只能修改为当前时间戳，并且需要同时修改俩个，不能只动一个；后者可以精确到毫秒
    //futimens和utimensat的区别是前者需要文件描述符，后者需要指定路径，俩者可以修改时间戳为任意时间戳，且有俩个特殊的宏
    //UTIME_OMIT：任一数组元素的 tv_nsec 字段的值设置为 UTIME_OMIT，则表示相应的时间戳保持不变，此时忽略 tv_sec 字段。
    //UTIME_NOW：任一数组元素的 tv_nsec 字段的值设置为 UTIME_NOW，则表示相应的时间戳设置为当前时间，此时忽略相应的 tv_sec 字段。
    //NULL：表示将访问时间和修改时间都设置为当前时间。
    //tv_nsec:纳秒          tv_sec:秒

    //utimensat参数：int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags);
    // dirfd：目录文件描述符（一般传 AT_FDCWD，表示使用绝对路径/当前工作目录的相对路径）
    // pathname：文件路径（相对/绝对路径，比如 "./test_utimensat.txt" 或 "/tmp/test.txt"）
    // times：时间戳数组（times[0]=atime，times[1]=mtime），规则和 futimens 一致：
    //   - tv_nsec=UTIME_NOW：对应时间戳更新为当前系统时间
    //   - tv_nsec=UTIME_OMIT：对应时间戳不修改
    //   - tv_nsec=普通值（如0）：设置为 tv_sec 指定的Unix时间戳
    // flags：一般传0，特殊场景可用 AT_SYMLINK_NOFOLLOW（不跟随符号链接）
    // 返回值：成功返回0，失败返回-1（errno 存错误码）

    //需要引用fcntl.h头文件

    struct timespec time[2];
    time[0].tv_sec=1735689600;
    time[0].tv_nsec=0;
    
    time[1].tv_nsec=UTIME_NOW;
    time[1].tv_sec=0;

    if(utimensat(AT_FDCWD,"file.txt",time,0)==-1)
    {
        perror("timensat file error");
    }
    

    /* 创建链接 */
    // 1. 硬链接（Hard Link）
    // 创建逻辑：在目录中新增一条 “文件名→原有 Inode” 的映射（给同一个 Inode 加新名字）；
    // 关键特性：
    // ① 硬链接和原文件共用同一个 Inode（Inode 号相同）；
    // ② 删除原文件，硬链接仍能访问数据（只要还有至少一个硬链接指向该 Inode，数据就不会被删除）；
    // ③ 不能跨文件系统（比如从 ext4 到 tmpfs），不能链接目录（Linux 禁止，避免循环引用）；
    // ④ 没有独立的权限 / 时间戳（因为和原文件共用 Inode，元数据完全一致）。

    // 2. 软链接（Symbolic Link）
    // 创建逻辑：新建一个「独立的小文件」，文件内容是原文件的绝对 / 相对路径（相当于快捷方式）；
    // 关键特性：
    // ① 软链接有自己独立的 Inode（Inode 号和原文件不同）；
    // ② 删除原文件，软链接会变成 “死链接”（指向的路径不存在，无法访问数据）；
    // ③ 可以跨文件系统，可以链接目录（比如 ln -s /home/test /tmp/test_link）；
    // ④ 有独立的权限 / 时间戳（但权限无实际意义，访问权限由原文件决定）。

    if(link("file.txt","hard1")==-1)
    {
        perror("link file error");
    }
    if(symlink("file.txt","soft1")==-1)
    {
        perror("symlink file error");
    }
    else
    {
        //读取硬链接文件可以直接read，因为是直接指向inode节点
        //但若想读取软链接文件的路径信息，需要下列函数
        char buff[10];
        if(readlink("soft1",buff,sizeof(buff))==-1)
        {
            perror("read soft link error");
        }
        else
        {
            printf("%s\n",buff);
        }
    }

    /* 删除文件 */
    if(unlink("soft1")==-1)     //或者remove
    {
        perror("unlink soft1 error");
    }

    /* 文件重命名 */
    if(rename("hard1","hard2")==-1)
    {
        perror("rename hard1 error");
    }
    

    return 0;
}
