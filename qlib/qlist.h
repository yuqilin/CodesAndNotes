
#ifndef __Q_LIST_H__
#define __Q_LIST_H__

#include "qtypes.h"

typedef struct _QList QList;

struct _QList {
    void *data;
    _QList *prev;
    _QList *next;
};

QList*      q_list_alloc(void);
void        q_list_free(QList *list);
void        q_list_free_one(QList *list);
void        q_list_free_full(QList *list, QDestroyNotify free_func);
QList*      q_list_append(QList *list, void *data);
QList*      q_list_prepend(QList *list, void *data);
QList*      q_list_remove(QList *list, const void *data);
QList*      q_list_find(QList *list, const void *data);
QList*      q_list_last(QList *list);
QList*      q_list_first(QList* list);
int         q_list_length(QList *list);
void        q_list_foreach(QList *list, QFunc func, void *user_data);

#endif