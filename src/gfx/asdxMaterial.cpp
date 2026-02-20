//-----------------------------------------------------------------------------
// File : asdxMaterial.cpp
// Desc : Material.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <gfx/asdxMaterial.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxTextureManager.h>
#include <fnd/asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Material class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Material::Material()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Material::~Material()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Material::Init(std::vector<uint8_t>&& binary)
{
    MaterialBinary matBinary;
    matBinary.Load(std::move(binary));

    m_Kind              = matBinary.GetKind();
    m_BlendState        = matBinary.GetBlendState();
    m_DepthState        = matBinary.GetDepthState();
    m_RasterizerState   = matBinary.GetRasterizerState();

    auto buf = matBinary.GetBuffer();

    if (buf.Size > 0)
    {
        if (!m_Buffer.Init(buf.Size))
        { return false; }

        auto ptr = m_Buffer.Map();
        memcpy(ptr, buf.pData, buf.Size);
    }

    auto textureCount = matBinary.GetTextureCount();
    if (textureCount > 0)
    {
        m_Textures.resize(textureCount);
        for(auto i=0u; i<textureCount; ++i)
        {
            auto tex = matBinary.GetTexture(i);
            m_Textures[i] = TextureManager::Instance().GetOrCreate(tex.Path.c_str());
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Material::Term()
{
    m_Buffer.Term();

    for(size_t i=0; i<m_Textures.size(); ++i)
    { m_Textures[i].Reset(); }

    m_Textures.clear();
    m_Textures.shrink_to_fit();

    m_Kind            = 0;
    m_BlendState      = MaterialBlendState::Opaque;
    m_DepthState      = MaterialDepthState::ReadWrite;
    m_RasterizerState = MaterialRasterizerState::CullNone;
}

//-----------------------------------------------------------------------------
//      参照カウントを増やします.
//-----------------------------------------------------------------------------
void Material::AddRef()
{ m_RefCount++; }

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void Material::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-----------------------------------------------------------------------------
//      参照カウントを取得します.
//-----------------------------------------------------------------------------
uint32_t Material::GetRefCount() const
{ return m_RefCount; }

//-----------------------------------------------------------------------------
//      マテリアル種別を取得します.
//-----------------------------------------------------------------------------
uint32_t Material::GetKind() const
{ return m_Kind; }

//-----------------------------------------------------------------------------
//      ブレンドステートを取得します.
//-----------------------------------------------------------------------------
MaterialBlendState Material::GetBlendState() const
{ return m_BlendState; }

//-----------------------------------------------------------------------------
//      深度ステートを取得します.
//-----------------------------------------------------------------------------
MaterialDepthState Material::GetDepthState() const
{ return m_DepthState; }

//-----------------------------------------------------------------------------
//      ラスタライザーステートを取得します.
//-----------------------------------------------------------------------------
MaterialRasterizerState Material::GetRasterizerState() const
{ return m_RasterizerState; }

//-----------------------------------------------------------------------------
//      定数バッファを取得します.
//-----------------------------------------------------------------------------
ConstantBuffer& Material::GetBuffer()
{ return m_Buffer; }

//-----------------------------------------------------------------------------
//      定数バッファを取得します.
//-----------------------------------------------------------------------------
const ConstantBuffer& Material::GetBuffer() const
{ return m_Buffer; }

//-----------------------------------------------------------------------------
//      定数バッファのGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Material::GetGpuAddress() const
{ return m_Buffer.GetGpuAddress(); }

//-----------------------------------------------------------------------------
//      テクスチャ数を取得します.
//-----------------------------------------------------------------------------
uint32_t Material::GetTextureCount() const
{ return uint32_t(m_Textures.size()); }

//-----------------------------------------------------------------------------
//      CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE Material::GetHandleCPU(uint32_t index) const
{
    assert(index < m_Textures.size());
    if (!m_Textures[index].IsValid())
        return {};
    return m_Textures[index].GetHandleCPU();
}

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE Material::GetHandleGPU(uint32_t index) const
{
    assert(index < m_Textures.size());
    if (!m_Textures[index].IsValid())
        return {};
    return m_Textures[index].GetHandleGPU();
}

//-----------------------------------------------------------------------------
//      バインドレスインデックスを取得します.
//-----------------------------------------------------------------------------
uint32_t Material::GetBindlessIndex(uint32_t index) const
{
    assert(index < m_Textures.size());
    if (!m_Textures[index].IsValid())
        return UINT32_MAX;
    return m_Textures[index].GetBindlessIndex();
}

//-----------------------------------------------------------------------------
//      生成処理を行います.
//-----------------------------------------------------------------------------
bool Material::Create(std::vector<uint8_t>&& binary, Material** ppMaterial)
{
    auto instance = new(std::nothrow) Material();
    if (instance == nullptr)
    {
        ELOG("Error : Out of Memory");
        return false;
    }

    if (!instance->Init(std::move(binary)))
    {
        ELOG("Error : Material::Init() Failed.");
        instance->Release();
        instance = nullptr;
        return false;
    }

    (*ppMaterial) = instance;
    return true;
}

} // namespace asdx
