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
#include <asdxCommandQueue.h>


namespace asdx {

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------
struct IPassGraphBuilder;
struct IPassGraphContext;
class  PassResource;

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
// CLEAR_TYPE
///////////////////////////////////////////////////////////////////////////////
enum CLEAR_TYPE
{
    CLEAR_TYPE_RTV,
    CLEAR_TYPE_DSV,
    CLEAR_TYPE_UAV_FLOAT,
    CLEAR_TYPE_UAV_UINT
};

////////////////////////////////////////////////////////////////////////////////
// ClearValue structure
///////////////////////////////////////////////////////////////////////////////
struct ClearValue
{
    CLEAR_TYPE Type;
    union
    {
        float Color[4];
        struct 
        {
            float   Depth;
            uint8_t Stencil;
        };
        float    Float[4];
        uint32_t Uint[4];
    };
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
    ClearValue              ClearValue          = {};
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
    virtual void AsyncComputeEnable(bool value) = 0;
    virtual PassResource* Read(PassResource* resource) = 0;
    virtual PassResource* Write(PassResource* resource) = 0;
    virtual PassResource* Create(PassResourceDesc& desc) = 0;

    virtual PassResource* Import(
        ID3D12Resource*         resource,
        D3D12_RESOURCE_STATES   state,
        bool                    uav,
        Descriptor*             pDescriptorRes,
        Descriptor**            pDescriptorRTVs,
        Descriptor**            pDescriptorDSVs) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// IPassGraphContext interface
///////////////////////////////////////////////////////////////////////////////
struct IPassGraphContext
{
    virtual ~IPassGraphContext() {}
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(PassResource* resource, uint16_t index = 0) const = 0;
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(PassResource* resource, uint16_t index = 0) const = 0;
    virtual D3D12_GPU_DESCRIPTOR_HANDLE GetRes(PassResource* resource) const = 0;
    virtual D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress(PassResource* resource) const = 0;
    virtual D3D12_RESOURCE_DESC GetDesc(PassResource* resource) const = 0;
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
    virtual WaitPoint Execute(const WaitPoint& value) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// PassGraphDesc structure
///////////////////////////////////////////////////////////////////////////////
struct PassGraphDesc
{
    uint32_t        MaxPassCount;
    uint32_t        MaxResourceCount;
    uint8_t         MaxThreadCount;
    CommandQueue*   pGraphicsQueue;
    CommandQueue*   pComputeQueue;
};

bool CreatePassGraph(const PassGraphDesc& desc, IPassGraph** ppGraph);

} // namespace asdx
