#version 330

in vec4 position;
uniform mat4 mvp;

flat out int VertId;    // индекс для определения выделенного фрагмента

void main()
{
  VertId = gl_VertexID;
  gl_Position = mvp * position;
}

