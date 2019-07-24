// std
#include <array>
#include <iostream>

// ours
#include "pilcrow/modules/jellyfish_renderer/GL/GLShader.h"
#include "pilcrow/modules/jellyfish_renderer/Util.h"  //ShaderPath()

namespace Jellyfish {
GLShader::GLShader(const std::string &name) : Resource(name) {
  auto ext{this->Extension()};

  this->Load(IOType::binary);
}

GLShader::GLShader(GLShader &&shader)
  : Resource(std::move(shader)) {
}

GLShader::~GLShader() { this->Unload(); }

void GLShader::Use() const {
  // TODO(unknown): , not necessary yet
}

bool GLShader::Check() const {

  return true;
}

bool GLShader::Reloadable() const { return true; }

std::string GLShader::Directory() const { return ShaderPath(); }

bool GLShader::LoadImpl() {
  return true;
}

void GLShader::UnloadImpl() {

}

}  // namespace Jellyfish
