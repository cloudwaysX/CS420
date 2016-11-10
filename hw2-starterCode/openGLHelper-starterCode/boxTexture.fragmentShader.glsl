#version 150

in vec3 tc;

out vec4 c;
uniform samplerCube textureImage;

void main()
{
  // compute the final fragment color by looking up the texture map
  // texture() is GLSL command to query into texture map
  c = texture(textureImage, tc); 
}
