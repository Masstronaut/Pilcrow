// std
#include <array>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

// GLM
#include "pilcrow/engine/core/ReflectedGlm.hpp"

// ours
#include "pilcrow/modules/jellyfish_renderer/GL/GLProgram.h"
#include "pilcrow/modules/jellyfish_renderer/GL/GLShader.h"
#include "pilcrow/modules/jellyfish_renderer/Util.h"

namespace Jellyfish {
GLProgram::GLProgram(const std::string &name) : Resource(name), m_ProgramID(0) {
  this->Load();
}

GLProgram::~GLProgram() { this->Unload(); }

bool GLProgram::Reloadable() const { return true; }

std::string GLProgram::Directory() const { return ShaderPath(); }

unsigned GLProgram::ID() const { return m_ProgramID; }

void GLProgram::Use(bool use) const {
  auto device           = GlobalDeviceResources.GetD3DDevice();
  auto commandList      = GlobalDeviceResources.GetCommandList();
  auto commandAllocator = GlobalDeviceResources.GetCommandAllocator();

  if(use) {
    if(GlobalPipelineState != m_pipelineState) {
      GlobalPipelineState = m_pipelineState;

      commandList->SetPipelineState(GlobalPipelineState.Get());
      SetupHeap();
    }
  } else {
    GlobalPipelineState = nullptr;
  }
}

void GLProgram::SetModel(glm::mat4 mat) {
  dirty = true;
  model = mat;
}

void GLProgram::SetView(glm::mat4 mat) {
  dirty = true;
  view  = mat;
}

void GLProgram::SetProjection(glm::mat4 mat) {
  dirty = true;
  proj  = mat;
}

void GLProgram::UpdateCB() {
  if(!dirty) { return; }

  auto device           = GlobalDeviceResources.GetD3DDevice();
  auto commandList      = GlobalDeviceResources.GetCommandList();
  auto commandAllocator = GlobalDeviceResources.GetCommandAllocator();

  auto modelViewProj = proj * view * model;

  float *pt                           = glm::value_ptr(modelViewProj);
  sceneBuffer.constants.modelViewProj = DirectX::XMMATRIX(pt);

  // CBV
  Microsoft::WRL::ComPtr<ID3D12Resource> cbv;
  {
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc
      = CD3DX12_RESOURCE_DESC::Buffer(sizeof(AlignedSceneConstantBuffer));
    DX::ThrowIfFailed(
      device->CreateCommittedResource(&uploadHeapProperties,
                                      D3D12_HEAP_FLAG_NONE,
                                      &bufferDesc,
                                      D3D12_RESOURCE_STATE_GENERIC_READ,
                                      nullptr,
                                      IID_PPV_ARGS(&cbv)));
  }

  {
    AlignedSceneConstantBuffer *ptr;
    cbv->Map(0, nullptr, (void **)&ptr);
    { *ptr = sceneBuffer; }
    cbv->Unmap(0, nullptr);
  }

  {
    {
      // Change state to read.
      D3D12_RESOURCE_BARRIER barrier[]
        = {CD3DX12_RESOURCE_BARRIER::
             Transition(GlobalCBV.Get(),
                        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                        D3D12_RESOURCE_STATE_COPY_DEST)};
      commandList->ResourceBarrier(_countof(barrier), barrier);
    }

    // Copy over data.
    commandList->CopyBufferRegion(GlobalCBV.Get(),
                                  0,
                                  cbv.Get(),
                                  0,
                                  sizeof(AlignedSceneConstantBuffer));

    {
      // Change state to read.
      D3D12_RESOURCE_BARRIER barrier[]
        = {CD3DX12_RESOURCE_BARRIER::
             Transition(GlobalCBV.Get(),
                        D3D12_RESOURCE_STATE_COPY_DEST,
                        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)};
      commandList->ResourceBarrier(_countof(barrier), barrier);
    }
  }

  GlobalDeviceResources.StoreResource(cbv);

  dirty = false;
}

bool GLProgram::LoadImpl() {
  auto device = GlobalDeviceResources.GetD3DDevice();

  std::stringstream ss(Data());
  std::string       line;
  while(std::getline(ss, line)) {
    m_shaders.emplace_back(line);
  }

  bool success{true};

  D3D12_SHADER_BYTECODE byteCode[2] = {};
  for(int i = 0; i < m_shaders.size(); i++) {
    success &= m_shaders[i].Loaded();

    byteCode[i]
      = {reinterpret_cast<UINT8 *>((void *)m_shaders[i].Data().c_str()),
         m_shaders[i].Data().length()};
  }

  if(success) {
    const D3D12_INPUT_ELEMENT_DESC layout[] = {
      {"SV_POSITION",
       0,
       DXGI_FORMAT_R32G32B32_FLOAT,
       0,
       0,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
       0},
      {"COLOR",
       0,
       DXGI_FORMAT_R32G32B32A32_FLOAT,
       0,
       12,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
       0},
      {"TEXCOORD",
       0,
       DXGI_FORMAT_R32G32B32_FLOAT,
       0,
       28,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
       0},
      {"NORMAL",
       0,
       DXGI_FORMAT_R32G32B32_FLOAT,
       0,
       40,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
       0},
      {"TANGENT",
       0,
       DXGI_FORMAT_R32G32B32_FLOAT,
       0,
       52,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
       0},
      {"BITANGENT",
       0,
       DXGI_FORMAT_R32G32B32_FLOAT,
       0,
       64,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
       0},
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout                        = {layout, _countof(layout)};
    psoDesc.pRootSignature                     = GlobalRootSignature.Get();
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState      = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable   = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask                      = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets      = 1;
    psoDesc.RTVFormats[0]         = DXGI_FORMAT_R10G10B10A2_UNORM;
    psoDesc.SampleDesc.Count      = 1;
    psoDesc.VS                    = byteCode[0];
    psoDesc.PS                    = byteCode[1];
    DX::ThrowIfFailed(
      device->CreateGraphicsPipelineState(&psoDesc,
                                          IID_PPV_ARGS(&m_pipelineState)));

  } else {
    std::cout << "Failed to load shader program: " << this->Filename() << '.'
              << std::endl;
  }
  return success;
}

void GLProgram::UnloadImpl() {
  m_ProgramID = 0;
  m_shaders.clear();
}
}  // namespace Jellyfish