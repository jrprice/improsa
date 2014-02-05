#include <stddef.h>

#include "Filter.h"

namespace improsa
{
  Filter::Filter()
  {
    m_statusCallback = NULL;
  }

  const char* Filter::getName() const
  {
    return m_name;
  }

  void Filter::reportStatus(const char *format, ...) const
  {
    if (m_statusCallback)
    {
      va_list args;
      va_start(args, format);
      m_statusCallback(format, args);
      va_end(args);
    }
  }

  void Filter::setStatusCallback(int (*callback)(const char*, va_list args))
  {
    m_statusCallback = callback;
  }

  inline int clamp(int x, int min, int max)
  {
    return x < min ? min : x > max ? max : x;
  }

  unsigned char getPixel(Image image, int x, int y, int c)
  {
    int _x = clamp(x, 0, image.width-1);
    int _y = clamp(y, 0, image.height-1);
    return image.data[(_x + _y*image.width)*4 + c];
  }

  void setPixel(Image image, int x, int y, int c, unsigned char value)
  {
    int _x = clamp(x, 0, image.width-1);
    int _y = clamp(y, 0, image.height-1);
    image.data[(_x + _y*image.width)*4 + c] = value;
  }
}
