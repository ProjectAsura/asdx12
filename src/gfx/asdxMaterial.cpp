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
#include <gfx/asdxTexture.h>
#include <gfx/asdxDevice.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Material class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Material::Material()
: m_ConstantBuffer  (nullptr)
, m_TextureCount    (0)
, m_Textures        ()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Material::~Material()
{ Reset(); }

//-----------------------------------------------------------------------------
//      リセットします.
//-----------------------------------------------------------------------------
void Material::Reset()
{
    auto resource = m_ConstantBuffer.Detach();
    Dispose(resource);

    for(auto i=0; i<m_Textures.size(); ++i)
    { m_Textures[i] = nullptr; }
}

//-----------------------------------------------------------------------------
//      定数バッファを設定します.
//-----------------------------------------------------------------------------
void Material::SetConstantBuffer(ID3D12Resource* pResource)
{ m_ConstantBuffer = pResource; }

//-----------------------------------------------------------------------------
//      テクスチャ数を設定します.
//-----------------------------------------------------------------------------
void Material::SetTextureCount(uint32_t count)
{
    assert(count < kMaxTextureCount);
    m_TextureCount = count;
}

//-----------------------------------------------------------------------------
//      テクスチャを設定します.
//-----------------------------------------------------------------------------
void Material::SetTexture(uint32_t index, const Texture* pTexture)
{
    assert(index < m_TextureCount);
    m_Textures[index] = pTexture;
}

//-----------------------------------------------------------------------------
//      定数バッファを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* Material::GetConstantBuffer() const
{ return m_ConstantBuffer.GetPtr(); }

//-----------------------------------------------------------------------------
//      定数バッファのGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Material::GetGpuAddress() const
{
    D3D12_GPU_VIRTUAL_ADDRESS result = {};
    if (m_ConstantBuffer.GetPtr() != nullptr)
    { result = m_ConstantBuffer->GetGPUVirtualAddress(); }
    return result;
}

//-----------------------------------------------------------------------------
//      テクスチャ数を取得します.
//-----------------------------------------------------------------------------
uint32_t Material::GetTextureCount() const
{ return m_TextureCount; }

//-----------------------------------------------------------------------------
//      テクスチャを取得します.
//-----------------------------------------------------------------------------
const Texture* Material::GetTexture(uint32_t index) const
{
    assert(index < m_TextureCount);
    return m_Textures[index];
}

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE Material::GetGpuHandleSRV(uint32_t index) const
{
    assert(index < m_TextureCount);
    return m_Textures[index]->GetGpuHandleSRV();
}

//-----------------------------------------------------------------------------
//      バインドレスインデックスを取得します.
//-----------------------------------------------------------------------------
uint32_t Material::GetBindlessIndexSRV(uint32_t index) const
{
    assert(index < m_TextureCount);
    return m_Textures[index]->GetBindlessIndexSRV();
}

} // namespace asdx
