#pragma once

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
    virtual char* getName() const = 0;

    virtual void runReference(Image input, Image output) = 0;
  };

  unsigned char getPixel(Image image, int x, int y, int c);
  void setPixel(Image image, int x, int y, int c, unsigned char value);
}
