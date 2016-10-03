#version 150

in vec3 position;
in vec4 color;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  // students need to implement this
  
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);

  //col = vec4(position[0], position[1],  position[2], 1.0);
  col = color;
  //col = vec4(position[2], position[2],  position[2], 1.0);



}

