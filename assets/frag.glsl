#version 330
// входные параметры
in vec2 vFragment; // 2D коодината в текстурной карте
in vec4 vColor;    // цвет
in vec4 vDiff;     // диффузный свет

uniform sampler2D texture_0;

out vec4 FragColor;
//out vec4 FragData;

void main(void)
{
  vec2 flip_fragment = vec2(vFragment.x, 1.0f - vFragment.y);
  FragColor = vColor * vDiff + texture(texture_0, flip_fragment);
  FragColor.a = vColor.a;
  if(!gl_FrontFacing) FragColor = FragColor - vec4(0.75, 0.75, 0.5, 0.0);

  /*
  int r = 0;
  int g = 0;
  int b = gl_PrimitiveID;

  r  = b % 255;
  b /= 255;
  g  = b % 255;
  b /= 255;
  FragColor[1] = vec4 (r, g, b, 255);
  */
}
