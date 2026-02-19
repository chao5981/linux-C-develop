
/* 其实文件锁的东西不多，这个练习有点像为了一盘醋包了叠饺子 */
/* 这里只涉及到fcntl的建议锁，用途是锁文件的一部分字节 */
/* 留意一个知识点，文件建议锁 */
//不同进程之间：读锁和写锁遵循 “读共享、写独占”—— 有读锁时加写锁会阻塞，有写锁时加读锁也会阻塞；
//同一个进程内部：不存在 “阻塞”，内核允许你用新锁直接替换自己已加的旧锁（哪怕是读锁换写锁、写锁换读锁）。

#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "sys/wait.h"
#include <sys/types.h>
#include "fcntl.h"
#include "string.h"

#define DATA_FILE "data.txt"



// 存储行的偏移和长度
typedef struct {
    off_t offset;  // 行起始偏移
    size_t length; // 行长度（含换行符）
} LineInfo;


// 第一步：获取指定行的偏移和长度（返回0成功，-1失败）
int get_line_info(int fd, int line_num, LineInfo *info)
{
    if (line_num <= 0 || !info) return -1;

    off_t current_offset = 0;
    int current_line = 0;
    char buf[1024];
    ssize_t n;

    // 定位到文件开头
    lseek(fd, 0, SEEK_SET);

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            if (current_line + 1 == line_num) {
                // 找到目标行的起始偏移
                if (info->offset == 0) info->offset = current_offset + i;
            }
            // 遇到换行符，行号+1
            if (buf[i] == '\n') {
                current_line++;
                // 记录目标行的长度
                if (current_line == line_num) {
                    info->length = (current_offset + i + 1) - info->offset;
                    return 0; // 找到目标行，返回
                }
            }
        }
        current_offset += n;
    }

    // 行号超出文件范围
    return -1;
}

int lockline(int fd,const LineInfo *Lifo,short lock_type)
{
    struct flock lock=
    {
        /* data */
        .l_type=lock_type,
        .l_whence=SEEK_SET,
        .l_start=Lifo->offset,
        .l_len=Lifo->length,
        .l_pid=-1
    };


    /* 给文件上锁 */
    //第一个参数：文件描述符
    //第二个参数：锁命令
    //F_SETLK	设置锁（非阻塞）
    //F_SETLKW	设置锁（阻塞 W=Wait）
    //F_GETLK	获取锁（检查锁），不常用
    //第三个参数：struct flock 结构体
        //⚫ l_type：所希望的锁类型，可以设置为 F_RDLCK、F_WRLCK 和 F_UNLCK 三种类型之一，
            //F_RDLCK表示共享性质的读锁，
            //F_WRLCK 表示独占性质的写锁，
            //F_UNLCK 表示解锁一个区域。
        //l_whence 和 l_start：这两个变量用于指定要加锁或解锁区域的起始字节偏移量，如果锁全部文件，那么就用fstat获取文件大小即可
        //l_len：需要加锁或解锁区域的字节长度。
        //l_pid：一个 pid，指向一个进程，表示该进程持有的锁能阻塞当前进程，当 cmd=F_GETLK 时有效。所以一般可以不用管
    if(fcntl(fd,F_SETLKW,&lock)==-1)
    {
        perror("fcntl fail lock");
        return -1;
    }
    return 0;
}

int read_line(int line_num,int *value)
{
    int fd = open(DATA_FILE, O_RDONLY);
    if (fd == -1)
     {
        perror("open file failed");
        return -1;
    }

    LineInfo info = {0};
    if (get_line_info(fd, line_num, &info) == -1) 
    {
        fprintf(stderr, "Line %d does not exist\n", line_num);
        close(fd);
        return -1;
    }

    if(lockline(fd,&info,F_RDLCK)==-1)
    {
        printf("read_line lockline error\n");
        close(fd);
        return -1;
    }

    // 读取目标行内容
    char buf[1024] = {0};
    lseek(fd, info.offset, SEEK_SET);
    read(fd, buf, info.length);
    // 去掉换行符，转换为整数
    buf[strcspn(buf, "\n")] = '\0';
    *value = atoi(buf);

    printf("[PID: %d] Read line %d: %d (read lock held)\n", getpid(), line_num, *value);
    // 模拟读操作耗时（方便测试多进程并发）
    sleep(2);

    lockline(fd,&info,F_UNLCK);
    close(fd);
    return 0;
}

// 功能2：更新指定行（加写锁）
int update_line(int line_num, int delta) 
{
    int fd = open(DATA_FILE, O_RDWR);
    if (fd == -1) 
    {
        perror("open file failed");
        return -1;
    }
     LineInfo info = {0};
    if (get_line_info(fd, line_num, &info) == -1) 
    {
        fprintf(stderr, "Line %d does not exist\n", line_num);
        close(fd);
        return -1;
    }

    if(lockline(fd,&info,F_WRLCK)==-1)
    {
        printf("update line lockline error\n");
        close(fd);
        return -1;
    }

    // 读取原有值
    char buf[1024] = {0};
    lseek(fd, info.offset, SEEK_SET);
    read(fd, buf, info.length);
    buf[strcspn(buf, "\n")] = '\0';
    int old_value = atoi(buf);
    int new_value = old_value + delta;

    // 写回新值（覆盖原有行）
    lseek(fd, info.offset, SEEK_SET);
    snprintf(buf, sizeof(buf), "%d\n", new_value);
    write(fd, buf, strlen(buf));
    fsync(fd); // 确保写入磁盘

    printf("[PID: %d] Update line %d: %d → %d (write lock held)\n", 
           getpid(), line_num, old_value, new_value);
    // 模拟写操作耗时（方便测试阻塞）
    sleep(3);

    lockline(fd,&info,F_UNLCK);
    close(fd);
    return 0;
}


// 测试函数：创建多进程验证锁特性
void test_locks() {
    pid_t pid1, pid2, pid3, pid4;

    // 进程1：更新第3行（加写锁）
    pid1 = fork();
    if (pid1 == 0) {
        update_line(3, 50);
        exit(0);
    }

    // 进程2：读取第3行（被进程1的写锁阻塞）
    sleep(1); // 确保进程1先加锁
    pid2 = fork();
    if (pid2 == 0) {
        int val;
        read_line(3, &val);
        exit(0);
    }

    // 进程3：读取第5行（不受进程1的锁影响）
    pid3 = fork();
    if (pid3 == 0) {
        int val;
        read_line(5, &val);
        exit(0);
    }

    // 进程4：读取第3行（和进程2共享读锁，等待进程1释放后执行）
    pid4 = fork();
    if (pid4 == 0) {
        int val;
        read_line(3, &val);
        exit(0);
    }

    // 父进程等待所有子进程结束
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);
    waitpid(pid4, NULL, 0);
    printf("All processes finished\n");
}

// 初始化测试文件（创建5行数据）
void init_file() {
    FILE *fp = fopen(DATA_FILE, "w");
    if (!fp) {
        perror("fopen failed");
        exit(1);
    }
    fprintf(fp, "100\n200\n300\n400\n500\n");
    fclose(fp);
}


int main()
{
    init_file();

    test_locks();

    exit(0);
}