// Blur.h (ImProSA)
// Copyright (c) 2014, James Price and Simon McIntosh-Smith,
// University of Bristol. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

#include "Filter.h"

namespace improsa
{
  class Blur : public Filter
  {
  public:
    Blur();

    virtual bool runHalideCPU(Image input, Image output, const Params& params);
    virtual bool runHalideGPU(Image input, Image output, const Params& params);
    virtual bool runOpenCL(Image input, Image output, const Params& params);
    virtual bool runReference(Image input, Image output);
  };
}
