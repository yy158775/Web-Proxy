#include "csapp.h"
#include "csapp.h"
#include "sbuf.h"

static pthread_cond_t cond_slot;
static pthread_cond_t cond_item;
static pthread_mutex_t mutex_slot;
static pthread_mutex_t mutex_item;


void sbuf_init(s_buf *s) {
    s->buf_ = (int *)malloc(sizeof(int)*MAXCON);
    s->front_ = s->rear_ = 0;
    s->slots_ = 32;
    s->items_ = 0;
    s->n_ = 32;
}

void sbuf_insert(s_buf *s,int item) {
    pthread_mutex_lock(&mutex_slot);
        while(s->slots_ == 0)
            pthread_cond_wait(&cond_slot,&mutex_slot);
    s->buf_[(s->rear_++)%(s->n_)] = item;
    s->slots_--;
    pthread_mutex_unlock(&mutex_slot);

    pthread_mutex_lock(&mutex_item);
    s->items_++;
    if(s->items_ == 1)
        pthread_cond_signal(&cond_item);
    pthread_mutex_unlock(&mutex_item);

}

/* 保护的是变量还是过程 */

int sbuf_remove(s_buf *s) {
    int res;
    pthread_mutex_lock(&mutex_item);
    while (s->items_ == 0)  
        pthread_cond_wait(&cond_item,&mutex_item);
    res = s->buf_[(s->front_++)%(s->n_)];
    s->items_--;
    pthread_mutex_unlock(&mutex_item);

    pthread_mutex_lock(&mutex_slot);
    s->slots_++;
    if(s->slots_ == 1)
        pthread_cond_signal(&cond_slot);
    pthread_mutex_unlock(&mutex_slot);
    return res;
}


void sbuf_free(s_buf *s) {
    free(s->buf_);
}