#include "Filter.h"

namespace improsa
{
  class Bilateral : public Filter
  {
  public:
    Bilateral();

    virtual bool runHalideCPU(Image input, Image output, const Params& params);
    virtual bool runHalideGPU(Image input, Image output, const Params& params);
    virtual bool runOpenCL(Image input, Image output, const Params& params);
    virtual bool runReference(Image input, Image output);
  };
}
