#version 330

in vec2 vCoordXY;
in vec4 vColor;
in vec2 vCoordUV;

out vec4 FgColor;
out vec4 BgColor;
out vec2 CoordUV;

void main(void)
{
  FgColor = vec4(0.f, 0.f, 0.f, 1.f);
  BgColor = vColor;
  CoordUV = vCoordUV;

  gl_Position = vec4(vCoordXY, 0.0, 1.0);
}
