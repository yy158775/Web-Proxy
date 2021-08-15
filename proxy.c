#include "csapp.h"
#include "sbuf.h"
#include <stdio.h>
#include "cache.h"
#include "proxy.h"
s_buf buf;


int main(int argc,char **argv) {
    if(argc != 2) {
        printf("Usage:%s <listening port>\n",argv[0]);
        exit(1);
    }
    inin_cache();
    int listenfd,connfd;
    int sendfd;
    char hostname[MAXLINE],port[MAXLINE];
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    listenfd = open_listenfd(argv[1]);
    
    int pid[Nthread]; //传递给子线程的变量
    pthread_t pthid;
    for(int i = 0;i < Nthread;i++) {
        pid[i] = i;
        pthread_create(&pthid,NULL,thread,&pid[i]);
    }    

    sbuf_init(&buf);
    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd,(SA *)&clientaddr,&clientlen);
        sbuf_insert(&buf,connfd);
        Getnameinfo((SA *)&clientaddr,clientlen,hostname,MAXLINE,port,MAXLINE,0);
        printf("Accepted connection from (%s,%s)\n",hostname,port);
    }
    return 0;
}



void *thread(void *var) {
    Pthread_detach(pthread_self());
    int connfd;
    int pid = *((int *)(var));
    while(1) {
        connfd  = sbuf_remove(&buf);
        /* 处理 */
#ifdef debug
        printf("Thread%d Get connection\n",pid);
#endif
        slove(connfd);
#ifdef debug       
        printf("Thread%d Run next\n",pid);
#endif
        close(connfd);
    }
}


void slove(int connfd) {
    char buf[MAXLINE];
    char method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    rio_t rp;

    /* read reuqest headers*/ 
    Rio_readinitb(&rp,connfd);
    Rio_readlineb(&rp,buf,MAXLINE);
    printf("PROXY:Request headers:\n");
    printf("%s",buf);
    read_requesthdrs(&rp);

    /* GET http://localhost:2021/home.html HTTP/1.1 */
    /* 将uri提取转化 http://host:port/requuri */
    char host[20];
    char requuri[50];
    char port[6] = "null";
    int r;
    sscanf(buf,"%s %s %s",method,uri,version);
    if(strcmp(method,"GET")) {
        clienterror(connfd,method,"501","Not implemented",
                    "Tiny does not implement this method\n");
        return;
    }

    /* 处理端口，打开request_fd */
    if((r = parse_uri(uri,host,requuri,port)) != 0)
    {
        clienterror(connfd,method,"501","Not parsed successfully",
                    "Tiny can't parse this siuaiton\n");
        return;
    }    


#ifdef debug       
        printf("Begin to read cache\n");
#endif

    /* cache hit */
    if(( r = rcache(host,requuri,buf)) == 0) {
        Rio_writen(connfd,buf,strlen(buf));
#ifdef debug       
        printf("Cache hit\n");
#endif
        return;
    }

    /* cache miss */
    /* send request */
    if(port == "null") strcpy(port,"8080");
#ifdef debug
    printf("client port is %s\n",port);
#endif
    int clientfd = Open_clientfd("localhost",port);
    rio_t srp;
    Rio_readinitb(&srp,clientfd);

    sprintf(buf,"GET %s HTTP/1.0\r\n",requuri);
#ifdef debug
    printf("buf is %s\n",buf);
#endif
    Rio_writen(clientfd,buf,strlen(buf));
    sprintf(buf,"Host: %s\r\n",host);
    Rio_writen(clientfd,buf,strlen(buf));
    sprintf(buf,"%s",user_agent_hdr); /* 后面已经有\r\n了 */
    Rio_writen(clientfd,buf,strlen(buf));
    sprintf(buf,"Connection: close\r\n");
    Rio_writen(clientfd,buf,strlen(buf));
    sprintf(buf,"Proxy-Connection: close\r\n\r\n");
    Rio_writen(clientfd,buf,strlen(buf));

    /* read response and send back */
    ssize_t size;
    /* 响应头 响应头 */
    //char cachebuf[MAXLINE];
    char *cachebuf;
    cachebuf = (char *)malloc(102400); /* 看你分配多少了 但是一定要释放，不要用栈 */
    cachebuf[0] = '\0';
    while(strcmp(buf,"\r\n")) {
       size = Rio_readlineb(&srp,buf,MAXLINE);  /* 读取了时候按行读取，就是遇到/r/n直接终止 返回实际读取的数目*/
       strncat(cachebuf,buf,size);
       rio_writen(connfd,buf,size);
    }
    buf[0] = '0';
    
    /* 相应体 */
    while((size = Rio_readlineb(&srp,buf,MAXLINE)) != 0) {
        rio_writen(connfd,buf,size);
        strncat(cachebuf,buf,size);
    }
    if((r = wcache(cachebuf,host,requuri,strlen(cachebuf))) != 0) {
        printf("Cache write successfully\n");
    }
    free(cachebuf);
#ifdef debug       
    printf("Solve before close\n");
#endif
    close(clientfd);
#ifdef debug       
    printf("After close\n");
#endif
}


int parse_uri(char *rawuri,char *host,char *requuri,char *port) 
{
    if(strstr(rawuri,"http://")) {
        rawuri += 7;
        size_t len = 0;
        while(rawuri[len] != '/' && rawuri[len] != ':') len++;
        strncpy(host,rawuri,len);
        if(rawuri[len] == ':') {
            rawuri += len;
            rawuri += 1;
            len = 0;
            while(rawuri[len] >= '0' && rawuri[len] <= '9') len++;
            strncpy(port,rawuri,len);
            rawuri += len;
        } 
        len = 0;
        while(rawuri[len] != ' ') len++;
        strncpy(requuri,rawuri,len);
        return 0;
    } else {
        return PROXY_INVALID_REQUEST_HEADER;
    }    
}

void clienterror(int fd,char *cause,char *errnum,
                char *shortmsg,char *longmsg) 
{
    char buf[MAXLINE],body[MAXBUF]; //生成一行写一行

    //响应body
    sprintf(body,"<html><title>Tiny Error</title>");
    sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body); /* 看不太懂 */
    sprintf(body,"%s%s:%s\r\n",body,errnum,shortmsg);
    sprintf(body,"%s<p>%s: %s\r\n",body,longmsg,cause);
    sprintf(body,"%s<hr><em>The Tiny Web server</em>\r\n",body);

    //响应头部
    sprintf(buf,"HTTP/1.0 %s %s \r\n",errnum,shortmsg);
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Content-type:text/html\r\n");
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Content-length:%d\r\n\r\n",(int)strlen(body));
    Rio_writen(fd,buf,strlen(buf));
    Rio_writen(fd,body,strlen(body));
}


void read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];
    Rio_readlineb(rp,buf,MAXLINE);
    while(strcmp(buf,"\r\n")) {
        Rio_readlineb(rp,buf,MAXLINE);
        printf("%s",buf);
    }
    return;
}

