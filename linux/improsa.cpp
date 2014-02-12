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

void printUsage();
int updateStatus(const char *format, va_list args);

int main(int argc, char *argv[])
{
  size_t size = 0;
  Filter *filter = NULL;
  unsigned int method = 0;

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
      setPixel(input, x, y, 0, rand()/RAND_MAX);
      setPixel(input, x, y, 1, rand()/RAND_MAX);
      setPixel(input, x, y, 2, rand()/RAND_MAX);
      setPixel(input, x, y, 3, 255);
    }
  }

  // Run filter
  Filter::Params params = {{0,0}};
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

void printUsage()
{
  cout << endl << "Usage: improsa SIZE FILTER METHOD" << endl;

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

  cout << endl;
}

int updateStatus(const char *format, va_list args)
{
  vprintf(format, args);
  printf("\n");
  return 0;
}
