/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code
  Student username: <type your USC username here>
*/

#include <iostream>
#include <cstring>
#include <vector>
#include "openGLHeader.h"
#include "glutHeader.h"
#include <cmath>
#include <cstdio>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#ifdef WIN32
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#ifdef WIN32
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

#define BUFFER_OFFSET(i) ((void*)(i)) //define the buffer offset

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not


//three rendering types
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;


// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";

//global variables
GLuint vbo;
GLuint ebo;
GLuint vao;
GLuint *indices_lines;
GLuint loc;
std::vector<GLfloat> positions;
glm::mat4 basisMatrix;
glm::mat4 controlMatrix;
glm::vec4 parameterVec;



//Set some constant parameter for the window view
const float fovy = 60;//the view angle is 45 degrees
const float aspect=(float)windowWidth/(float)windowHeight;//calculate the perpective 

//program variable
OpenGLMatrix *openGLMatrix;
BasicPipelineProgram *pipelineProgram;
GLuint program;

// represents one control point along the spline 
struct Point 
{
  double x;
  double y;
  double z;
};

// spline struct 
// contains how many control points the spline has, and an array of control points 
struct Spline 
{
  int numControlPoints;
  Point * points;
};

// the spline array 
Spline * splines;
// total number of splines 
int numSplines;

int loadSplines(char * argv) 
{
  char * cName = (char *) malloc(128 * sizeof(char));
  FILE * fileList;
  FILE * fileSpline;
  int iType, i = 0, j, iLength;
  cout<<fopen(argv, "r")<<endl;

  // load the track file 
  fileList = fopen(argv, "r");
  if (fileList == NULL) 
  {
    printf ("can't open file\n");
    exit(1);
  }
  
  // stores the number of splines in a global variable 
  fscanf(fileList, "%d", &numSplines);

  splines = (Spline*) malloc(numSplines * sizeof(Spline));

  // reads through the spline files 
  for (j = 0; j < numSplines; j++) 
  {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) 
    {
      cout<<"a"<<endl;
      printf ("can't open file\n");
      exit(1);
    }

    // gets length for spline file
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    // allocate memory for all the points
    splines[j].points = (Point *)malloc(iLength * sizeof(Point));
    splines[j].numControlPoints = iLength;

    // saves the data to the struct
    while (fscanf(fileSpline, "%lf %lf %lf", 
     &splines[j].points[i].x, 
     &splines[j].points[i].y, 
     &splines[j].points[i].z) != EOF) 
    {
      i++;
    }
  }

  free(cName);

  return 0;
}

int initTexture(const char * imageFilename, GLuint textureHandle)
{
  // read the texture image
  ImageIO img;
  ImageIO::fileFormatType imgFormat;
  ImageIO::errorType err = img.load(imageFilename, &imgFormat);

  if (err != ImageIO::OK) 
  {
    printf("Loading texture from %s failed.\n", imageFilename);
    return -1;
  }

  // check that the number of bytes is a multiple of 4
  if (img.getWidth() * img.getBytesPerPixel() % 4) 
  {
    printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
    return -1;
  }

  // allocate space for an array of pixels
  int width = img.getWidth();
  int height = img.getHeight();
  unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

  // fill the pixelsRGBA array with the image pixels
  memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
  for (int h = 0; h < height; h++)
    for (int w = 0; w < width; w++) 
    {
      // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
      pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
      pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
      pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
      pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

      // set the RGBA channels, based on the loaded image
      int numChannels = img.getBytesPerPixel();
      for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
        pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
    }

  // bind the texture
  glBindTexture(GL_TEXTURE_2D, textureHandle);

  // initialize the texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

  // generate the mipmaps for this texture
  glGenerateMipmap(GL_TEXTURE_2D);

  // set the texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // query support for anisotropic texture filtering
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  printf("Max available anisotropic samples: %f\n", fLargest);
  // set anisotropic texture filtering
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

  // query for any errors
  GLenum errCode = glGetError();
  if (errCode != 0) 
  {
    printf("Texture initialization error. Error code: %d.\n", errCode);
    return -1;
  }
  
  // de-allocate the pixel array -- it is no longer needed
  delete [] pixelsRGBA;

  return 0;
}



void bindProgram(){
    loc = glGetAttribLocation(program, "position"); 
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

}

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

void renderSpline(){
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    //glBindBuffer(GL_ARRAY_BUFFER,vbo);
    pipelineProgram->Bind();//bind shader
    bindProgram();
    cout<<"5"<<endl;
    cout<<(positions.size()/3-1)*2<<endl;
    glDrawElements(GL_LINES,(positions.size()/3-1)*2,GL_UNSIGNED_INT,BUFFER_OFFSET(0));
    cout<<"6"<<endl;
    //glDrawArrays(GL_LINES,0,positions.size()/3);
}

void doTransform()
{
    //Calculate the transform matrix and store it into m_view
  openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
  openGLMatrix->LoadIdentity();
  openGLMatrix->LookAt(0.0f, 0.0f, 2.0f, 0.0f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f);  
  openGLMatrix->Translate(landTranslate[0],landTranslate[1],landTranslate[2]);
  openGLMatrix->Rotate(landRotate[0], 1.0, 0.0, 0.0);
  openGLMatrix->Rotate(landRotate[1], 0.0, 1.0, 0.0);
  openGLMatrix->Rotate(landRotate[2], 0.0, 0.0, 1.0);
  openGLMatrix->Scale(landScale[0],landScale[1],landScale[2]);

  //store the ModelView matrix into m_view
  float m_view[16];
  openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
  openGLMatrix->GetMatrix(m_view);

  //store the perspective matrix into m_perspective
  float m_perspective[16];
  openGLMatrix->SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix->GetMatrix(m_perspective);

  //upload the transformation to shader
  pipelineProgram->SetModelViewMatrix(m_view);
  pipelineProgram->SetProjectionMatrix(m_perspective);
}

void displayFunc()
{
  //clear the Window
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  //clear buffer
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

  doTransform();

  glBindVertexArray(vao);//bind vao

  renderSpline();


  glDisableVertexAttribArray(loc);


  glutSwapBuffers();

}
void idleFunc()
{
    glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  // setup perspective matrix...
  glViewport(0, 0, w, h);
  // setup perspective matrix...
  openGLMatrix->SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix->LoadIdentity();
  openGLMatrix->Perspective(fovy,aspect,0.01f,1000.0f);
  openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1];
        landRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      delete []indices_lines;
        exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;

    break;

    case 'x':
      // take a screenshot
      saveScreenshot("screenshot.jpg");
    break;
  }
}

void LoadBasisMatrix(float s) {
  float m[16] = { -1 * s, 2.0f - s, s - 2.0f, s,
                  2 * s, s - 3.0f, 3 - 2.0f * s, -1 * s,
                  -1 * s, 0, s, 0,
                  0, 1, 0, 0 };
  basisMatrix = glm::make_mat4(m);
  
}

void LoadControlMatrix(Point &p1,Point &p2,Point &p3,Point &p4 ){
  float m[16] = { p1.x, p1.y, p1.z,0,
                  p2.x, p2.y, p2.z,0,
                  p3.x, p3.y, p3.z,0,
                  p4.x, p4.y, p4.z,0 };
  controlMatrix = glm::make_mat4(m);
}

void LoadParamter(float u){
  float v[4]={pow(u,3),pow(u,2),u,1.0f};
  parameterVec = glm::make_vec4(v);
}

void CalculateVertice(float step){
  for(float u=0;u<=1;u+=step){
    LoadParamter(u);
    glm::vec4 temp=controlMatrix*basisMatrix*parameterVec;
    positions.push_back(temp[0]);
    positions.push_back(temp[1]);
    positions.push_back(temp[2]);
  }

}

void CreateVerices(){
  LoadBasisMatrix(0.5f);
  int numOfInterval=splines[0].numControlPoints-3;
  for(int i=0;i<numOfInterval;i++){
    LoadControlMatrix(splines[0].points[i],splines[0].points[i+1],splines[0].points[i+2],splines[0].points[i+3]);
    CalculateVertice(0.001f);
  }

}

void GenLineIndices(){
  int count=0;
  int numOfLines=positions.size()/3-1;
  indices_lines= new GLuint[numOfLines*2];
  for(int i=0;i<numOfLines;i++){
    //cout<<count<<endl;
    indices_lines[count++]=i;
    indices_lines[count++]=i+1;
  }
}




void initScene(int argc, char *argv[])
{

  //initialize OpenGLMatrix and BasicPipelineProgram
  openGLMatrix = new OpenGLMatrix();
  pipelineProgram = new BasicPipelineProgram();

  // Enable depth testing, then prioritize fragments closest to the camera
  glEnable(GL_DEPTH_TEST);

  //clear the window
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  CreateVerices();
  cout<<"1"<<endl;
  cout<<positions.size()<<endl;
  GenLineIndices();
  cout<<"2"<<endl;


  //create VAO
  glGenVertexArrays(1,&vao);
  glBindVertexArray(vao);


  // Generate 1 vbo buffer to store all vertices information
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,positions.size()*sizeof(GLfloat),positions.data(),GL_STATIC_DRAW);

  glGenBuffers(1,&ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,(positions.size()/3-1)*2*sizeof(GLuint),indices_lines,GL_STATIC_DRAW);
  bindProgram();//bind vao
  cout<<"3"<<endl;


  //initiate the shader program
  if(pipelineProgram->Init(shaderBasePath)!=0){
    cout << "Error finding the shaderBasePath " << shaderBasePath << "." << endl;
    exit(EXIT_FAILURE);
  }

  //get pipeline program handle
  program=pipelineProgram->GetProgramHandle();

  //unbind vao
  glBindVertexArray(0);
  cout<<"4"<<endl;

}

int main(int argc, char *argv[])
{
  if(argc<2){
    printf ("usage: %s <trackfile>\n", argv[0]);
    exit(0);    
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  // load the splines from the provided filename
  loadSplines(argv[1]);

  printf("Loaded %d spline(s).\n", numSplines);
  for(int i=0; i<numSplines; i++)
    printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);


  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();

}