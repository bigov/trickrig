#version 330

in vec2 vPos;
in vec4 vColor;
in vec2 vTex;

out vec4 color;
out vec2 tex_coord;

void main(void)
{
  gl_Position = vec4(vPos, 0.0, 1.0);
  color = vColor;
  tex_coord = vTex;
}
