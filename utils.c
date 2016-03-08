#include "utils.h"


/*****************
 * IO处理
 ****************/
ssize_t readn(int fd, void *vptr, size_t n)
{
    size_t nleft;   //文件描述符中剩下的
    ssize_t nread;  //已读的
    char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0){
        if ( (nread = read(fd, ptr, nleft)) < 0){
            if (errno == EINTR)
                nread = 0;      //读入过程被系统中断，重新调用read
            else
                return (-1);
        } else if (nread == 0)
            break;               //EOF
        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft);
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;    //描述符中没有被写入的
    ssize_t nwritten; //已希尔到缓冲中的
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0){
        if ( (nwritten = write(fd, ptr, nleft)) <= 0){
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;                //写入被中断，重新写
            else
                return (-1);                //调用write出错，立即返回
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
}

/* 采用循环读入一个字符的方式读入行，被原作者标记为极慢的 */
ssize_t readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc; //n是循环计数器，rc读入的数据量
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
    again:
        if ( (rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            *ptr = 0;
            return (n - 1);
        } else {
            if (errno == EINTR)
                goto again;
            return (-1);
        }
    }

    *ptr = 0;
    return (n);
}


/*****************
 * 包裹函数
 ******************/
/* 系统IO包裹函数 */
char *Fgets(char *ptr, int n, FILE *stream)
{
    char *rptr;
    if ( ( (rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream))
        server_error("Fgets error");

    return rptr;
}

int Fputs(const char *ptr, FILE *stream)
{
    int rf;
    if ( (rf = fputs(ptr, stream)) == EOF)
        unix_error("Fputs error");

    return rf;
}

/* 网络包裹函数 */
int Socket(int family, int type, int protocol)
{
    int rc;

    if ( (rc = socket(family, type, protocol)) < 0)
        unix_error("Socket error");
    return rc;
}

int Connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addr_len)
{
    int rc;
    
    if ( (rc = connect(sockfd, serv_addr, addr_len)) < 0)
        unix_error("Connect error");

    return rc;
}

int Bind(int sockfd, const struct sockaddr *myaddr, socklen_t addr_len)
{
    int rc;
    if ( (rc = bind(sockfd, myaddr, addr_len)) < 0)
        unix_error("Listen error");

    return rc;
}

int Listen(int s, int backlog)
{
    int rc;

    if ( (rc = listen(s, backlog)) < 0)
        unix_error("Listen error");

    return rc;
}

void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host,
        size_t hostlen, char *serv, size_t servlen, int flags)
{
    int rc;
    if ( (rc = getnameinfo(sa, salen, host, hostlen, serv, 
                    servlen, flags)) != 0)
        unix_error("Getnameinfo error");
}

int Accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addr_len)
{
    int rc;

    if ( (rc = accept(sockfd, cliaddr, addr_len)) < 0)
        unix_error("Accept error");

    return rc;
}

/* 系统调用的包裹函数 */
pid_t Fork(void)
{
    pid_t pid;

    if ( (pid = fork()) < 0)
        unix_error("Fork error");

    return pid;
}

int Close(int fd)
{
    int rc;
    if ( (rc = close(fd)) < 0)
        unix_error("Close error");

    return rc;
    
}
/* 错误处理函数 */
void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

void server_error(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(0);
}

