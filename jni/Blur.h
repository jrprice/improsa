#include "Filter.h"

namespace improsa
{
  class Blur : public Filter
  {
  public:
    Blur();
    virtual ~Blur();

    virtual bool runHalideCPU(Image input, Image output);
    virtual bool runHalideGPU(Image input, Image output);
    virtual bool runOpenCL(Image input, Image output);
    virtual bool runReference(Image input, Image output);

  private:
    Image m_reference;
  };
}
