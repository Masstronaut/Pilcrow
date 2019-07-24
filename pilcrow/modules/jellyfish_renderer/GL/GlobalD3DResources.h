#pragma once

#define NUM_TEXTURES 50

#include "DescriptorHeap.h"
#include "DeviceResources.h"

// TODO: move to methods and don't have static.  That is why this is a shared
// pointer.
extern DX::DeviceResources                         GlobalDeviceResources;
extern Microsoft::WRL::ComPtr<ID3D12RootSignature> GlobalRootSignature;

extern Microsoft::WRL::ComPtr<ID3D12Resource> GlobalCBV;

extern Microsoft::WRL::ComPtr<ID3D12PipelineState> GlobalPipelineState;

extern DX::DescriptorHeap CSUDescriptors;
extern DX::DescriptorHeap SamplerDescriptors;

extern D3D12_VIEWPORT GlobalViewport;
extern D3D12_RECT     GlobalScissorRect;

extern unsigned int GlobalWidth;
extern unsigned int GlobalHeight;

extern int GlobalTexID;  // TODO: check if re-using id
extern int LastTexID;

struct SceneConstantBuffer {
  DirectX::XMMATRIX modelViewProj;
};

// Align to a certain value of power of 2.
inline constexpr UINT Align(const UINT size, const UINT alignment) {
  assert(((alignment - 1) & alignment) == 0);
  return (size + (alignment - 1)) & ~(alignment - 1);
}

// Align a constant buffer.
inline constexpr UINT CalculateConstantBufferByteSize(const UINT size) {
  return Align(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
}

union AlignedSceneConstantBuffer {
  SceneConstantBuffer constants;
  uint8_t             alignmentPadding[CalculateConstantBufferByteSize(
    sizeof(SceneConstantBuffer))];
};

extern AlignedSceneConstantBuffer *GlobalMappedCBV;

static void GlobalSetupRootParams() {
  auto device = GlobalDeviceResources.GetD3DDevice();

  CD3DX12_DESCRIPTOR_RANGE            ranges[2];
  std::vector<CD3DX12_ROOT_PARAMETER> rootParameters;

  {
    auto v = CD3DX12_ROOT_PARAMETER();
    v.InitAsConstantBufferView(0);

    rootParameters.emplace_back(v);
  }

  {
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, NUM_TEXTURES, 0);
    auto v = CD3DX12_ROOT_PARAMETER();
    v.InitAsDescriptorTable(1, &ranges[0]);

    rootParameters.emplace_back(v);
  }

  {
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
    auto v = CD3DX12_ROOT_PARAMETER();
    v.InitAsDescriptorTable(1, &ranges[1]);

    rootParameters.emplace_back(v);
  }

  CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
  rootSignatureDesc
    .Init(rootParameters.size(),
          rootParameters.data(),
          0,
          nullptr,
          D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
            | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT);

  Microsoft::WRL::ComPtr<ID3DBlob> signature;
  Microsoft::WRL::ComPtr<ID3DBlob> error;
  DX::ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc,
                                                D3D_ROOT_SIGNATURE_VERSION_1,
                                                &signature,
                                                &error));
  DX::ThrowIfFailed(
    device->CreateRootSignature(0,
                                signature->GetBufferPointer(),
                                signature->GetBufferSize(),
                                IID_PPV_ARGS(&GlobalRootSignature)));

  // CBV
  {
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc
      = CD3DX12_RESOURCE_DESC::Buffer(sizeof(AlignedSceneConstantBuffer));
    DX::ThrowIfFailed(
      device->CreateCommittedResource(&uploadHeapProperties,
                                      D3D12_HEAP_FLAG_NONE,
                                      &bufferDesc,
                                      D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                                      nullptr,
                                      IID_PPV_ARGS(&GlobalCBV)));
  }

  // Heaps
  {{CSUDescriptors.Setup(device,
                         D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                         D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                         NUM_TEXTURES);
}

{
  SamplerDescriptors.Setup(device,
                           D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                           D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                           2);
}
}

// Set sampler heap values.
{
  D3D12_SAMPLER_DESC samplerDesc = {};
  samplerDesc.Filter             = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU           = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  samplerDesc.AddressV           = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  samplerDesc.AddressW           = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

  device->CreateSampler(&samplerDesc, SamplerDescriptors.GetCpuHandle(0));
}
}

static void SetupHeap() {
  auto device           = GlobalDeviceResources.GetD3DDevice();
  auto commandList      = GlobalDeviceResources.GetCommandList();
  auto commandAllocator = GlobalDeviceResources.GetCommandAllocator();

  // Setup heaps
  {
    ID3D12DescriptorHeap *ppHeaps[]
      = {CSUDescriptors.Heap(), SamplerDescriptors.Heap()};
    commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    commandList->SetGraphicsRootSignature(GlobalRootSignature.Get());
  }

  {
    commandList
      ->SetGraphicsRootConstantBufferView(
		  0,
		  GlobalCBV->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(1,
                                                CSUDescriptors.GetGpuHandle(0));
    commandList->SetGraphicsRootDescriptorTable(2,
                                                SamplerDescriptors.GetGpuHandle(
                                                  0));
  }

  {
    {
      GlobalViewport          = {};
      GlobalViewport.Width    = static_cast<float>(GlobalWidth);
      GlobalViewport.Height   = static_cast<float>(GlobalHeight);
      GlobalViewport.MaxDepth = 1.0f;
    }

    {
      GlobalScissorRect        = {};
      GlobalScissorRect.right
        = static_cast<LONG>(GlobalWidth);
      GlobalScissorRect.bottom = static_cast<LONG>(GlobalHeight);
    }

    commandList->RSSetViewports(1, &GlobalViewport);
    commandList->RSSetScissorRects(1, &GlobalScissorRect);
  }
}

static int SetupTexture(
	Microsoft::WRL::ComPtr<ID3D12Resource> tex,
	int texID) {
  auto device           = GlobalDeviceResources.GetD3DDevice();
  auto commandList      = GlobalDeviceResources.GetCommandList();
  auto commandAllocator = GlobalDeviceResources.GetCommandAllocator();

  // Bind.
  {
    // SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format                  = tex->GetDesc().Format;
    srvDesc.Texture2D.MipLevels     = tex->GetDesc().MipLevels;

	auto id = texID == -1 ? GlobalTexID : texID;
    device->CreateShaderResourceView(tex.Get(),
                                     &srvDesc,
                                     CSUDescriptors.GetCpuHandle(id));

	if(texID != -1) return texID;

	GlobalTexID++;
    return id;
  }
}

static void SetTexture(
	unsigned int index) {
  auto device           = GlobalDeviceResources.GetD3DDevice();
  auto commandList      = GlobalDeviceResources.GetCommandList();
  auto commandAllocator = GlobalDeviceResources.GetCommandAllocator();

  LastTexID = index;

  commandList->SetGraphicsRootDescriptorTable(1,
                                              CSUDescriptors.GetGpuHandle(
                                                index));
}