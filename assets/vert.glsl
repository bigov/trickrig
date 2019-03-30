#version 330

in vec3 position;  // координаты вершины
in vec4 color;     // цвет (r,g,b,a)
in vec3 normal;    // направление нормали
in vec2 fragment;  // 2D коодината в текстурной карте

uniform mat4 mvp;
uniform vec3 light_direction; // направление света
uniform vec3 light_bright;    // яркость света по цветам

// выходные параметры
out vec2 vFragment;   // 2D коодината в текстурной карте
out vec4 vColor;      // цвет (r,g,b,a)
flat out int vertId;  // индекс вершины

void main(void)
{
  vFragment = fragment; // координаты фрагмента текстуры передаем без изменений

  float amb = 0.4f;     // рассеяный(фоновый) свет
  // Скалярное произведение нормали к поверхности и направления на
  // источник света [dot(vN, vL)] - это число, выражающее количество
  // света, попадающего на поверхность из этого источника
  vec3 light =  light_bright * max(dot(normal, light_direction), amb);
  vColor = color * vec4(light, 1.0f);
  gl_Position = mvp * vec4(position, 1.0f);

  vertId = gl_VertexID;
}

