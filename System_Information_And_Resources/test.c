#include "stdio.h"
#include "errno.h"
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include "time.h"
#include "sys/time.h"
#include "sys/times.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"


int main(void)
{
    struct utsname os_info;
    struct sysinfo sys_info;

    /* 获取操作系统内核名称和信息 */
    if(uname(&os_info)==-1)
    {
        perror("uname error");
        return -1;
    }
    //uname结构体
    // struct utsname {
    // char sysname[]; /* 当前操作系统的名称 */
    // char nodename[]; /* 网络上的名称（主机名） */
    // char release[]; /* 操作系统内核版本 */
    // char version[]; /* 操作系统发行版本 */
    // char machine[]; /* 硬件架构类型 */
    // #ifdef _GNU_SOURCE
    // char domainname[];/* 当前域名 */
    // #endif
    // };

    //gethostname可以直接获取linux系统的主机名，会置位errno。char hostname[20]； gethostname(hostname, sizeof(hostname));
    printf("操作系统名称: %s\n", os_info.sysname);
    printf("主机名: %s\n", os_info.nodename);
    printf("内核版本: %s\n", os_info.release);
    printf("发行版本: %s\n", os_info.version);
    printf("硬件架构: %s\n", os_info.machine);

    /* 获取系统统计信息 */
    if(sysinfo(&sys_info)==-1)
    {
        perror("sysinfo error");
        return -1;
    }
    //sysinfo结构体
    // struct sysinfo {
    // long uptime; /* 自系统启动之后所经过的时间（以秒为单位） */
    // unsigned long loads[3]; /* 1, 5, and 15 minute load averages */
    // unsigned long totalram; /* 总的可用内存大小 */
    // unsigned long freeram; /* 还未被使用的内存大小 */
    // unsigned long sharedram; /* Amount of shared memory */
    // unsigned long bufferram; /* Memory used by buffers */
    // unsigned long totalswap; /* Total swap space size */
    // unsigned long freeswap; /* swap space still available */
    // unsigned short procs; /* 系统当前进程数量 */
    // unsigned long totalhigh; /* Total high memory size */
    // unsigned long freehigh; /* Available high memory size */
    // unsigned int mem_unit; /* 内存单元大小（以字节为单位） */
    // char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding to 64 bytes */
    // };

    printf("uptime: %ld\n", sys_info.uptime);
    printf("totalram: %lu\n", sys_info.totalram);
    printf("freeram: %lu\n", sys_info.freeram);
    printf("procs: %u\n", sys_info.procs);

    /* 获取系统的一些配置信息 */
    //sysconf传入参数的含义：
    // ⚫ _SC_ARG_MAX：exec 族函数的参数的最大长度，exec 族函数后面会介绍，这里先不管！
    // ⚫ _SC_CHILD_MAX：每个用户的最大并发进程数，也就是同一个用户可以同时运行的最大进程数。
    // ⚫ _SC_HOST_NAME_MAX：主机名的最大长度。
    // ⚫ _SC_LOGIN_NAME_MAX：登录名的最大长度。
    // ⚫ _SC_CLK_TCK：每秒时钟滴答数，也就是系统节拍率。 
    // ⚫ _SC_OPEN_MAX：一个进程可以打开的最大文件数。
    // ⚫ _SC_PAGESIZE：系统页大小（page size）。
    // ⚫ _SC_TTY_NAME_MAX：终端设备名称的最大长度。

    long sc_child_max=0;

    sc_child_max=sysconf(_SC_CHILD_MAX);
    if(sc_child_max==-1)
    {
        perror("get sc_child_max error");
        return 0;
    }
    printf("个用户的最大并发进程数为:%ld\n",sc_child_max);
    
    /* 获取时间(秒数) */
    time_t t;
    t=time(NULL);
    if(t==-1)
    {
        perror("get time error");
    }
    printf("UTC second:%ld\n",t);

    char timebuff[100]={0};
    /* 将时间秒数转换为特定格式 */

    //转化为字符串格式
    ctime_r(&t,timebuff);
    printf("time : %s\n",timebuff);

    //转化并储存到结构体中
    struct tm tm;
    //tm的结构体

    // struct tm {
    // int tm_sec; /* 秒(0-60) */
    // int tm_min; /* 分(0-59) */
    // int tm_hour; /* 时(0-23) */
    // int tm_mday; /* 日(1-31) */
    // int tm_mon; /* 月(0-11) */
    // int tm_year; /* 年(这个值表示的是自 1900 年到现在经过的年数) */
    // int tm_wday; /* 星期(0-6, 星期日 Sunday = 0、星期一=1…) */
    // int tm_yday; /* 一年里的第几天(0-365, 1 Jan = 0) */
    // int tm_isdst; /* 夏令时 */
    // };

    //localtime是获取本地时间，gmtime是获取UTC国际标准时间
    localtime_r(&t,&tm);    
    printf("当前时间: %d 年%d 月%d 日 %d:%d:%d\n",tm.tm_year+1900,tm.tm_mon,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);

    //mktime:把结构体的数据转化为秒数
    printf("转换得到的秒数: %ld\n", mktime(&tm));

    //将 struct tm 表示的分解时间转换为固定格式的字符串
    char time2buff[100]={0};
    asctime_r(&tm,time2buff);
    printf("本地时间：%s",time2buff);

    //功能更强大的将 struct tm 表示的分解时间转换为固定格式的字符串,可自定义输出格式,具体格式由于太多，到时候需要用到在查
    char time3buff[100]={0};
    strftime(time3buff, sizeof(time3buff), "%Y-%m-%d %A %H:%M:%S", &tm);
    printf("本地时间: %s\n", time3buff);

    /* 设置时间 */
    //int settimeofday(const struct timeval *tv, NULL);
    //只有超级用户（root）才可以设置系统时间，普通用户将无操作权限。

    /* 获取进程时间 */

    //墙上时间（Wall Time / Real Time）,从外部钟表看，进程从开始到结束用了多久,包括：等待 IO、等待 CPU、睡觉、阻塞
    //用户时间（User Time）:进程在用户态执行自己代码的时间,你的业务逻辑、循环、计算、算法
    //系统时间（System Time / Sys Time）:进程进入内核执行系统调用的时间,比如：open、read、write、malloc、fork、sleep
    //三者加起来为总时间
    struct tms time_start,time_end;
    clock_t t_start,t_end;

    //tms结构体成员：
    // struct tms {
    // clock_t tms_utime; /* user time, 进程的用户 CPU 时间, tms_utime 个系统节拍数 */
    // clock_t tms_stime; /* system time, 进程的系统 CPU 时间, tms_stime 个系统节拍数 */
    // clock_t tms_cutime; /* user time of children, 已死掉子进程的 tms_utime + tms_cutime 时间总和 */
    // clock_t tms_cstime; /* system time of children, 已死掉子进程的 tms_stime + tms_cstime 时间总和 */
    // };


    //返回从过去任意的一个时间点（譬如系统启动时间）所经过的时钟滴答数
    t_start=times(&time_start);
    if(t_start==-1)
    {
        perror("get times_start error");
    }

    int fd1=open("file1",O_CREAT|O_EXCL|O_RDWR,S_IRWXU);
    if(fd1==-1)
    {
        perror("open file error");
        return 0;
    }
    close(fd1);

    for (int i = 0; i < 20000; i++)
    {
        for(int j=0;j<10000;j++);
    }
    /* 程序休眠 */
    sleep(1);
    //还有精度更高的微秒级休眠: usleep；高精度休眠: nanosleep

    t_end=times(&time_end);
    if(t_end==-1)
    {
        perror("get times_start error");
    }

    long tck=sysconf(_SC_CLK_TCK);
    if(tck==-1)
    {
        perror("get sys clk_tck error");
        return 0;
    }

    printf("total time:%fs\n",(t_end-t_start)/(double)tck);
    printf("t_utime:%fs\n",(time_end.tms_utime-time_start.tms_utime)/(double)tck);
    printf("t_stime:%fs\n",(time_end.tms_stime-time_start.tms_stime)/(double)tck);

    /* 设置随机数 */
    //rand()的实质是算法算出来的，这样每次运行一次程序的随机数都一样，需要给随机数种子
    srand(times(NULL));
    int rand_buff[3];
    for(int i=0;i<3;i++)
    {
        rand_buff[i]=rand()%100;
        printf("rand_buff[%d]:%d\n",i,rand_buff[i]);
    }

    /* 内存分配 */
    //如果不需要内存初始化的话可以用malloc
    char *buf=(char*)calloc(10,2);
    if(buf==NULL)
    {
        printf("calloc error\n");
        return 0;
    }
    free(buf);

    int *buf2;
    //分配对齐的内存
    //参数
    // memptr：void **类型的指针，内存申请成功后会将分配的内存地址存放在*memptr 中。
    // alignment：设置内存对其的字节数，alignment 必须是 2 的幂次方（譬如 2^4、2^5、2^8 等），同时也
    // 要是 sizeof(void *)的整数倍。
    // size：设置分配的内存大小，以字节为单位，如果参数 size 等于 0，那么*memptr 中的值是 NULL。
    if(posix_memalign((void**)&buf2,32,256)!=0)
    {
        printf("posix_memalign error\n");
        return 0;
    }
    free(buf2);


    //，Linux 系统中，当一个进程终止时，内核会自动关闭它没有关闭的所有文件（该进程打开的文件，但是在进程终止时未调用 close()关闭它）。
    //这在程序中分配了多块内存的情况下可能会特别有用，因为加入多次对 free()的调用不但会消耗品大量的 CPU 时间，而且可能会使代码趋于复杂。
    //虽然依靠终止进程来自动释放内存对大多数程序来说是可以接受的，但最好能够在程序中显式调用free()释放内存，


    return 0;
}