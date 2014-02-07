#include <stddef.h>
#include <stdlib.h>
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
  }

  const char* Filter::getName() const
  {
    return m_name;
  }

  bool Filter::initCL(const char *source, const char *options)
  {
    // Ensure no existing context
    releaseCL();

    cl_int err;

    cl_platform_id platform;
    err = clGetPlatformIDs(1, &platform, NULL);
    CHECK_ERROR_OCL(err, "getting platforms", return false);

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &m_device, NULL);
    CHECK_ERROR_OCL(err, "getting devices", return false);

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
          int r = getPixel(ref, x, y, c);
          int o = getPixel(output, x, y, c);
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

  unsigned char getPixel(Image image, int x, int y, int c)
  {
    int _x = clamp(x, 0, image.width-1);
    int _y = clamp(y, 0, image.height-1);
    return image.data[(_x + _y*image.width)*4 + c];
  }

  void setPixel(Image image, int x, int y, int c, unsigned char value)
  {
    int _x = clamp(x, 0, image.width-1);
    int _y = clamp(y, 0, image.height-1);
    image.data[(_x + _y*image.width)*4 + c] = value;
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
