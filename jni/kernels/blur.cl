const sampler_t sampler =
  CLK_NORMALIZED_COORDS_FALSE |
  CLK_ADDRESS_CLAMP_TO_EDGE   |
  CLK_FILTER_NEAREST;

kernel void blur(read_only image2d_t input,
                 write_only image2d_t output)
{
  int x = get_global_id(0);
  int y = get_global_id(1);

  uint4 sum = 0;
  for (int j = -1; j <= 1; j++)
  {
    for (int i = -1; i <= 1; i++)
    {
      sum += read_imageui(input, sampler, (int2)(x+i, y+j));
    }
  }
  write_imageui(output, (int2)(x, y), sum/9);
}
