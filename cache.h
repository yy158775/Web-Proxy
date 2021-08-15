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


extern pthread_mutex_t read_lock;
extern pthread_mutex_t write_lock;
extern pthread_cond_t read_cond;
extern pthread_cond_t write_cond;

extern pthread_mutex_t rwlock;

extern size_t readers;
extern size_t writers;
extern size_t wait_readers;
extern size_t wait_writers;

void inin_cache();
int rcache(const char *host,const char *requuri,char *buf);
int wcache(const char *buf,const char *host,const char *requuri,size_t size);


#endif