//-----------------------------------------------------------------------------
// File : asdxMaterial.cpp
// Desc : Material Object.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <string>
#include <map>
#include <fnd/asdxLogger.h>
#include <gfx/asdxMaterial.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxTextureManager.h>


namespace {

///////////////////////////////////////////////////////////////////////////////
// ParamDef structure
///////////////////////////////////////////////////////////////////////////////
struct ParamDef
{
    std::string     Name;
    uint32_t        Offset;
    float           Default;
};

///////////////////////////////////////////////////////////////////////////////
// TextureDef structure
///////////////////////////////////////////////////////////////////////////////
struct TextureDef
{
    std::string     Name;
    uint32_t        Index;
    std::string     Default;
};

///////////////////////////////////////////////////////////////////////////////
// KindDef structure
///////////////////////////////////////////////////////////////////////////////
struct KindDef
{
    uint32_t                BufferSize;
    std::vector<ParamDef>   Params;
    std::vector<TextureDef> Textures;
};

//-----------------------------------------------------------------------------
// Global Variables.
//-----------------------------------------------------------------------------
std::map<uint32_t, KindDef> g_KindDefs;

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MaterialSchema class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool MaterialSchema::Init(std::span<KindDesc> descs)
{
    if (descs.empty())
        return false;

    for(auto& desc : descs)
    {
        KindDef def = {};
        def.BufferSize = desc.BufferSize;
        def.Params.resize(desc.Params.size());
        for(auto i=0u; i<desc.Params.size(); ++i)
        {
            def.Params[i].Name    = desc.Params[i].Name;
            def.Params[i].Offset  = desc.Params[i].Offset;
            def.Params[i].Default = desc.Params[i].Default;
        }

        def.Textures.resize(desc.Textures.size());
        for(auto i=0u; i<desc.Textures.size(); ++i)
        {
            def.Textures[i].Name    = desc.Textures[i].Name;
            def.Textures[i].Index   = desc.Textures[i].Index;
            def.Textures[i].Default = desc.Textures[i].Default;
        }

        g_KindDefs[desc.Kind] = def;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MaterialSchema::Term()
{ g_KindDefs.clear(); }

//-----------------------------------------------------------------------------
//      初期化済みかどうかチェックします.
//-----------------------------------------------------------------------------
bool MaterialSchema::IsInit()
{ return !g_KindDefs.empty(); }


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
bool Material::Init(const MaterialBinary& binary)
{
    m_Kind              = binary.GetKind();
    m_BlendState        = binary.GetBlendState();
    m_DepthState        = binary.GetDepthState();
    m_RasterizerState   = binary.GetRasterizerState();

    auto itr = g_KindDefs.find(m_Kind);
    if (itr == g_KindDefs.end())
    {
        ELOG("Error : Invalid Kind.");
        return false;
    }

    const auto& desc = itr->second;

    if (desc.BufferSize > 0 && !desc.Params.empty())
    {
        if (!m_Buffer.Init(desc.BufferSize))
        {
            ELOG("Error : ConstantBuffer::Init() Failed.");
            return false;
        }

        auto ptr = m_Buffer.MapAs<uint8_t>();
        for(auto& param : desc.Params)
        {
            MaterialParameter info = {};
            if (!binary.FindParameter(param.Name.c_str(), info))
                memcpy(ptr + param.Offset, &info.Value, sizeof(float));
            else
                memcpy(ptr + param.Offset, &param.Default, sizeof(float));
        }
        m_Buffer.Unmap();
    }

    if (!desc.Textures.empty())
    {
        m_Textures.resize(desc.Textures.size());
        for(auto& texture : desc.Textures)
        {
            MaterialTexture info = {};
            if (!binary.FindTexture(texture.Name.c_str(), info))
                m_Textures[texture.Index] = TextureManager::Instance().GetOrCreate(info.Path.c_str());
            else
                m_Textures[texture.Index] = TextureManager::Instance().GetOrCreate(texture.Default.c_str());
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
bool Material::Create(const MaterialBinary& binary, Material** ppMaterial)
{
    if (!MaterialSchema::IsInit())
    {
        ELOG("Error : MaterialInitailizer not initialized.");
        return false;
    }

    // インスタンス生成.
    auto instance = new(std::nothrow) Material();
    if (instance == nullptr)
    {
        ELOG("Error : Out of Memory.");
        return false;
    }

    // インスタンス初期化.
    if (!instance->Init(binary))
    {
        ELOG("Error : Material::Init() Failed.");
        instance->Release();
        instance = nullptr;
        return false;
    }

    // インスタンスを可能.
    (*ppMaterial) = instance;

    // 正常終了.
    return true;
}

} // namespace asdx
