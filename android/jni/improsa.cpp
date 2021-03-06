// improsa.cpp (ImProSA)
// Copyright (c) 2014, James Price and Simon McIntosh-Smith,
// University of Bristol. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

#include <android/bitmap.h>
#include <android/log.h>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>

#include "Bilateral.h"
#include "Blur.h"
#include "Copy.h"
#include "Sharpen.h"
#include "Sobel.h"

#define METHOD_REFERENCE  (1<<1)
#define METHOD_HALIDE_CPU (1<<2)
#define METHOD_HALIDE_GPU (1<<3)
#define METHOD_OPENCL     (1<<4)

using namespace improsa;

extern "C"
{
  static Filter *filters[] =
  {
    new Copy(),
    new Bilateral(),
    new Blur(),
    new Sharpen(),
    new Sobel()
  };
  static const int numFilters = sizeof(filters) / sizeof(Filter*);

  static Filter::Params params;

  JNIEXPORT void JNICALL
    Java_com_jprice_improsa_ImProSA_clearReferenceCache(
      JNIEnv *env, jobject obj)
  {
    for (int i = 0; i < numFilters; i++)
    {
      filters[i]->clearReferenceCache();
    }
  }

  JNIEXPORT jobjectArray JNICALL
    Java_com_jprice_improsa_ImProSA_getFilterList(
      JNIEnv *env, jobject obj)
  {
    jobjectArray list = (jobjectArray)env->NewObjectArray(
      numFilters,
      env->FindClass("java/lang/String"),
      env->NewStringUTF(""));
    for (int i = 0; i < numFilters; i++)
    {
      jstring name = env->NewStringUTF(filters[i]->getName());
      env->SetObjectArrayElement(list, i, name);
      env->DeleteLocalRef(name);
    }
    return list;
  }

  JNIEXPORT void JNICALL
    Java_com_jprice_improsa_ImProSA_setIterations(
      JNIEnv *env, jobject obj, jint i)
  {
    params.iterations = i;
  }

  JNIEXPORT void JNICALL
    Java_com_jprice_improsa_ImProSA_setVerificationEnabled(
      JNIEnv *env, jobject obj, jboolean enabled)
  {
    params.verify = enabled;
  }

  JNIEXPORT void JNICALL
    Java_com_jprice_improsa_ImProSA_setWorkGroupSize(
      JNIEnv *env, jobject obj, jint x, jint y)
  {
    params.wgsize[0] = x;
    params.wgsize[1] = y;
  }

  // Handles for sending status updates to GUI
  JNIEnv *m_env;
  jobject m_obj;
  jmethodID m_updateStatus;

  int updateStatus(const char *format, va_list args)
  {
    // Generate message
    size_t sz = vsnprintf(NULL, 0, format, args) + 1;
    char *msg = (char*)malloc(sz);
    vsprintf(msg, format, args);

    // Send message to log and GUI
    jstring jmsg = m_env->NewStringUTF(msg);
    __android_log_print(ANDROID_LOG_DEBUG, "improsa", "%s", msg);
    m_env->CallVoidMethod(m_obj, m_updateStatus, jmsg);
    m_env->DeleteLocalRef(jmsg);

    free(msg);

    return 0;
  }

  // Variadic argument wrapper for updateStatus
  void status(const char *fmt, ...)
  {
    va_list args;
    va_start(args, fmt);
    updateStatus(fmt, args);
    va_end(args);
  }

  JNIEXPORT jboolean JNICALL
    Java_com_jprice_improsa_ImProSA_00024ProcessTask_process(
      JNIEnv *env, jobject obj,
      jobject bmpInput, jobject bmpOutput,
      jint filterIndex, jint filterMethod,
      jint width, jint height)
  {
    // Set handles for status updates
    m_env = env;
    m_obj = obj;
    m_updateStatus = env->GetMethodID(
      env->GetObjectClass(obj), "updateStatus", "(Ljava/lang/String;)V");

    // Ensure filter index in range
    if (filterIndex >= numFilters)
    {
      status("Filter index out of range (%d).", filterIndex);
      return false;
    }

    // Lock bitmaps
    Image input = {NULL, width, height};
    Image output = {NULL, width, height};
    AndroidBitmap_lockPixels(env, bmpInput, (void**)&input.data);
    AndroidBitmap_lockPixels(env, bmpOutput, (void**)&output.data);

    // Force use of GPU for Halide pipelines
    setenv("HL_OCL_DEVICE", "gpu", 1);

    // Run filter method
    jboolean success = false;
    Filter *filter = filters[filterIndex];
    filter->setStatusCallback(updateStatus);
    switch (filterMethod)
    {
      case METHOD_REFERENCE:
        success = filter->runReference(input, output);
        break;
      case METHOD_HALIDE_CPU:
        success = filter->runHalideCPU(input, output, params);
        break;
      case METHOD_HALIDE_GPU:
        success = filter->runHalideGPU(input, output, params);
        break;
      case METHOD_OPENCL:
        params.type = CL_DEVICE_TYPE_GPU;
        success = filter->runOpenCL(input, output, params);
        break;
      default:
        status("Invalid filter method (%d).", filterMethod);
        break;
    }

    // Unlock bitmaps
    AndroidBitmap_unlockPixels(env, bmpInput);
    AndroidBitmap_unlockPixels(env, bmpOutput);

    return success;
  }
}
