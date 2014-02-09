#include "Filter.h"

namespace improsa
{
  class Sharpen : public Filter
  {
  public:
    Sharpen();

    virtual bool runHalideCPU(Image input, Image output);
    virtual bool runHalideGPU(Image input, Image output);
    virtual bool runOpenCL(Image input, Image output);
    virtual bool runReference(Image input, Image output);
  };
}
