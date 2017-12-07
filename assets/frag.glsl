#version 330
in vec4 vBright;
in vec2 vFragment;

uniform sampler2D texture_0;
out vec4 FragColor;

void main(void)
{
  vec2 flip_fragment = vec2(vFragment.x, 1.0f - vFragment.y);
  FragColor = vBright + texture(texture_0, flip_fragment);
  if(!gl_FrontFacing) FragColor = FragColor - vec4(0.75, 0.75, 0.5, 0.0);
}
