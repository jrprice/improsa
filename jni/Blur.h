#include "Filter.h"

namespace improsa
{
  class Blur : public Filter
  {
  public:
    Blur();

    virtual void runHalideCPU(Image input, Image output);
    virtual void runHalideGPU(Image input, Image output);
    virtual void runOpenCL(Image input, Image output);
    virtual void runReference(Image input, Image output);
  };
}
