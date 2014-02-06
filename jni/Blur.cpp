#include <string.h>

#include "Blur.h"
#include "kernels/blur.h"

#include <CL/cl.h>

namespace improsa
{
  Blur::Blur() : Filter()
  {
    m_name = "Blur";
  }

  void Blur::runHalideCPU(Image input, Image output)
  {
    reportStatus("runHalideCPU not implemented");
  }

  void Blur::runHalideGPU(Image input, Image output)
  {
    reportStatus("runHalideGPU not implemented");
  }

  void Blur::runOpenCL(Image input, Image output)
  {
    if (!initCL(blur_kernel))
    {
      return;
    }

    cl_int err;
    cl_kernel kernel;
    cl_mem d_input, d_output;
    cl_image_format format = {CL_RGBA, CL_UNSIGNED_INT8};

    kernel = clCreateKernel(m_program, "blur", &err);
    CHECK_ERROR_OCL(err, "creating kernel", return);

    d_input = clCreateImage2D(
      m_context, CL_MEM_READ_ONLY, &format,
      input.width, input.height, 0, NULL, &err);
    CHECK_ERROR_OCL(err, "creating input image", return);

    d_output = clCreateImage2D(
      m_context, CL_MEM_WRITE_ONLY, &format,
      input.width, input.height, 0, NULL, &err);
    CHECK_ERROR_OCL(err, "creating output image", return);

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {input.width, input.height, 1};
    err = clEnqueueWriteImage(
      m_queue, d_input, CL_TRUE,
      origin, region, 0, 0, input.data, 0, NULL, NULL);
    CHECK_ERROR_OCL(err, "writing image data", return);

    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_input);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_output);
    CHECK_ERROR_OCL(err, "setting kernel arguments", return);

    reportStatus("Running OpenCL kernel");

    double start = getCurrentTime();

    size_t global[2] = {output.width, output.height};
    err = clEnqueueNDRangeKernel(
      m_queue, kernel, 2, NULL, global, NULL, 0, NULL, NULL);
    CHECK_ERROR_OCL(err, "enqueuing kernel", return);

    err = clFinish(m_queue);
    CHECK_ERROR_OCL(err, "running kernel", return);

    double end = getCurrentTime();

    reportStatus("Finished OpenCL kernel");

    err = clEnqueueReadImage(
      m_queue, d_output, CL_TRUE,
      origin, region, 0, 0, output.data, 0, NULL, NULL);
    CHECK_ERROR_OCL(err, "reading image data", return);

    bool foo = verify(input, output);
    reportStatus(
      "Finished in %.1lf ms (%s)",
      (end-start)*1e-3, foo ? "PASSED" : "FAILED");

    clReleaseMemObject(d_input);
    clReleaseMemObject(d_output);
    clReleaseKernel(kernel);
    releaseCL();
  }

  void Blur::runReference(Image input, Image output)
  {
    for (int y = 0; y < output.height; y++)
    {
      for (int x = 0; x < output.width; x++)
      {
        int r = 0;
        int g = 0;
        int b = 0;
        for (int j = -1; j <= 1; j++)
        {
          for (int i = -1; i <= 1; i++)
          {
            r += getPixel(input, x+i, y+j, 0);
            g += getPixel(input, x+i, y+j, 1);
            b += getPixel(input, x+i, y+j, 2);
          }
        }
        setPixel(output, x, y, 0, r/9);
        setPixel(output, x, y, 1, g/9);
        setPixel(output, x, y, 2, b/9);
        setPixel(output, x, y, 3, getPixel(input, x, y, 3));
      }
      reportStatus("Completed %.1f%% of reference", (100.f*y)/(input.height-1));
    }
    reportStatus("Finished reference");
  }
}
