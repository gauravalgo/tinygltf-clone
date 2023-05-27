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
void DrawMesh(Scene& scene , const Mesh& mesh )
{
  for(size_t i =0 ; i< mesh.primitives.size();i++)
  {
    const Primitive& primitive = mesh.primitives[i];
    if(primitive.indices.empty()) return;
    
    std::map<std::string,std::string>::const_iterator it(primitive.attributes.begin());
    std::map<std::string,std::string>::const_iterator itEnd(primitive.attributes.end());
    
    for(;it!= itEnd ;it++) {
      const Accessor& accessor = scene.accessors[it->second];
      glBindBuffer(GL_ARRAY_BUFFER,gBufferState[accessor.bufferView].vb);
      CheckErrors("bind buffer");

      int count =1 ;
      if(accessor.type == TINYGLTF_TYPE_SCALAR) {
        count =1;
      }
      else if(accessor.type == TINYGLTF_TYPE_VEC2) {
        count =2;
      }
      else if(accessor.type == TINYGLTF_TYPE_VEC3) {
        count =3;

      }
      else if(accessor.type == TINYGLTF_TYPE_VEC4) {
        count =4;
      }

      if((it->first.compare("POSITION") == 0 || (it->first.compare("NORMAL") == 0))){
        glVertexAttribPointer(gGLProgramState.attribs[it->first],count,accessor.componentType,GL_FALSE,accessor.byteStride,BUFFER_OFFSET(accessor.byteOffset));
       CheckErrors("vertex attrib pointer");
       glEnableVertexAttribArray(gGLProgramState.attribs[it->first]);
       CheckErrors("enable vertex attrib array");
      }
    }
    const Accessor& indexAccessor = scene.accessors[primitive.indices];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,gBufferState[indexAccessor.bufferView].vb);
    CheckErrors("bind buffer");
    int mode =-1;
    if(primitive.mode == TINYGLTF_MODE_TRIANGLES) {
      mode = GL_TRIANGLES;
    }
    else if(primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP)
    {
      mode = GL_TRIANGLE_STRIP;
    }
    else if(primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN){
      mode = GL_TRIANGLE_FAN;
    }
    else if(primitive.mode == TINYGLTF_MODE_POINTS)
    {
      mode = GL_POINTS;
    }
    else if(primitive.mode == TINYGLTF_MODE_LINE) {
      mode = GL_LINES;
    }
    else if(primitive.mode == TINYGLTF_MODE_LINE_LOOP) {
      mode = GL_LINE_LOOP;
    };
    glDrawElements(mode,indexAccessor.count,indexAccessor.componentType,BUFFER_OFFSET(indexAccessor.byteOffset));
    CheckErrors("drawElements");
    {
      std::map<std::string, std::string>::const_iterator it (primitive.attributes.begin());
      std::map<std::string, std::string>::const_iterator itEnd(primitive.attributes.end());

      for(;it!= itEnd;it++) {
        if((it->first.compare("POSITION")==0) || (it->first.compare("NORMAL") == 0)) {
          glDisableVertexAttribArray(gGLProgramState.attribs[it->first]);
        }
      }
    }
    }
  }
  
}
void DrawScene(Scene& scene) {
  std::map<std::string,std::string>::const_iterator it(primitive.attributes.begin());
  std::map<std::string,std::string>::const_iterator itEnd(primitive.attributes.end());
  for(;it!=itEnd;it++)
  {
  DrawMesh(scene,it->second);    
  }

}
static void Init() {
  trackball (curr_quat,0,0,0,0);
  eye[0] = 0.0f;
  eye[1] = 0.0f;
  eye[2] = CAM_Z;

  lookat[0] = 0.0f;
  lookat[1] = 0.0f;
  lookat[2] = 0.0f;

  up[0] = 0.0f;
  up[1] = 1.0f;
  up[2] = 0.0f;
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

  bool ret = loader.LoadFromFile(scene , err,argv[1]);
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
    //camera (define it in projection matrix)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    gluLookAt(eye[0],eye[1],eye[2],look_at[0],look_at[1],look_at[2],up[0],up[1],up[2]);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixd(&mat[0][0]);

    glScalef(scale,scale,scale);

    DrawScene(scene);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glFlush();

    glfwSwapBuffers(window);

  }
}
