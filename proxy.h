#ifndef PROXY_WEB_H__
#define PROXY_WEB_H__

#include "csapp.h" 


/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define Nthread 8
#define debug 1

void *thread(void *var);
void read_requesthdrs(rio_t *rp);
void clienterror(int fd,char *cause,char *errnum,
                char *shortmsg,char *longmsg);
int parse_uri(char *rawuri,char *host,char *requuri,char *port);
void slove(int connfd);



/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

enum {
    PROXY_PARSE_OK = 0,
    PROXY_INVALID_REQUEST_HEADER,
    PROXY_CACHE_MISS, // 没有写入
    PROXY_CACHE_OBJECT_TOOLARGE
};

#endif //PROXY_WEB_H__