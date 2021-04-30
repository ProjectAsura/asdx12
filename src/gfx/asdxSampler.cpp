//-----------------------------------------------------------------------------
// File : asdxSampler.cpp
// Desc : Sampler.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxSampler.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Sampler class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Sampler::Sampler()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Sampler::~Sampler()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Sampler::Init
(
    GraphicsDevice&         device,
    SAMPLER_TYPE            type,
    D3D12_COMPARISON_FUNC   compareFunc
)
{
    D3D12_SAMPLER_DESC desc = {};
    desc.MipLODBias         = 0;
    desc.MaxAnisotropy      = 0;
    desc.ComparisonFunc     = compareFunc;
    desc.BorderColor[0]     = 1.0f;
    desc.BorderColor[1]     = 1.0f;
    desc.BorderColor[2]     = 1.0f;
    desc.BorderColor[3]     = 1.0f;
    desc.MinLOD             = 0;
    desc.MaxLOD             = D3D12_FLOAT32_MAX;

    switch(type)
    {
    case ST_POINT_CLAMP:
        {
            desc.Filter     = D3D12_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressV   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressW   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case ST_POINT_WRAP:
        {
            desc.Filter     = D3D12_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressV   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressW   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case ST_POINT_MIRROR:
        {
            desc.Filter     = D3D12_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressV   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressW   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case ST_LINEAR_CLAMP:
        {
            desc.Filter     = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressV   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressW   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case ST_LINEAR_WRAP:
        {
            desc.Filter     = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressV   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressW   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case ST_LINEAR_MIRROR:
        {
            desc.Filter     = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressV   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressW   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case ST_ANISOTROPIC_CLAMP:
        {
            desc.Filter         = D3D12_FILTER_ANISOTROPIC;
            desc.AddressU       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressV       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressW       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.MaxAnisotropy  = 16;
        }
        break;

    case ST_ANISOTROPIC_WRAP:
        {
            desc.Filter         = D3D12_FILTER_ANISOTROPIC;
            desc.AddressU       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressV       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressW       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.MaxAnisotropy  = 16;
        }
        break;

    case ST_ANISOTROPIC_MIRROR:
        {
            desc.Filter         = D3D12_FILTER_ANISOTROPIC;
            desc.AddressU       = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressV       = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressW       = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.MaxAnisotropy  = 16;
        }
        break;
    }

    return Init(device, &desc);
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Sampler::Init(GraphicsDevice& device, const D3D12_SAMPLER_DESC* pDesc)
{
    if (!device.AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, m_Descriptor.GetAddress()))
    {
        ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
        return false;
    }

    device->CreateSampler(pDesc, m_Descriptor->GetHandleCPU());
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Sampler::Term()
{ m_Descriptor.Reset(); }

//-----------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-----------------------------------------------------------------------------
const Descriptor* Sampler::GetDescriptor() const
{ return m_Descriptor.GetPtr(); }

} // namespace asdx
