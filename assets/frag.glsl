#version 330
// входные параметры
in vec2 vFragment; // 2D коодината в текстурной карте
in vec4 vColor;    // цвет
in vec4 vDiff;     // диффузный свет
flat in int vId;        // индекс вершины для определения выделенного фрагмента

uniform sampler2D texture_0;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragData;

void main(void)
{
  vec2 flip_fragment = vec2(vFragment.x, 1.0f - vFragment.y);
  FragColor = vColor * vDiff + texture(texture_0, flip_fragment);
  FragColor.a = vColor.a;
  if(!gl_FrontFacing) FragColor = FragColor - vec4(0.75, 0.75, 0.5, 0.0);

  // Индексы примитивов пишем во второй буфер


  int r = 0;
  int g = 0;
  int b = 0;

  //b = vId;
  //r  = b % 255;
  //b /= 255;
  //g  = b % 255;
  //b /= 255;

  r = (vId & 0x000000FF) >>  0;
  g = (vId & 0x0000FF00) >>  8;
  b = (vId & 0x00FF0000) >> 16;


  FragData = vec4 (r/255.0f, g/255.0f, b/255.0f, 1.0f);

  //FragColor.r = FragData.r;
  //FragColor.g = FragData.g;
  //FragColor.b = FragData.b;
}
