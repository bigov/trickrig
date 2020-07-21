#version 330

in vec2 Texcoord;
uniform sampler2D WinTexture; // 3D сцена
uniform vec3 Cursor;         // x,y - 2d координаты; z - длина стороны курсора

out vec4 FragColor;

void main(void)
{
  float thickness = 1.5f; // Толщина линий курсора

  FragColor = texture2D(WinTexture, Texcoord); // "texture2D()" - deprecated
  //FragColor = texture(WinTexture, Texcoord); // recommended using "texture()"

  // Формирование курсора c длиной стороны, равной Cursor.z
  // в точке с координатами (Cursor.x; Cursor.y)
  if((
    abs(gl_FragCoord.x - Cursor.x) < thickness &&
    abs(gl_FragCoord.y - Cursor.y) < Cursor.z
  )||(
    abs(gl_FragCoord.y - Cursor.y) < thickness &&
    abs(gl_FragCoord.x - Cursor.x) < Cursor.z
    ))
  {
    float m = (FragColor.r + FragColor.g + FragColor.b)/3.f;
    if(m < 0.6f) m += 0.5f; else m -= 0.5f; // "инвертировать" среднюю яркость
    FragColor = vec4(m, m, m, 1.f);         // получим курсор с инверсной яркостью
  }
}
