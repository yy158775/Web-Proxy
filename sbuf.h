#ifndef PROXY_SBUF_H__
#define PROXY_SBUF_H__

#include "csapp.h"

#define MAXCON 32

typedef struct {
    int *buf_;
    size_t n_;
    int front_;
    int rear_;
    int slots_;
    int items_;
}s_buf;

void sbuf_init(s_buf *s);
void sbuf_insert(s_buf *s,int item);
int sbuf_remove(s_buf *s);
void sbuf_free(s_buf *s);

#endif 