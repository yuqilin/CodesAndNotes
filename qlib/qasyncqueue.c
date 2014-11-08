#include "qasyncqueue.h"
#include <pthread.h>

/**
* QAsyncQueue:
*
* The QAsyncQueue struct is an opaque data structure which represents
* an asynchronous queue. It should only be accessed through the
* q_async_queue_* functions.
*/
struct _QAsyncQueue {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    QQueue queue;
    QDestroyNotify item_free_func;
    unsigned waiting_threads;
    int ref_count;
};

/*
 * Creates a new asynchronous queue.
 * Returns: a new #GAsyncQueue. Free with q_async_queue_unref()
 */
QAsyncQueue* q_async_queue_new(void)
{
    return q_async_queue_new_full(NULL);
}

/*
 * Creates a new asynchronous queue and sets up a destroy notify
 * function that is used to free any remaining queue items when
 * the queue is destroyed after the final unref.
 *
 * Returns: a new #GAsyncQueue. Free with g_async_queue_unref()
 */
QAsyncQueue*    q_async_queue_new_full(QDestroyNotify item_free_func)
{
    GAsyncQueue *queue;
    
    queue = malloc(sizeof(GAsyncQueue));
    memset(queue, 0, sizeof(queue));
    pthread_mutex_init(&queue->mutex);
    pthread_cond_init(&queue->cond);
    q_queue_init(&queue->queue);
    queue->waiting_threads = 0;
    queue->ref_count = 1;
    queue->item_free_func = item_free_func;

    return queue;
}

void            q_async_queue_lock(QAsyncQueue *queue);
void            q_async_queue_unlock(QAsyncQueue *queue);
QAsyncQueue *   q_async_queue_ref(QAsyncQueue *queue);
void            q_async_queue_unref(QAsyncQueue *queue);
void            q_async_queue_ref_unlocked(QAsyncQueue *queue);
void            q_async_queue_unref_and_unlock(QAsyncQueue *queue);
void            q_async_queue_push(QAsyncQueue *queue, void *data);
void            q_async_queue_push_unlocked(QAsyncQueue *queue, void *data);
void*           q_async_queue_pop(QAsyncQueue *queue);
void*           q_async_queue_pop_unlocked(QAsyncQueue *queue);
void*           q_async_queue_try_pop(QAsyncQueue *queue);
void*           q_async_queue_try_pop_unlocked(QAsyncQueue *queue);
void*           q_async_queue_timeout_pop(QAsyncQueue *queue, uint64_t timeout);
void*           q_async_queue_timeout_pop_unlocked(QAsyncQueue *queue, uint64_t timeout);
int             q_async_queue_length(QAsyncQueue *queue);
int             q_async_queue_length_unlocked(QAsyncQueue *queue);

