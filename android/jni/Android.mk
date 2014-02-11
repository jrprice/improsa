LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC_PATH := ../../src

LOCAL_CFLAGS    += -I$(SRC_PATH) -g -Wno-deprecated-declarations
LOCAL_CFLAGS    += -DSHOW_REFERENCE_PROGRESS=1
LOCAL_MODULE    := improsa
LOCAL_SRC_FILES := improsa.cpp \
	$(SRC_PATH)/Filter.cpp \
	$(SRC_PATH)/Blur.cpp \
	halide/blur_cpu.s \
	halide/blur_gpu.s \
	$(SRC_PATH)/Sharpen.cpp \
	halide/sharpen_cpu.s \
	halide/sharpen_gpu.s \
	$(SRC_PATH)/Sobel.cpp \
	halide/sobel_cpu.s \
	halide/sobel_gpu.s
LOCAL_LDLIBS := -llog -ljnigraphics -lOpenCL

include $(BUILD_SHARED_LIBRARY)
