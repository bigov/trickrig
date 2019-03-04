#version 330

layout (location=0) in vec3 VertexPosition;
layout (location=1) in vec3 VertexColor;

out vec3 FrColor;

void main()
{
  FrColor  = VertexColor;
  gl_Position = vec4(VertexPosition, 1.0);
}
