#include "qlist.h"

/*
 * Allocates space for one #QList element.
 */
QList*  q_list_alloc(void)
{
    QList *list = malloc(sizeof(QList));
    memset(list, 0, sizeof(QList));
    return list;
}

/*
 * Frees all of the memory used by a #QList
 * If list elements contain dynamically-allocated memory, you should
 * either use g_list_free_full() or free them manually first.
 */
void q_list_free(QList *qlist)
{
    while (qlist) {
        free(qlist);
        qlist = qlist->next;
    }
}

/*
 * Frees one #QList element.
 * It is usually used after q_list_remove_link().
 */
void q_list_free_one(QList *list)
{
    free(list);
}

/*
 * Convenience method, which frees all the memory used by a #QList,
 * and calls @free_func on every element's data.
 */
void q_list_free_full(QList *list, QDestroyNotify free_func)
{
    q_list_foreach(list, (QFunc)free_func, NULL);
    q_list_free(list);
}

/*
 * Adds a new element on to the end of the list.
 * Return the new start of the list.
 */
QList *q_list_append(QList *list, void *data)
{
    QList *new_list;
    QList *last;

    new_list = q_list_alloc();
    new_list->data = data;
    new_list->next = NULL;

    if (list) {
        last = q_list_last(list);
        /* assert (last != NULL); */
        last->next = new_list;
        new_list->prev = last;

        return list;
    } else {
        new_list->prev = NULL;
        return new_list;
    }
}

/*
 * Prepends a new element on to the start of the list.
 * Return the new start of the list.
 */
QList *q_list_prepend (QList *list, void *data)
{
    QList *new_list;

    new_list = q_list_alloc ();
    new_list->data = data;
    new_list->next = list;

    if (list) {
        new_list->prev = list->prev;
        if (list->prev)
            list->prev->next = new_list;
        list->prev = new_list;
    } else {
        new_list->prev = NULL;
    }

    return new_list;
}

static inline QList* _q_list_remove_link(QList *list, QList *link)
{
    if (link == NULL)
        return list;

    if (link->prev) {
        if (link->prev->next == link) {
            link->prev->next = link->next;
        } else {
            //g_warning("corrupted double-linked list detected");
        }
    }
    if (link->next) {
        if (link->next->prev == link) {
            link->next->prev = link->prev;
        } else {
            //g_warning("corrupted double-linked list detected");
        }
    }

    if (link == list)
        list = list->next;

    link->next = NULL;
    link->prev = NULL;

    return list;
}

/*
 * Removes an element from a #QList.
 * If two elements contain the same data, only the first is removed.
 * If none of the elements contain the data, the #QList is unchanged.
 */
QList*  q_list_remove(QList *list, const void *data)
{
    QList *tmp;

    tmp = list;
    while (tmp) {
        if (tmp->data != data)
            tmp = tmp->next;
        else {
            list = _q_list_remove_link(list, tmp);
            q_list_free(tmp);
            break;
        }
    }
    return list;
}

/*
 * Finds the element in a #QList which contains the given data.
 */
QList* q_list_find(QList *list,const void *data)
{
    while (list) {
        if (list->data == data)
            break;
        list = list->next;
    }
    return list;
}

/*
 * Gets the last element in a #QList.
 */
QList* q_list_last(QList *list)
{
    if (list) {
        while (list->next)
            list = list->next;
    }
    return list;
}

/*
* Gets the first element in a #QList.
*/
QList* q_list_first(QList *list)
{
    if (list) {
        while (list->prev)
            list = list->prev;
    }
    return list;
}

/*
 *Gets the number of elements in a #QList.
 */
int q_list_length(QList *list)
{
    int length = 0;
    while (list) {
        length++;
        list = list->next;
    }
    return length;
}

/*
 * Calls a function for each element of a #QList.
 */
void q_list_foreach(QList *list, QFunc func, void *user_data)
{
    while (list) {
        QList *next = list->next;
        (*func) (list->data, user_data);
        list = next;
    }
}
