#include "utils.h"

void str_echo(int sockfd);

int main(int argc, char **argv)
{
    //使用Fork模型的并发服务器
    int listenfd, connfd;         //listenfd监听套接字，connfd连接套接字
    pid_t childpid;               //子进程描述符，用于处理到来的连接
    socklen_t clilen;
    char hostname[MAXLINE], port[MAXLINE];
    struct sockaddr_in cliaddr, servaddr;  //远程客户的地址和服务器地址

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);  //创建监听套接字

    bzero(&servaddr, sizeof(servaddr));    //将服务器地址归零，并做相应处理
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);

    Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)); //将监听套接字绑定到服务器地址
    Listen(listenfd, LISTENQ);    //监听远端连接

    for ( ; ; ) {
        clilen = sizeof(cliaddr);
        connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);//已连接的通过connfd返回
        Getnameinfo( (struct sockaddr *) &cliaddr, clilen, hostname, MAXLINE,
                port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        if ( ( childpid = Fork()) == 0) {
            Close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        Close(connfd);
    }
    return (0);
}

/* 返回输入的服务器代码 */
void str_echo(int sockfd)
{
    ssize_t n;
    char buf[MAXLINE];

again:
    while ( (n = read(sockfd, buf, MAXLINE)) > 0)
        writen(sockfd, buf, n);

    if (n < 0 && errno == EINTR)
        goto again;
    else if (n < 0)
        server_error("str_echo: read error");
}
