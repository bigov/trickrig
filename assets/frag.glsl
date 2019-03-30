#version 330

// входные параметры
in vec2 vFragment; // 2D коодината в текстурной карте
in vec4 vColor;    // цвет вершины (r,g,b,a)
flat in int vertId;

uniform sampler2D texture_0;  // координаты текстуры
//uniform float tex_transp;   // прозрачность текстуры

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int FragData;

void main(void)
{
  //vec2 flip_fragment = vec2(vFragment.x, 1.0f - vFragment.y);

  vec4 texColor = texture(texture_0, vFragment);

  // Можно управлять прозрачностью текстуры при помощи uniform-переменной
  //FragColor.x = (1.0f - tex_transp * (1.0f - texColor.x)) * vColor.x;
  //FragColor.y = (1.0f - tex_transp * (1.0f - texColor.y)) * vColor.y;
  //FragColor.z = (1.0f - tex_transp * (1.0f - texColor.z)) * vColor.z;

  FragColor.x = texColor.x * vColor.x;
  FragColor.y = texColor.y * vColor.y;
  FragColor.z = texColor.z * vColor.z;
  FragColor.a = vColor.a;

  if(!gl_FrontFacing) FragColor =
      vec4(FragColor.r * 0.5f, FragColor.g * 0.5f, FragColor.b * 0.5f, FragColor.a);

  FragData = vertId;
}
