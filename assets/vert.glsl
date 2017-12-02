#version 330
// атрибуты вершин базового элемента
in vec3 C3df;   // 3D коодинаты вершины
in vec2 TxCo;   // 2D коодината в текстурной карте

// изменяемые атрибуты инстансов
in vec3 Origin; // положение центральной точки
in vec3 Norm;   // нормали вершин

uniform mat4 mvp;
uniform float HalfSide; // Половина стороны инстанса
uniform vec3 Selected;  // Подсветка выделения

out vec2 f_texcoord;
out vec3 f_bright;  // R-G-B

void main(void)
{
  f_texcoord = TxCo;

  vec3 V = C3df + Norm * HalfSide;

  vec3 l_direct   = normalize(vec3(0.2f, 0.9f, 0.5f)); // вектор освещения
  vec3 l_bright   = vec3(0.16f, 0.16f, 0.2f);          // яркость источника
  vec3 mat_absorp = vec3(0.2f, 0.2f, 0.3f);            // поглощение материалом

  // яркость освещения в формате RGBA
  f_bright = vec3(dot(l_direct, Norm)) * l_bright - mat_absorp;
  if(Origin == Selected) f_bright += vec3(0.05f, 0.05f, 0.1f);

  float bg = (f_bright[0] + f_bright[1] + f_bright[2])/3;
  float lt = (l_bright[0] +l_bright[1] +l_bright[2])/3;

  float shadow = 0.4 * (1 - 3*lt + bg);
  float reflex = -0.07;

  gl_Position = mvp * vec4((V + Origin), 1.0f);
}
