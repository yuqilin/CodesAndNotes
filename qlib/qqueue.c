#include "qqueue.h"


/*
 * Creates a new #QQueue.
 */
QQueue* q_queue_new(void)
{
    QQueue *queue = malloc(sizeof(QQueue));
    memset(queue, 0, sizeof(QQueue));
    return queue;
}

/*
 * Frees the memory allocated for the #QQueue. Only call this function
 * if @queue was created with g_queue_new(). If queue elements contain
 * dynamically-allocated memory, they should be freed first.
 *
 * If queue elements contain dynamically-allocated memory, you should
 * either use q_queue_free_full() or free them manually first.
 */
void q_queue_free(QQueue *queue)
{
    q_return_if_fail(queue != NULL);
    
    q_list_free(queue->head);
    free(queue);
}

/*
 * Convenience method, which frees all the memory used by a #QQueue,
 * and calls the specified destroy function on every element's data.
 */
void q_queue_free_full(QQueue *queue, QDestroyNotify free_func)
{
    q_queue_foreach(queue, (QFunc)free_func, NULL);
    q_queue_free(queue);
}

/*
 * A statically-allocated #QQueue must be initialized with this function
 * before it can be used. Alternatively you can initialize it with
 * #Q_QUEUE_INIT. It is not necessary to initialize queues created with
 * q_queue_new().
 */
void q_queue_init(QQueue *queue)
{
    q_return_if_fail(queue != NULL);

    queue->head = queue->tail = NULL;
    queue->length = 0;
}

/*
 * Removes all the elements in @queue. If queue elements contain
 * dynamically-allocated memory, they should be freed first.
 */
void q_queue_clear(QQueue *queue)
{
    q_return_if_fail(queue != NULL);

    q_list_free(queue->head);
    q_queue_init(queue);
}

/*
 * Returns %TRUE if the queue is empty.
 */
int q_queue_is_empty(QQueue *queue)
{
    q_return_val_if_fail(queue != NULL, TRUE);

    return queue->head == NULL;
}

/*
 * Returns the number of items in @queue.
 */
unsigned int q_queue_get_length(QQueue *queue)
{
    q_return_val_if_fail(queue != NULL, 0);

    return queue->length;
}

/*
 * Calls @func for each element in the queue passing @user_data to the function.
 */
void q_queue_foreach(QQueue *queue, QFunc func, void *user_data)
{
    QList *list;

    q_return_if_fail(queue != NULL);
    q_return_if_fail(func != NULL);

    list = queue->head;
    while (list) {
        QList *next = list->next;
        func(list->data, user_data);
        list = next;
    }
}

/*
 * Finds the first link in @queue which contains @data.
 */
QList* q_queue_find(QQueue *queue, const void *data)
{
    q_return_val_if_fail(queue != NULL, NULL);
    
    return q_list_find(queue->head, data);
}

/*
 * Adds a new element at the head of the queue.
 */
void q_queue_push_head(QQueue *queue, void *data)
{
    q_return_if_fail(queue != NULL);

    queue->head = q_list_prepend(queue->head, data);
    if (!queue->tail)
        queue->tail = queue->head;
    queue->length++;
}

/*
 * Adds a new element at the tail of the queue.
 */
void q_queue_push_tail(QQueue *queue, void *data)
{
    q_return_if_fail(queue != NULL);

    queue->tail = q_list_append(queue->tail, data);
    if (queue->tail->next)
        queue->tail = queue->tail->next;
    else
        queue->head = queue->tail;
    queue->length++;
}

/*
 * Removes the first element of the queue and returns its data.
 * Returns: the data of the first element in the queue, or %NULL if the queue is empty.
 */
void* q_queue_pop_head(QQueue *queue)
{
    q_return_val_if_fail(queue != NULL, NULL);

    if (queue->head) {
        QList *node = queue->head;
        void *data = node->data;

        queue->head = node->next;
        if (queue->head)
            queue->head->prev = NULL;
        else
            queue->tail = NULL;
        q_list_free_one(node);
        queue->length--;

        return data;
    }

    return NULL;
}

/*
 * Removes the last element of the queue and returns its data.
 * Returns: the data of the first element in the queue, or %NULL if the queue is empty.
 */
void* q_queue_pop_tail(QQueue *queue)
{
    q_return_val_if_fail(queue != NULL, NULL);

    if (queue->tail) {
        QList *node = queue->tail;
        void *data = node->data;

        queue->tail = node->prev;
        if (queue->tail)
            queue->tail->next = NULL;
        else
            queue->head = NULL;
        queue->length--;
        q_list_free_one(node);

        return data;
    }

    return NULL;
}

/*
 * Returns the first element of the queue.
 * Returns: the data of the first element in the queue, or %NULL
 *     if the queue is empty
 */
void* q_queue_peek_head(QQueue *queue)
{
    q_return_val_if_fail(queue != NULL, NULL);

    return queue->head ? queue->head->data : NULL;
}

/*
 * Returns the last element of the queue.
 *
 * Returns: the data of the last element in the queue, or %NULL
 *     if the queue is empty
 */
void* q_queue_peek_tail(QQueue *queue)
{
    q_return_val_if_fail(queue != NULL, NULL);

    return queue->tail ? queue->tail->data : NULL;
}

