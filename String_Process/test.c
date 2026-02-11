#include "stdio.h"
#include "strings.h"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"

/* 9 给应用程序传参 */
//只需在要执行的时候后面输入参数即可
int main(char argc,char *argv[])
{

    FILE *fp=NULL;
    fp=fopen("file1","w+");
    if(fp==NULL)
    {
        perror("open file errror");
        exit(-1);
    }

    printf("Number of parameters: %d\n", argc);
    for (int i = 0; i < argc; i++)
    printf(" %s\n", argv[i]);

    /* 输出字符串 */
    //推荐用fputc，可以指定输出文件
    if(fputs("1abc",fp)==-1)
    {
        perror("fputs file1 error");
    }

    //若只需要输出一个，则用fputc
    if(fputc('d',fp)==-1)
    {
        perror("fputc file1 error");
    }


    char buff[10];

    /* 初始化缓存区为0 */
    bzero(buff,sizeof(buff));

    /* 获取用户输入的字符串 */    
    //只可以用fgets(),gets()有bug
    //gets()函数允许输入的字符串带有空格、制表符，输入的空格和制表符也是字符串的一部分，仅以回车换行符作为字符串的分割符
    //scanf 以%s 格式输入的时候，空格、换行符、TAB 制表符等都是作为字符串分割符存在

    /* 注意！ */
    //1.读到文件末尾（EOF）且未读取到任何字符：返回 NULL，不设置 errno；因 I/O 错误失败（如磁盘错误、权限问题、文件描述符失效等）：返回 NULL，同时设置 errno。
    //2.fgets所读取的文件偏移量会被fputs影响，就和write,read一样


    if(fseek(fp,0,SEEK_SET)==-1)
    {
        perror("01:fseek file error");
    }

    if(fgets(buff,sizeof(buff),fp)==NULL)
    {
        perror("fgets file1 error");
    }
    printf("%s\n",buff);


    if(fseek(fp,0,SEEK_SET)==-1)
    {
        perror("02:fseek file error");
    }

    //该函数以无符号 char 强制转换为 int 的形式返回读取的字符
    int ch=fgetc(fp);
    if(ch==-1)
    {
        perror("fgetc file1 error");
    }
    printf("%c\n",ch);

    /* 字符串长度 */
    printf("length:%ld\n",strlen(buff));

    /* 字符串拼接 */    //不会置位errno

    //////////////////////////////////
    //低级错误：数组直接赋值会导致缺少\0导致内存泄露和污染，记得在结尾加上\0

    // char buff2[3]={'b','c','d'};
    char buff2[3]={'b','c','\0'};

    //用strcat()保证  dest 的内存够大（能装下自己原本的内容 + src 的内容 + 最后一个 \0dest 末尾的' \0 '结束字符会被覆盖，src 末尾的结束字符' \0 '会一起被复制过去，最终的字符串只有一个' \0 '。
    strncat(buff,buff2,sizeof(buff2));

    char buff3[10];

    bzero(buff3,sizeof(buff3));
    /* 字符串复制 */        //必须保证 dest 指向的内存空间足够大，能够容纳下拷贝过来的字符串，否则会导致溢出错误。  //不会置位errno
    strcpy(buff3,buff2);

    printf("buff3:%s\n",buff3);

    /* 字符串内容填充 */        //不会置位errno
    //memset()

    /* 字符串比较函数 */
    printf("%d\n", strncmp("ABC", "ABC", 3));
    printf("%d\n", strncmp("ABCD", "ABC", 4));
    printf("%d\n", strncmp("ABC", "ABCD", 4));

    // 如果返回值小于 0，则表示 str1 小于 str2
    // 如果返回值大于 0，则表示 str1 大于 str2
    // 如果返回值等于 0，则表示字符串 str1 等于字符串 str2

    /* 字符串查找 */
    //strchr是从左往右找，而strrchr是从右往左找，最后返回找到的字符的地址
    char *ptr = NULL;
    char str[] = "I love my home";
    ptr = strchr(str, 'o');
    if (NULL != ptr)
    //返回的地址减去首字母的地址，即为第几个数
    printf("strchr: %ld\n", ptr - str);
    ptr = strrchr(str, 'o');
    if (NULL != ptr)
    printf("strrchr: %ld\n", ptr - str);

    //strstr是寻找一串字符串，返回最开始匹配上字符的地址
    ptr = strstr(str, "home");
    if (NULL != ptr) {
    printf("String: %s\n", ptr);
    printf("Offset: %ld\n", ptr - str);
    }

    /* 字符串数字转化 */        //只能是数字形式的字符串转化为数字

    //最后一个参数的含义：
    //数字基数，参数 base 必须介于 2 和 36（包含）之间，或者是特殊值 0。参数 base 决定了字符串
    //转换为整数时合法字符的取值范围，譬如，当 base=2 时，合法字符为' 0 '、' 1 '（表示是一个二进制表示的
    //数字字符串）；当 base=8 时，合法字符为' 0 '、' 1 '、' 2 '、' 3 '……' 7 '（表示是一个八进制表示的数字字符
    //串）；当 base=16 时，合法字符为' 0 ' 、' 1 '、' 2 '、' 3 '……' 9 '、' a '……' f '（表示是一个十六进制表示的数
    //字字符串）；当 base 大于 10 的时候，' a '代表 10、' b '代表 11、' c '代表 12，依次类推，' z '代表 35（不区分大小写）。

    //在 base=0 的情况下，如果字符串包含一个了“0x”前缀，表示该数字将以 16 为基数；如果包含的是“0”前缀，表示该数字将以 8 为基数。当 base=16 时，字符串可以使用“0x”前缀。

    //第二个参数：char **类型的指针，如果 endptr 不为 NULL，则 strtol()或 strtoll()会将字符串中第一个无效字
    //符的地址存储在*endptr 中。如果根本没有数字，strtol()或 strtoll()会将 nptr 的原始值存储在*endptr 中（并返回 0）。也可将参数 endptr 设置为 NULL，表示不接收相应信息。
    printf("strtol: %ld\n", strtol("0x500", NULL, 16));
    printf("strtol: %ld\n", strtol("0x500", NULL, 0));
    printf("strtol: %ld\n", strtol("500", NULL, 16));
    printf("strtol: %ld\n", strtol("0777", NULL, 8));
    printf("strtol: %ld\n", strtol("0777", NULL, 0));
    printf("strtol: %ld\n", strtol("1111", NULL, 2));
    printf("strtol: %ld\n", strtol("-1111", NULL, 2));

    /*字符串转浮点型数据*/
    //strtof()、strtod()以及 strtold()三个库函数可分别将字符串转换为 float 类型数据、double 类型数据、longdouble 类型数据，参数和strtol差不多，少了base参数
    printf("strtof: %f\n", strtof("0.123", NULL));
    printf("strtod: %lf\n", strtod("-1.1185", NULL));
    printf("strtold: %Lf\n", strtold("100.0123", NULL));

    /* 数字转字符串 */
    //fprintf即可


    return 0;
}