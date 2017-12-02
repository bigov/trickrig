#version 330
in vec2 f_texcoord;
in vec3 f_bright;
uniform sampler2D texture_0;

out vec4 FragColor;

void main(void)
{
  vec2 flipped_texcoord = vec2(f_texcoord.x, 1.0f - f_texcoord.y);
  vec3 Shadow = vec3(0.0);

  if (gl_FrontFacing)
  {
    FragColor = texture(texture_0, flipped_texcoord) + vec4(f_bright, 0.0);

  } else
  {
    FragColor = texture(texture_0, flipped_texcoord) - vec4(0.75, 0.75, 0.5, 0.0) +
      vec4(f_bright * vec3(0.15), 1.0);
  }

}
