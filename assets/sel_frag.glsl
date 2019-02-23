#version 330 core

out uvec3 FragColor;
//uniform uint DrawIndex;
uniform uint ObjectId;
flat in int VertId;

void main()
{
  //FragColor = uvec3(ObjectId, DrawIndex, gl_PrimitiveID + 1);
  FragColor = uvec3(ObjectId, VertId, gl_PrimitiveID + 1);
}
