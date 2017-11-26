#version 330

in vec2 Texcoord;
uniform sampler2D texFramebuffer;
uniform sampler2D texHUD;
out vec4 FragColor;

void main(void)
{
  vec2 flipped_coord = vec2(Texcoord.x, 1.0f - Texcoord.y);
  vec4 texhud = texture(texHUD, flipped_coord);
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

    //(gl_FragCoord.x > 400.5) && (gl_FragCoord.x < 400.6) &&
    (gl_FragCoord.x == 400.5) &&

    (gl_FragCoord.y > 298.0) && (gl_FragCoord.y < 303.0)
    )||(
    (gl_FragCoord.x > 398.0) && (gl_FragCoord.x < 403.0) &&

    //(gl_FragCoord.y > 300.0) && (gl_FragCoord.y < 301.0)
    (gl_FragCoord.y == 300.5)

    ))
  {
    m = (FragColor.r + FragColor.g + FragColor.b)/3;
    if(m < 0.6f) n = m + 0.3f;
    else n = m - 0.3f;
    FragColor = vec4(n, n, n, 1.f);
  }
}
