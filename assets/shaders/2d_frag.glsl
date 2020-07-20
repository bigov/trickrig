#version 330

in vec4 FgColor;
in vec4 BgColor;
in vec2 CoordUV;
uniform sampler2D font_texture;

out vec4 FragmentColor;

void main(void)
{
  vec4 tColor = texture2D(font_texture, CoordUV);
  FragmentColor = mix(FgColor, BgColor, tColor.r);
}

