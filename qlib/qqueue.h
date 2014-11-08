#ifndef __Q_QUEUE_H__
#define __Q_QUEUE_H__

#include "qlist.h"

typedef struct _QQueue QQueue;

struct _QQueue {
    QList *head;
    QList *tail;
    unsigned int length;
};

/*
 * To initialize a statically - allocated GQueue, use #Q_QUEUE_INIT or q_queue_init().
 */
#define Q_QUEUE_INIT { NULL, NULL, 0 }

QQueue*         q_queue_new(void);
void            q_queue_free(QQueue *queue);
void            q_queue_free_full(QQueue *queue, QDestroyNotify free_func);
void            q_queue_init(QQueue *queue);
void            q_queue_clear(QQueue *queue);
int             q_queue_is_empty(QQueue *queue);
unsigned int    q_queue_get_length(QQueue *queue);
void            q_queue_foreach(QQueue *queue, QFunc func, void *user_data);
QList*          q_queue_find(QQueue *queue, const void *data);
void            q_queue_push_head(QQueue *queue, void *data);
void            q_queue_push_tail(QQueue *queue, void *data);
void*           q_queue_pop_head(QQueue *queue);
void*           q_queue_pop_tail(QQueue *queue);
void*           q_queue_peek_head(QQueue *queue);
void*           q_queue_peek_tail(QQueue *queue);



#endif