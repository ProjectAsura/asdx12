//-----------------------------------------------------------------------------
// File : asdxRootSignature.cpp
// Desc : Root Signature.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <gfx/asdxRootSignature.h>
#include <core/asdxLogger.h>


namespace asdx {

RANGE_CBV::RANGE_CBV(UINT baseRegister, UINT registerSpace)
{
    RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    NumDescriptors                      = 1;
    BaseShaderRegister                  = baseRegister;
    RegisterSpace                       = registerSpace;
    OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

RANGE_SRV::RANGE_SRV(UINT baseRegister, UINT registerSpace)
{
    RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    NumDescriptors                      = 1;
    BaseShaderRegister                  = baseRegister;
    RegisterSpace                       = registerSpace;
    OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

RANGE_UAV::RANGE_UAV(UINT baseRegister, UINT registerSpace)
{
    RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    NumDescriptors                      = 1;
    BaseShaderRegister                  = baseRegister;
    RegisterSpace                       = registerSpace;
    OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

RANGE_SMP::RANGE_SMP(UINT baseRegister, UINT registerSpace)
{
    RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    NumDescriptors                      = 1;
    BaseShaderRegister                  = baseRegister;
    RegisterSpace                       = registerSpace;
    OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}


///////////////////////////////////////////////////////////////////////////////
// RootSignature class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
RootSignature::RootSignature()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
RootSignature::~RootSignature()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool RootSignature::Init(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC* pDesc)
{
    asdx::RefPtr<ID3DBlob> pBlob;
    asdx::RefPtr<ID3DBlob> pErrorBlob;

    auto hr = D3D12SerializeRootSignature(
        pDesc,
        D3D_ROOT_SIGNATURE_VERSION_1_0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());
    if (FAILED(hr))
    {
        const char* msg = "";
        if (pErrorBlob != nullptr)
        { msg = static_cast<const char*>(pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x, msg = %s", hr, msg);
        return false;
    }

    hr = pDevice->CreateRootSignature(
        0,
        pBlob->GetBufferPointer(),
        pBlob->GetBufferSize(),
        IID_PPV_ARGS(m_RootSignature.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void RootSignature::Term()
{ m_RootSignature.Reset(); }

//-----------------------------------------------------------------------------
//      ルートシグニチャを取得します.
//-----------------------------------------------------------------------------
ID3D12RootSignature* RootSignature::GetPtr() const
{ return m_RootSignature.GetPtr(); }

} // namespace asdx
