/****
 * 处理http请求的服务器代码实现
 ****/

#include "utils.h"


/*****************************
 * 处理http请求的实现代码
 ****************************/
void do_http_1_0(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* 读入请求头 */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE)){
        clienterror(fd, method, "500", "Inner error",
                "Azha's inner error");
        return;
    }
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not Implemented",
                "Azha dose not implement this method");
        return ;
    }
    read_requesthdrs(&rio);

    /* 从GET请求分解URI */
    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not found",
                "Tiny couldn't find this file");
        return ;
    }

    if (is_static) { //静态内容
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                    "Azha couldn't read the file");
            return ;
        }
        server_static(fd, filename, sbuf.st_size);
    } else {         //动态内容
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbideen",
                    "Azha coudn't read the file");
            return ;
        }
        server_dynamic(fd, filename, cgiargs);
    }
}

//读取http请求头部
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);         //先读一行http头
    printf("%s", buf);
    while (strcmp(buf, "\r\n")) {            //判读buf中的是否是\r\n(http头结束的标志)
        Rio_readlineb(rp, buf, MAXLINE);      
        printf("%s", buf);
    }
}

//把URI分解成文件名和CGI参数
//返回0是有CGI参数，1表示没有
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) {
        //uri中没有cgi-bin，静态内容
        strcpy(cgiargs, "");        //相当于置空cgiargs
        strcpy(filename, ".");      
        strcat(filename, uri);      //将.加在uri后面
        if (uri[strlen(uri)-1] == '/')         //倒退一个字符，跳过.
            strcat(filename, "home.html"); 
        return 1;
    } else {
        //动态内容
        ptr = index(uri, '?');
        if (ptr) {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        } else
            strcpy(cgiargs, ""); //有？但是参数还是为空
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

//复制文件到客户端
void server_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    //构造并发送响应头部
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Azha Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));
    printf("Response header:\n");
    printf("%s", buf);

    //发送响应正文
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);    //取消start所指向的映射内存其实地址，length是要取消内存的大小
}

//从文件名分析文件类型
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpg");
    else
        strcpy(filetype, "text/plain");
}

//运行一个CGI程序
void server_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emtylist[] = { NULL };

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Azha Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) {
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO);
        Execve(filename, emtylist, environ);
    }
    Wait(NULL);
}

//返回一个服务器内部错误给客户端
void clienterror(int fd, char *cause, char *errnum,
        char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    //构建http响应正文
    sprintf(body, "<html><title>Azha Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\r", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Azha Web Server</em>\r\n", body);

    //打印http响应
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));  //这里为什么要一行行，写入到客户端不明
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));

}
