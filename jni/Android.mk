LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS    += -g
LOCAL_MODULE    := improsa
LOCAL_SRC_FILES := improsa.cpp Filter.cpp Blur.cpp
LOCAL_LDLIBS    := -llog -ljnigraphics -lOpenCL

include $(BUILD_SHARED_LIBRARY)
