#pragma once

// std
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

// glm
#include "pilcrow/engine/core/ReflectedGlm.hpp"

// ours
#include "Utils/Resource.hpp"

#include "pilcrow/modules/jellyfish_renderer/GL/GLShader.h"

namespace Jellyfish {
class GLProgram : public Resource {
public:
  GLProgram(const std::string &name);
  ~GLProgram();

  virtual bool        Reloadable() const override;
  virtual std::string Directory() const override;

  void SetModel(glm::mat4 mat);
  void SetView(glm::mat4 mat);
  void SetProjection(glm::mat4 mat);

  unsigned ID() const;
  void     Use(bool use = true) const;
  void     UpdateCB();


protected:
  virtual bool          LoadImpl() final;
  virtual void          UnloadImpl() final;


private:
  AlignedSceneConstantBuffer sceneBuffer;
  bool                       dirty;

  glm::mat4                                       model, view, proj;
  unsigned                                        m_ProgramID;
  std::vector<GLShader>                           m_shaders;

  Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
};  // end class GLProgram

}  // end namespace Jellyfish
