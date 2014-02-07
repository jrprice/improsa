LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS    += -g -Wno-deprecated-declarations
LOCAL_MODULE    := improsa
LOCAL_SRC_FILES := improsa.cpp Filter.cpp
LOCAL_SRC_FILES += Blur.cpp halide/blur_cpu.s halide/blur_gpu.s
LOCAL_LDLIBS    := -llog -ljnigraphics -lOpenCL

include $(BUILD_SHARED_LIBRARY)
