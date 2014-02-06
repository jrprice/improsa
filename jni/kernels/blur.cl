const sampler_t sampler =
  CLK_NORMALIZED_COORDS_FALSE |
  CLK_ADDRESS_CLAMP_TO_EDGE   |
  CLK_FILTER_NEAREST;

kernel void blur(read_only image2d_t input,
                 write_only image2d_t output)
{
  int x = get_global_id(0);
  int y = get_global_id(1);

  float4 sum = 0.f;
  for (int j = -2; j <= 2; j++)
  {
    for (int i = -2; i <= 2; i++)
    {
      sum += read_imagef(input, sampler, (int2)(x+i, y+j));
    }
  }
  write_imagef(output, (int2)(x, y), sum/25.f);
}
