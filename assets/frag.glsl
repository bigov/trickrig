#version 330

// входные параметры
in vec2 vFragment; // 2D коодината в текстурной карте
in vec4 vColor;    // цвет

uniform sampler2D texture_0;
uniform uint Xid;   // индекс вершины для определения выделенного фрагмента

layout(location = 0) out vec4 FragColor;
layout(location = 1) out uvec3 FragData;

void main(void)
{
  //vec2 flip_fragment = vec2(vFragment.x, 1.0f - vFragment.y);

  vec4 texColor = texture(texture_0, vFragment);
  FragColor.x = vColor.x * (1.0 - texColor.a * (1 - texColor.x));
  FragColor.y = vColor.y * (1.0 - texColor.a * (1 - texColor.x));
  FragColor.z = vColor.z * (1.0 - texColor.a * (1 - texColor.x));
  FragColor.a = 1.0f; //vColor.a;

  if(!gl_FrontFacing) FragColor =
      vec4(FragColor.r * 0.5, FragColor.g * 0.5, FragColor.b * 0.5, FragColor.a);

  FragData = uvec3(Xid, 0, 0);

  //DEBUG
  //FragData = uvec3(vId, gl_PrimitiveID, 4222111000);
}
