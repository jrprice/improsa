// bilateral.cl (ImProSA)
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

kernel void bilateral(read_only image2d_t input,
                      write_only image2d_t output)
{
  int x = get_global_id(0);
  int y = get_global_id(1);

  float coeff = 0.f;
  float4 sum = 0.f;
  float4 center = read_imagef(input, sampler, (int2)(x, y));

  for (int j = -2; j <= 2; j++)
  {
    for (int i = -2; i <= 2; i++)
    {
      float norm, weight;
      float4 pixel = read_imagef(input, sampler, (int2)(x+i, y+j));

      norm = sqrt((float)(i*i) + (float)(j*j)) * (1.f/3.f);
      weight = native_exp(-0.5f * (norm*norm));

      norm = fast_distance(pixel.xyz, center.xyz) * (1.f/0.2f);
      weight *= native_exp(-0.5f * (norm*norm));

      coeff += weight;
      sum += weight*pixel;
    }
  }

  sum /= coeff;
  sum.w = center.w;

  write_imagef(output, (int2)(x, y), sum);
}
