# Android.mk (ImProSA)
# Copyright (c) 2014, James Price and Simon McIntosh-Smith,
# University of Bristol. All rights reserved.
#
# This program is provided under a three-clause BSD license. For full
# license terms please see the LICENSE file distributed with this
# source code.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC_PATH := ../../src

LOCAL_CFLAGS    += -I$(SRC_PATH) -g -Wno-deprecated-declarations
LOCAL_CFLAGS    += -DSHOW_REFERENCE_PROGRESS=1
LOCAL_MODULE    := improsa
LOCAL_SRC_FILES := improsa.cpp \
	$(SRC_PATH)/Filter.cpp \
	$(SRC_PATH)/Bilateral.cpp \
	$(SRC_PATH)/Blur.cpp \
	$(SRC_PATH)/Copy.cpp \
	$(SRC_PATH)/Sharpen.cpp \
	$(SRC_PATH)/Sobel.cpp

ifeq ($(HALIDE),1)
LOCAL_CFLAGS    += -DENABLE_HALIDE=1
LOCAL_SRC_FILES += \
	halide/bilateral_cpu.s \
	halide/bilateral_gpu.s \
	halide/blur_cpu.s \
	halide/blur_gpu.s \
	halide/sharpen_cpu.s \
	halide/sharpen_gpu.s \
	halide/sobel_cpu.s \
	halide/sobel_gpu.s
endif

LOCAL_LDLIBS := -llog -ljnigraphics -lOpenCL

include $(BUILD_SHARED_LIBRARY)
