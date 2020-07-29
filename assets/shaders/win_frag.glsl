#version 330

in vec2 Texcoord;
uniform sampler2D WinTexture; // 3D сцена
uniform vec3 Cursor;         // x,y - 2d координаты; z - длина стороны курсора

out vec4 FragColor;

void main(void)
{
  float thickness = 1.5f; // Толщина линий курсора

  FragColor = texture2D(WinTexture, Texcoord);

  float dx = abs(gl_FragCoord.x - Cursor.x);
  float dy = abs(gl_FragCoord.y - Cursor.y);


  // Формирование курсора в точке с координатами (Cursor.x; Cursor.y)
  // Длинна стороны курсора равна Cursor.z
  if((
       dx < thickness && 1.f < dy && dy < Cursor.z
     )||(
       dy < thickness && 1.f < dx && dx < Cursor.z
    ))
  {
    float m = (FragColor.r + FragColor.g + FragColor.b)/3.f;
    if(m > 0.5f) m -= 0.5f; else m += 0.5f; // "инвертировать" среднюю яркость
    FragColor = vec4(m, m, m, 1.f);         // получим курсор с инверсной яркостью
  }
}
