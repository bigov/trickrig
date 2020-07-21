#version 330

in vec2 Texcoord;
uniform sampler2D WinTexture; // 3D сцена
out vec4 FragColor;

void main(void)
{
  FragColor = texture2D(WinTexture, Texcoord); // "texture2D()" - deprecated
  //FragColor = texture(WinTexture, Texcoord); // recommended for using - "texture()"
}
