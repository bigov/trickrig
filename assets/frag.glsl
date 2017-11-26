#version 330

in vec2 f_texcoord;
in vec3 f_bright;
in vec4 f_rings_sh;

out vec4 FragColor;
uniform sampler2D texture_0;

void main(void)
{
  vec2 flipped_texcoord = vec2(f_texcoord.x, 1.0f - f_texcoord.y);
  vec3 Shadow = vec3(0.0);

  if (gl_FrontFacing)
  {
    Shadow = mix(vec3(f_rings_sh[0]), Shadow, smoothstep( 0.00f, 0.02f, f_texcoord.x ));
    Shadow = mix(vec3(f_rings_sh[1]), Shadow, smoothstep( 0.125f, 0.105f, f_texcoord.x ));
    Shadow = mix(vec3(f_rings_sh[2]), Shadow, smoothstep( 0.00f, 0.02f, f_texcoord.y ));
    Shadow = mix(vec3(f_rings_sh[3]), Shadow, smoothstep( 0.125f, 0.105f, f_texcoord.y ));

    FragColor = texture(texture_0, flipped_texcoord) + vec4(f_bright - Shadow, 0.0);

  } else
  {
    FragColor = texture(texture_0, flipped_texcoord) - vec4(0.75, 0.75, 0.5, 0.0) +
      vec4(f_bright * vec3(0.15), 1.0);
  }

}
