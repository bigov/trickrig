#version 330

in vec2 Texcoord;
uniform sampler2D texFramebuffer;
uniform sampler2D texHUD;
uniform vec3 Cursor; // x,y - 2d координаты; z - длина стороны курсора

out vec4 FragColor;

void main(void)
{
  float thickness = 1.5f; // Толщина линий курсора

  vec4 tex_hud = texture(texHUD, Texcoord);
  vec4 display = texture(texFramebuffer, Texcoord);
  FragColor = mix(display, tex_hud, tex_hud.a);

  // Формирование курсора c длиной стороны, равной Cursor.z
  // в точке с координатами (Cursor.x; Cursor.y)
  float m, n;
  if((
    abs(gl_FragCoord.x - Cursor.x) < thickness &&
    abs(gl_FragCoord.y - Cursor.y) < Cursor.z
  )||(
    abs(gl_FragCoord.y - Cursor.y) < thickness &&
    abs(gl_FragCoord.x - Cursor.x) < Cursor.z
    ))
  {
    m = (FragColor.r + FragColor.g + FragColor.b)/3;      // средняя яркость текущей точки
    if(m < 0.6f) { n = m + 0.3f; } else { n = m - 0.3f; } // "инвертировать" значение
    FragColor = vec4(n, n, n, 1.f);                       // в итоге получим инверсный курсор
  }
}
