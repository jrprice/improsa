#include <string.h>

#include "Blur.h"
#include "opencl/blur.h"
#include "halide/blur_cpu.h"
#include "halide/blur_gpu.h"

namespace improsa
{
  Blur::Blur() : Filter()
  {
    m_name = "Blur";
    m_reference.data = NULL;
  }

  Blur::~Blur()
  {
    if (m_reference.data)
    {
      delete[] m_reference.data;
    }
  }

  bool Blur::runHalideCPU(Image input, Image output)
  {
    // Create halide buffers
    buffer_t inputBuffer = createHalideBuffer(input);
    buffer_t outputBuffer = createHalideBuffer(output);

    reportStatus("Running Halide CPU filter");

    // Warm-up run
    halide_blur_cpu(&inputBuffer, &outputBuffer);

    // Timed runs
    const int iterations = 8;
    double start = getCurrentTime();
    for (int i = 0; i < iterations; i++)
    {
      halide_blur_cpu(&inputBuffer, &outputBuffer);
    }
    double end = getCurrentTime();

    // Verification
    bool passed = verify(input, output);
    reportStatus(
      "Finished in %.1lf ms (verification %s)",
      (end-start)*1e-3/iterations, passed ? "passed" : "failed");

    return passed;
  }

  bool Blur::runHalideGPU(Image input, Image output)
  {
    // Create halide buffers
    buffer_t inputBuffer = createHalideBuffer(input);
    buffer_t outputBuffer = createHalideBuffer(output);

    reportStatus("Running Halide GPU filter");

    // Warm-up run
    inputBuffer.host_dirty = true;
    halide_blur_gpu(&inputBuffer, &outputBuffer);
    halide_dev_sync(NULL);

    // Timed runs
    const int iterations = 8;
    double start = getCurrentTime();
    for (int i = 0; i < iterations; i++)
    {
      halide_blur_gpu(&inputBuffer, &outputBuffer);
    }
    halide_dev_sync(NULL);
    double end = getCurrentTime();

    halide_copy_to_host(NULL, &outputBuffer);

    // Verification
    bool passed = verify(input, output);
    reportStatus(
      "Finished in %.1lf ms (verification %s)",
      (end-start)*1e-3/iterations, passed ? "passed" : "failed");

    return passed;
  }

  bool Blur::runOpenCL(Image input, Image output)
  {
    if (!initCL(blur_kernel, "-cl-fast-relaxed-math"))
    {
      return false;
    }

    cl_int err;
    cl_kernel kernel;
    cl_mem d_input, d_output;
    cl_image_format format = {CL_RGBA, CL_UNORM_INT8};

    kernel = clCreateKernel(m_program, "blur", &err);
    CHECK_ERROR_OCL(err, "creating kernel", return false);

    d_input = clCreateImage2D(
      m_context, CL_MEM_READ_ONLY, &format,
      input.width, input.height, 0, NULL, &err);
    CHECK_ERROR_OCL(err, "creating input image", return false);

    d_output = clCreateImage2D(
      m_context, CL_MEM_WRITE_ONLY, &format,
      input.width, input.height, 0, NULL, &err);
    CHECK_ERROR_OCL(err, "creating output image", return false);

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {input.width, input.height, 1};
    err = clEnqueueWriteImage(
      m_queue, d_input, CL_TRUE,
      origin, region, 0, 0, input.data, 0, NULL, NULL);
    CHECK_ERROR_OCL(err, "writing image data", return false);

    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_input);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_output);
    CHECK_ERROR_OCL(err, "setting kernel arguments", return false);

    reportStatus("Running OpenCL kernel");

    size_t global[2] = {output.width, output.height};

    // Timed runs
    int iterations = 8;
    double start = getCurrentTime();
    for (int i = 0; i < iterations + 1; i++)
    {
      err = clEnqueueNDRangeKernel(
        m_queue, kernel, 2, NULL, global, NULL, 0, NULL, NULL);
      CHECK_ERROR_OCL(err, "enqueuing kernel", return false);

      // Start timing after warm-up run
      if (i == 0)
      {
        err = clFinish(m_queue);
        CHECK_ERROR_OCL(err, "running kernel", return false);
        start = getCurrentTime();
      }
    }
    err = clFinish(m_queue);
    CHECK_ERROR_OCL(err, "running kernel", return false);
    double end = getCurrentTime();

    reportStatus("Finished OpenCL kernel");

    err = clEnqueueReadImage(
      m_queue, d_output, CL_TRUE,
      origin, region, 0, 0, output.data, 0, NULL, NULL);
    CHECK_ERROR_OCL(err, "reading image data", return false);

    // Verification
    bool passed = verify(input, output);
    reportStatus(
      "Finished in %.1lf ms (verification %s)",
      (end-start)*1e-3/iterations, passed ? "passed" : "failed");

    clReleaseMemObject(d_input);
    clReleaseMemObject(d_output);
    clReleaseKernel(kernel);
    releaseCL();
    return passed;
  }

  bool Blur::runReference(Image input, Image output)
  {
    // Check for cached result
    if (m_reference.data)
    {
      memcpy(output.data, m_reference.data, output.width*output.height*4);
      reportStatus("Finished reference (cached)");
      return true;
    }

    for (int y = 0; y < output.height; y++)
    {
      for (int x = 0; x < output.width; x++)
      {
        float r = 0;
        float g = 0;
        float b = 0;
        for (int j = -2; j <= 2; j++)
        {
          for (int i = -2; i <= 2; i++)
          {
            r += getPixel(input, x+i, y+j, 0)/255.f;
            g += getPixel(input, x+i, y+j, 1)/255.f;
            b += getPixel(input, x+i, y+j, 2)/255.f;
          }
        }
        setPixel(output, x, y, 0, 255*r/25.f);
        setPixel(output, x, y, 1, 255*g/25.f);
        setPixel(output, x, y, 2, 255*b/25.f);
        setPixel(output, x, y, 3, getPixel(input, x, y, 3));
      }
      reportStatus("Completed %.1f%% of reference", (100.f*y)/(input.height-1));
    }
    reportStatus("Finished reference");

    // Cache result
    m_reference.width = output.width;
    m_reference.height = output.height;
    m_reference.data = new unsigned char[output.width*output.height*4];
    memcpy(m_reference.data, output.data, output.width*output.height*4);

    return true;
  }
}
