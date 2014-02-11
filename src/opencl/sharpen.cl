const sampler_t sampler =
  CLK_NORMALIZED_COORDS_FALSE |
  CLK_ADDRESS_CLAMP_TO_EDGE   |
  CLK_FILTER_NEAREST;

constant float mask[3][3] =
{
  {-1, -1, -1},
  {-1,  8, -1},
  {-1, -1, -1}
};

kernel void sharpen(read_only image2d_t input,
                    write_only image2d_t output)
{
  int x = get_global_id(0);
  int y = get_global_id(1);

  float4 value = 0.f;
  for (int j = -1; j <= 1; j++)
  {
    for (int i = -1; i <= 1; i++)
    {
      value += read_imagef(input, sampler, (int2)(x+i, y+j)) * mask[i+1][j+1];
    }
  }
  float4 orig = read_imagef(input, sampler, (int2)(x, y));
  write_imagef(output, (int2)(x, y), orig+value/8);
}
