#version 330 core

in vec2 Texcoord;
uniform sampler2D texFramebuffer;
uniform sampler2D texHUD;
uniform vec3 Cursor;

out vec4 FragColor;

void main(void)
{
  float c_x = Cursor.x; // 400.5f; // координаты курсора
  float c_y = Cursor.y; // 300.5f;
  float c_l = Cursor.z; // 4.0f;   // длина луча курсора

  //vec2 flipped_coord = vec2(Texcoord.x, 1.0f - Texcoord.y);
  //vec4 texhud = texture(texHUD, flipped_coord);
  //vec4 screen = texture(texFramebuffer, Texcoord);

  vec4 texhud = texture(texHUD, Texcoord);
  vec4 screen = texture(texFramebuffer, Texcoord);


  if(texhud.a == 0)
  {
    FragColor = screen;
  } else
  {
    FragColor = mix(screen, texhud, texhud.a);
  }

  //FragColor = screen - texhud;

  //FragColor = texture(texFramebuffer, Texcoord);

  // Формирование курсора
  float m, n;
  if((
    (gl_FragCoord.x == c_x) && (gl_FragCoord.y > (c_y - c_l)) && (gl_FragCoord.y < (c_y + c_l))
    )||(
    (gl_FragCoord.y == c_y) && (gl_FragCoord.x > (c_x - c_l)) && (gl_FragCoord.x < (c_x + c_l))
    ))
  {
    m = (FragColor.r + FragColor.g + FragColor.b)/3;
    if(m < 0.6f) n = m + 0.3f;
    else n = m - 0.3f;
    FragColor = vec4(n, n, n, 1.f);
  }
}
