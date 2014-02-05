#include <android/bitmap.h>
#include <android/log.h>
#include <jni.h>
#include <stdlib.h>

#include "Blur.h"

#define METHOD_REFERENCE  (1<<0)
#define METHOD_HALIDE_CPU (1<<1)
#define METHOD_HALIDE_GPU (1<<2)
#define METHOD_OPENCL     (1<<3)

using namespace improsa;

extern "C"
{
  static Filter *filters[] =
  {
    new Blur(),
  };
  static const int numFilters = sizeof(filters) / sizeof(Filter);

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
      char *name = filters[i]->getName();
      env->SetObjectArrayElement(list, i, env->NewStringUTF(name));
      free(name);
    }
    return list;
  }

  JNIEXPORT void JNICALL
    Java_com_jprice_improsa_ImProSA_process(
      JNIEnv *env, jobject obj,
      jobject bmpInput, jobject bmpOutput,
      jint filterIndex, jint filterMethod,
      jint width, jint height)
  {
    if (filterIndex >= numFilters)
    {
      // TODO: Log/return error message
      return;
    }

    Image input = {NULL, width, height};
    Image output = {NULL, width, height};
    AndroidBitmap_lockPixels(env, bmpInput, (void**)&input.data);
    AndroidBitmap_lockPixels(env, bmpOutput, (void**)&output.data);

    Filter *filter = filters[filterIndex];
    switch (filterMethod)
    {
      case METHOD_REFERENCE:
        filter->runReference(input, output);
        break;
      case METHOD_HALIDE_CPU:
        // TODO
        break;
      case METHOD_HALIDE_GPU:
        // TODO
        break;
      case METHOD_OPENCL:
        // TODO
        break;
      default:
        // TODO: Log/return error message
        break;
    }

    AndroidBitmap_unlockPixels(env, bmpInput);
    AndroidBitmap_unlockPixels(env, bmpOutput);
  }
}
