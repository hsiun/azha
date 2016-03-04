# azha所使用的工具函数集

#ifndef __UTILS_H
#define __UTILS_H

#include <sys/socket.h> //socket的定义
#include <sys/types.h>  //socket的类型
#include <arpa/inet.h> //inet(3)函数
#include <sys/wait.h>
#include <unistd.h>    //unix函数集合
#include <errno.h>     //错误处理，错误码
#include <signal.h>    //信号处理


#include <stdio.h>     //标准库io函数
#include <stdlib.h>
#include <string.h>    //strlen()函数
#include <strings.h>   //bzero函数

/* 函数常量定义 */
#define LISTENQ (1024)
#define MAXLINE (8192)
#define SERVER_PORT (8800)  

/*****************************
 * IO处理
 ****************************/
ssize_t readn(int filedes, void *buff, size_t nbytes);            //从描述符中读入nbytes个字符到buff中
ssize_t writen(int filedes, const void *buff, size_t nbytes);     //从buff中写入nbytes个字符到描述符中
ssize_t readline(int filedes, void *buff, size_t maxlen);         //行处理，极慢


/******************************
 * 包裹函数的定义
 * 分成错误，系统，网络三部分定义
 *******************************/
/* 网络 */
int Socket(int family, int type, int protocol);
int Connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen);
int Bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);

/* 系统 */
pid_t Fork(void);
int Close(int sockfd);

/* 错误处理 */
void unix_error(char *msg);
void server_error(char *msg);


#endif // __UTILS_H
