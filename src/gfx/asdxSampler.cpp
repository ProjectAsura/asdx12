//-----------------------------------------------------------------------------
// File : asdxSampler.cpp
// Desc : Sampler State.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxSampler.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxDescriptorHeap.h>
#include <fnd/asdxLogger.h>


namespace asdx {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
const D3D12_SAMPLER_DESC Sampler::PointWrap = {
    D3D12_FILTER_MIN_MAG_MIP_POINT,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    0,
    D3D12_COMPARISON_FUNC_NEVER,
    { 0.0f, 0.0f, 0.0f, 0.0f },
    0.0f,
    D3D12_FLOAT32_MAX
};

const D3D12_SAMPLER_DESC Sampler::PointClamp = {
    D3D12_FILTER_MIN_MAG_MIP_POINT,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    0,
    D3D12_COMPARISON_FUNC_NEVER,
    { 0.0f, 0.0f, 0.0f, 0.0f },
    0.0f,
    D3D12_FLOAT32_MAX
};

const D3D12_SAMPLER_DESC Sampler::PointMirror = {
    D3D12_FILTER_MIN_MAG_MIP_POINT,
    D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
    D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
    D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    0,
    D3D12_COMPARISON_FUNC_NEVER,
    { 0.0f, 0.0f, 0.0f, 0.0f },
    0.0f,
    D3D12_FLOAT32_MAX
};

const D3D12_SAMPLER_DESC Sampler::LinearWrap = {
    D3D12_FILTER_MIN_MAG_MIP_LINEAR,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    0,
    D3D12_COMPARISON_FUNC_NEVER,
    { 0.0f, 0.0f, 0.0f, 0.0f },
    0.0f,
    D3D12_FLOAT32_MAX
};

const D3D12_SAMPLER_DESC Sampler::LinearClamp = {
    D3D12_FILTER_MIN_MAG_MIP_LINEAR,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    0,
    D3D12_COMPARISON_FUNC_NEVER,
    { 0.0f, 0.0f, 0.0f, 0.0f },
    0.0f,
    D3D12_FLOAT32_MAX
};

const D3D12_SAMPLER_DESC Sampler::LinearMirror = {
    D3D12_FILTER_MIN_MAG_MIP_LINEAR,
    D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
    D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
    D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    0,
    D3D12_COMPARISON_FUNC_NEVER,
    { 0.0f, 0.0f, 0.0f, 0.0f },
    0.0f,
    D3D12_FLOAT32_MAX
};

const D3D12_SAMPLER_DESC Sampler::AnisotropicWrap = {
    D3D12_FILTER_ANISOTROPIC,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    16,
    D3D12_COMPARISON_FUNC_NEVER,
    { 0.0f, 0.0f, 0.0f, 0.0f },
    0.0f,
    D3D12_FLOAT32_MAX
};

const D3D12_SAMPLER_DESC Sampler::AnisotropicClamp = {
    D3D12_FILTER_ANISOTROPIC,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    16,
    D3D12_COMPARISON_FUNC_NEVER,
    { 0.0f, 0.0f, 0.0f, 0.0f },
    0.0f,
    D3D12_FLOAT32_MAX
};

const D3D12_SAMPLER_DESC Sampler::AnisotropicMirror = {
    D3D12_FILTER_ANISOTROPIC,
    D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
    D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
    D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    16,
    D3D12_COMPARISON_FUNC_NEVER,
    { 0.0f, 0.0f, 0.0f, 0.0f },
    0.0f,
    D3D12_FLOAT32_MAX
};


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
    assert(pDevice != nullptr);

    m_HandleSampler = GetSamplerDescriptorHeap()->Alloc(1);
    if (!m_HandleSampler.IsValid())
    {
        ELOG("Error : DescriptorHeap::Alloc() Failed.");
        return false;
    }

    pDevice->CreateSampler(pDesc, GetHandleCPU());
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Sampler::Term()
{
    if (m_HandleSampler.IsValid())
    { GetSamplerDescriptorHeap()->Free(m_HandleSampler); }
}

//-----------------------------------------------------------------------------
//      オフセットハンドルを取得します.
//-----------------------------------------------------------------------------
const OffsetHandle& Sampler::GetOffsetHandle() const
{ return m_HandleSampler; }

//-----------------------------------------------------------------------------
//      CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE Sampler::GetHandleCPU() const
{
    D3D12_CPU_DESCRIPTOR_HANDLE result = {};
    if (m_HandleSampler.IsValid())
    { result = GetSamplerDescriptorHeap()->GetHandleCPU(m_HandleSampler); }
    return result;
}

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE Sampler::GetHandleGPU() const
{
    D3D12_GPU_DESCRIPTOR_HANDLE result = {};
    if (m_HandleSampler.IsValid())
    { result = GetSamplerDescriptorHeap()->GetHandleGPU(m_HandleSampler); }
    return result;
}

} // namespace asdx
