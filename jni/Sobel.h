#include "Filter.h"

namespace improsa
{
  class Sobel : public Filter
  {
  public:
    Sobel();

    virtual bool runHalideCPU(Image input, Image output);
    virtual bool runHalideGPU(Image input, Image output);
    virtual bool runOpenCL(Image input, Image output);
    virtual bool runReference(Image input, Image output);
  };
}
