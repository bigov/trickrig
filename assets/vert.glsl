#version 330
// атрибуты вершин базового элемента
in vec3 C3df;   // 3D коодинаты вершины
in vec2 TxCo;   // 2D коодината в текстурной карте

// изменяемые атрибуты инстансов
in vec3 Origin; // положение центральной точки
in vec3 Norm;   // вектор нормали (общий для всех вершин элемента)

uniform mat4 mvp;
//uniform float HalfSide; // Половина стороны инстанса
uniform vec3 Selected;  // Подсветка выделения

out vec2 f_texcoord;
out vec3 f_bright;  // R-G-B

void main(void)
{
  // текстура
  f_texcoord = TxCo; // текстура копируется из базового элемента

  gl_Position = mvp * vec4((Origin + C3df), 1.0f);

  // освещение
  vec3 l_direct   = normalize(vec3(0.2f, 0.9f, 0.5f)); // вектор освещения
  vec3 l_bright   = vec3(0.16f, 0.16f, 0.2f);          // яркость источника
  vec3 mat_absorp = vec3(0.2f, 0.2f, 0.3f);            // поглощение материалом
  f_bright = vec3(dot(l_direct, Norm)) * l_bright - mat_absorp;
  if(Origin == Selected) f_bright += vec3(0.05f, 0.05f, 0.1f);
}
