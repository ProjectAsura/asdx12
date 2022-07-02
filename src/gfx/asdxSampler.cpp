//-----------------------------------------------------------------------------
// File : asdxSampler.cpp
// Desc : Sampler.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxSampler.h>
#include <gfx/asdxGraphicsSystem.h>
#include <fnd/asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// STATIC_SAMPLER_DESC structure
///////////////////////////////////////////////////////////////////////////////
STATIC_SAMPLER_DESC::STATIC_SAMPLER_DESC
(
    SAMPLER_TYPE            type,
    D3D12_SHADER_VISIBILITY visibility,
    UINT                    baseRegister,
    UINT                    registerSpace
)
{
    MipLODBias         = 0;
    MaxAnisotropy      = 0;
    ComparisonFunc     = D3D12_COMPARISON_FUNC_ALWAYS;
    BorderColor        = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    MinLOD             = 0;
    MaxLOD             = D3D12_FLOAT32_MAX;
    ShaderRegister     = baseRegister;
    RegisterSpace      = registerSpace;
    ShaderVisibility   = visibility;

    switch(type)
    {
    case SAMPLER_POINT_CLAMP:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case SAMPLER_POINT_WRAP:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case SAMPLER_POINT_MIRROR:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case SAMPLER_LINEAR_CLAMP:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case SAMPLER_LINEAR_WRAP:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case SAMPLER_LINEAR_MIRROR:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case SAMPLER_ANISOTROPIC_CLAMP:
        {
            Filter = D3D12_FILTER_ANISOTROPIC;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            MaxAnisotropy = 16;
        }
        break;

    case SAMPLER_ANISOTROPIC_WRAP:
        {
            Filter = D3D12_FILTER_ANISOTROPIC;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            MaxAnisotropy = 16;
        }
        break;

    case SAMPLER_ANISOTROPIC_MIRROR:
        {
            Filter = D3D12_FILTER_ANISOTROPIC;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            MaxAnisotropy = 16;
        }
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// SAMPLER_DESC structure
///////////////////////////////////////////////////////////////////////////////
SAMPLER_DESC::SAMPLER_DESC(SAMPLER_TYPE type)
{
    MipLODBias         = 0;
    MaxAnisotropy      = 0;
    ComparisonFunc     = D3D12_COMPARISON_FUNC_ALWAYS;
    BorderColor[0]     = 0.0f;
    BorderColor[1]     = 0.0f;
    BorderColor[2]     = 0.0f;
    BorderColor[3]     = 1.0f;
    MinLOD             = 0;
    MaxLOD             = D3D12_FLOAT32_MAX;

    switch(type)
    {
    case SAMPLER_POINT_CLAMP:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case SAMPLER_POINT_WRAP:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case SAMPLER_POINT_MIRROR:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case SAMPLER_LINEAR_CLAMP:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case SAMPLER_LINEAR_WRAP:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case SAMPLER_LINEAR_MIRROR:
        {
            Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case SAMPLER_ANISOTROPIC_CLAMP:
        {
            Filter = D3D12_FILTER_ANISOTROPIC;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            MaxAnisotropy = 16;
        }
        break;

    case SAMPLER_ANISOTROPIC_WRAP:
        {
            Filter = D3D12_FILTER_ANISOTROPIC;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            MaxAnisotropy = 16;
        }
        break;

    case SAMPLER_ANISOTROPIC_MIRROR:
        {
            Filter = D3D12_FILTER_ANISOTROPIC;
            AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            MaxAnisotropy = 16;
        }
        break;
    }
}

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
bool Sampler::Init(const D3D12_SAMPLER_DESC* pDesc)
{
    auto pDevice = GetD3D12Device();
    if (pDevice == nullptr || pDesc == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    if (!AllocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, m_Descriptor.GetAddress()))
    {
        ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
        return false;
    }

    pDevice->CreateSampler(pDesc, m_Descriptor->GetHandleCPU());
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Sampler::Term()
{
    auto descriptor = m_Descriptor.Detach();
    Dispose(descriptor);
}

//-----------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-----------------------------------------------------------------------------
const Descriptor* Sampler::GetDescriptor() const
{ return m_Descriptor.GetPtr(); }

} // namespace asdx
