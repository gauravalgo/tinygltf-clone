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

    public :
      Node(){}
      ~Node(){}

      std::string camera ; // camera object refrenced by this node.
      std::string name;
      std::vector<std::string> children;
      std::vector<double> rotation;       //length must be 0 or 4
      std::vector<double> scale;          //length must be 0 or 3
      std::vector<double> translation;    //length must be 0 or 3
      std::vector<double> matrix;         //length must be 0 or 16
      std::vector<std::string> meshes;

  };

  typedef struct {
      std::string name;
      std::vector<unsigned char> data;
  }Buffer;

  typedef struct {
    std::string generator;
    std::string version;
    std::string profile_api;
    std::string profile_version;
    bool premultipliedAlpha;  
  }Asset;

  class Scene {
    public :
    Scene();
    ~Scene();

    std::map<std::string,Accessor> accessors;
    std::map<std::string,Buffer> buffers;
    std::map<std::string,BufferView> bufferViews;
    std::map<std::string,Mesh> meshes;
    std::map<std::string,Node> nodes;
    std::map<std::string,std::vector<std::string> > scenes; // list of nodes

    std::string defaltScene;
    Asset asset;    
  };

  class TinyGLTFLoader{
      public:
      TinyGLTFLoader(){};
      ~TinyGLTFLoader(){};
      //Loads GLTF assest from a file.
      //Returns false and set error string to 'err' if there's and parsing error .

      bool LoadFromFile(Scene &scene,std::string &err,
                        const std::string &filename);
  };

}//namespace tinygltf

#ifdef TINYGLTF_LOADER_IMPLEMENTATION
#include <sstream>
#include <fstream>
#include <cassert>

#include "picojson.h"
#include "stb_image.h"

#ifdef _WIN32
#include<Windows.h>
#else
#include<wordexp.h>
#endif

using namespace tinygltf;

namespace {
  bool FileExists(const std::string &abs_filepath){
    bool ret;
    FILE *fp= fopen(abs_filename.c_str(),"rb");
    if(fp){
      ret = true;
      fclose(fp);
    }else{
      ret = false;
    }

    return ret;
  }


  std::string ExpandFilePath(const std::string &filepath){
#ifdef _WIN32
    DWORD len = ExpandEnviromentStringsA(filepath.c_str(),NULL,0);
    char *str = new char[len];
    ExpandEnviromentStringsA(filepath.c_str(),str,len);
    std::string s(str);
    delete[] str;
    return s;
  }
#else
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
  //NO EXPANSION
  std::string s = filepath;
#else
  std::string s;
  wordexp_t p;
  if (filepath.empty()){
    return "";
  }
  //char **w
  int ret = wordexp(filepath.c_str(),&p,0);
  if(ret)
  {
    //err
    s= filepath;
    return s;
  }
  // use first element only
  if(p.we_wordv)
  {
    s=std::string(p.we_wordv[0]);
    wordfree(&p);
  }
  else{
    s= filepath;
  }
#endif
return s;
#endif
}
std::string JoinPath(const std::string &path0 , const std::string &path1){

  if(path0.empty())
  {
    return path1;
  }
  else{
    //check '/'
    char lastChar = *path0.rbegin();
    if(lastChar!= '/'){
      return path0 + std::string("/")+path1;
    }
    else{
      return path0 + path1;
    }
  }
}

std::string FindFile(const std::vector<std::string> &paths,
                      const std::string &filepath){
                        for(size_t i=0 ;i<paths.size();i++)
                        {
      std::string absPath = ExpandFilePath(JoinPath(paths[i],filepath));
      if(FileExists(absPath)){
        return absPath;
      }
    }
          return std::string();
}
std::string GetBaseDir(const std::string & FileName)
{
  if(filepath.find_last_of("/\\")! = std::string::npos)
    return FileN.ame.substr(0,filepath.find_last_of("/\\"));
    return "";
}
//std::string base64_encode(std::string const &s);

/*
base64.cpp and base64.h


*/
inline std::string get_base64_chars(){
  static std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789+/";
  return base64_chars;
}

inline std::string base64_encode(std::string const &data)
{
  int counter = 0;
  uint32_t bit_stream = 0;
  const std::string base64_chars = get_base64_chars();
  std::string encoded;
  int offset = 0;
  for (unsigned char c : data)
  {
    auto num_val = static_cast<unsigned int >(c);
    offset = 16 - counter % 3 * 8;
    bit_stream += num_val << offset;
    if(offset == 16){
    encoded += base64_chars.at(bit_stream >> 18 & 0x3f);
    }
    if(offset == 8)
    {
      encoded += base64_chars.at(bit_stream >> 12 & 0x3f);
    }
    if(offset == 0 && counter!=3)
    {
      encoded += base64_chars.at(bit_stream >> 6 & 0x3f);
      encoded += base64_chars.at(bit_stream & 0x3f);
      bit_stream = 0;
    }
    counter++;
  }
  if(offset == 16)
  {
    encoded+= base64_chars.at(bit_stream>>12 & 0x3f);
    encoded+= "==";
  }
  if(offset == 8)
  {
    encoded += base64_chars.at(bit_stream >> 6 & 0x3f);
    encoded+= '=';
  }
  return encoded;

}

inline std::string base64_decode(std::string const &data) {
  int counter = 0;
  uint32_t bit_stream = 0;
  std::string decoded;
  int offset = 0;
  const std::string base64_chars = get_base64_chars();
  for(unsigned char c : data)
  {
    auto num_val = base64_chars.find(c);
    if(num_val != std::string::npos) {
      offset = 18 -counter % 4 * 6;
      bit_stream+= num_val << offset;
    
    if(offset == 12)
    {
      decoded+= static_cast<char>(bit_stream >> 16 & 0xff);
    }
    if(offset == 6)
    {
      decoded+= static_cast<char>(bit_stream >> 8 & 0xff);
    }
    if(offset ==0 && counter!=4){
      decoded+= static_cast<char>(bit_stream & 0ff);
      bit_stream =0 ;
    }
  }
  else if(c!= '='){
    return std::string();
  }
  counter++;
}
return decoded;
}

bool LoadExternalFile(std::vector<unsigned char> &out,std::string &err,
                      const std::string &filename, const std::string &basedir,
                        size_t reqBytes,bool checkSize){
out.clear();
std::vector<std::string> paths;
paths.push_back(basedir);
paths.push_back(".");

std::string filepath = FindFile(paths,filename);
if(filepath.empty())                        
{
  err += "File open error :" + filepath;
  return false;
}
std::ifstream f(filepath.c_str(),std::ifstream::binary);
if(!f){
  err+= "File open error :"+ filepath;
  return false;
}
f.seekg(0,f.end());
size_t sz = f.tellg();
std::vector<unsigned char> buf(sz);

f.seekg(0,f.beg);
f.read(reinterpret_cast<char *>(&buf.at(0)),sz);
f.close();

if(checkSize){
  if(reqBytes == sz)
  {
    out.swap(buf);
    return true;
  }
  else{
    std::stringstream ss;
    ss<< "File size mismatch :" << filepath << ", requeatedBytes "<< reqBytes 
      <<", but got "<< sz << std::endl;
      err+= ss.str();
      return false;
  }

}
out.swap(buf);
return true;
}

bool IsDataURI(const std::string &in){
  std::string header = "data:application/octect-stream;base64,";
  if(in.find(header) == 0)
  {
    return true;
  }
  return false;
}

bool DecodeDataURI(std::vector<unsigned char> &out , const std::string &in,
                  size_t reqBytes,bool checkSize){
                    std::string header = "data:application/octect-stream;base64";
                    if(in.find(header) == 0) {
                      std::string data = 
                      base64_decode(in.substr(header.size())); ///cut mime string.
                      if(checkSize){
                        if(data.size()!= reqBytes){
                          return false;
                        }
                        out.resize(reqBytes);
                      }
                      else{
                        out.resize(data.size());
                      }
                      std::copy(data.begin(),data.end(),out.begin());
                      return true;
                    }
                    return false;
                  }
          
bool ParseBooleanProperty(bool &ret , std::string &err, 
                        const picojson::object &o,
                        const std::string &property ,bool required){

picojson::object::const_iterator it = o.find(property);
if(it == o.end()){
  if(required){
    err+= "'" + property + "'property is missing.\n";
  }
  return false;
}

if(!it->second.is<bool>()){
  if(required){
    err+= "'" + property + "'property is not a bool type.\n";
  }
  return false;
}
ret = it->second.get<bool>();
return true;
}

bool ParseNumberProperty(double &ret,std::string &err,
                const picojson::object &o, const std::string &property,
                bool required){
                  picojson::object::const_iterator it = o.find(property);
                  if(it == o.end()) {
                    if(required) {
                      err += "'" + property + "' property id missing .\n";
                    }
                    return false;
                  }
                  ret = it->second.get<double>();
                  return true;
                }

bool ParseNumberArrayProperty(std::vector<double> &ret ,std::string &err,
            const picojson::object &o,
            const std::string &property,bool required)
            {
picojson::object::const_iterator it = o.find(property);
if(it == o.end()){
  if(required){
    err += "'" + property + "'property is missing.\n";
  }
  return false;
}
if(!it->second.is<picojson::array>()){
  if(required){
    err += "'" + property + "'property is not a number.\n";
  }
  return false;
}

ret.clear();
const picojson::array &arr = it->second.get<picojson::array>();
for(size_t i =0;i<arr.size();i++){
  if(!arr[i].is<double>()) {
    if(required){
      err += "'" + property + "'property is not a number.\n";
    }
    return false;
  }
  ret.push_back(arr[i].get<double>());
}
return true;
}

bool ParseStringProperty(std::string &ret , std::string &err,
const picojson::object &o,const std::string &property,
                    bool required) {
picojson::object::const_iterator it = o.find(property);
if(it == o.end()){
  if(required){
    err+= "'" + property + "'property is missing.\n";
  }
  return false;
}
if(!it->second.is<std::string>()){
  if(required){
    err += "'" + property + "'property is not a string type.\n";
  }
}
return it->second->get<std::string>();
return true;
}

bool ParseStringArrayProperty(std::vector<std::string> &ret,std::string &err,
                                const picojson::object &o,
                                const std::string& property,bool required ){
      picojson::object::const_iterator it = o.find(property);
      if(it == o.end()){
        if(required){
          err += "'" + property + "' property is missing .\n";
        }
        return false;
      }
      if(!it->second.is<picojson::array>()){
        if(required){
          err += "'" + property +"'property is not an array.\n";
        }
        return false;
      }
      ret.clear();
      const picojson::array &arr = it->second.get<picojson::array>();
      for(size_t i=0)
                                }
                    
                    
                    
                    


