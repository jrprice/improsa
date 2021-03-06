// Filter.h (ImProSA)
// Copyright (c) 2014, James Price and Simon McIntosh-Smith,
// University of Bristol. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

#pragma once

#include <CL/cl.h>
#include <math.h>
#include <stdarg.h>

#define CHECK_ERROR_OCL(err, op, action)                       \
  if (err != CL_SUCCESS)                                       \
  {                                                            \
    reportStatus("Error during operation '%s' (%d)", op, err); \
    releaseCL();                                               \
    action;                                                    \
  }

#ifndef BUFFER_T_DEFINED
#define BUFFER_T_DEFINED
typedef struct buffer_t {
    uint64_t dev;
    uint8_t* host;
    int32_t extent[4];
    int32_t stride[4];
    int32_t min[4];
    int32_t elem_size;
    bool host_dirty;
    bool dev_dirty;
} buffer_t;
#endif
extern "C" void halide_dev_sync(void *user_context);
extern "C" void halide_copy_to_dev(void *user_context, buffer_t *buf);
extern "C" void halide_copy_to_host(void *user_context, buffer_t *buf);
extern "C" void halide_release(void *user_context);

namespace improsa
{
  typedef struct
  {
    unsigned char *data;
    size_t width, height;
  } Image;

  class Filter
  {
  public:
    typedef struct _Params_
    {
      // General parameters
      bool verify;
      unsigned int iterations;

      // OpenCL parameters
      cl_device_type type;
      cl_uint platformIndex, deviceIndex;
      size_t wgsize[2];

      _Params_()
      {
        verify = true;
        iterations = 8;

        type = CL_DEVICE_TYPE_ALL;
        platformIndex = 0;
        deviceIndex = 0;
        wgsize[0] = wgsize[1] = 0;
      }
    } Params;

  public:
    Filter();
    virtual ~Filter();

    virtual void clearReferenceCache();
    virtual const char* getName() const;

    virtual bool runHalideCPU(Image input, Image output,
                              const Params& params) = 0;
    virtual bool runHalideGPU(Image input, Image output,
                              const Params& params) = 0;
    virtual bool runOpenCL(Image input, Image output,
                           const Params& params) = 0;
    virtual bool runReference(Image input, Image output) = 0;

    virtual void setStatusCallback(int (*callback)(const char*, va_list args));

  protected:
    const char *m_name;
    Image m_reference;
    int (*m_statusCallback)(const char*, va_list args);
    void reportStatus(const char *format, ...) const;
    virtual bool verify(Image input, Image output, int tolerance=1);

    double m_startTime, m_endTime;
    bool outputResults(Image input, Image output, const Params& params);
    void startTiming();
    void stopTiming();

    cl_device_id m_device;
    cl_context m_context;
    cl_command_queue m_queue;
    cl_program m_program;
    bool initCL(const Params& params, const char *source, const char *options);
    void releaseCL();
  };

  // Image utils
  buffer_t createHalideBuffer(Image image);
  float getPixel(Image image, int x, int y, int c);
  float getPixelGrayscale(Image image, int x, int y);
  void setPixel(Image image, int x, int y, int c, float value);
  void setPixelGrayscale(Image image, int x, int y, float value);

  // Timing utils
  double getCurrentTime();
}
