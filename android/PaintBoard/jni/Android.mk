LOCAL_PATH		:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE 	:= ImageProc
LOCAL_SRC_FILES	:= com_myandroid_paintboard_ImageProc.c
LOCAL_LDLIBS	:= -llog -ljnigraphics

include $(BUILD_SHARED_LIBRARY)