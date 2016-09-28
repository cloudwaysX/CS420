/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code

  Student username: <type your USC username here>
*/

#include <iostream>
#include <cstring>
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
RENDER_STATE renderState = POINT;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;

//global variables
OpenGLMatrix *openGLMatrix = new OpenGLMatrix();
GLuint ebo_triangle;
GLuint vbo;
BasicPipelineProgram *pipelineProgram;
GLint program;
GLuint vao;
int numOfVertices =0;
int numOfStripPoint = 0;

//Set some constent parameter
const float fovy = 60;//the view angle is 45 degrees
const float aspect=(float)windowWidth/(float)windowHeight;//calculate the perpective 

//Set some helper function
void doTransform()
{
  //Calculate the transform matrix and store it into m_view
	openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
  openGLMatrix->LoadIdentity();
  openGLMatrix->Translate(landTranslate[0],landTranslate[1],landTranslate[2]);
  openGLMatrix->Rotate(landRotate[0], 1.0, 0.0, 0.0);
  openGLMatrix->Rotate(landRotate[1], 0.0, 1.0, 0.0);
  openGLMatrix->Rotate(landRotate[2], 0.0, 0.0, 1.0);
  openGLMatrix->Scale(landScale[0],landScale[1],landScale[2]);
  float m_view[16];
  openGLMatrix->GetMatrix(m_view);

  //pass the transform matrix into pipeline program
  //cout<<m_view[0]<<" "<<m_view[5]<<" "<<m_view[10]<<" "<<m_view[15]<<endl;
  pipelineProgram->SetModelViewMatrix(m_view);

}

void renderHeightField()
{
	//more to be added ....

	switch(renderState)
	{
		//render the heightfield as points
		case POINT:
		//more to be added
    glDrawArrays(GL_POINTS,0,numOfVertices);
		break;

		//render the heightfield as lines
		case LINE:
		//more to be added
    glDrawArrays(GL_LINES_ADJACENCY,0,numOfVertices);
		break;

		//render the heightfield as solid triangles
		case TRIANGLE:
		//more to be added
    glDrawElements(GL_TRIANGLE_STRIP,numOfStripPoint,GL_UNSIGNED_INT,BUFFER_OFFSET(0));
		break;
	}

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

void displayFunc()
{
	//render some stuff

  //bind the pipleline program
  pipelineProgram->Bind();
  glBindVertexArray(vao);


  //clear the Window
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	//do the transformation
	doTransform();
	renderHeightField();

  glBindVertexArray(0); //unbind the VAO

	//delete the double buffers
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
	openGLMatrix->SetMatrixMode(OpenGLMatrix::Projection);
	openGLMatrix->LoadIdentity();
	openGLMatrix->Perspective(fovy,aspect,0.01,10.0);
  //openGLMatrix->Ortho(0,0,0,0,0.01,10.0);
	float m_perspective[16];
	openGLMatrix->GetMatrix(m_perspective);

	pipelineProgram->SetProjectionMatrix(m_perspective);


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

void initScene(int argc, char *argv[]){

  // do additional initialization here...

  // load the image from a jpeg disk file to main memory
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  //get image height and width
  int height = heightmapImage->getHeight();
  int width = heightmapImage-> getWidth();
  numOfVertices =height*width;
  numOfStripPoint = height*width*2-width*2;
  cout<<"There is "<<numOfStripPoint<<" strip points and "<<numOfVertices<<" of positions"<<endl;

  int count=0;
  int normalizeParameter=(height+width)/2/256; //set depth in the similar scale as width and height

  //create VAO
  glGenVertexArrays(1,&vao);
  glBindVertexArray(vao);

  //Load the positions information and color information from image
  GLfloat *positions=new GLfloat[numOfVertices*3];
  GLfloat *colors=new GLfloat[numOfVertices*4];
  for(int x=-height/2;x<height/2;x++){
    for(int y=-width/2;y<width/2;y++){
      // assign the position value for each positions
      GLfloat z = heightmapImage->getPixel(x + height / 2, z + width / 2, 0);
      positions[3*count]=x;
      positions[3*count+1]=y;
      positions[3*count+2]=z*normalizeParameter;

      //assign the color value for each positions
      GLfloat color = heightmapImage->getPixel(x + height / 2, z + width / 2, 0)/(float)255.0f;
      colors[3*count]=color;
      colors[3*count+1]=color;
      colors[3*count+2]=color;
      colors[3*count+3]=1.0f;

      //the index of positions +1
      count++;
    }
  }
  //assign the indices information for element draw of triangle
  int count2=0;
  GLuint *indices_triangle= new GLuint[numOfStripPoint];
  for(int i=0;i<height-1;i++){
    for(int j=0;j<width;j++){
      indices_triangle[count2]=i*width+j;
      indices_triangle[count2+1]=i*width+j+width;
      count2=count2+2;
    }
  }

  cout<<"Now successfully load all positions and indices information"<<endl;

  //Connect VBO to VAO
  //generate and bind the EBO buffer for indices
  glGenBuffers(1,&ebo_triangle);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo_triangle);
  glBufferData(GL_ARRAY_BUFFER,numOfStripPoint*sizeof(GLuint),indices_triangle,GL_STATIC_DRAW);


  //generate and bind the VBO "buffer" for positions
  glGenBuffers(1,&vbo);
  glBindBuffer(GL_ARRAY_BUFFER,vbo);
  glBufferData(GL_ARRAY_BUFFER,numOfVertices*(3+4)*sizeof(GLfloat),NULL,GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER,0,numOfVertices*3*sizeof(GLfloat),positions);//upload position data
  glBufferSubData(GL_ARRAY_BUFFER,numOfVertices*3*sizeof(GLfloat),numOfVertices*4*sizeof(GLfloat),colors);

  //initilize the pipeline program
  pipelineProgram = new BasicPipelineProgram();
  if(pipelineProgram->Init(shaderBasePath)!=0){
    cout << "Error finding the shaderBasePath " << shaderBasePath << "." << endl;
    exit(EXIT_FAILURE);
  }
  //already load the shader in pipelineProgram->Init()

  // Enable depth testing, then prioritize fragments closest to the camera
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  //Bind the Shaders
  pipelineProgram->Bind();

  //get pipeline program handle
  program=pipelineProgram->GetProgramHandle();
  //activate the program
  glUseProgram(program);   

   //get location index of the "position"shader variable
  GLuint loc=glGetAttribLocation(program,"position");
  glEnableVertexAttribArray(loc);//enable "position attribe"
  GLsizei stride=0;
  GLboolean normalized=GL_FALSE;
  //set the layout of the "position" attribute data
  glVertexAttribPointer(loc,3,GL_FLOAT,normalized,stride,BUFFER_OFFSET(0));

  //get location index of the "color"shader variable
  GLuint loc2=glGetAttribLocation(program,"color");
  glEnableVertexAttribArray(loc2);//enable "position attribe"
  //set the layout of the "color" attribute data
  glVertexAttribPointer(loc2,4,GL_FLOAT,normalized,stride,BUFFER_OFFSET(numOfVertices*3*sizeof(GLfloat)));

  cout<<"Now successfully set layout of the position and color attribute data"<<endl;

  glBindVertexArray(0); //unbind the VAO   

}


int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
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

  //unbind VAO 
  glBindVertexArray(0);

  delete pipelineProgram;
}


