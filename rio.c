/********************
 * 对原本系统提供的不健壮的IO，系统的封装
 ********************/
#include "utils.h"

/* 从套接字中读入n字符到缓存中 */
ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n; //没有读入的字节数，要读入的
    ssize_t nread;    //已读入的字符数
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ( (nread = read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR) //被中断，不是出错状态，继续读
                nread = 0;
            else
                return -1;      //出错
        } else if(nread == 0) {
            break;              //读到文件终止符
        }

        nleft = nleft - nread;  //更新未读入数
        bufp = bufp + nread;    //缓存指针前移 
    }
    return (n - nleft);         //读入的字符数，>=0
}

/* 从缓存中写入n字符到套接字描述符中 */
ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n; //没有写入的字符数
    ssize_t nwriten;  //写入到套接字描述符中的字符数
    char *buf = usrbuf;
    
    while (nleft > 0) {
        if ( (nwriten = write(fd, buf, nleft)) <= 0) {
            if (errno == EINTR)
                nwriten = 0;   //中断
            else
                return -1;     //出错
        }

        nleft = nleft - nwriten;
        buf = buf + nwriten;
    }
    return n;    
}

/*
 * Unix系统函数read()的包裹函数
 * 从内部缓存中读入min(n, rio_cnt)字符到用户缓存
 * n是用户请求的字节数
 * rio_cnt是内部缓存中没有读入的字符数
 * rio_read通过反复调用read来读尽，内部缓存中的所有字符
 */
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;
    
    //从内核中读入字符到rio内部缓存中，读光内部缓存为止
    while (rp->rio_cnt <= 0) {
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0) {
            if (errno != EINTR)
                return -1;     //如果不是被信号中断，那么就是出错
        } else if (rp->rio_cnt == 0) {
            return 0;          //读到文件终止符
        } else {
            rp->rio_bufptr = rp->rio_buf;
        }
    }
    
    // 从rio的内部缓存复制min(n, rp->rio_cnt)个字节到用户缓存
    cnt = 0;
    if (rp->rio_cnt < n)  //从内部缓存中读到的字符数少于n
        cnt = rp->rio_cnt;   
   memcpy(usrbuf, rp->rio_bufptr, cnt);
   rp->rio_bufptr += cnt;
   rp->rio_cnt -= cnt;
   return cnt;  
}

//用要读的文件描述符fd初始化rio_t数据结构
void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

//读入n个字符，并且将其缓存在rio的缓存中
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    size_t nleft = n; //未读入的，或者说要读入的
    ssize_t nread;    //已读入的
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ( (nread = rio_read(rp, bufp, nleft)) < 0) {
            return -1;      //rio_read中已考虑是中断情况，所以返回值小于0，便直接返回
        } else if (nread == 0) {
            break;
        }

        nleft -= nread;
        bufp += nread;
    }

    return (n - nleft);
}

//读入一个文本行
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) {
        if ( (rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n') {
                n++;
                break;
            }
        } else if (rc == 0) {
            if (n == 1) {
                return 0;   //读到文件终止符，没有数据
            } else {
                break;      //读到文件终止符，有数据
            }
        } else {
            return -1;     //rio_read返回值小于0，出错
        }
    }

    *bufp = '\0';
    return (n -1);
}

/******************************
 * rio的包裹函数
 *****************************/

ssize_t Rio_readn(int fd, void *ptr, size_t nbytes)
{
    ssize_t n;

    if ( (n = rio_readn(fd, ptr, nbytes)) < 0)
        unix_error("Rio_readn error");
    return n;
}

void Rio_writen(int fd, void *usrbuf, size_t n)
{
    if (rio_writen(fd, usrbuf, n) != n)
        unix_error("Rio_writen error");
}

void Rio_readinitb(rio_t *rp, int fd)
{
    rio_readinitb(rp, fd);
}

ssize_t Rio_readnb(rio_t *rp, void *usrptr, size_t n)
{
    ssize_t rc;

    if ( (rc = rio_read(rp, usrptr, n)) < 0)
        unix_error("Rio_readnb error");

    return rc;
}

ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    ssize_t rc;

    if ( ( rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
        unix_error("Rio_readlineb error");

    return rc;
}
