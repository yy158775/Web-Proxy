#ifndef PROXY_SBUF_H__
#define PROXY_SBUF_H__

#include "csapp.h"

#define MAXCON 32

typedef struct {
    int *buf;
    size_t n;
    int front;
    int rear;
    int slots;
    int items;
}s_buf;

extern pthread_cond_t cond_slot; //slot in insert function 
extern pthread_cond_t cond_item; 
extern pthread_mutex_t mutex_slot;
extern pthread_mutex_t mutex_item;


void sbuf_init(s_buf *s);
void sbuf_insert(s_buf *s,int item);
int sbuf_remove(s_buf *s);
void sbuf_free(s_buf *s);

#endif 