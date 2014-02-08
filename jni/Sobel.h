#include "Filter.h"

namespace improsa
{
  class Sobel : public Filter
  {
  public:
    Sobel();
    virtual ~Sobel();

    virtual bool runHalideCPU(Image input, Image output);
    virtual bool runHalideGPU(Image input, Image output);
    virtual bool runOpenCL(Image input, Image output);
    virtual bool runReference(Image input, Image output);

  private:
    Image m_reference;
  };
}
