#include "pthread.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "unistd.h"

/* *****  注意：此篇的目的是熟悉相关函数，绝非实际用途，用于演示  ****** */
/* *****  特别是重复上锁，一个内容重复上锁，子线程不退出不取消  ***** */


// 缓存结构体定义
typedef struct {
    int data[100];          // 缓存数据区
    int data_count;         // 有效数据数量
    
    pthread_mutex_t mutex;  // 互斥锁：保护缓存整体操作
    pthread_cond_t cond;    // 条件变量：通知数据就绪/缓存有空位
    
    pthread_spinlock_t spinlock; // 自旋锁：保护高频次的count增减
    pthread_rwlock_t rwlock;     // 读写锁：读线程共享读取，写线程独占写入
} DataCache;

DataCache cache;
pthread_mutexattr_t mutexattr;
pthread_rwlockattr_t rwattr;

void Lock_init(void);
void *write_thread_func(void *arg);
void *read_thread_func(void *arg);
void *monitor_thread_func(void *arg);
void destroy_Lock(void);


int main(void)
{

    int ret=0;

    Lock_init();

    pthread_t write_thread[2];
    pthread_t read_thread[3];
    pthread_t monitor_thread;

    // 创建2个写线程
    for (int i = 0; i < 2; i++) 
    {
        int* tid = malloc(sizeof(int));
        *tid = i + 1;
        ret=pthread_create(&write_thread[i], NULL, write_thread_func, tid);
        if(ret!=0)
        {
            printf("create write thread error:%s",strerror(ret));
            exit(-1);
        }
    }
    
    // 创建3个读线程
    for (int i = 0; i < 3; i++) 
    {
        int* tid = malloc(sizeof(int));
        *tid = i + 1;
        ret=pthread_create(&read_thread[i], NULL, read_thread_func, tid);
        if(ret!=0)
        {
            printf("create read thread error:%s",strerror(ret));
            exit(-1);
        }
    }
    
    // 创建监控线程
    ret=pthread_create(&monitor_thread, NULL, monitor_thread_func, NULL);
    if(ret!=0)
    {
        printf("create read thread error:%s",strerror(ret));
        exit(-1);
    }

    pthread_join(monitor_thread,NULL);
    printf("主线程：监控线程已退出，准备清理资源并退出\n");

    destroy_Lock();

    exit(0);
}

void Lock_init(void)
{
    cache.data_count=0;
    int ret=0;

    /* 互斥锁属性初始化 */
    pthread_mutexattr_init(&mutexattr);

    /* 互斥锁属性设置 */
    //PTHREAD_MUTEX_NORMAL：一种标准的互斥锁类型，不做任何的错误检查或死锁检测。
    //PTHREAD_MUTEX_ERRORCHECK：此类互斥锁会提供错误检查，这类互斥锁运行起来比较慢，可用于调试
    //PTHREAD_MUTEX_RECURSIVE：此类互斥锁允许同一线程在互斥锁解锁之前对该互斥锁进行多次加锁，然后维护互斥锁加锁的次数，把这种互斥锁称为递归互斥锁
    //但是如果解锁次数不等于加速次数，则是不会释放锁的
    ret=pthread_mutexattr_settype(&mutexattr,PTHREAD_MUTEX_NORMAL);
    if(ret!=0)
    {
        printf("set mutex attr error:%s\n",strerror(ret));
        exit(-1);
    }
    int type;
    pthread_mutexattr_gettype(&mutexattr,&type);
    if(type==PTHREAD_MUTEX_RECURSIVE)
    {
        printf("the mutex type is PTHREAD_MUTEX_RECURSIVE\n");
    }
    else
    {
        printf("the mutex type is %s\n",type==PTHREAD_MUTEX_NORMAL?"PTHREAD_MUTEX_NORMAL":"PTHREAD_MUTEX_ERRORCHECK");
    }

    /* 互斥锁初始化 */
    //第二个参数是互斥锁的属性，NULL表示默认属性
    ret=pthread_mutex_init(&cache.mutex,&mutexattr);
    if(ret!=0)
    {
        printf("create mutex lock error:%s\n",strerror(ret));
        exit(-1);
    }


    /* 条件变量初始化 */
    //第二个参数是条件变量的属性，为进程共享属性和时钟属性，比较少用
    ret=pthread_cond_init(&cache.cond,NULL);
    if(ret!=0)
    {
        printf("create cond lock error:%s\n",strerror(ret));
        exit(-1);
    }

    /* 自旋锁初始化 */
    //第二个变量是自旋锁的属性
    //PTHREAD_PROCESS_SHARED：共享自旋锁。该自旋锁可以在多个进程中的线程之间共享；
    //PTHREAD_PROCESS_PRIVATE：私有自旋锁。只有本进程内的线程才能够使用该自旋锁。
    ret=pthread_spin_init(&cache.spinlock,PTHREAD_PROCESS_PRIVATE);
    if(ret!=0)
    {
        printf("create spin lock error:%s\n",strerror(ret));
        exit(-1);
    }


    /* 读写锁属性初始化 */
    pthread_rwlockattr_init(&rwattr);


    /* 修改读写锁的属性 */
    //PTHREAD_PROCESS_SHARED：共享读写锁。该读写锁可以在多个进程中的线程之间共享；
    //PTHREAD_PROCESS_PRIVATE：私有读写锁。只有本进程内的线程才能够使用该读写锁
    ret=pthread_rwlockattr_setpshared(&rwattr,PTHREAD_PROCESS_PRIVATE);
    if(ret!=0)
    {
        printf("set rwlock attr error:%s\n",strerror(ret));
        exit(-1);
    }
    int pshared;


    /* 读取读写锁的属性 */
    pthread_rwlockattr_getpshared(&rwattr,&pshared);
    printf("the rwlock type is %s\n",type==PTHREAD_PROCESS_PRIVATE?"PTHREAD_PROCESS_PRIVATE":"PTHREAD_PROCESS_PRIVATE");

    /* 读写锁初始化 */
    //第二个参数是读写锁的属性，NULL为默认值
    ret=pthread_rwlock_init(&cache.rwlock,NULL);
    if(ret!=0)
    {
        printf("create rw lock error:%s\n",strerror(ret));
        exit(-1);
    }

    srand(time(NULL));
}

void *write_thread_func(void *arg)
{
    int thread_id=*(int *)arg;
    free(arg);
    printf("write_thread[%d] is running\n",thread_id);

    while(1)
    {
        /* 互斥锁上锁 */
        //会被阻塞，直到互斥锁解锁
        //调用 pthread_mutex_trylock()函数尝试对互斥锁进
        //行加锁，如果互斥锁处于未锁住状态，那么调用 pthread_mutex_trylock()将会锁住互斥锁并立马返回，如果
        //互斥锁已经被其它线程锁住，调用 pthread_mutex_trylock()加锁失败，但不会阻塞，而是返回错误码 EBUSY。
        pthread_mutex_lock(&cache.mutex);


        //注意：1.一定是先上互斥锁在等待信号
        //2.等待信号时用while循环而不是if，因为其他线程可能抢占
        while(cache.data_count>=100)
        {
            printf("写线程%d：缓存已满，等待消费...\n", thread_id);
            /* 等待条件变量状态(阻塞) */
            pthread_cond_wait(&cache.cond,&cache.mutex);
        }

        int rand_num=rand()%1000;
        cache.data[cache.data_count]=rand_num;


        /* 自旋锁加锁 */
        //pthread_spin_trylock()函数也可以对自旋锁进行加锁，如果未能获取到锁，就立刻返回错误，错误码为 EBUSY
        pthread_spin_lock(&cache.spinlock);
        cache.data_count++;

        /* 自旋锁解锁 */
        pthread_spin_unlock(&cache.spinlock);

        printf("写线程%d：写入数据%d，当前缓存数量：%d\n", thread_id, rand_num, cache.data_count);

        /* 互斥锁解锁 */
        pthread_mutex_unlock(&cache.mutex);


        /* 发送条件变量信号 */
        //pthread_cond_signal()函数至少能唤醒一个线程，
        //而 pthread_cond_broadcast()函数则能唤醒所有线程。
        //。使用 pthread_cond_broadcast()函数总能产生正确的结果，唤醒所有等待状态的线程，
        //但函数 pthread_cond_signal()会更为高效
        pthread_cond_signal(&cache.cond);

        // 模拟写操作耗时
        usleep(rand() % 500000);
    }
}


void *read_thread_func(void *arg)
{
    int thread_id=*(int *)arg;
    free(arg);
    printf("read_thread[%d] is running\n",thread_id);
    while(1)
    {

        pthread_mutex_lock(&cache.mutex);

        while (cache.data_count <= 0) 
        {
            printf("读线程%d：缓存为空，等待数据...\n", thread_id);
            pthread_cond_wait(&cache.cond, &cache.mutex);
        }

        /* 读写锁写上锁 */
        ///pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
        //如果线程不希望被阻塞，可以调用 pthread_rwlock_trywrlock()来尝试加锁


        /* 读写锁读上锁 */
        //如果线程不希望被阻塞，可以调用 pthread_rwlock_tryrdlock()来尝试加锁
        pthread_rwlock_rdlock(&cache.rwlock);
        int read_num=cache.data[cache.data_count-1];

        /* 读写锁读解锁 */
        //无论是读解锁还是写解锁都是用这个函数
        pthread_rwlock_unlock(&cache.rwlock);

        pthread_spin_lock(&cache.spinlock);
        cache.data_count--;
        pthread_spin_unlock(&cache.spinlock);
    
        printf("读线程%d：读取数据%d，当前缓存数量：%d\n", thread_id, read_num, cache.data_count);

        pthread_mutex_unlock(&cache.mutex);
        pthread_cond_signal(&cache.cond);

        // 模拟读操作耗时
        usleep(rand() % 300000);
    }
}

// 监控线程函数：定期打印缓存状态
void* monitor_thread_func(void* arg) {
    int count = 0;
    while (count < 5) 
    { // 运行10秒（每2秒一次，共5次）
        sleep(2);
        
        pthread_rwlock_rdlock(&cache.rwlock);
        printf("监控线程：当前缓存有效数据数量 = %d\n", cache.data_count);
        pthread_rwlock_unlock(&cache.rwlock);
        
        count++;
    }
    
    printf("监控线程：运行结束，准备退出\n");
    return NULL;
}

void destroy_Lock(void)
{
    /* 互斥锁销毁 */
    pthread_mutex_destroy(&cache.mutex);

    /* 条件变量销毁 */
    pthread_cond_destroy(&cache.cond);

    /* 自旋锁销毁 */
    pthread_spin_destroy(&cache.spinlock);

    /* 读写锁销毁 */
    pthread_rwlock_destroy(&cache.rwlock);

    /* 互斥锁属性销毁 */
    pthread_mutexattr_destroy(&mutexattr);

    /* 读写锁属性销毁 */
    pthread_rwlockattr_destroy(&rwattr);

}