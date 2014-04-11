// Filter.cpp (ImProSA)
// Copyright (c) 2014, James Price and Simon McIntosh-Smith,
// University of Bristol. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "Filter.h"

namespace improsa
{
  Filter::Filter()
  {
    m_statusCallback = NULL;
    m_context = 0;
    m_queue = 0;
    m_program = 0;
    m_reference.data = NULL;
  }

  Filter::~Filter()
  {
    clearReferenceCache();
  }

  void Filter::clearReferenceCache()
  {
    if (m_reference.data)
    {
      delete[] m_reference.data;
      m_reference.data = NULL;
    }
  }

  const char* Filter::getName() const
  {
    return m_name;
  }

  bool Filter::initCL(const Params& params,
                      const char *source, const char *options)
  {
    // Ensure no existing context
    releaseCL();

    cl_int err;
    cl_uint numPlatforms, numDevices;

    cl_platform_id platform, platforms[params.platformIndex+1];
    err = clGetPlatformIDs(params.platformIndex+1, platforms, &numPlatforms);
    CHECK_ERROR_OCL(err, "getting platforms", return false);
    if (params.platformIndex >= numPlatforms)
    {
      reportStatus("Platform index %d out of range (%d platforms found)",
        params.platformIndex, numPlatforms);
      return false;
    }
    platform = platforms[params.platformIndex];

    cl_device_id devices[params.deviceIndex+1];
    err = clGetDeviceIDs(platform, params.type,
                         params.deviceIndex+1, devices, &numDevices);
    CHECK_ERROR_OCL(err, "getting devices", return false);
    if (params.deviceIndex >= numDevices)
    {
      reportStatus("Device index %d out of range (%d devices found)",
        params.deviceIndex, numDevices);
      return false;
    }
    m_device = devices[params.deviceIndex];

    char name[64];
    clGetDeviceInfo(m_device, CL_DEVICE_NAME, 64, name, NULL);
    reportStatus("Using device: %s", name);

    m_context = clCreateContext(NULL, 1, &m_device, NULL, NULL, &err);
    CHECK_ERROR_OCL(err, "creating context", return false);

    m_queue = clCreateCommandQueue(m_context, m_device, 0, &err);
    CHECK_ERROR_OCL(err, "creating command queue", return false);

    m_program = clCreateProgramWithSource(m_context, 1, &source, NULL, &err);
    CHECK_ERROR_OCL(err, "creating program", return false);

    err = clBuildProgram(m_program, 1, &m_device, options, NULL, NULL);
    if (err == CL_BUILD_PROGRAM_FAILURE)
    {
      size_t sz;
      clGetProgramBuildInfo(
        m_program, m_device, CL_PROGRAM_BUILD_LOG, 0, NULL, &sz);
      char *log = (char*)malloc(++sz);
      clGetProgramBuildInfo(
        m_program, m_device, CL_PROGRAM_BUILD_LOG, sz, log, NULL);
      reportStatus(log);
      free(log);
    }
    CHECK_ERROR_OCL(err, "building program", return false);

    reportStatus("OpenCL context initialised.");
    return true;
  }

  bool Filter::outputResults(Image input, Image output, const Params& params)
  {
    // Verification
    bool success = true;
    const char *verifyStr = "";
    if (params.verify)
    {
      success = verify(input, output);
      if (success)
      {
        verifyStr = "(verification passed)";
      }
      else
      {
        verifyStr = "(verification failed)";
      }
    }

    // Compute average runtime
    double runtime = ((m_endTime-m_startTime)*1e-3)/params.iterations;

    // Find required DP for 2 significant figures
    int dp = 1 - floor(log10(runtime));

    // Output timing
    char fmt[32];
    sprintf(fmt, "Finished in %%.%dlf ms %%s", dp<0 ? 0 : dp);
    reportStatus(fmt, runtime, verifyStr);

    return success;
  }

  void Filter::releaseCL()
  {
    if (m_program)
    {
      clReleaseProgram(m_program);
      m_program = 0;
    }
    if (m_queue)
    {
      clReleaseCommandQueue(m_queue);
      m_queue = 0;
    }
    if (m_context)
    {
      clReleaseContext(m_context);
      m_context = 0;
    }
  }

  void Filter::reportStatus(const char *format, ...) const
  {
    if (m_statusCallback)
    {
      va_list args;
      va_start(args, format);
      m_statusCallback(format, args);
      va_end(args);
    }
  }

  void Filter::setStatusCallback(int (*callback)(const char*, va_list args))
  {
    m_statusCallback = callback;
  }

  void Filter::startTiming()
  {
    m_startTime = getCurrentTime();
  }

  void Filter::stopTiming()
  {
    m_endTime = getCurrentTime();
  }

  bool Filter::verify(Image input, Image output, int tolerance)
  {
    // Compute reference image
    Image ref =
    {
      (unsigned char*)malloc(output.width*output.height*4),
      output.width,
      output.height
    };
    runReference(input, ref);

    // Compare pixels
    int errors = 0;
    const int maxErrors = 16;
    for (int y = 0; y < output.height; y++)
    {
      for (int x = 0; x < output.width; x++)
      {
        for (int c = 0; c < 4; c++)
        {
          int r = getPixel(ref, x, y, c)*255;
          int o = getPixel(output, x, y, c)*255;
          int diff = abs(r - o);
          if (diff > tolerance)
          {
            // Only report first few errors
            if (errors < maxErrors)
            {
              reportStatus("Mismatch at (%d,%d,%d): %d vs %d", x, y, c, r, o);
            }
            if (++errors == maxErrors)
            {
              reportStatus("Supressing further errors");
            }
          }
        }
      }
    }

    free(ref.data);

    return errors == 0;
  }

  /////////////////
  // Image utils //
  /////////////////

  inline int clamp(int x, int min, int max)
  {
    return x < min ? min : x > max ? max : x;
  }

  inline float clamp(float x, float min, float max)
  {
    return x < min ? min : x > max ? max : x;
  }

  buffer_t createHalideBuffer(Image image)
  {
    buffer_t buffer = {0};
    buffer.host = image.data;
    buffer.extent[0] = image.width;
    buffer.extent[1] = image.height;
    buffer.extent[2] = 4;
    buffer.stride[0] = 4;
    buffer.stride[1] = image.width*4;
    buffer.stride[2] = 1;
    buffer.elem_size = 1;
    return buffer;
  }

  float getPixel(Image image, int x, int y, int c)
  {
    int _x = clamp(x, 0, image.width-1);
    int _y = clamp(y, 0, image.height-1);
    return image.data[(_x + _y*image.width)*4 + c]/255.f;
  }

  float getPixelGrayscale(Image image, int x, int y)
  {
    int _x = clamp(x, 0, image.width-1);
    int _y = clamp(y, 0, image.height-1);
    float r = image.data[(_x + _y*image.width)*4 + 0]/255.f * 0.299f;
    float g = image.data[(_x + _y*image.width)*4 + 1]/255.f * 0.587f;
    float b = image.data[(_x + _y*image.width)*4 + 2]/255.f * 0.114f;
    return (r + g + b);
  }

  void setPixel(Image image, int x, int y, int c, float value)
  {
    int _x = clamp(x, 0, image.width-1);
    int _y = clamp(y, 0, image.height-1);
    image.data[(_x + _y*image.width)*4 + c] = clamp(value, 0.f, 1.f)*255.f;
  }

  void setPixelGrayscale(Image image, int x, int y, float value)
  {
    int _x = clamp(x, 0, image.width-1);
    int _y = clamp(y, 0, image.height-1);
    image.data[(_x + _y*image.width)*4 + 0] = clamp(value, 0.f, 1.f)*255;
    image.data[(_x + _y*image.width)*4 + 1] = clamp(value, 0.f, 1.f)*255;
    image.data[(_x + _y*image.width)*4 + 2] = clamp(value, 0.f, 1.f)*255;
    image.data[(_x + _y*image.width)*4 + 3] = 255;
  }

  //////////////////
  // Timing utils //
  //////////////////

  double getCurrentTime()
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec + tv.tv_sec*1e6;
  }
}
