#version 330

layout (location = 0) in vec3 Position;
uniform mat4 mvp;

flat out int VertId;    // индекс для определения выделенного фрагмента

void main()
{
  VertId = gl_VertexID;
  gl_Position = mvp * vec4(Position, 1.0);
}

