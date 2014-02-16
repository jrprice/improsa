#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>

#include "Blur.h"
#include "Sharpen.h"
#include "Sobel.h"

#define METHOD_REFERENCE  (1<<1)
#define METHOD_HALIDE_CPU (1<<2)
#define METHOD_HALIDE_GPU (1<<3)
#define METHOD_OPENCL     (1<<4)

using namespace improsa;
using namespace std;

struct _options_
{
  map<string, Filter*> filters;
  map<string, unsigned int> methods;
  _options_()
  {
    filters["blur"] = new Blur();
    filters["sharpen"] = new Sharpen();
    filters["sobel"] = new Sobel();

    methods["reference"] = METHOD_REFERENCE;
    methods["opencl"] = METHOD_OPENCL;

#if ENABLE_HALIDE
    methods["halide_cpu"] = METHOD_HALIDE_CPU;
    methods["halide_gpu"] = METHOD_HALIDE_GPU;
#endif
  }
} Options;

void clinfo();
void printUsage();
int updateStatus(const char *format, va_list args);

int main(int argc, char *argv[])
{
  size_t size = 0;
  Filter *filter = NULL;
  unsigned int method = 0;
  Filter::Params params;

  // Parse arguments
  for (int i = 1; i < argc; i++)
  {
    if (!filter && Options.filters.find(argv[i]) != Options.filters.end())
    {
      filter = Options.filters[argv[i]];
    }
    else if (!method && Options.methods.find(argv[i]) != Options.methods.end())
    {
      method = Options.methods[argv[i]];
    }
    else if (!strcmp(argv[i], "-cldevice"))
    {
      ++i;
      if (i >= argc)
      {
        cout << "Platform/device index required with -cldevice." << endl;
        exit(1);
      }

      char *next;
      params.platformIndex = strtoul(argv[i], &next, 10);
      if (strlen(next) == 0 || next[0] != ':')
      {
        cout << "Invalid platform/device index." << endl;
        exit(1);
      }
      params.deviceIndex = strtoul(++next, &next, 10);
      if (strlen(next) != 0)
      {
        cout << "Invalid platform/device index." << endl;
        exit(1);
      }
    }
    else if (!strcmp(argv[i], "-clinfo"))
    {
      clinfo();
      exit(0);
    }
    else
    {
      char *next;
      size_t sz = strtoul(argv[i], &next, 10);
      if (strlen(next) > 0 || size != 0 || sz == 0)
      {
        cout << "Invalid argument '" << argv[i] << "'" << endl;
        printUsage();
        exit(1);
      }
      size = sz;
    }
  }
  if (size == 0 || filter == NULL || method == 0)
  {
    printUsage();
    exit(1);
  }

  // Allocate input/output images
  Image input = {new unsigned char[size*size*4], size, size};
  Image output = {new unsigned char[size*size*4], size, size};

  // Initialize input image with random data
  for (int y = 0; y < size; y++)
  {
    for (int x = 0; x < size; x++)
    {
      setPixel(input, x, y, 0, rand()/(float)RAND_MAX);
      setPixel(input, x, y, 1, rand()/(float)RAND_MAX);
      setPixel(input, x, y, 2, rand()/(float)RAND_MAX);
      setPixel(input, x, y, 3, 255);
    }
  }

  // Run filter
  filter->setStatusCallback(updateStatus);
  switch (method)
  {
    case METHOD_REFERENCE:
      filter->runReference(input, output);
      break;
    case METHOD_HALIDE_CPU:
      filter->runHalideCPU(input, output, params);
      break;
    case METHOD_HALIDE_GPU:
      filter->runHalideGPU(input, output, params);
      break;
    case METHOD_OPENCL:
      filter->runOpenCL(input, output, params);
      break;
    default:
      assert(false && "Invalid method.");
  }

  return 0;
}

void clinfo()
{
#define MAX_PLATFORMS 8
#define MAX_DEVICES   8
#define MAX_NAME    256
  cl_uint numPlatforms, numDevices;
  cl_platform_id platforms[MAX_PLATFORMS];
  cl_device_id devices[MAX_DEVICES];
  char name[MAX_NAME];
  cl_int err;

  err = clGetPlatformIDs(MAX_PLATFORMS, platforms, &numPlatforms);
  if (err != CL_SUCCESS)
  {
    cout << "Error retrieving platforms (" << err << ")" << endl;
    return;
  }
  if (numPlatforms == 0)
  {
    cout << "No platforms found." << endl;
    return;
  }

  for (int p = 0; p < numPlatforms; p++)
  {
    clGetPlatformInfo(platforms[p], CL_PLATFORM_NAME, MAX_NAME, name, NULL);
    cout << endl << "Platform " << p << ": " << name << endl;

    err = clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL,
                         MAX_DEVICES, devices, &numDevices);
    if (err != CL_SUCCESS)
    {
      cout << "Error retrieving devices (" << err << ")" << endl;
      continue;
    }
    if (numDevices == 0)
    {
      cout << "No devices found." << endl;
      continue;
    }

    for (int d = 0; d < numDevices; d++)
    {
      clGetDeviceInfo(devices[d], CL_DEVICE_NAME, MAX_NAME, name, NULL);
      cout << "-> Device " << d << ": " << name << endl;
    }
  }
  cout << endl;
}

void printUsage()
{
  cout << endl << "Usage: improsa SIZE FILTER METHOD [-cldevice P:D]";
  cout << endl << "       improsa -clinfo" << endl;

  cout << endl << "Where FILTER is one of:" << endl;
  map<string, Filter*>::iterator fItr;
  for (fItr = Options.filters.begin(); fItr != Options.filters.end(); fItr++)
  {
    cout << "\t" << fItr->first << endl;
  }

  cout << endl << "Where METHOD is one of:" << endl;
  map<string, unsigned int>::iterator mItr;
  for (mItr = Options.methods.begin(); mItr != Options.methods.end(); mItr++)
  {
    cout << "\t" << mItr->first << endl;
  }

  cout << endl
    << "If specifying an OpenCL device with -cldevice, " << endl
    << "P and D correspond to the platform and device " << endl
    << "indices reported by running -clinfo."
    << endl;

  cout << endl;
}

int updateStatus(const char *format, va_list args)
{
  vprintf(format, args);
  printf("\n");
  return 0;
}
