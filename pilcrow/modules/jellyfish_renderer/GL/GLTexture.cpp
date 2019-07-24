// std
#include <iostream>  //cout
#include <string>    //std::string

// stb image
#include <stb_image/stb_image.h>  //image data loader

// ours
#include "Utils/ResourceSettings.hpp"
#include "GlobalD3DResources.h"

#include "pilcrow/modules/jellyfish_renderer/GL/GLTexture.h"

namespace Jellyfish {
GLTexture::GLTexture(const std::string &                   name)
  : Resource(name) {
  this->Load(IOType::binary);
}

GLTexture::~GLTexture() { this->Unload(); }

void GLTexture::Use(int TextureUnit) const {
  // glActiveTexture(GL_TEXTURE0 + TextureUnit);
  // glBindTexture(GL_TEXTURE_2D, this->ID());
}

iTexture::TextureType GLTexture::Type() const { return m_Type; }

Microsoft::WRL::ComPtr<ID3D12Resource> GLTexture::ID() const {
  return m_textureResource;
}

bool GLTexture::Reloadable() const { return true; }

std::string GLTexture::Directory() const {
  return std::string(g_ResourcePath) + "Textures/";
}

GLTexture &GLTexture::Type(iTexture::TextureType type) {
  m_Type = type;
  return *this;
}

bool GLTexture::LoadImpl() { return TextureFromData(); }

void GLTexture::UnloadImpl() {
  m_Width = m_Height = m_NumChannels = 0;
  m_textureResource                  = nullptr;
}

bool GLTexture::TextureFromData() {
  auto device           = GlobalDeviceResources.GetD3DDevice();
  auto commandList      = GlobalDeviceResources.GetCommandList();
  auto commandAllocator = GlobalDeviceResources.GetCommandAllocator();

  unsigned char *imgData{
    stbi_load_from_memory(reinterpret_cast<const unsigned char *>(
                            this->Data().c_str()),
                          static_cast<int>(this->Data().size()),
                          &m_Width,
                          &m_Height,
                          &m_NumChannels,
                          0)};  // end imgData
                                // allocation

  if(imgData != nullptr) {
    std::cout << "Texture read: " << Filename() << std::endl;

    unsigned int stride;
    if(m_NumChannels == 4) {
      stride   = 4;
      m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    } else if(m_NumChannels == 3) {
      stride   = 4;
      m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    } else if(m_NumChannels == 1) {
      stride   = 1;
      m_Format = DXGI_FORMAT_R8_UNORM;
    }

    {
      unsigned char *data = imgData;

      // Convert for copy.
      if(m_NumChannels == 3) {
        data = new unsigned char[m_Width * m_Height * 4];

        for(unsigned int i = 0; i < m_Width * m_Height; i++) {
          auto indexOld = i * 3;
          auto indexNew = i << 2;

          data[indexNew]     = imgData[indexOld];
          data[indexNew + 1] = imgData[indexOld + 1];
          data[indexNew + 2] = imgData[indexOld + 2];
          data[indexNew + 3] = 1;
        }
      }

      // Upload.
      Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadResource;
      {
        PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Texture Upload");
        {
          auto defaultHeapProperties
            = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
          auto texDesc
            = CD3DX12_RESOURCE_DESC::Tex2D(m_Format, m_Width, m_Height, 1);
          DX::ThrowIfFailed(
            device->CreateCommittedResource(&defaultHeapProperties,
                                            D3D12_HEAP_FLAG_NONE,
                                            &texDesc,
                                            D3D12_RESOURCE_STATE_COPY_DEST,
                                            nullptr,
                                            IID_PPV_ARGS(&m_textureResource)));

          auto uploadHeapProperties
            = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
          CD3DX12_RESOURCE_DESC buffer = CD3DX12_RESOURCE_DESC::Buffer(
            GetRequiredIntermediateSize(m_textureResource.Get(), 0, 1));

          DX::ThrowIfFailed(
            device->CreateCommittedResource(&uploadHeapProperties,
                                            D3D12_HEAP_FLAG_NONE,
                                            &buffer,
                                            D3D12_RESOURCE_STATE_GENERIC_READ,
                                            nullptr,
                                            IID_PPV_ARGS(&m_uploadResource)));

          // Upload the texture info to the GPU.
          D3D12_SUBRESOURCE_DATA resource;
          resource.pData      = data;
          resource.RowPitch   = m_Width * stride;
          resource.SlicePitch = resource.RowPitch * m_Height;

          UpdateSubresources(commandList,
                             m_textureResource.Get(),
                             m_uploadResource.Get(),
                             0,
                             0,
                             1,
                             &resource);

          // Change state to read.
          D3D12_RESOURCE_BARRIER barrier[]
            = {CD3DX12_RESOURCE_BARRIER::
                 Transition(m_textureResource.Get(),
                            D3D12_RESOURCE_STATE_COPY_DEST,
                            D3D12_RESOURCE_STATE_GENERIC_READ)};
          commandList->ResourceBarrier(_countof(barrier), barrier);
        }
        PIXEndEvent(commandList);
      }

	  // Store.
      {
        GlobalDeviceResources.StoreResource(m_uploadResource);
        GlobalDeviceResources.StoreResource(m_textureResource);
      }

      // Delete
      if(m_NumChannels == 3) { delete[] data; }
    }

    // free memory and unbind
    stbi_image_free(imgData);

    std::cout << "Texture was loaded successfully: " << Filename() << std::endl;

    return true;
  }  // endif

  std::cout << "Failed to read texture data: " << this->Directory()
            << this->Filename() << std::endl;
  return false;
}

}  // end namespace Jellyfish
