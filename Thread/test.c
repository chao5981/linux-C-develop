
/* gcc -o testApp testApp.c -lpthread */

#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"

/* 使用 PTHREAD_ONCE_INIT 宏对其进行初始化 */
pthread_once_t once=PTHREAD_ONCE_INIT;
pthread_key_t key;

/* 线程局部存储的主要优点在于，比线程特有数据的使用要简单。要创建线程局部变量，只需简单地在全
局或静态变量的声明中包含__thread 修饰符即可 */
//是只能在全局变量或者静态变量中加__thread
static __thread char buff[512];


void *start_routine(void *arg);
void init_routine_once(void);
void destructor(void *count);


int main(void)
{
    int res=0;
    pthread_attr_t attr;
    pthread_t thread1,thread2,thread3;
    void * stackaddr;
    size_t stacksize;


    /* 线程属性 */
    //线程属性可以修改栈区，栈地址，分离状态属性
    //不可以重复初始化，用完后必须删除
    res=pthread_attr_init(&attr);
    if(res!=0)
    {
        fprintf(stdout,"\npthread attr init error:%s\n",strerror(res));
        exit(-1);
    }


    /* 修改和获取栈地址和栈大小 */
    //线程栈大小有要求，过高或过低会报错，ulimit -s可以默认查到线程栈大小为8MB


    // pthread_attr_setstackaddr设置地址
    res=pthread_attr_setstacksize(&attr,1024*1024);
    if(res!=0)
    {
        fprintf(stdout,"\nset pthread stack error:%s\n",strerror(res));
    }


    //获取栈大小和起始地址
    res=pthread_attr_getstack(&attr,&stackaddr,&stacksize);
    if(res!=0)
    {
        fprintf(stdout,"\nget stack addr and stacksize error:%s\n",strerror(res));
    }
    else
    {
        fprintf(stdout,"\nstackaddr:%p,stacksize:%lu byte\n",stackaddr,(unsigned long)stacksize);
    }

    /* 修改和获取线程分离属性 */
    res=pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
    if(res!=0)
    {
        fprintf(stdout,"set deach state error:%s\n",strerror(res));
    }
    int deachstate;
    res=pthread_attr_getdetachstate(&attr,&deachstate);
    if(res!=0)
    {
        fprintf(stdout,"get deach state error:%s\n",strerror(res));
    }
    else
    {
        fprintf(stdout,"deach state:%s\n",deachstate==PTHREAD_CREATE_JOINABLE?"join":"deach");
    }


    /* 创建线程 */
    //第一个参数：新创建的线程的线程 ID 会保存在参数 thread所指向的内存中，后续的线程相关函数会使用该标识来引用此线程
    //第二个参数：线程属性，可设置为null为默认值
    //第三个参数：线程进入函数，类型为void *(void *)
    //第四个参数：传入到线程进入函数的参数，可设置为NULL即为不传
    res=pthread_create(&thread1,&attr,start_routine,"thread1");
    if(res!=0)
    {
        fprintf(stdout,"thread1 create error:%s\n",strerror(res));
        exit(-1);
    }
    res=pthread_create(&thread2,&attr,start_routine,"thread2");
    if(res!=0)
    {
        fprintf(stdout,"thread2 create error:%s\n",strerror(res));
        exit(-1);
    }
    res=pthread_create(&thread3,&attr,start_routine,"thread3");
    if(res!=0)
    {
        fprintf(stdout,"thread3 create error:%s\n",strerror(res));
        exit(-1);
    }


    /* 分离线程 */
    //可在其他线程分离，也可以自己分离
    res=pthread_detach(thread3);
    if(res!=0)
    {
        fprintf(stdout,"set pthread detach error%s\n",strerror(res));
    }

    /* 销毁线程属性 */
    pthread_attr_destroy(&attr);


    /* 获取线程ID */
    pthread_t main_thread=pthread_self();
    fprintf(stdout,"main thread id:%lu",(unsigned long)main_thread);


    /* 对比线程ID是否相等 */
    fprintf(stdout,"main thread and thread1 is equal:%s\n",pthread_equal(main_thread,thread1)?"yes":"no");
    
    sleep(5);


    /* 任意线程向任意线程发送取消请求 */
    res=pthread_cancel(thread1);
    if(res!=0)
    {
        fprintf(stdout,"cancel pthread1 error:%s\n",strerror(res));
    }
    else
    {
        fprintf(stdout,"cancel request of pthread1 is sent\n");
    }
    res=pthread_cancel(thread2);
    if(res!=0)
    {
        fprintf(stdout,"cancel pthread2 error:%s\n",strerror(res));
    }
    else
    {
        fprintf(stdout,"cancel request of pthread2 is sent\n");
    }
    res=pthread_cancel(thread3);
    if(res!=0)
    {
        fprintf(stdout,"cancel pthread3 error:%s\n",strerror(res));
    }
    else
    {
        fprintf(stdout,"cancel request of pthread3 is sent\n");
    }
    
    printf("\n[主线程] 等待Thread-1和Thread-2结束...\n");
    

    /* 阻塞型回收线程 */
    res = pthread_join(thread1, NULL);
    if (res != 0) {
        fprintf(stdout, "等待Thread-1失败: %s\n", strerror(res));
    } else {
        printf("[线程结束] Thread-1已正常结束\n");
    }
    res = pthread_join(thread2, NULL);
    if (res != 0) {
        fprintf(stdout, "等待Thread-2失败: %s\n", strerror(res));
    } else {
        printf("[线程结束] Thread-2已正常结束\n");
    }

    sleep(2);


    /* 销毁特有数据 */
    if(pthread_key_delete(key)==0)
    {
        fprintf(stdout,"delete key successfully,main_pthread ready back\n");
    }
    exit(0);
    
}

void *start_routine(void *arg)
{
    strcpy(buff, arg);
    pthread_t pthread_id=pthread_self();
    fprintf(stdout,"thread:%s is running,id:%lu\n",buff,pthread_id);


    /* 一次性初始化函数 */
    //所有线程只会执行一次，常用来初始化线程特有数据键
    //第一个参数：使用 PTHREAD_ONCE_INIT 宏对其进行初始化
    //第二个参数：指定只执行一次的函数
    int res=pthread_once(&once,init_routine_once);
    if(res!=0)
    {
        fprintf(stdout,"run once error:%s\n",strerror(res));

        /* 线程推出函数 */
        pthread_exit(NULL);
    }

    int *count;


    /* 常用来判断是否绑定了数据键 */
    //返回NULL为没有绑定
    count=pthread_getspecific(key);
    if(count==NULL)
    {

        //先分配内存
        count=malloc(sizeof(int));
        if(count==NULL)
        {
            fprintf(stdout,"malloc error\n");
            pthread_exit(NULL);
        }

        //再绑定数据键
        if(pthread_setspecific(key,count))
        {
            fprintf(stdout,"set specific error\n");
            pthread_exit(NULL);
        }
    }

    if(strcmp(buff,"thread1")==0)
    {

        /* 设置取消状态和类型 */

        //设置是否可以取消
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);

        //设置是只能在取消点取消还是在任意时间段取消（后者不常用）
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
        *count=2;
    }
    if(strcmp(buff,"thread2")==0)
    {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
    }
    if(strcmp(buff,"thread1")==0||strcmp(buff,"thread2")==0)
    {
        while(1)
        {

            /* 产生一个可取消点 */
            pthread_testcancel();
            sleep(1);
            (*count)++;
            fprintf(stdout,"thread name:%s,running count:%d\n",buff,*count);
            if(*count>=5&&strcmp(buff,"thread2")==0)
            {
                pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
            }
        }
    }
    sleep(2);
    fprintf(stdout,"\nthread3 exit\n");
    pthread_exit(NULL);
}

void init_routine_once(void)
{
    printf("init_routine_once 被执行: 线程 ID<%lu>\n", pthread_self());

    /* 创建数据键 */
    //第二个参数是一个自定义的解构函数
    //该函数通常用于释放与特有数据键关联的线程私有数据区占用的内存空间，当使用线程特有数据的线程终止时，destructor()函数会被自动调用。
    int res=pthread_key_create(&key,destructor);
    if(res!=0)
    {
        fprintf(stdout,"create key error:%s\n",strerror(res));
        pthread_exit(NULL);
    }
}

void destructor(void *count)
{
    //释放count内存
    free(count);
}