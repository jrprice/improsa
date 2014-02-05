#include <string.h>

#include "Blur.h"
#include "kernels/blur.h"

namespace improsa
{
  Blur::Blur() : Filter()
  {
    m_name = "Blur";
  }

  void Blur::runHalideCPU(Image input, Image output)
  {
    reportStatus("runHalideCPU not implemented");
  }

  void Blur::runHalideGPU(Image input, Image output)
  {
    reportStatus("runHalideGPU not implemented");
  }

  void Blur::runOpenCL(Image input, Image output)
  {
    reportStatus("runOpenCL not implemented");
  }

  void Blur::runReference(Image input, Image output)
  {
    for (int y = 0; y < output.height; y++)
    {
      for (int x = 0; x < output.width; x++)
      {
        int r = 0;
        int g = 0;
        int b = 0;
        for (int j = -1; j <= 1; j++)
        {
          for (int i = -1; i <= 1; i++)
          {
            r += getPixel(input, x+i, y+j, 0);
            g += getPixel(input, x+i, y+j, 1);
            b += getPixel(input, x+i, y+j, 2);
          }
        }
        setPixel(output, x, y, 0, r/9);
        setPixel(output, x, y, 1, g/9);
        setPixel(output, x, y, 2, b/9);
        setPixel(output, x, y, 3, getPixel(input, x, y, 3));
      }
      reportStatus("Completed %.1f%% of reference", (100.f*y)/(input.height-1));
    }
    reportStatus("Finished reference");
  }
}
