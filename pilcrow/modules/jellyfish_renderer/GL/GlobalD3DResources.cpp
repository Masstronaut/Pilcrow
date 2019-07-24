#include "GlobalD3DResources.h"
#include "DeviceResources.h"

DX::DeviceResources GlobalDeviceResources(DXGI_FORMAT_R10G10B10A2_UNORM,
                                          DXGI_FORMAT_D32_FLOAT,
                                          2,
                                          D3D_FEATURE_LEVEL_12_0);

Microsoft::WRL::ComPtr<ID3D12RootSignature> GlobalRootSignature;

Microsoft::WRL::ComPtr<ID3D12Resource>             GlobalCBV;

Microsoft::WRL::ComPtr<ID3D12PipelineState> GlobalPipelineState;

DX::DescriptorHeap CSUDescriptors;
DX::DescriptorHeap SamplerDescriptors;

D3D12_VIEWPORT    GlobalViewport;
D3D12_RECT GlobalScissorRect;

unsigned int GlobalWidth;
unsigned int GlobalHeight;

int GlobalTexID = 0;
int LastTexID = 0;

AlignedSceneConstantBuffer *GlobalMappedCBV;