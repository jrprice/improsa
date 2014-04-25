// copy.cl (ImProSA)
// Copyright (c) 2014, James Price and Simon McIntosh-Smith,
// University of Bristol. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

const sampler_t sampler =
  CLK_NORMALIZED_COORDS_FALSE |
  CLK_ADDRESS_CLAMP_TO_EDGE   |
  CLK_FILTER_NEAREST;

kernel void buffer(global uchar4 *input,
                   global uchar4 *output)
{
  size_t pixel = get_global_id(0) + get_global_id(1)*WIDTH;
  uchar4 value = input[pixel + get_global_id(2)*WIDTH*HEIGHT];
  output[pixel] = value;
}

kernel void image_float(read_only image3d_t input,
                        write_only image2d_t output)
{
  int4 pixel = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 1);
  float4 value = read_imagef(input, sampler, pixel);
  write_imagef(output, pixel.xy, value);
}

kernel void image_int(read_only image3d_t input,
                      write_only image2d_t output)
{
  int4 pixel = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 1);
  uint4 value = read_imageui(input, sampler, pixel);
  write_imageui(output, pixel.xy, value);
}
