#include <iostream>
#include <cstring>
#include "openGLHeader.h"
#include "basicPipelineProgram.h"
using namespace std;

int BasicPipelineProgram::Init(const char * shaderBasePath) 
{
  if (BuildShadersFromFiles(shaderBasePath, "basic.vertexShader.glsl", "basic.fragmentShader.glsl") != 0)
  {
    cout << "Failed to build the pipeline program." << endl;
    return 1;
  }

  cout << "Successfully built the pipeline program." << endl;
  return 0;
}

void BasicPipelineProgram::SetModelViewMatrix(const float * m) 
{
  // pass "m" to the pipeline program, as the modelview matrix
  // students need to implement this
  GLboolean isRowMajor = GL_FALSE;//colum-major
  //upload m to the GPU
  glUniformMatrix4fv(h_modelViewMatrix, 1, isRowMajor, m);

}

void BasicPipelineProgram::SetProjectionMatrix(const float * m) 
{
  // pass "m" to the pipeline program, as the projection matrix
  // students need to implement this
  GLboolean isRowMajor = GL_FALSE;//colum-major
  //upload m to the GPU
  glUniformMatrix4fv(h_projectionMatrix, 1, isRowMajor, m);
}

int BasicPipelineProgram::SetShaderVariableHandles() 
{
  // set h_modelViewMatrix and h_projectionMatrix
  // students need to implement this

  GLuint h_program= GetProgramHandle();

  //get a handle to the modelViewMatrix shader variable
  h_modelViewMatrix = glGetUniformLocation(h_program,"modelViewMatrix");
  //get a handle to the projectionMatrix shader variable
  h_projectionMatrix = glGetUniformLocation(h_program,"projectionMatrix");

  return 0;
}

