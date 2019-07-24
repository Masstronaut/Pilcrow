#pragma once

// GL
#include "GlobalD3DResources.h"

// ours
#include "Utils/Resource.hpp"

#include "pilcrow/modules/jellyfish_renderer/iShader.h"

namespace Jellyfish {
class GLShader : public iShader, public Resource {
public:
  GLShader(const std::string &name);
  GLShader(GLShader &&shader);
  ~GLShader();

  // Overriding iShader:
  void     Use() const override;
  bool     Check() const override;

  // Overriding Resource:
  bool        Reloadable() const override;
  std::string Directory() const override;

private:
  // Overriding Resource:
  bool LoadImpl() final;
  void UnloadImpl() final;

  Microsoft::WRL::ComPtr<ID3D12Resource> m_shader;

};  // class GLShader

}  // namespace Jellyfish
