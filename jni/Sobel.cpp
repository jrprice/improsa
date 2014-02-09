#include <string.h>

#include "Sobel.h"
#include "opencl/sobel.h"
#include "halide/sobel_cpu.h"
#include "halide/sobel_gpu.h"

namespace improsa
{
  Sobel::Sobel() : Filter()
  {
    m_name = "Sobel";
    m_reference.data = NULL;
  }

  bool Sobel::runHalideCPU(Image input, Image output)
  {
    // Create halide buffers
    buffer_t inputBuffer = createHalideBuffer(input);
    buffer_t outputBuffer = createHalideBuffer(output);

    reportStatus("Running Halide CPU filter");

    // Warm-up run
    halide_sobel_cpu(&inputBuffer, &outputBuffer);

    // Timed runs
    const int iterations = 8;
    double start = getCurrentTime();
    for (int i = 0; i < iterations; i++)
    {
      halide_sobel_cpu(&inputBuffer, &outputBuffer);
    }
    double end = getCurrentTime();

    // Verification
    bool passed = verify(input, output);
    reportStatus(
      "Finished in %.1lf ms (verification %s)",
      (end-start)*1e-3/iterations, passed ? "passed" : "failed");

    halide_release(NULL);

    return passed;
  }

  bool Sobel::runHalideGPU(Image input, Image output)
  {
    // Create halide buffers
    buffer_t inputBuffer = createHalideBuffer(input);
    buffer_t outputBuffer = createHalideBuffer(output);

    reportStatus("Running Halide GPU filter");

    // Warm-up run
    inputBuffer.host_dirty = true;
    halide_sobel_gpu(&inputBuffer, &outputBuffer);
    halide_dev_sync(NULL);

    // Timed runs
    const int iterations = 8;
    double start = getCurrentTime();
    for (int i = 0; i < iterations; i++)
    {
      halide_sobel_gpu(&inputBuffer, &outputBuffer);
    }
    halide_dev_sync(NULL);
    double end = getCurrentTime();

    halide_copy_to_host(NULL, &outputBuffer);

    // Verification
    bool passed = verify(input, output);
    reportStatus(
      "Finished in %.1lf ms (verification %s)",
      (end-start)*1e-3/iterations, passed ? "passed" : "failed");

    halide_release(NULL);

    return passed;
  }

  bool Sobel::runOpenCL(Image input, Image output)
  {
    if (!initCL(sobel_kernel, "-cl-fast-relaxed-math"))
    {
      return false;
    }

    cl_int err;
    cl_kernel kernel;
    cl_mem d_input, d_output;
    cl_image_format format = {CL_RGBA, CL_UNORM_INT8};

    kernel = clCreateKernel(m_program, "sobel", &err);
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

  bool Sobel::runReference(Image input, Image output)
  {
    // Check for cached result
    if (m_reference.data)
    {
      memcpy(output.data, m_reference.data, output.width*output.height*4);
      reportStatus("Finished reference (cached)");
      return true;
    }

    const float mask[3][4] =
    {
      {-1, -2, -1},
      {0, 0, 0},
      {1, 2, 1}
    };

    for (int y = 0; y < output.height; y++)
    {
      for (int x = 0; x < output.width; x++)
      {
        float g_x = 0;
        float g_y = 0;
        for (int j = -1; j <= 1; j++)
        {
          for (int i = -1; i <= 1; i++)
          {
            g_x += getPixelGrayscale(input, x+i, y+j) * mask[i+1][j+1];
            g_y += getPixelGrayscale(input, x+i, y+j) * mask[j+1][i+1];
          }
        }
        float g_mag = sqrt(g_x*g_x + g_y*g_y);
        setPixelGrayscale(output, x, y, g_mag);
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
