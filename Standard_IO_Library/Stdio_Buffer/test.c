#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* stido缓冲模式默认为行缓冲 */

//行缓冲测试就打开这一段测试
// int main(void)
// {
//     printf("Hello World!\n");
//     printf("Hello World!");
//     for ( ; ; )
//     sleep(1);
// }

// int main(void)
// {
//     //调用setvbuf设置为无缓冲模式。

//     //setvbuf的mode的三个参数
//     //_IONBF：不对 I/O 进行缓冲（无缓冲）。
//     //_IOLBF：采用行缓冲 I/O。
//     //_IOFBF：采用全缓冲 I/O。在这种情况下，在填满 stdio 缓冲区后才进行文件 I/O 操作（read、write）。
//     //最后一个size设置缓冲区大小


//     //与这个函数类似的还有
//     //setbuffer()函数类似于 setbuf()，但允许调用者指定 buf 缓冲区的大小，除了没有返回值，相当于setvbuf(stream, buf, buf ? _IOFBF : _IONBF, size);
//     //setbuf()函数构建与 setvbuf()之上，setbuf()调用除了不返回函数结果（void）外，就相当于：setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
//     if (setvbuf(stdout, NULL, _IONBF, 0)) 
//     {
//         perror("setvbuf error");
//         exit(0);
//     }

//     printf("Hello World!\n");
//     printf("Hello World!");
//     for ( ; ; )
//     sleep(1);
// }

// int main(void)
// {
//     printf("Hello World!\n");
//     printf("Hello World!");
//     fflush(stdout); //刷新标准输出 stdio 缓冲区
//     for ( ; ; )
//     sleep(1);
// }


// int main(void)
// {
//     printf("Hello World!\n");
//     printf("Hello World!");
//     fclose(stdout); //关闭标准输出 
//     for ( ; ; )
//     sleep(1);
// }

// //关于刷新 stdio 缓冲区相关内容，最后进行一个总结：
// //调用 fflush()库函数可强制刷新指定文件的 stdio 缓冲区；
// //调用 fclose()关闭文件时会自动刷新文件的 stdio 缓冲区；
// //程序退出时会自动刷新 stdio 缓冲区（注意区分不同的情况）。

//但是，与程序退出方式有关，如果使用 exit()、return 或像上述示例代码一样不显式调用相关函数或执行 return 语句来结束程序，这些情况下程序终止时会自动刷新 stdio 缓冲区；如果使用_exit 或_Exit()终止程序则不会刷新，
