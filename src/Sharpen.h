#include "Filter.h"

namespace improsa
{
  class Sharpen : public Filter
  {
  public:
    Sharpen();

    virtual bool runHalideCPU(Image input, Image output, const Params& params);
    virtual bool runHalideGPU(Image input, Image output, const Params& params);
    virtual bool runOpenCL(Image input, Image output, const Params& params);
    virtual bool runReference(Image input, Image output);
  };
}
