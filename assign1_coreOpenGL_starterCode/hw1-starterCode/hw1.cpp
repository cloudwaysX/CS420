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

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

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

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

//three rendering types
typedef enum {POINT, LINE, TRIANGLE} RENDER_STATE;
RENDER_STATE renderState = TRIANGLE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;
ImageIO * colormapImage;

//global variables
GLuint vbo;
GLuint ebo;
GLuint vao;
int width;
int height;
int numOfStrips;
int numOfVertices;
int numOfLines;
GLuint *indices_triangles;
GLuint *indices_lines;
bool isColored =false;
GLuint loc;
GLuint loc2;

//Set some constant parameter for the window view
const float fovy = 60;//the view angle is 45 degrees
const float aspect=(float)windowWidth/(float)windowHeight;//calculate the perpective 

//program variable
OpenGLMatrix *openGLMatrix;
BasicPipelineProgram *pipelineProgram;
GLuint program;



void bindProgram(){
    loc = glGetAttribLocation(program, "position"); 
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    loc2 = glGetAttribLocation(program, "color"); 
    glEnableVertexAttribArray(loc2);
    glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(numOfVertices*3*sizeof(GLfloat)));

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

void renderHeightField(){
  switch(renderState)
  {
    //render the heightfield as points
    case POINT:
      glBindBuffer(GL_ARRAY_BUFFER,vbo);
      pipelineProgram->Bind();//bind shader
      bindProgram();
      glDrawArrays(GL_POINTS,0,numOfVertices);
    break;

    //render the heightfield as lines
    case LINE:
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,numOfLines*2*sizeof(GLuint),indices_lines,GL_STATIC_DRAW);
      bindProgram();
      pipelineProgram->Bind();//bind shader
      glDrawElements(GL_LINES,numOfLines*2,GL_UNSIGNED_INT,BUFFER_OFFSET(0));
    break;

    //render the heightfield as solid triangles
    case TRIANGLE:
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,numOfStrips*2*width*sizeof(GLuint),indices_triangles,GL_STATIC_DRAW);
      pipelineProgram->Bind();//bind shader
      bindProgram();
      glDrawElements(GL_TRIANGLE_STRIP,numOfStrips*2*width,GL_UNSIGNED_INT,BUFFER_OFFSET(0));
    break;
  }

}

void doTransform()
{
    //Calculate the transform matrix and store it into m_view
  openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
  openGLMatrix->LoadIdentity();
  openGLMatrix->LookAt(0.0f, 0.0f, 2.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);  
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
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

  doTransform();

  glBindVertexArray(vao);//bind vao

  renderHeightField();//rendering

  //glBindVertexArray(0);//unbind the vao

  glDisableVertexAttribArray(loc);
  glDisableVertexAttribArray(loc2);

  glutSwapBuffers();

}

void idleFunc()
{
  // do some stuff... 

  // for example, here, you can save the screenshots to disk (to make the animation)

  // make the screen update 
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
      exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;

      //switch the render state when each time the spacebar is pressed
      switch (renderState){
        case TRIANGLE:
          renderState=POINT;
        break;

        case POINT:
          renderState=LINE;
        break;

        case LINE:
          renderState=TRIANGLE;
        break;
      }

      cout<<"Rendering heightfield as state: "<<renderState<<endl;
    break;

    case 'x':
      // take a screenshot
      saveScreenshot("screenshot.jpg");
    break;
  }
}


void GenLineIndices( ){
  //assign the indices information for element draw of triangle
    int count=0;
    indices_lines= new GLuint[numOfLines*2];
    for(int i=0;i<height-1;i++){
      for(int j=0;j<width;j++){
        indices_lines[count]=i*width+j;
        indices_lines[count+1]=i*width+j+1;
        indices_lines[count+2]=i*width+j+1;
        indices_lines[count+3]=i*width+j+width;
        indices_lines[count+4]=i*width+j+width;
        indices_lines[count+5]=i*width+j;
        count=count+5;
      }
  }
  for(int i=0;i<height-1;i++){
    indices_lines[count]=i*width+width-1;
    indices_lines[count+1]=i*width+2*width-1;
    count=count+2;

  }

  for(int j=0;j<width-1;j++){
    indices_lines[count]=(height-2)*width+j;
    indices_lines[count+1]=(height-2)*width+j+1;

    count=count+2;
  }

}

void GenTriangleIndices( ) {

  //assign the indices information for element draw of triangle
    int count=0;
    indices_triangles= new GLuint[numOfStrips*2*width];
    for(int i=0;i<height-1;i++){
      for(int j=0;j<width;j++){
      	if(i%2==0){
      		indices_triangles[count]=i*width+j;
        	indices_triangles[count+1]=i*width+j+width;
      	}
      	else{
      		indices_triangles[count]=i*width+width-1-j;
        	indices_triangles[count+1]=i*width+width-1-j+width;
      	}
      count=count+2;
      }
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

  // load the image from a jpeg disk file to main memory
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }
  cout<<heightmapImage->getBytesPerPixel()<<endl;

  colormapImage = new ImageIO();
  if(isColored){
  	if (colormapImage->loadJPEG(argv[2]) != ImageIO::OK)
  	{
    	cout << "Error reading image " << argv[2] << "." << endl;
    	exit(EXIT_FAILURE);
  	}
  }  

  //get image height and width
  height = heightmapImage->getHeight();
  width = heightmapImage->getWidth();
  numOfVertices =height*width;
  numOfStrips = height-1;
  numOfLines=(height-1)*(width-1)*3+(width-1)+(height-1);

  int count=0;
  float normalizeParameter_xy=1.0f/255.0f;
  float normalizeParameter_z=1.0f/255.0f; //set depth in the similar scale as width and height

  //Load the positions information and color information from image
  GLfloat *positions=new GLfloat[numOfVertices*3];
  GLfloat *colors=new GLfloat[numOfVertices*3];
  for(int x=0;x<height;x++){
    for(int y=0;y<width;y++){
    	GLfloat z;
    	GLfloat color0;
    	GLfloat color1;
    	GLfloat color2;
    	//read in the color value of image and convert it to the height value  	
    	if(heightmapImage->getBytesPerPixel()==3){
    		//if input image has RGB value
    		color0 = heightmapImage->getPixel(x,y,0)*normalizeParameter_z;
    		color1 = heightmapImage->getPixel(x,y,1)*normalizeParameter_z;
    		color2 = heightmapImage->getPixel(x,y,2)*normalizeParameter_z;
    		z=(color2+color1+color0)/(float)3.0f;
    	}
    	else{
    		//if input only has gray value
      		z = heightmapImage->getPixel(x,y,0)*normalizeParameter_z;
      		color0=z;color1=z;color2=z;
    	}
    	// assign the position value for each positions
    	positions[3*count]=(x-height/2)*normalizeParameter_xy;
      	positions[3*count+1]=(y-width/2)*normalizeParameter_xy;
      	positions[3*count+2]=z;
		//assign the color value for each positions
      	if(isColored){
      		//if another color map is provided
      		color0 = colormapImage->getPixel(x,y,0)*normalizeParameter_z;
      		color1 = colormapImage->getPixel(x,y,1)*normalizeParameter_z;
      		color2 = colormapImage->getPixel(x,y,2)*normalizeParameter_z;
      	}
      	colors[3*count]=color0;
      	colors[3*count+1]=color1;
      	colors[3*count+2]=color2;
      //the index of positions +1
      	count++;
    }
  }
  
  //store the indices array for Lines and Triangles
  GenLineIndices();
  GenTriangleIndices();
  

  //create VAO
  glGenVertexArrays(1,&vao);
  glBindVertexArray(vao);


  // Generate 1 vbo buffer to store all vertices information
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,numOfVertices*(3+3)*sizeof(GLfloat),NULL,GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER,0,numOfVertices*3*sizeof(GLfloat),positions);//upload position data
  glBufferSubData(GL_ARRAY_BUFFER,numOfVertices*3*sizeof(GLfloat),numOfVertices*3*sizeof(GLfloat),colors);

  //Generate 1 veo buffer to store indices information, assign the buffer data later
  glGenBuffers(1,&ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);

  //initiate the shader program
  if(pipelineProgram->Init(shaderBasePath)!=0){
    cout << "Error finding the shaderBasePath " << shaderBasePath << "." << endl;
    exit(EXIT_FAILURE);
  }

  //get pipeline program handle
  program=pipelineProgram->GetProgramHandle();

  //bind vao, link the shader to vao
  bindProgram();
  
  //unbinf vao
  glBindVertexArray(0);

}

int main(int argc, char *argv[])
{
  if (argc < 2||argc>3)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }
  else if(argc==3){
  	//if color map is provided
  	isColored=true;
  	cout<<"rendering color image"<<endl;
  }
  else{
  	cout<<"rendering gray scale image"<<endl;
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


