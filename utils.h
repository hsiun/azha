// azha所使用的工具函数集

#ifndef __UTILS_H
#define __UTILS_H

#include <sys/socket.h> //socket的定义
#include <sys/types.h>  //socket的类型
#include <arpa/inet.h> //inet(3)函数
#include <sys/wait.h>
#include <netdb.h>     //getnameinfo函数
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
#define MAXBUF  (8192)
#define SERVER_PORT (8800)  


/* 用于简化bind(), connect(), 和 accept()函数的调用 */
typedef struct sockaddr SA;


/******************************
 * 健壮I/O系统（RIO），是一个自行实现的安全io系统
 *****************************/
#define RIO_BUFFER 8192
typedef struct {
    int rio_fd;      //rio内部缓冲的描述符
    int rio_cnt;     //内部缓存中没有被读的字节数
    char *rio_bufptr;//指向缓存中下一个没有被读的字符
    char rio_buf[RIO_BUFFER]; //rio的缓存区
} rio_t;

/* RIO函数 */
ssize_t rio_readn(int fd, void *userbuf, size_t n);
ssize_t rio_writen(int fd, void *userbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd); //初始化读入，将fd中的内容读入rio中
ssize_t rio_readnb(rio_t *rp, void *userbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *userbuf, size_t maxlen);

/* RIO函数的包裹函数 */
ssize_t Rio_readn(int fd, void *userbuf, size_t n);
void Rio_writen(int fd, void *userbuf, size_t n);
void Rio_readinitb(rio_t *rp, int fd);
ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);


/***************************
 * 处理HTTP请求，在http_1_0.c文件中实现
 **************************/
void do_http_1_0(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void server_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void server_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, 
        char *shortmsg, char *longmsg);


/*****************************
 * IO处理
 ****************************/
char *Fgets(char *ptr, int n, FILE *stream);
int Fputs(const char *ptr, FILE *stream);
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
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host,
        size_t hostlen, char * serv, size_t servlen, int flags);

/* 系统 */
pid_t Fork(void);
int Close(int sockfd);

/* 错误处理 */
void unix_error(char *msg);
void server_error(char *msg);


#endif // __UTILS_H
