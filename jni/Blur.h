#include "Filter.h"

namespace improsa
{
  class Blur : public Filter
  {
  public:
    virtual char* getName() const;

    virtual void runReference(Image input, Image output);
  };
}
