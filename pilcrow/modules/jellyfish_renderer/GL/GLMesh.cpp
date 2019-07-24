// std
#include <iostream>

// ours
#include "pilcrow/modules/jellyfish_renderer/GL/GLMesh.h"
#include "pilcrow/modules/jellyfish_renderer/GL/GLTexture.h"

namespace Jellyfish {
GLMesh::GLMesh(const std::vector<Vertex> &              Vertices,
               const std::vector<unsigned> &            Indices,
               std::vector<std::shared_ptr<GLTexture>> &Textures)
  : iMesh(Vertices, Indices, Textures) {
  Load();
}

void GLMesh::Draw() {
  auto device           = GlobalDeviceResources.GetD3DDevice();
  auto commandList      = GlobalDeviceResources.GetCommandList();
  auto commandAllocator = GlobalDeviceResources.GetCommandAllocator();

  D3D12_VERTEX_BUFFER_VIEW vbv;
  {
    // setup vertex and index buffer structs
    vbv.BufferLocation = m_verts->GetGPUVirtualAddress();
    vbv.StrideInBytes  = sizeof(Vertex);
    vbv.SizeInBytes    = vbv.StrideInBytes * m_Vertices.size();
  }

  D3D12_INDEX_BUFFER_VIEW ibv;
  {
    ibv.BufferLocation = m_indices->GetGPUVirtualAddress();
    ibv.Format         = DXGI_FORMAT_R32_UINT;
    ibv.SizeInBytes    = m_Indices.size() * sizeof(UINT32);
  }

  //if (!diffuse)
  {
    {
      for(unsigned i{0}; i < m_Textures.size(); ++i) {
        switch(m_Textures[i]->Type()) {
          case iTexture::TextureType::diffuse: {
            diffuse = m_Textures[i]->ID();

            break;
          }
        }
      }

      if(!diffuse) {
        return;
      } else {
        texID = SetupTexture(
			diffuse,
			texID);
      }
    }
  }

  m_shader->Use(true);
  {
    // Draw.
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render Mesh");
    {
      m_shader->UpdateCB();

      if(!diffuse)
        SetTexture(LastTexID);
      else
        SetTexture(texID);

      commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      commandList->IASetVertexBuffers(0, 1, &vbv);
      commandList->IASetIndexBuffer(&ibv);
      commandList->DrawIndexedInstanced(m_Indices.size(), 1, 0, 0, 0);

      // Store.
    {
      GlobalDeviceResources.StoreResource(m_verts);
      GlobalDeviceResources.StoreResource(m_indices);
      GlobalDeviceResources.StoreResource(diffuse);
    }
  }
  PIXEndEvent(commandList);
  }
}

void GLMesh::Load() {
  auto device           = GlobalDeviceResources.GetD3DDevice();
  auto commandList      = GlobalDeviceResources.GetCommandList();
  auto commandAllocator = GlobalDeviceResources.GetCommandAllocator();

  auto vertSize  = m_Vertices.size() * sizeof(Vertex);
  auto indexSize = m_Indices.size() * sizeof(decltype(m_Indices)::value_type);

  {
    Microsoft::WRL::ComPtr<ID3D12Resource> upload;
    {
      unsigned int maxDataSize = vertSize;
      auto         uploadHeapProperties
        = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
      auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(maxDataSize);
      DX::ThrowIfFailed(
        device->CreateCommittedResource(&uploadHeapProperties,
                                        D3D12_HEAP_FLAG_NONE,
                                        &bufferDesc,
                                        D3D12_RESOURCE_STATE_GENERIC_READ,
                                        nullptr,
                                        IID_PPV_ARGS(&upload)));
    }

    {
      void *pMappedData;
      upload->Map(0, nullptr, &pMappedData);
      memcpy(pMappedData, m_Vertices.data(), vertSize);
      upload->Unmap(0, nullptr);
    }

    {
      {
        auto defaultProperties
          = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertSize);
        DX::ThrowIfFailed(
          device->CreateCommittedResource(&defaultProperties,
                                          D3D12_HEAP_FLAG_NONE,
                                          &bufferDesc,
                                          D3D12_RESOURCE_STATE_COPY_DEST,
                                          nullptr,
                                          IID_PPV_ARGS(&m_verts)));
      }

      PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Vert Upload");
      {
        // Copy over data.
        commandList->CopyBufferRegion(m_verts.Get(),
                                      0,
                                      upload.Get(),
                                      0,
                                      vertSize);

        // Change state to index buffer.
        D3D12_RESOURCE_BARRIER barrier[]
          = {CD3DX12_RESOURCE_BARRIER::
               Transition(m_verts.Get(),
                          D3D12_RESOURCE_STATE_COPY_DEST,
                          D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)};
        commandList->ResourceBarrier(_countof(barrier), barrier);
      }
      PIXEndEvent(commandList);

      // Store.
      {
        GlobalDeviceResources.StoreResource(m_verts);
        GlobalDeviceResources.StoreResource(upload);
      }
    }
  }

  {
    Microsoft::WRL::ComPtr<ID3D12Resource> upload;
    {
      unsigned int maxDataSize = indexSize;
      auto         uploadHeapProperties
        = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
      auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(maxDataSize);
      DX::ThrowIfFailed(
        device->CreateCommittedResource(&uploadHeapProperties,
                                        D3D12_HEAP_FLAG_NONE,
                                        &bufferDesc,
                                        D3D12_RESOURCE_STATE_GENERIC_READ,
                                        nullptr,
                                        IID_PPV_ARGS(&upload)));
    }

    {
      void *pMappedData;
      upload->Map(0, nullptr, &pMappedData);
      memcpy(pMappedData, m_Indices.data(), indexSize);
      upload->Unmap(0, nullptr);
    }

    {
      auto defaultProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
      auto bufferDesc        = CD3DX12_RESOURCE_DESC::Buffer(indexSize);
      DX::ThrowIfFailed(
        device->CreateCommittedResource(&defaultProperties,
                                        D3D12_HEAP_FLAG_NONE,
                                        &bufferDesc,
                                        D3D12_RESOURCE_STATE_COPY_DEST,
                                        nullptr,
                                        IID_PPV_ARGS(&m_indices)));
    }

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Index Upload");
    {
      // Copy over data.
      commandList->CopyBufferRegion(m_indices.Get(),
                                    0,
                                    upload.Get(),
                                    0,
                                    indexSize);

	   // Change state to index buffer.
      D3D12_RESOURCE_BARRIER barrier[]
        = {CD3DX12_RESOURCE_BARRIER::
             Transition(m_indices.Get(),
                        D3D12_RESOURCE_STATE_COPY_DEST,
                        D3D12_RESOURCE_STATE_INDEX_BUFFER)};
      commandList->ResourceBarrier(_countof(barrier), barrier);
    }
    PIXEndEvent(commandList);

    // Store.
    {
      GlobalDeviceResources.StoreResource(m_indices);
      GlobalDeviceResources.StoreResource(upload);
    }
  }
}

void GLMesh::AssignShader(GLProgram &shader) { m_shader = &shader; }
}  // namespace Jellyfish
