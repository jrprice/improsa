const sampler_t sampler =
  CLK_NORMALIZED_COORDS_FALSE |
  CLK_ADDRESS_CLAMP_TO_EDGE   |
  CLK_FILTER_NEAREST;

constant float mask[3][3] =
{
  {-1, -2, -1},
  {0, 0, 0},
  {1, 2, 1}
};

kernel void sobel(read_only image2d_t input,
                  write_only image2d_t output)
{
  int x = get_global_id(0);
  int y = get_global_id(1);

  float g_x = 0.f;
  float g_y = 0.f;
  for (int j = -1; j <= 1; j++)
  {
    for (int i = -1; i <= 1; i++)
    {
      float4 p = read_imagef(input, sampler, (int2)(x+i, y+j));
      g_x += (p.x*0.299f + p.y*0.587f + p.z*0.114f) * mask[i+1][j+1];
      g_y += (p.x*0.299f + p.y*0.587f + p.z*0.114f) * mask[j+1][i+1];
    }
  }
  float g_mag = sqrt(g_x*g_x + g_y*g_y);
  write_imagef(output, (int2)(x, y), (float4)(g_mag,g_mag,g_mag,1));
}
