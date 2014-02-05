#pragma once

#include <stdarg.h>

namespace improsa
{
  typedef struct
  {
    unsigned char *data;
    int width, height;
  } Image;

  class Filter
  {
  public:
    Filter();

    virtual const char* getName() const;

    virtual void runHalideCPU(Image input, Image output) = 0;
    virtual void runHalideGPU(Image input, Image output) = 0;
    virtual void runOpenCL(Image input, Image output) = 0;
    virtual void runReference(Image input, Image output) = 0;

    virtual void setStatusCallback(int (*callback)(const char*, va_list args));

  protected:
    const char *m_name;
    int (*m_statusCallback)(const char*, va_list args);

    void reportStatus(const char *format, ...) const;
  };

  unsigned char getPixel(Image image, int x, int y, int c);
  void setPixel(Image image, int x, int y, int c, unsigned char value);
}
