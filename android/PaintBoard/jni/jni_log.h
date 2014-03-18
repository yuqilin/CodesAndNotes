#include <android/log.h>
#ifndef _Included_jni_log
#define _Included_jni_log

#define LOGTAG			"PaintBoard"

#define LOGV(...)		__android_log_print(ANDROID_LOG_VERBOSE,LOGTAG, __VA_ARGS__)
#define LOGD(...)		__android_log_print(ANDROID_LOG_DEBUG, 	LOGTAG, __VA_ARGS__)
#define LOGI(...)		__android_log_print(ANDROID_LOG_INFO, 	LOGTAG, __VA_ARGS__)
#define LOGW(...)		__android_log_print(ANDROID_LOG_WARN, 	LOGTAG, __VA_ARGS__)
#define LOGE(...)		__android_log_print(ANDROID_LOG_ERROR, 	LOGTAG, __VA_ARGS__)
#define LOGF(...)		__android_log_print(ANDROID_LOG_FATAL, 	LOGTAG, __VA_ARGS__)

#endif
