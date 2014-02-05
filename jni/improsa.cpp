#include <android/bitmap.h>
#include <android/log.h>
#include <jni.h>
#include <stdlib.h>

#include "Blur.h"

#define METHOD_REFERENCE  (1<<0)
#define METHOD_HALIDE_CPU (1<<1)
#define METHOD_HALIDE_GPU (1<<2)
#define METHOD_OPENCL     (1<<3)

#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "improsa", __VA_ARGS__)

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
    Java_com_jprice_improsa_ImProSA_00024ProcessTask_process(
      JNIEnv *env, jobject obj,
      jobject bmpInput, jobject bmpOutput,
      jint filterIndex, jint filterMethod,
      jint width, jint height)
  {
    // Get handle to status update method
    jmethodID updateStatus = env->GetMethodID(
      env->GetObjectClass(obj), "updateStatus", "(Ljava/lang/String;)V");
    #define STATUS(MSG) \
      env->CallVoidMethod(obj, updateStatus, env->NewStringUTF(MSG))

    // Ensure filter index in range
    if (filterIndex >= numFilters)
    {
      LOG("Filter index out of range (%d).", filterIndex);
      STATUS("Filter index out of range.");
      return;
    }

    // Lock bitmaps
    Image input = {NULL, width, height};
    Image output = {NULL, width, height};
    AndroidBitmap_lockPixels(env, bmpInput, (void**)&input.data);
    AndroidBitmap_lockPixels(env, bmpOutput, (void**)&output.data);

    // Run filter method
    Filter *filter = filters[filterIndex];
    switch (filterMethod)
    {
      case METHOD_REFERENCE:
        STATUS("Running reference");
        filter->runReference(input, output);
        STATUS("Finished reference");
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
        LOG("Invalid filter method (%d).", filterMethod);
        STATUS("Invalid filter method.");
        break;
    }

    // Unlock bitmaps
    AndroidBitmap_unlockPixels(env, bmpInput);
    AndroidBitmap_unlockPixels(env, bmpOutput);
  }
}
