#ifndef TINY_GLTF_LOADER_H
#define TINY_GLTF_LOADER_H

#include <string>
#include <vector>
#include <map>

namespace tinygltf {

  #define TINYGLTF_MODE_POINTS (0)
  #define TINYGLTF_MODE_LINE (1)
  #define TINYGLTF_MODE_LINE_LOOP (2)
  #define TINYGLTF_MODE_TRIANGLES (4)
  #define TINYGLTF_MODE_TRIANGLE_STRIP (5)
  #define TINYGLTF_MODE_TRIANGLE_FAN (6)

  #define TINYGLTF_COMPONENT_TYPE_BYTE (5120)
  #define TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE (5121)
  #define TINYGLTF_COMPONENT_TYPE_SHORT (5122)
  #define TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT (5123) 
  #define TINYGLTF_COMPONENT_TYPE_INT (5124)
  #define TINYGLTF_COMPONRNT_TYPE_UNSIGNED_INT (5125)
  #define TINYGLTF_COMPONENT_TYPE_FLOAT (5126)
  #define TINYGLTF_COMPONENT_TYPE_DOUBLE (5127)

  #define TINYGLTF_TYPE_VEC2 (2)
  #define TINYGLTF_TYPE_VEC3 (3)
  #define TINYGLTF_TYPE_VEC4 (4)
  #define TINYGLTF_TYPE_MAT2 (32 + 2)
  #define TINYGLTF_TYPE_MAT2 (32 + 3)
  #define TINYGLTF_TYPE_MAT2 (32 + 4)
  #define TINYGLTF_TYPE_SCALAR (64 + 1)
  #define TINYGLTF_TYPE_VECTOR (64 + 4)
  #define TINYGLTF_TYPE_MATRIX (64 + 16)

  #define TINYGLTF_IMAGE_FORMAT_JPEG (0)
  #define TINYGLTF_IMAGE_FORMAT_PNG (1)
  #define TINYGLTF_IMAGE_FORMAT_BMP (2)
  #define TINYGLTF_IMAGE_FORMAT_GIF (3)

  #define TINYGLTF_TARGET_ARRAY_BUFFER (34962)
  #define TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER (34963)

  typedef std::map<std::string,std::vector<double> > FloatParameterMap;

  //LDR 8bit image 
  typedef struct {
    std::string name;
    int width;
    int height;
    int component;
    std::vector<unsigned char> image;
  }Image;

  typedef struct {
    std::string name;
    std::string technique;
    FloatParameterMap values;
  }Material;

  typedef struct {
    std::string name;
    std::string buffer;
    size_t byteOffset;
    size_t byteLength;
    int target;
  }BufferView;

  typedef struct {
    std::string BufferView;
    std::string name;
    size_t byteoffset;
    size_t byteStride;
    int componentType; //one of the TINYGLTF_COMPONENT_TYPE type
    size_t count;
    int type;
    std::vector<double> minValues;
    std::vector<double> maxValues;
  }Accessor;

  class Camera{
    public:
    Camera() {};
    ~Camera() {};
    std::string name;
    bool isOrthographic; //false == perspective

    //some common properties
    float aspectRatio;
    float yFov;
    float zFar;
    float zNear;
  };

  typedef struct {
    std::map<std::string,std::string> attributes; // A dictonary object of strings
                                                  //where each string is the id of the accesscor
                                                  //conataining an attribute.
    std::string material;                         //The ID of the material to apply to this primitive
                                                  //when rendering
    std::string indicies;                         //the id of the accessor that contains the indices.
    int mode;                                     //one of the TINYGLTF_MODE***.
  }Primitive;

  typedef struct {
    std::string name;
    std::vector<Primitive> primitives;
  } Mesh; 

  class Node{
    publiv
  };

}
