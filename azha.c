#include "utils.h"

/*****************
 * 包裹函数
 ******************/
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
