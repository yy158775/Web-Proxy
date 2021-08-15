#include "csapp.h"
#include "csapp.h"
#include "sbuf.h"

pthread_cond_t cond_slot;
pthread_cond_t cond_item;
pthread_mutex_t mutex_slot;
pthread_mutex_t mutex_item;


void sbuf_init(s_buf *s) {
    s->buf = (int *)malloc(sizeof(int)*MAXCON);
    s->front = s->rear = 0;
    s->slots = 32;
    s->items = 0;
    s->n = 32;
}

void sbuf_insert(s_buf *s,int item) {
    pthread_mutex_lock(&mutex_slot);
        while(s->slots == 0)
            pthread_cond_wait(&cond_slot,&mutex_slot);
    s->buf[(s->rear++)%(s->n)] = item;
    s->slots--;
    pthread_mutex_unlock(&mutex_slot);

    pthread_mutex_lock(&mutex_item);
    s->items++;
    if(s->items == 1)
        pthread_cond_signal(&cond_item);
    pthread_mutex_unlock(&mutex_item);

}

/* 保护的是变量还是过程 */

int sbuf_remove(s_buf *s) {
    int res;
    pthread_mutex_lock(&mutex_item);
    while (s->items == 0)  
        pthread_cond_wait(&cond_item,&mutex_item);
    res = s->buf[(s->front++)%(s->n)];
    s->items--;
    pthread_mutex_unlock(&mutex_item);

    pthread_mutex_lock(&mutex_slot);
    s->slots++;
    if(s->slots == 1)
        pthread_cond_signal(&cond_slot);
    pthread_mutex_unlock(&mutex_slot);
    return res;
}


void sbuf_free(s_buf *s) {
    free(s->buf);
}