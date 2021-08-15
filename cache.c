#include "csapp.h"
#include <stdio.h>
#include "cache.h"
#include "proxy.h"


/* 读者写者用几把锁来表示 一把锁还是两把锁 */

//读者写者问题
//直接实现一个读写锁吧
//和消费者 生产者不同 读者优先
CacheObject head; //头指针
size_t cachesize = 0;

pthread_mutex_t read_lock;
pthread_mutex_t write_lock;
pthread_cond_t read_cond;
pthread_cond_t write_cond;


pthread_mutex_t rwlock;
size_t readers = 0;
size_t writers = 0;
size_t wait_readers = 0;
size_t wait_writers = 0;


void inin_cache() {
    head.next = &head;
    head.last = &head;
}


void readlock() {
    pthread_mutex_lock(&read_lock);
    readers++;
    if(readers == 1) 
        pthread_mutex_lock(&write_lock);
    pthread_mutex_unlock(&read_lock);
}

void readunlock() {
    pthread_mutex_lock(&read_lock);
    readers--;
    if(readers == 0)
        pthread_mutex_unlock(&write_lock);
    pthread_mutex_unlock(&read_lock);
}

void writelock() {
    pthread_mutex_lock(&write_lock);
}


void writeunlock() {
    pthread_mutex_unlock(&write_lock);
}


int wcache(const char *buf,const char *host,const char *requuri,size_t size) {
    writelock();
    /* 太大了被淘汰 */
#ifdef debug
    printf("begin to write cache\n");
#endif
    if(size >= MAX_OBJECT_SIZE) {
        writeunlock();
        return PROXY_CACHE_OBJECT_TOOLARGE;
    }
    /* 移走一些尾部的LRU */
    while(cachesize + size >= MAX_CACHE_SIZE)
    {
        CacheObject *lastobj = head.last;
        if(lastobj == &head) {
            writeunlock();
            return PROXY_CACHE_OBJECT_TOOLARGE;
        }
        head.last = lastobj->last;
        lastobj->last->next = &head;
        free(lastobj->buf);
        cachesize -= lastobj->bsize;
    }
    /* 注意这一点是插入链表的，因此即便这个函数返回后 这个连边里面东西不能变，因此必须在堆上 */
    CacheObject *nextobj = head.next;
    CacheObject *newobj;  /* 如果是一开始呢 */
    newobj = (CacheObject *)malloc(sizeof(CacheObject));
    strcpy(newobj->host,host); /* host 必须以零结尾 strcpy 这些函数都会自动补0 */
    strcpy(newobj->requuri,requuri);
    newobj->buf = (char *)malloc(size + 1);
    strcpy(newobj->buf,buf);
    newobj->bsize = size;
#ifdef debug
    printf("insert cache\n");
#endif

    newobj->last = &head;
    newobj->next = nextobj;
    nextobj->last = newobj;
    head.next = newobj;
    cachesize += size;
    writeunlock();
    return 0;
}

int rcache(const char *host,const char *requuri,char *buf) {
    readlock();
    CacheObject *obj = head.next;
    while(obj != &head) {
        if(!strcmp(obj->host,host) && !strcmp(obj->requuri,requuri)) { /* 匹配成功 */
            strcpy(buf,obj->buf);
            readunlock();
#ifdef debug
    printf("rcache success\n");
#endif
            writelock();
            obj->last->next = obj->next;            
            obj->next->last = obj->last;

            obj->last = &head;
            obj->next = head.next;
            head.next->last = obj;
            head.next = obj;
            writeunlock();
            return 0;
        }
#ifdef debug
    printf("repeat cache\n");
#endif
        obj = obj->next;
    }
    readunlock();
    return PROXY_CACHE_MISS;
}