//-----------------------------------------------------------------------------
// File : asdxGraphicsDevice.h
// Desc : Graphics Device.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d12.h>
#include <dxgi1_6.h>
#include <res/asdxResTexture.h>


namespace asdx {

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------
class Descriptor;
class CommandQueue;

///////////////////////////////////////////////////////////////////////////////
// DeviceDesc structure
///////////////////////////////////////////////////////////////////////////////
struct DeviceDesc
{
    uint32_t    MaxShaderResourceCount;         //!< 最大シェーダリソース数です.
    uint32_t    MaxSamplerCount;                //!< 最大サンプラー数です.
    uint32_t    MaxColorTargetCount;            //!< 最大カラーターゲット数です.
    uint32_t    MaxDepthTargetCount;            //!< 最大深度ターゲット数です.
    bool        EnableDebug          = false;   //!< デバッグモードを有効にします.
    bool        EnableDRED           = true;    //!< DREDを有効にします
    bool        EnableCapture        = false;   //!< PIXキャプチャーを有効にします.
    bool        EnableBreakOnWarning = false;   //!< 警告時にブレークするなら true.
    bool        EnableBreakOnError   = true;    //!< エラー時にブレークするなら true.
};

bool SystemInit(const DeviceDesc& desc);
void SystemTerm();
void SystemWaitIdle();
void FrameSync();
void Dispose(ID3D12Resource*& pResource);
void Dispose(Descriptor*& pDescriptor);
void Dispose(ID3D12PipelineState*& pPipelineState);
void ClearDisposer();
void SetDescriptorHeaps(ID3D12GraphicsCommandList* pCmdList);
CommandQueue* GetGraphicsQueue();
CommandQueue* GetComputeQueue();
CommandQueue* GetCopyQueue();
CommandQueue* GetVideoProcessQueue();
CommandQueue* GetVideoEncodeQueue();
CommandQueue* GetVideoDecodeQueue();
bool AllocDescriptor(uint8_t heapType, Descriptor** ppResult); 
ID3D12Device8* GetD3D12Device();
IDXGIFactory7* GetDXGIFactory();

} // namespace asdx
