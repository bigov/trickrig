#version 330

in vec4 color;
in vec2 tex_coord;
uniform sampler2D font;

out vec4 FragmentColor;

void main(void)
{
  vec4 tColor = texture2D(font, tex_coord);
  FragmentColor = vec4(color.rgb, 1.f - tColor.r);
}

