#pragma once

// ours
#include "pilcrow/modules/jellyfish_renderer/iMesh.h"
#include "pilcrow/modules/jellyfish_renderer/GL/GLTexture.h"
#include "GlobalD3DResources.h"

namespace Jellyfish {
class GLMesh : public iMesh {
public:
  GLMesh(const std::vector<Vertex> &              Vertices,
         const std::vector<unsigned> &            Indices,
         std::vector<std::shared_ptr<GLTexture>> &Textures);

  ~GLMesh() {}

  void Load();
  void Draw() override;

  void AssignShader(GLProgram &shader) override;

private:
  Microsoft::WRL::ComPtr<ID3D12Resource> m_verts, m_indices;

  Microsoft::WRL::ComPtr<ID3D12Resource> diffuse;

  unsigned int texID = -1;
};
}  // namespace Jellyfish
