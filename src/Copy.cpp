// Copy.cpp (ImProSA)
// Copyright (c) 2014, James Price and Simon McIntosh-Smith,
// University of Bristol. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

#include <string.h>

#include "Copy.h"
#include "opencl/copy.h"

namespace improsa
{
  Copy::Copy() : Filter()
  {
    m_name = "Copy";
  }

  bool Copy::runHalideCPU(Image input, Image output, const Params& params)
  {
    reportStatus("Halide not implemented for this filter.");
    return true;
  }

  bool Copy::runHalideGPU(Image input, Image output, const Params& params)
  {
    reportStatus("Halide not implemented for this filter.");
    return true;
  }

  bool Copy::runOpenCL(Image input, Image output, const Params& params)
  {
    if (!initCL(params, copy_kernel, "-cl-fast-relaxed-math"))
    {
      return false;
    }

    const char *kernels[] =
    {
      "buffer",
      "image_float",
      "image_int",
    };
    size_t numKernels = sizeof(kernels)/sizeof(const char*);

    for (int k = 0; k < numKernels; k++)
    {
      cl_int err;
      cl_kernel kernel;
      cl_mem d_input, d_output;
      bool images = !strncmp(kernels[k], "image", 5);
      cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
      size_t origin[3] = {0, 0, 0};
      size_t region[3] = {input.width, input.height, 1};

      kernel = clCreateKernel(m_program, kernels[k], &err);
      CHECK_ERROR_OCL(err, "creating kernel", return false);

      if (images)
      {
        d_input = clCreateImage2D(
          m_context, CL_MEM_READ_ONLY, &format,
          input.width, input.height, 0, NULL, &err);
        CHECK_ERROR_OCL(err, "creating input image", return false);

        d_output = clCreateImage2D(
          m_context, CL_MEM_WRITE_ONLY, &format,
          output.width, output.height, 0, NULL, &err);
        CHECK_ERROR_OCL(err, "creating output image", return false);

        err = clEnqueueWriteImage(
          m_queue, d_input, CL_TRUE,
          origin, region, 0, 0, input.data, 0, NULL, NULL);
        CHECK_ERROR_OCL(err, "writing image data", return false);
      }
      else
      {
        d_input = clCreateBuffer(
          m_context, CL_MEM_READ_ONLY,
          input.width*input.height*4, NULL, &err);
        CHECK_ERROR_OCL(err, "creating input buffer", return false)

        d_output = clCreateBuffer(
          m_context, CL_MEM_WRITE_ONLY,
          output.width*output.height*4, NULL, &err);
        CHECK_ERROR_OCL(err, "creating output buffer", return false)

        err = clEnqueueWriteBuffer(
          m_queue, d_input, CL_TRUE, 0, input.width*input.height*4,
          input.data, 0, NULL, NULL);
        CHECK_ERROR_OCL(err, "writing buffer data", return false);
      }

      err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_input);
      err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_output);
      CHECK_ERROR_OCL(err, "setting kernel arguments", return false);

      const size_t global[2] = {output.width, output.height};
      const size_t *local = NULL;
      if (params.wgsize[0] && params.wgsize[1])
      {
        local = params.wgsize;
      }

      // Timed runs
      for (int i = 0; i < params.iterations + 1; i++)
      {
        err = clEnqueueNDRangeKernel(
          m_queue, kernel, 2, NULL, global, local, 0, NULL, NULL);
        CHECK_ERROR_OCL(err, "enqueuing kernel", return false);

        // Start timing after warm-up run
        if (i == 0)
        {
          err = clFinish(m_queue);
          CHECK_ERROR_OCL(err, "running kernel", return false);
          startTiming();
        }
      }
      err = clFinish(m_queue);
      CHECK_ERROR_OCL(err, "running kernel", return false);
      stopTiming();

      if (images)
      {
        err = clEnqueueReadImage(
          m_queue, d_output, CL_TRUE,
          origin, region, 0, 0, output.data, 0, NULL, NULL);
        CHECK_ERROR_OCL(err, "reading image data", return false);
      }
      else
      {
        err = clEnqueueReadBuffer(
          m_queue, d_output, CL_TRUE, 0, output.width*output.height*4,
          output.data, 0, NULL, NULL);
        CHECK_ERROR_OCL(err, "writing buffer data", return false);
      }

      double totalBytes = input.width*input.height*4*2*params.iterations;
      double seconds = ((m_endTime-m_startTime)*1e-6);
      double bandwidth = (totalBytes/seconds)*1e-9;
      reportStatus("%12s: %.1lf GB/s (%s)",
                   kernels[k], bandwidth,
                   verify(input, output) ? "passed" : "failed");

      clReleaseMemObject(d_input);
      clReleaseMemObject(d_output);
      clReleaseKernel(kernel);
    }

    releaseCL();

    return true;
  }

  bool Copy::runReference(Image input, Image output)
  {
    memcpy(output.data, input.data, output.width*output.height*4);
    return true;
  }
}
