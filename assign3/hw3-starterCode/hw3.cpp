/* **************************
 * CSCI 420
 * Assignment 3 Raytracer
 * Name: <Your name here>
 * *************************
*/

#ifdef WIN32
  #include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
  #include <GL/gl.h>
  #include <GL/glut.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
  #define strcasecmp _stricmp
#endif

#include <imageIO.h>
#include <iostream>

//geometric transform header
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define EPSILON 0.001

using namespace glm;

#define MAX_TRIANGLES 20000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100

char * filename = NULL;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define fov 60.0

unsigned char buffer[HEIGHT][WIDTH][3];

struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

struct Triangle
{
  Vertex v[3];
};

struct Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
};

struct Light
{
  double position[3];
  double color[3];
};

struct IntersectPoint
{
  //light parameter
  vec3 color_specular; //ks
  vec3 color_diffuse; //kd
  vec3 normal;//n
  double shininess;
  double t;
  vec3 position;
};

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;

//three rendering types
typedef enum {SHADOW, RAY} INTERSECT_TYPE;

//Two shadow mode: soft shadow and hard shadow
typedef enum {HARD, SOFT} SHADOW_TYPE;
SHADOW_TYPE shadowType=SOFT;
//in order to render softshadow we add the surrounding light as a cube with length 0.2f
double shadowInterval = 0.13f;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);
vec3 ComputeLlight(IntersectPoint p,Light the_light);
bool TriangleIntersection(vec3 origin,vec3 dir,IntersectPoint &p, Triangle tri, INTERSECT_TYPE mode);
void TriInterpolation(double alpha,double beta,double gamma,Triangle tri, IntersectPoint &p);
bool SphereIntersec(vec3 origin,vec3 dir,IntersectPoint &p, Sphere sph,INTERSECT_TYPE mode);
void SphereInterpolation(Sphere sph, IntersectPoint &p);


vec3 ComputeLlight(IntersectPoint p,Light the_light){
  vec3 N=p.normal;
  vec3 L(the_light.position[0]-p.position.x,the_light.position[1]-p.position.y,the_light.position[2]-p.position.z);
  double distance=length(L);
  L=normalize(L);
  //decide if the intersection part is in the shadow
	bool ifShadow = false;
	IntersectPoint dummy;
	dummy.t=distance;
	for(int i=0;i<num_triangles && !ifShadow;i++){
		vec3 hahah(p.position+L*vec3(distance));
		ifShadow=TriangleIntersection(p.position,L,dummy, triangles[i], SHADOW);
		if(ifShadow) return vec3(0.0f); //if in shadow, no color
	}
	for(int i=0;i<num_spheres && !ifShadow;i++){
		ifShadow=SphereIntersec(p.position,L,dummy, spheres[i], SHADOW);
		if(ifShadow) return vec3(0.0f); //if in shadow, no color
	}


  vec3 V(-p.position);//origin(0,0,0)-p.position
  V=normalize(V);
  vec3 R(-reflect(L,N));

  double LdotN=std::max(0.0f,dot(L,N));
  double RdotV=std::max(0.0f,dot(R,V));
  vec3 I=p.color_diffuse*vec3(LdotN)+p.color_specular*vec3(pow(RdotV,p.shininess));

  I=vec3(the_light.color[0],the_light.color[1],the_light.color[2])*I;
  return I;
}

bool TriangleIntersection(vec3 origin,vec3 dir,IntersectPoint &p, Triangle tri, INTERSECT_TYPE mode){
	double alpha,beta,gamma;

	//vector for the three triangle vertex
	glm::vec3 vertex0(tri.v[0].position[0],tri.v[0].position[1],tri.v[0].position[2]);
	glm::vec3 vertex1(tri.v[1].position[0],tri.v[1].position[1],tri.v[1].position[2]);
	glm::vec3 vertex2(tri.v[2].position[0],tri.v[2].position[1],tri.v[2].position[2]);
	//calculate a,b,c,d for ax+by+cz+d=0 
	glm::vec3 norm = glm::cross(vertex1-vertex0,vertex2-vertex0);
	double d=-glm::dot(norm,vertex0);

	//decide if the ray is intersect with the plane
	if(abs(glm::dot(norm,dir))<EPSILON)return false;//if ray is parrallel to the plane, return no intersect
	double temp_t=-(dot(norm,origin)+d)/(double)glm::dot(norm,dir);
	if(temp_t<EPSILON)return false;//intersection is behine the origin
	if(temp_t>p.t-EPSILON && p.t!=-1) return false; //if intersection is far away from the already existing intersection
	if(mode==SHADOW){ // in case intersect with something behine the camera
		vec3 cur_P(origin+dir*vec3(temp_t));
		if(cur_P.z>-1- EPSILON){
			return false;
		}
	}
	//decide if the intersection point is inside the triangle
	glm::vec3 intersectP(dir*vec3(temp_t)+origin); //P
	
	//Another method determine if the point is in the triangle here
	/*glm::vec3 v01(vertex1 - vertex0); // v0=b-a
  	glm::vec3 v02(vertex2 - vertex0); // v1=c-a 
	glm::vec3 v0p(intersectP- vertex0); // v2=p-a
	float d00 = dot(v01, v01);
    float d01 = dot(v01, v02);
    float d11 = dot(v02, v02);
    float d20 = dot(v0p, v01);
    float d21 = dot(v0p, v02);
    float denom = d00 * d11 - d01 * d01;
    double v = (d11 * d20 - d01 * d21) / denom;
    double w = (d00 * d21 - d01 * d20) / denom;

    if(v>= - EPSILON&&w>= -EPSILON&&v+w <1+ EPSILON){
    	alpha=1-v-w;
    	beta=v;
    	gamma=w;
    	if(mode==SHADOW){return true;}
    }
    else{
    	return false;
    }*/

	//total area
	glm::vec3 v12(vertex2 - vertex1);
	glm::vec3 v10(vertex0 - vertex1);
	glm::vec3 totalArea=glm::cross(v12,v10);
	//area 0
	glm::vec3 vp1(vertex1 -intersectP);
	glm::vec3 vp2(vertex2 -intersectP);
	glm::vec3 area0=glm::cross(vp1,vp2);
	double tempDir=glm::dot(totalArea,area0);
	if(tempDir<0)return false;
	//area 1
	glm::vec3 vp0(vertex0 -intersectP);
	glm::vec3 area1=glm::cross(vp2,vp0);
	tempDir=glm::dot(totalArea,area1);
	if(tempDir<0)return false;
	//area 2
	glm::vec3 area2=glm::cross(vp0,vp1);
	tempDir=glm::dot(totalArea,area2);
	if(tempDir<0)return false;
	if(mode==SHADOW) return true; // when determining the shadow, no need to compute

	alpha=glm::length(area0)/glm::length(totalArea);
	beta=glm::length(area1)/glm::length(totalArea);
	gamma=glm::length(area2)/glm::length(totalArea);
	p.t=temp_t;
	p.position=dir*vec3(temp_t);

	TriInterpolation(alpha,beta,gamma,tri,p);

	return true;
}


void TriInterpolation(double alpha,double beta,double gamma,Triangle tri, IntersectPoint &p){

	glm::vec3 n0(tri.v[0].normal[0]*alpha,tri.v[0].normal[1]*alpha,tri.v[0].normal[2]*alpha);
	glm::vec3 n1(tri.v[1].normal[0]*beta,tri.v[1].normal[1]*beta,tri.v[1].normal[2]*beta);
	glm::vec3 n2(tri.v[2].normal[0]*gamma,tri.v[2].normal[1]*gamma,tri.v[2].normal[2]*gamma);
	p.normal=glm::normalize(n0+n1+n2);

	glm::vec3 d0(tri.v[0].color_diffuse[0]*alpha,tri.v[0].color_diffuse[1]*alpha,tri.v[0].color_diffuse[2]*alpha);
	glm::vec3 d1(tri.v[1].color_diffuse[0]*beta,tri.v[1].color_diffuse[1]*beta,tri.v[1].color_diffuse[2]*beta);
	glm::vec3 d2(tri.v[2].color_diffuse[0]*gamma,tri.v[2].color_diffuse[1]*gamma,tri.v[2].color_diffuse[2]*gamma);
	p.color_diffuse =d0+d1+d2;

	glm::vec3 s0(tri.v[0].color_specular[0]*alpha,tri.v[0].color_specular[1]*alpha,tri.v[0].color_specular[2]*alpha);
	glm::vec3 s1(tri.v[1].color_specular[0]*beta,tri.v[1].color_specular[1]*beta,tri.v[1].color_specular[2]*beta);
	glm::vec3 s2(tri.v[2].color_specular[0]*gamma,tri.v[2].color_specular[1]*gamma,tri.v[2].color_specular[2]*gamma);
	p.color_specular =s0+s1+s2;

	p.shininess=tri.v[0].shininess*alpha+tri.v[1].shininess*beta+tri.v[2].shininess*gamma;

}

bool SphereIntersec(vec3 origin,vec3 dir,IntersectPoint &p, Sphere sph,INTERSECT_TYPE mode){

	glm::vec3 center(sph.position[0],sph.position[1],sph.position[2]);
	double b=2*(double)glm::dot(dir,origin-center);
	double c=pow((double)glm::length(origin-center),2) - pow(sph.radius,2);
	if(b*b-4*c<0)return false;
	double delta=b*b-4*c;
	double temp_t0=(-b-sqrt(delta))/2.0;
	double temp_t1=(-b+sqrt(delta))/2.0;
	if(temp_t1<EPSILON)return false;//intersection is behine the view plane or is behind the nearest t
	if(temp_t0>p.t+EPSILON && p.t!=-1) return false; //if intersection is far away from the already existing intersection
	if(temp_t0<EPSILON && (temp_t1>p.t+EPSILON && p.t!=-1)) return false;
	if(mode==SHADOW) return true; // when determining the shadow, no need to compute
	if(temp_t0>EPSILON){
		p.t=temp_t0;
		p.position=dir*vec3(temp_t0);
	}
	else{
		p.t=temp_t1;
		p.position=dir*vec3(temp_t1);
	}
	SphereInterpolation(sph,p);

	return true;

}

void SphereInterpolation(Sphere sph, IntersectPoint &p){

	glm::vec3 center(sph.position[0],sph.position[1],sph.position[2]);
	p.normal=glm::normalize(p.position-center);
	p.shininess=sph.shininess;
	p.color_diffuse=vec3(sph.color_diffuse[0],sph.color_diffuse[1],sph.color_diffuse[2]);
	p.color_specular=vec3(sph.color_specular[0],sph.color_specular[1],sph.color_specular[2]);
}


vec3 SINGLE_RAYTRACE(double camX, double camY, double camZ){
	   	glm::vec3 rayDir(camX,camY,camZ);
    	rayDir=normalize(rayDir);

    	IntersectPoint nearestT;
    	nearestT.t=-1;

    	bool ifIntersect=false;
    	for(int i=0;i<num_triangles;i++){
    		ifIntersect=TriangleIntersection(vec3(0.0f),rayDir,nearestT, triangles[i], RAY) || ifIntersect; //if there is one intersect, then always true
    	}
    	for(int i=0;i<num_spheres;i++){
    		ifIntersect=SphereIntersec(vec3(0.0f),rayDir,nearestT, spheres[i], RAY) || ifIntersect;
    	}

    	vec3 lColor(ambient_light[0],ambient_light[1],ambient_light[2]);
    	if(ifIntersect){   	
    		for(int i=0;i<num_lights;i++){
    			if(shadowType==HARD){
    				lColor+=ComputeLlight(nearestT,lights[i]);
    			}
    			else{
    				Light surroundLight = lights[i];
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight.position[1]+=shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight.position[0]+=shadowInterval;
    				surroundLight.position[2]+=shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight.position[2]-=2*shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight.position[0]-=2*shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight.position[2]+=2*shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);

    				surroundLight = lights[i];
    				surroundLight.position[1]-=shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight.position[0]+=shadowInterval;
    				surroundLight.position[2]+=shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight.position[2]-=2*shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight.position[0]-=2*shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight.position[2]+=2*shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);

    				surroundLight = lights[i];
    				surroundLight.position[0]+=shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight = lights[i];
    				surroundLight.position[0]-=shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight = lights[i];
    				surroundLight.position[2]+=shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);
    				surroundLight = lights[i];
    				surroundLight.position[2]-=shadowInterval;
    				lColor+=ComputeLlight(nearestT,surroundLight)/vec3(15.0);

    			}
    		}
    	}
    	else{
    		lColor=vec3(1.0f);
    	}

    	return lColor;
}


//MODIFY THIS FUNCTION
void draw_scene()
{

	double scale = tan(fov/2.0f/180.0f*M_PI);
	double aspectR = WIDTH/HEIGHT; //aspect ratio
	double camZ=-1.0f; 

  //a simple test output
  for(unsigned int x=0; x<WIDTH; x++)
  {
    glPointSize(2.0);  
    glBegin(GL_POINTS);
    for(unsigned int y=0; y<HEIGHT; y++)
    {
    	//Uniformly send out rays from the camera location
    	//using Direct supersampling to do antialiasing
    	double camX_tr = scale*aspectR*(2.0f*(x+0.25f)/(double)WIDTH-1); //top right grid
    	double camY_tr = -scale*(2*(HEIGHT-(y+0.25f))/(double)HEIGHT-1); 
    	double camX_br = scale*aspectR*(2.0f*(x+0.25f)/(double)WIDTH-1); //bottom right grid
    	double camY_br = -scale*(2*(HEIGHT-(y-0.25f))/(double)HEIGHT-1);
    	double camX_bl = scale*aspectR*(2.0f*(x-0.25f)/(double)WIDTH-1); //bottom left grid
    	double camY_bl = -scale*(2*(HEIGHT-(y-0.25f))/(double)HEIGHT-1);
    	double camX_tl = scale*aspectR*(2.0f*(x-0.25f)/(double)WIDTH-1); //top left grid
    	double camY_tl = -scale*(2*(HEIGHT-(y+0.25f))/(double)HEIGHT-1); 

    	vec3 lColor=SINGLE_RAYTRACE(camX_tr,camY_tr,camZ)/vec3(4.0); //top right grid
    	lColor+=SINGLE_RAYTRACE(camX_br,camY_br,camZ)/vec3(4.0); //bottom right grid
    	lColor+=SINGLE_RAYTRACE(camX_bl,camY_bl,camZ)/vec3(4.0); //bottom left grid
    	lColor+=SINGLE_RAYTRACE(camX_tl,camY_tl,camZ)/vec3(4.0);//top left grid
    	//vec3 lColor=SINGLE_RAYTRACE(camX,camY,camZ);
 
    	lColor.x=clamp(lColor.x,0.0f,1.0f);lColor.y=clamp(lColor.y,0.0f,1.0f);lColor.z=clamp(lColor.z,0.0f,1.0f);
    	plot_pixel(x, y, (int)255*lColor.x, (int)255*lColor.y, (int)255*lColor.z);
    }
    glEnd();
    glFlush();
  }
  printf("Done!\n"); fflush(stdout);
}

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  glColor3f(((float)r) / 255.0f, ((float)g) / 255.0f, ((float)b) / 255.0f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  buffer[y][x][0] = r;
  buffer[y][x][1] = g;
  buffer[y][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
    plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  printf("Saving JPEG file: %s\n", filename);

  ImageIO img(WIDTH, HEIGHT, 3, &buffer[0][0][0]);
  if (img.save(filename, ImageIO::FORMAT_JPEG) != ImageIO::OK)
    printf("Error in Saving\n");
  else 
    printf("File saved Successfully\n");
}

void parse_check(const char *expected, char *found)
{
  if(strcasecmp(expected,found))
  {
    printf("Expected '%s ' found '%s '\n", expected, found);
    printf("Parse error, abnormal abortion\n");
    exit(0);
  }
}

void parse_doubles(FILE* file, const char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE *file, double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE *file, double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE * file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i", &number_of_objects);

  printf("number of objects: %i\n",number_of_objects);

  parse_doubles(file,"amb:",ambient_light);

  for(int i=0; i<number_of_objects; i++)
  {
    fscanf(file,"%s\n",type);
    printf("%s\n",type);
    if(strcasecmp(type,"triangle")==0)
    {
      printf("found triangle\n");
      for(int j=0;j < 3;j++)
      {
        parse_doubles(file,"pos:",t.v[j].position);
        parse_doubles(file,"nor:",t.v[j].normal);
        parse_doubles(file,"dif:",t.v[j].color_diffuse);
        parse_doubles(file,"spe:",t.v[j].color_specular);
        parse_shi(file,&t.v[j].shininess);
      }

      if(num_triangles == MAX_TRIANGLES)
      {
        printf("too many triangles, you should increase MAX_TRIANGLES!\n");
        exit(0);
      }
      triangles[num_triangles++] = t;
    }
    else if(strcasecmp(type,"sphere")==0)
    {
      printf("found sphere\n");

      parse_doubles(file,"pos:",s.position);
      parse_rad(file,&s.radius);
      parse_doubles(file,"dif:",s.color_diffuse);
      parse_doubles(file,"spe:",s.color_specular);
      parse_shi(file,&s.shininess);

      if(num_spheres == MAX_SPHERES)
      {
        printf("too many spheres, you should increase MAX_SPHERES!\n");
        exit(0);
      }
      spheres[num_spheres++] = s;
    }
    else if(strcasecmp(type,"light")==0)
    {
      printf("found light\n");
      parse_doubles(file,"pos:",l.position);
      parse_doubles(file,"col:",l.color);

      if(num_lights == MAX_LIGHTS)
      {
        printf("too many lights, you should increase MAX_LIGHTS!\n");
        exit(0);
      }
      lights[num_lights++] = l;
    }
    else
    {
      printf("unknown type in scene description:\n%s\n",type);
      exit(0);
    }
  }
  return 0;
}

void display()
{
}

void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(1,1,1,0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
    draw_scene();
    if(mode == MODE_JPEG)
      save_jpg();
  }
  once=1;
}

int main(int argc, char ** argv)
{
  if ((argc < 2) || (argc > 3))
  {  
    printf ("Usage: %s <input scenefile> [output jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
  {
    mode = MODE_JPEG;
    filename = argv[2];
  }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}