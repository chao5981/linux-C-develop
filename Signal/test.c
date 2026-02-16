#include "string.h"
#include "signal.h"
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "sys/types.h"
#include "unistd.h"

pid_t child_pid=-1;

void signal_handler(int sig,siginfo_t *info,void *ucontext)
{
    printf("\n***receive signal!***\n");
    printf("signal serial number:%d,siganl name:%s\n",sig,strsignal(sig));
    /* 标准错误（stderr）上输出信号描述信息 */
    psignal(sig,"signal detailed information");

    switch (sig)
    {
    case SIGUSR1:
        printf("SIGUSR1:child possess pid:%d,the value is %d\n",info->si_pid,info->si_value.sival_int);
        break;
    
    case SIGUSR2:
        printf("SIGUSR2:receive value\n");
        break;

    case SIGALRM:
        printf("alarm 3s timeout\n");
        raise(SIGUSR2);
        break;
    case SIGINT:
        printf("receive abort signal\n");
        abort();
        break;
    case SIGABRT:
        printf("possess stop accidently\n");
        return ;
        break;
    default:
        break;
    }
}

int main(void)
{
    struct sigaction sig_act;
    //信号中断函数类似要void *(int)
    sig_act.sa_sigaction=signal_handler;
    sig_act.sa_flags=SA_SIGINFO;
    int signal[]={SIGUSR1,SIGUSR2,SIGALRM,SIGINT,SIGABRT};
    const char *sig_names[] = {"SIGUSR1", "SIGUSR2", "SIGALRM", "SIGINT", "SIGABRT"};
    for(int i=0;i<sizeof(signal)/sizeof(int);i++)
    {
        /* 设置信号的处理方式 */
        //第一个参数：指定要操作的信号编号，不能传入 SIGKILL（9 号信号）和 SIGSTOP（19 号信号）
        //第二个参数：用来设置新的信号处理行为。如果传入 NULL，表示不修改该信号的处理方式。
        //     struct sigaction {
        //     // 方式1：信号处理函数（最常用）
        //     void     (*sa_handler)(int);  
        //     // 方式2：更灵活的信号处理函数（可获取信号额外信息）
        //     void     (*sa_sigaction)(int, siginfo_t *, void *);
        //     // 信号掩码：处理该信号时，临时屏蔽哪些信号
        //     sigset_t   sa_mask;           
        //     // 行为标志：控制信号处理的细节
        //     int        sa_flags;      SA_RESTART：被信号打断的系统调用（如 read()、write()）自动重启，避免返回错误。
                                        // SA_SIGINFO：使用 sa_sigaction 作为处理函数（而非 sa_handler）。
                                        // SA_NODEFER：处理信号时不屏蔽自身（默认会屏蔽当前信号，避免递归触发）。
                                        // SA_RESETHAND：信号处理完成后，恢复为默认处理行为（类似传统 signal() 的行为）。    
        //     // 已废弃，无需关注
        //     void     (*sa_restorer)(void);
        // };
        // 第三个参数：用来保存该信号原来的处理行为
        if(sigaction(signal[i],&sig_act,NULL)==-1)
        {
            perror("set signal error");
            return -1;
        }
        printf("%s signal is set\n",sig_names[i]);
    }


    sigset_t block_set;
    /* 初始化信号集使其不包含任何信号 */
    if(sigemptyset(&block_set)==-1)
    {
        perror("set block_set empty error");
        return -1;
    }
    /* 向一个信号集里添加信号 */
    if(sigaddset(&block_set,SIGUSR1)==-1)
    {
        perror("add SIGUSR1 to block_set error");
        return -1;
    }
     if(sigaddset(&block_set,SIGALRM)==-1)
    {
        perror("add SIGALRM to block_set error");
        return -1;
    }
    sigset_t full_set;
    /* 数初始化信号集，使其包含所有信号（包括所有实时信号） */
    if(sigfillset(&full_set)==-1)
    {
        perror("set full_set error");
        return -1;
    }
    /* 向信号集里移除一个信号 */
    if(sigdelset(&full_set,SIGINT)==-1)
    {
        perror("delete SIGINT error");
        return -1;
    }

    sigset_t old_set;
    if(sigemptyset(&old_set)==-1)
    {
        perror("set old_set empty error");
        return -1;
    }

    /* 以测试某一个信号是否在指定的信号集里 */
    if(sigismember(&block_set,SIGUSR1)==1)
    {
        printf("SIGUSR1 in the block set\n");
    }

    /* 信号掩码（直接理解为一组要被屏蔽的信号集） */
    //第一个参数：参数 how 指定了调用函数时的一些行为
    //     SIG_BLOCK：将参数 set 所指向的信号集内的所有信号添加到进程的信号掩码中。换言之，将信
    // 号掩码设置为当前值与 set 的并集。
    // ⚫ SIG_UNBLOCK：将参数 set 指向的信号集内的所有信号从进程信号掩码中移除。
    // ⚫ SIG_SETMASK：进程信号掩码直接设置为参数 set 指向的信号集。
    //第二个参数：指向 sigset_t 类型的信号集合，是本次修改掩码的 “操作对象”（即要添加 / 移除 / 替换的信号列表）。
    //如果传入 NULL：表示 “不修改掩码”，此时 how 参数会被忽略，仅用于通过 oldset 获取当前掩码。
    // 第三个参数：指向 sigset_t 类型的指针，用来保存修改前的进程信号掩码。
    //如果传入 NULL：表示不需要保存旧的掩码，仅执行修改操作。
    if(sigprocmask(SIG_BLOCK,&block_set,&old_set)==-1)
    {
        perror("set block_set to mask error");
    }
    
    child_pid=fork();
    if(child_pid==-1)
    {
        perror("create child_pid error");
        return -1;
    }

    if(child_pid==0)
    {
        //子进程会继承父进程的数据，比如这里就继承了父进程的计时器阻塞
        sigset_t child_unblock;
        sigemptyset(&child_unblock);
        sigaddset(&child_unblock, SIGALRM);
        sigprocmask(SIG_UNBLOCK, &child_unblock, NULL);

        pid_t ppid=getppid();
        printf("child possess is running!child pid:%d,parent pid:%d\n",child_pid,ppid);
        /* 定时器函数 */
        //使用 alarm()函数可以设置一个定时器（闹钟），当定时器定时时间到时，内核会向该进程发送 SIGALRM信号
        alarm(1);

        union sigval value;
        value.sival_int=20;

        /* 发送实时信号 */
        //pid：指定接收信号的进程对应的 pid，将信号发送给该进程。
        // sig：指定需要发送的信号。与 kill()函数一样，也可将参数 sig 设置为 0，用于检查参数 pid 所指定的进程是否存在。
        // value：参数 value 指定了信号的伴随数据，union sigval 数据类型。
        if(sigqueue(ppid,SIGUSR1,value)==-1)
        {
            perror("send SIGUSR1 to ppid error");
        }
        /* 发送信号 */
        //可将信号发送给指定的进程或进程组中的每一个进程
        // pid：参数 pid 为正数的情况下，用于指定接收此信号的进程 
        // 如果 pid 为正，则信号 sig 将发送到 pid 指定的进程。
        // ⚫ 如果 pid 等于 0，则将 sig 发送到当前进程的进程组中的每个进程。
        // ⚫ 如果 pid 等于-1，则将 sig 发送到当前进程有权发送信号的每个进程，但进程 1（init）除外。
        // ⚫ 如果 pid 小于-1，则将 sig 发送到 ID 为-pid 的进程组中的每个进程
        // sig：参数 sig 指定需要发送的信号，也可设置为 0，如果参数 sig 设置为 0 则表示不发送信号，但任执行错误检查，这通常可用于检查参数 pid 指定的进程是否存在。
        if(kill(ppid,SIGUSR2)==-1)
        {
            perror("send SIGUSR2 to ppid error");
        }
        /* 进程休眠函数 */
        //以使得进程暂停运行、进入休眠状态，直到进程捕获到一个信号为止
        pause();
        return 0;
    }
    else
    {
        printf("parent possess is running,parent pid:%d,child pid :%d\n",getpid(),child_pid);
        printf("wait 5 second ,wait child possess send signal\n");
        sleep(5);

        sigset_t signal_pending;
        /* 确定进程中处于等待状态的是哪些信号（被阻塞） */
        if(sigpending(&signal_pending)==-1)
        {
            perror("sigpending error");
        }
        for(int i=0;i<sizeof(signal)/sizeof(int);i++)
        {
            if(sigismember(&signal_pending,signal[i])==1)
            {
                printf("%s is in the signal_pending\n",sig_names[i]);
            }
        }
        printf("调用sigsuspend()等待信号...\n");
        /* 原子操作：阻塞等待信号 */
        //会将参数 mask 所指向的信号集来替换进程的信号掩码，也就是将进程的信号掩码设置为参数 mask 所指向的信号集，然后挂起进程，直到捕获到信号被唤醒
        //（如果捕获的信号是 mask 信号集中的成员，将不会唤醒、继续挂起）、并从信号处理函数返回，一旦从信号处理函数返回，sigsuspend()会将进程的信号掩码恢复成调用前的值。
        if(sigsuspend(&old_set) == -1)
        {
            if (errno != EINTR) // 排除正常的信号中断
            {
                perror("sigsuspend error");
                return -1;
            }
            else // EINTR是正常情况，打印提示即可，不退出
            {
                printf("sigsuspend() 被信号正常中断（errno=EINTR）\n");
            }
        }
        printf("调用pause()等待最终信号（按Ctrl+C触发退出）...\n");
        pause();
    }
    return 0;
}
