//-----------------------------------------------------------------------------
// File : asdxPassGraph.h
// Desc : Pass Graph System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <d3d12.h>
#include <asdxTarget.h>
#include <asdxMath.h>


namespace asdx {

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------
struct IPassGraphBuilder;
struct IPassGraphContext;

using ResourceHandle    = uint32_t;
using PassSetup         = void (*) (IPassGraphBuilder* builder);
using PassExecute       = void (*) (IPassGraphContext* context);

///////////////////////////////////////////////////////////////////////////////
// PASS_RESOURCE_DIMENSION enum
///////////////////////////////////////////////////////////////////////////////
enum PASS_RESOURCE_DIMENSION
{
    PASS_RESOURCE_DIMENSION_BUFFER,
    PASS_RESOURCE_DIMENSION_1D,
    PASS_RESOURCE_DIMENSION_2D,
    PASS_RESOURCE_DIMENSION_3D,
};

///////////////////////////////////////////////////////////////////////////////
// PASS_RESOURCE_STATE enum
///////////////////////////////////////////////////////////////////////////////
enum PASS_RESOURCE_STATE
{
    PASS_RESOURCE_STATE_NONE,
    PASS_RESOURCE_STATE_CLEAR,
};

///////////////////////////////////////////////////////////////////////////////
// PASS_RESOURCE_USAGE enum
///////////////////////////////////////////////////////////////////////////////
enum PASS_RESOURCE_USAGE
{
    PASS_RESOURCE_USAGE_RTV     = 0x1,
    PASS_RESOURCE_USAGE_DSV     = 0x2,
    PASS_RESOURCE_USAGE_UAV     = 0x4,
};

///////////////////////////////////////////////////////////////////////////////
// PassResourceDesc enum
///////////////////////////////////////////////////////////////////////////////
struct PassResourceDesc
{
    PASS_RESOURCE_DIMENSION Dimension           = PASS_RESOURCE_DIMENSION_2D;
    uint64_t                Width               = 1920;
    uint32_t                Height              = 1080;
    uint16_t                DepthOrArraySize    = 1;
    uint16_t                MipLevels           = 1;
    DXGI_FORMAT             Format              = DXGI_FORMAT_UNKNOWN;
    Vector4                 ClearColor          = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    float                   ClearDepth          = 1.0f;
    uint32_t                ClearStencil        = 0;
    uint32_t                Stride              = 1;
    PASS_RESOURCE_STATE     InitState           = PASS_RESOURCE_STATE_NONE;
    uint8_t                 Usage               = PASS_RESOURCE_USAGE_RTV;
};

///////////////////////////////////////////////////////////////////////////////
// PassTag class
///////////////////////////////////////////////////////////////////////////////
class PassTag
{
public:
    PassTag() = default;
    PassTag(const char* tag)       { strcpy_s(m_Text, tag); }
    PassTag(const PassTag& value)  { strcpy_s(m_Text, value.m_Text); }
    const char* c_str() const      { return m_Text; }

private:
    char m_Text[32] = {};
};

///////////////////////////////////////////////////////////////////////////////
// IPassGraphBuilder interface
///////////////////////////////////////////////////////////////////////////////
struct IPassGraphBuilder
{
    virtual ~IPassGraphBuilder() {}
    virtual ResourceHandle Read(ResourceHandle resource, uint32_t flag) = 0;
    virtual ResourceHandle Write(ResourceHandle resource, uint32_t flag) = 0;
    virtual ResourceHandle Create(PassResourceDesc& desc) = 0;
    virtual ResourceHandle Import(ID3D12Resource* resource, D3D12_RESOURCE_STATES state) = 0;
    virtual ResourceHandle Import(ID3D12Resource* resource, Descriptor* descriptor, D3D12_RESOURCE_STATES state) = 0;
    virtual void AsyncComputeEnable(bool value) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// IPassGraphContext interface
///////////////////////////////////////////////////////////////////////////////
struct IPassGraphContext
{
    virtual ~IPassGraphContext() {}
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(ResourceHandle resource) const = 0;
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(ResourceHandle resource) const = 0;
    virtual D3D12_GPU_DESCRIPTOR_HANDLE GetRes(ResourceHandle resource) const = 0;
    virtual D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress(ResourceHandle resource) const = 0;
    virtual D3D12_RESOURCE_DESC GetDesc(ResourceHandle resource) const = 0;
    virtual ID3D12GraphicsCommandList6* GetCommandList() const = 0;
};

///////////////////////////////////////////////////////////////////////////////
// IPassGraph interface
///////////////////////////////////////////////////////////////////////////////
struct IPassGraph
{
    virtual ~IPassGraph() {}
    virtual void Release() = 0;
    virtual bool AddPass(PassTag& tag, PassSetup setup, PassExecute execute) = 0;
    virtual void Compile() = 0;
    virtual void Execute(ID3D12CommandQueue* pGraphics, ID3D12CommandQueue* pCompute) = 0;
};

bool CreatePassGraph(uint32_t maxPassCount);

} // namespace asdx
