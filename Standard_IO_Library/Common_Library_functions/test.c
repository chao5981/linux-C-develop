#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"

int main(void)
{
    int res=0;
    FILE *fp=NULL;
    //fopen()的第二个参数规则
    //r:以只读方式打开文件。
    //r+:以可读、可写方式打开文件。
    //w:以只写方式打开文件，如果参数 path 指定的文件存在，将文件长度截断为 0；如果指定文件不存在则创建该文件。
    //w+:以可读、可写方式打开文件，如果参数 path 指定的文件存在，将文件长度截断为 0；如果指定文件不存在则创建该文件。
    //a:以只写方式打开文件，打开以进行追加内容（在文件末尾写入），如果文件不存在则创建该文件。
    //a+:以可读、可写方式打开文件，以追加方式写入（在文件末尾写入），如果文件不存在则创建该文件。

    //虽然调用 fopen()函数新建文件时无法手动指定文件的权限，但却有一个默认值：S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH (0666)
    fp=fopen("file1","w+");
    if(fp==NULL)
    {
        perror("open file1 error");
        exit(-1);
    }
    char writebuff[10];
    char readbuff[10];
    memset(writebuff,'a',sizeof(writebuff));

    //fwrite():由中间俩个参数共同确定写入文件数据的大小
    //第二个参数size：fread()从文件读取 nmemb 个数据项，每一个数据项的大小为 size 个字节，所以总共读取的数据大小为 nmemb * size 个字节。
    //第三个参数nmemb：参数 nmemb 指定了读取数据项的个数。
    res=fwrite(writebuff,1,sizeof(writebuff),fp);
    if(res==-1)
    {
        perror("write file1 error");
        goto err1;
    }   

    //ftell():查看当前文件的读写偏移量
    res=ftell(fp);
    if(res==-1)
    {
        perror("ftell file1 error");
        goto err1;
    }
    printf("%d\n",res);

    res=fread(readbuff,1,sizeof(readbuff),fp);
    //read要这样判断错误标志
    if(res<sizeof(readbuff))
    {
        //feof():检查文件是否到达文件末尾
        if(feof(fp))
        {
            //涉及到标准输入，标准输出和标准错误的概念
            //标准输入设备指的就是计算机系统的标准的输入设备，通常指的是计算机所连接的键盘；
            //而标准输出设备指的是计算机系统中用于输出标准信息的设备，通常指的是计算机所连接的显示器；
            //标准错误设备则指的是计算机系统中用于显示错误信息的设备，通常也指的是显示器设备。

            //用户通过标准输入设备与系统进行交互，进程将从标准输入（stdin）文件中得到输入数据，将正常输出数据（譬如程序中 printf 打印输出的字符串）输出到标准输出（stdout）文件，
            //而将错误信息（譬如函数调用报错打印的信息）输出到标准错误（stderr）文件。
            //标准输出文件和标准错误文件都对应终端的屏幕，而标准输入文件则对应于键盘。
            //每个进程启动之后都会默认打开标准输入、标准输出以及标准错误，得到三个文件描述符，即 0、1、2，其中 0 代表标准输入、1 代表标准输出、2 代表标准错误；
            //在应用编程中可以使用宏 STDIN_FILENO、STDOUT_FILENO 和 STDERR_FILENO 分别代表 0、1、2，这些宏定义在 unistd.h 头文件中

            //相比于printf()函数用于将程序中的字符串信息输出显示到终端
            //fprintf()可将格式化数据写入到由 FILE 指针指定的文件中（例如写入标准错误文件），要求传入文件名
            //dprintf()可将格式化数据写入到由文件描述符 fd 指定的文件中
            //snprintf()函数将格式化数据存储在由参数 buf 所指定的缓冲区中。使用参数 size 显式的指定缓冲区的大小，如果写入到缓冲区的字节数大于参数 size 指定的大小，超出的部分将会被丢弃！
            //如果缓冲区空间足够大，snprintf()函数就会返回写入到缓冲区的字符数，与sprintf()函数相同，也会在字符串末尾自动添加终止字符'\0'。
            fprintf(stdout,"文件以到末尾\n");
            dprintf(STDOUT_FILENO,"文件已到达末尾\n");
            snprintf(readbuff,sizeof(readbuff),"end of file");
            printf("%s\n",readbuff);

            //fscanf，scanf,sscanf也是同理
            //fscanf()函数从指定文件中读取数据，作为格式转换的输入数据，文件通过 FILE 指针指定
            //sscanf()将从参数 str 所指向的字符串缓冲区中读取数据，作为格式转换的输入数据
        }
    }

    //ferror():库函数 ferror()用于测试参数 stream 所指文件的错误标志，如果错误标志被设置了，则调用 ferror()函数将返回一个非零值，如果错误标志没有被设置，则返回 0。
    //ferror() 不能 识别 end of file (EOF) 错误
    if(ferror(fp))
    {
        printf("file error\n");
        //clearerr()用于清end-of-file标志和错误标志，当调用 feof()或 ferror()校验这些标志后，通常需要清除这些标志，避免下次校验时使用到的是上一次设置的值，此时可以调用clearerr()。
        clearerr(fp);
    }

    //fseek():用于设置文件读写位置偏移量,并可以隐性清除end-of-file标志
    res=fseek(fp,0,SEEK_SET);
    if(res==-1)
    {
        perror("flseek error");
        goto err1;
    }

    res=fread(readbuff,1,sizeof(readbuff),fp);
    if(res==-1)
    {
        perror("read error");
        goto err1;
    }
    printf("%s\n",readbuff);

err1:
    fclose(fp);
    exit(res);
}