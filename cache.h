#ifndef PROXY_CACHE_H__
#define PROXY_CACHE_H__

#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

typedef struct CacheObject
{
    char *buf; //cache size cache的大小
    char host[20];
    char requuri[50];
    struct CacheObject *next;
    struct CacheObject *last;
    size_t bsize;
}CacheObject;

extern CacheObject head; //头指针

extern size_t cachesize;

void inin_cache();
int rcache(const char *host,const char *requuri,char *buf);
int wcache(const char *buf,const char *host,const char *requuri,size_t size);


#endif