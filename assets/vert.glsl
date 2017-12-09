#version 330
in vec4 position;
in vec4 color;
in vec4 normal;
in vec2 fragment;  // 2D коодината в текстурной карте

uniform mat4 mvp;
uniform vec4 light_direction; // вектор освещения
uniform vec4 light_bright;    // яркость источника

out vec2 vFragment;
out vec4 vBright;

void main(void)
{
  vFragment = fragment;
  vBright = color * light_bright * max(dot(light_direction, normal), 0.0f);
  gl_Position = mvp * position;
}
