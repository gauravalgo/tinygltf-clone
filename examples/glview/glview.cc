#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include <limits>
#include <cassert>
#include <cmath>

#include <GL/glew.h>

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif // DEBUG

#include <GLFW/glfw3.h>
#include "trackball.h"

#define TINYGLTF_LOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_gltf_loader.h"

#define BUFFER_OFFSET(i) ((char *)NULL +(I))

#define CAM_Z (3.0f)
int width = 768;
int height = 768;

double prevMouseX,prevMouseY;
bool mouseLeftPressed;
bool mouseMiddlePressed;
bool mouseRightPressed;
float curr_quat[4];
float prev_quat[4]
float eye[3],look[3],up[3];

GLFWwindow* window;
typedef struct{
  std::map<std::string,GLint> attribs;
}GLProgramState;
std::map<std::string,GLBufferState> gBufferState;
GLProgramState gGLProgramState;

void CheckErrors(std::string desc) {
  GLenum e = glGetError();
  if(e != GL_NO_ERROR){
    fprintf(stderr, "OpenGL error in \ %s \": %d (%d)\n ", desc.c_str(),e,e);
    exit(20);
  }
}
static void SetupGLState(Scene & scene, GLuint progId)
{
  std::map<std::string,BufferView>::const_iterator it(scene.bufferViews.begin());
  std::map<std::string,BufferView>::const_iterator itEnd(scene.bufferViews.end());

  for(;it!= itEnd;it++) {
    const BufferView & bufferView  = it->second;
    if(bufferView.target == 0) {
      continue;
    }

    const Buffer& buffer = scene.buffers[bufferView.buffer];
    GLBufferState state;
    glGenBuffers(1,&state.vb);
    glBindBuffer(bufferView.target,state.vb);
    glBufferData(bufferView.tatger,bufferView.byteLength,&buffer.data.at(0)+bufferView.byteOffset,GL_STATIC_DRAW);
    glBindBuffer(bufferView.target,0);
    gBufferState[it->first] = state;

  }
  glUseProgram(progId);
  GLint vtloc = glGetAttribLocation(progId,"in_vertex");
  GLint nrmloc = glGetAttribLocation(progId,"in_normal");
  gGLProgramState.attribs["POSITION"]= vtloc;
  gGLProgramState.attribs["NORMAL"]= nrmloc;


}
int main(int argc,char **argv)
{

  if(argc < 2){
    std::cout << "glview input.gltf <scale> \n " << std::endl;
    return 0;
  }
  float scale = 1.0f;
  if(argc > 2)
  {
    scale = atof(argv[2]);
  }
  Scene scene;
  TinyGLTFLoader loader;
  std::string err;

  bool ret = loader.LoadFromFile(scene , err,argvl[1]);
  if(!err.empty()){
    printf("Err: %s \n",err.c_str());
  }
  if(!ret){
    printf("Failed to load .gltf ")
  }
  Init();
  if(!glfwInit()){
    std::cerr << "Failed to intialize GLFW." << std::endl;
    return -1;
  }
  window = glfwCreateWindow(width,height,"Simple gltf geometry viewer",NULL,NULL);
  if(window == NULL)
  {
    std::cerr<< "Failed to open GLFW window ." <<std::endl;
    glfwTerminate();
    return 1;
  }

  glfwGetWindowSize(window, &width, &height);
  glfwMakeContextCurrent(window);

  //Callback
  glfwSetWindowCallback(window,reshapeFunc);
  glfwSetKeyCallback(window, keyboardFunc);
  glfwSetMouseCallback(window,motionFunc);

  glewExperimental = true;
  if(glewInit() != GLEW_OK){
    std::cerr << "Failed to initialize GLEW." << std::endl;
    return -1;
  }

  reshapeFunc(window,width,height);

  GLuint vertId =0, fragId = 0, progId =0;
  if(false == LoadShader(GL_VERTEX_SHADER,verId,"shader.vert")) {
    return -1;
  }
  CheckErrors("load vert shader");

  if(false == LoadShader(GL_FRAGMENT_SHADER,fragId,"shader.frag")){
    return -1;
  }
  CheckErrors("link") ;
  
  {
      GLint vtxLoc = glGetAttribLocation(progId,"in_vertex");
      if(vtxLoc < 0) {
        printf("vertex loc not found\n ");
        exit(-1);
      }
    
      GLint tnLoc = glGetAttribLocation(progId,"in_normal");
      if(tnLoc < 0 ) {
        printf("normal loc not found . \n");
        exit(-1);
      }
  }
  glUseProgram(progId);
  CheckErrors("useProgram");

  SetupGLState(scene , progId);
  while(glfwWindowShouldClose(window) == GL_FALSE) {
    glfwPollEvents();
    glClearColor(0.1f,0.2f,0.3f,1,0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    GLfloat mat[4][4];
    build_rotmatrix(mat,curr_quat);
  }
}
