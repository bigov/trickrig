#version 330

in vec4 position;
//in vec4 color;     // цвет точки
//in vec4 normal;    // направление нормали к поверхности
//in vec2 fragment;  // 2D коодината в текстурной карте
uniform mat4 mvp;  // матрица

flat out int VertId;    // индекс для определения выделенного фрагмента

void main()
{
  VertId = gl_VertexID;
  gl_Position = mvp * position;
}

