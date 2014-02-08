#include "common.h"

void cpu()
{
  ImageParam input(UInt(8), 3, "input");
  Func clamped("clamped");
  Func blur_x("blur_x"), blur_y("blur_y");
  Var c("c"), x("x"), y("y"), xi("xi"), yi("yi");

  // Algorithm
  clamped(x, y, c) = input(
    clamp(x, 0, input.width()-1),
    clamp(y, 0, input.height()-1),
    c) / 255.f;
  blur_x(x, y, c) = (
    clamped(x-2, y, c) +
    clamped(x-1, y, c) +
    clamped(x,   y, c) +
    clamped(x+1, y, c) +
    clamped(x+2, y, c)
    )/ 5.f;
  blur_y(x, y, c) = u8((
    blur_x(x, y-2, c) +
    blur_x(x, y-1, c) +
    blur_x(x, y,   c) +
    blur_x(x, y+1, c) +
    blur_x(x, y+2, c)
    ) / 5.f * 255);

  // Channel order
  input.set_stride(0, 4);
  input.set_extent(2, 4);
  blur_y.reorder_storage(c, x, y);
  blur_y.output_buffer().set_stride(0, 4);
  blur_y.output_buffer().set_extent(2, 4);

  // Schedule
  blur_y.parallel(y).vectorize(c, 4);

  std::vector<Argument> args;
  args.push_back(input);
  blur_y.compile_to_assembly("blur_cpu.s", args, "halide_blur_cpu");
  blur_y.compile_to_header("blur_cpu.h", args, "halide_blur_cpu");
}

void gpu()
{
  ImageParam input(UInt(8), 3, "input");
  Func clamped("clamped");
  Func blur_x("blur_x"), blur_y("blur_y");
  Var c("c"), x("x"), y("y"), xi("xi"), yi("yi");

  // Algorithm
  clamped(x, y, c) = input(
    clamp(x, 0, input.width()-1),
    clamp(y, 0, input.height()-1),
    c) / 255.f;
  blur_x(x, y, c) = (
    clamped(x-2, y, c) +
    clamped(x-1, y, c) +
    clamped(x,   y, c) +
    clamped(x+1, y, c) +
    clamped(x+2, y, c)
    )/ 5.f;
  blur_y(x, y, c) = u8((
    blur_x(x, y-2, c) +
    blur_x(x, y-1, c) +
    blur_x(x, y,   c) +
    blur_x(x, y+1, c) +
    blur_x(x, y+2, c)
    ) / 5.f * 255);

  // Channel order
  input.set_stride(0, 4);
  blur_y.reorder_storage(c, x, y);
  blur_y.output_buffer().set_stride(0, 4);

  // Schedule
  blur_y.cuda_tile(x, y, 8, 8);

  std::vector<Argument> args;
  args.push_back(input);
  blur_y.compile_to_assembly("blur_gpu.s", args, "halide_blur_gpu");
  blur_y.compile_to_header("blur_gpu.h", args, "halide_blur_gpu");
}

int main(int argc, char *argv[])
{
  cpu();
  gpu();

  return 0;
}
