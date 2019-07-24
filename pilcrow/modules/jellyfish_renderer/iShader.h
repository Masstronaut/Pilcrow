#pragma once

// std
#include <string>

namespace Jellyfish {
class iShader {
public:
  // Not using this yet
  //// Add others as needed
  // enum class ShaderType
  //{
  //	vertex,
  //	fragment,
  //	tesselation,
  //	geometry,
  //	compute
  //}; //end enum ShaderType

  virtual ~iShader() {}

  virtual void     Use() const  = 0;

  // planned functionality
  // AddToEffect() const = 0;

protected:
  virtual bool Check() const = 0;

private:
};
}  // namespace Jellyfish