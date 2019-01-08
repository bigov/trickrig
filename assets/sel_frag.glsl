#version 410

out uvec3 FragColor;
uniform uint DrawIndex;
uniform uint ObjectId;

void main()
{
  FragColor = uvec3(ObjectId, DrawIndex, gl_PrimitiveID + 1);
}
