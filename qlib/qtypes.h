#ifndef __Q_TYPES_H__
#define __Q_TYPES_H__

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

typedef void(*QDestroyNotify) (void *data);
typedef void(*QFunc) (void *data, void *user_data);


#define q_return_if_fail(expr)  do { \
    if (expr) {} else { return; } \
    } while(0);

#define q_return_val_if_fail(expr, val)  do { \
    if (expr) {} else { return; } \
    } while(0);

#endif