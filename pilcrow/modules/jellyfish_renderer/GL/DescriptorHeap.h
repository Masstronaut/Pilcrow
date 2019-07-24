//--------------------------------------------------------------------------------------
// File: DescriptorHeap.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#include "DeviceResources.h"

namespace DX {
// A contiguous linear random-access descriptor heap
class DescriptorHeap {
public:
  void Setup(ID3D12Device *              device,
                 D3D12_DESCRIPTOR_HEAP_TYPE  type,
                 D3D12_DESCRIPTOR_HEAP_FLAGS flags,
                 size_t                      count);

  DescriptorHeap()                  = default;
  DescriptorHeap(DescriptorHeap &&) = default;
  DescriptorHeap &operator=(DescriptorHeap &&) = default;

  DescriptorHeap(const DescriptorHeap &) = delete;
  DescriptorHeap &operator=(const DescriptorHeap &) = delete;

  D3D12_GPU_DESCRIPTOR_HANDLE __cdecl WriteDescriptors(
    ID3D12Device *device,
    uint32_t      offsetIntoHeap,
    uint32_t      totalDescriptorCount,
    _In_reads_(descriptorRangeCount)
      const D3D12_CPU_DESCRIPTOR_HANDLE *            pDescriptorRangeStarts,
    _In_reads_(descriptorRangeCount) const uint32_t *pDescriptorRangeSizes,
    uint32_t                                         descriptorRangeCount);

  D3D12_GPU_DESCRIPTOR_HANDLE __cdecl WriteDescriptors(
    ID3D12Device *device,
    uint32_t      offsetIntoHeap,
    _In_reads_(descriptorRangeCount)
      const D3D12_CPU_DESCRIPTOR_HANDLE *            pDescriptorRangeStarts,
    _In_reads_(descriptorRangeCount) const uint32_t *pDescriptorRangeSizes,
    uint32_t                                         descriptorRangeCount);

  D3D12_GPU_DESCRIPTOR_HANDLE __cdecl WriteDescriptors(
    ID3D12Device *device,
    uint32_t      offsetIntoHeap,
    _In_reads_(descriptorCount) const D3D12_CPU_DESCRIPTOR_HANDLE *pDescriptors,
    uint32_t descriptorCount);

  D3D12_GPU_DESCRIPTOR_HANDLE GetFirstGpuHandle() const {
    assert(m_desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    assert(m_pHeap != nullptr);
    return m_hGPU;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE GetFirstCpuHandle() const {
    assert(m_pHeap != nullptr);
    return m_hCPU;
  }

  D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(size_t index) const {
    assert(m_pHeap != nullptr);
    if(index >= m_desc.NumDescriptors) {
      throw std::out_of_range("D3DX12_GPU_DESCRIPTOR_HANDLE");
    }
    assert(m_desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    D3D12_GPU_DESCRIPTOR_HANDLE handle;
    handle.ptr = m_hGPU.ptr + UINT64(index) * UINT64(m_increment);
    return handle;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(size_t index) const {
    assert(m_pHeap != nullptr);
    if(index >= m_desc.NumDescriptors) {
      throw std::out_of_range("D3DX12_CPU_DESCRIPTOR_HANDLE");
    }

    D3D12_CPU_DESCRIPTOR_HANDLE handle;
    handle.ptr = m_hCPU.ptr + UINT64(index) * UINT64(m_increment);
    return handle;
  }

  size_t                     Count() const { return m_desc.NumDescriptors; }
  unsigned int               Flags() const { return m_desc.Flags; }
  D3D12_DESCRIPTOR_HEAP_TYPE Type() const { return m_desc.Type; }
  size_t                     Increment() const { return m_increment; }
  ID3D12DescriptorHeap *     Heap() const { return m_pHeap.Get(); }

  static void __cdecl DefaultDesc(D3D12_DESCRIPTOR_HEAP_TYPE  type,
                                  D3D12_DESCRIPTOR_HEAP_DESC *pDesc);

private:
  void __cdecl Create(ID3D12Device *                    pDevice,
                      const D3D12_DESCRIPTOR_HEAP_DESC *pDesc);

  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pHeap;
  D3D12_DESCRIPTOR_HEAP_DESC                   m_desc;
  D3D12_CPU_DESCRIPTOR_HANDLE                  m_hCPU;
  D3D12_GPU_DESCRIPTOR_HANDLE                  m_hGPU;
  uint32_t                                     m_increment;
};
}  // namespace Lightmass