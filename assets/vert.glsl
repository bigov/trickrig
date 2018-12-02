#version 330
in vec4 position;  // 3D координаты точки
in vec4 color;     // цвет точки
in vec4 normal;    // направление нормали к поверхности
in vec2 fragment;  // 2D коодината в текстурной карте

uniform mat4 mvp;
uniform vec4 light_direction; // вектор освещения
uniform vec4 light_bright;    // яркость источника

// выходные параметры
out vec2 vFragment; // 2D коодината в текстурной карте
out vec4 vColor;    // цвет пикселя
out vec4 vDiff;     // диффузное освещение

void main(void)
{
  vFragment = fragment;
  vColor = color;
  vDiff = light_bright * max(dot(normal, light_direction), 0.0f);
  gl_Position = mvp * position;
}
