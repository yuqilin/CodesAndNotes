#ifndef __Q_ASYNCQUEUE_H__
#define __Q_ASYNCQUEUE_H__

typedef struct _QAsyncQueue QAsyncQueue;

QAsyncQueue*    q_async_queue_new(void);
QAsyncQueue*    q_async_queue_new_full(QDestroyNotify item_free_func);
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

#endif /* __Q_ASYNCQUEUE_H__ */