#version 330

in vec4 color;
in vec2 tex_coord;
uniform sampler2D texture_0;

out vec4 FragmentColor;

void main()
{
  vec4 tColor = texture2D(texture_0, tex_coord);
  FragmentColor = vec4(color.rgb, 1.f - tColor.r);
}

