//----------------------------------------------------------------------------
// File : asdxTextureManager.cpp
// Desc : Texture Manager.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxHash.h>
#include <fnd/asdxPath.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxFileIO.h>
#include <res/asdxResTexture.h>
#include <gfx/asdxTexture.h>
#include <gfx/asdxTextureManager.h>
#include <gfx/asdxDevice.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// TextureHolder class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
TextureHolder::TextureHolder(Texture* pTexture, uint64_t hash)
: m_pTexture(pTexture)
, m_Hash    (hash)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void TextureHolder::Reset()
{ TextureManager::Instance().Remove(*this); }

//-----------------------------------------------------------------------------
//      有効かどうかチェックします.
//-----------------------------------------------------------------------------
bool TextureHolder::IsValid() const
{ return (m_pTexture != nullptr) && (m_Hash != 0); }

//-----------------------------------------------------------------------------
//      リソース設定を取得します.
//-----------------------------------------------------------------------------
D3D12_RESOURCE_DESC TextureHolder::GetDesc() const
{
    assert(m_pTexture != nullptr);
    return m_pTexture->GetDesc();
}

//-----------------------------------------------------------------------------
//      バインドレスインデックスを取得します.
//-----------------------------------------------------------------------------
uint32_t TextureHolder::GetBindlessIndex() const
{
    assert(m_pTexture != nullptr);
    return m_pTexture->GetBindlessIndex();
}

//-----------------------------------------------------------------------------
//      CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE TextureHolder::GetHandleCPU() const
{
    assert(m_pTexture != nullptr);
    return m_pTexture->GetHandleCPU();
}

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE TextureHolder::GetHandleGPU() const
{
    assert(m_pTexture != nullptr);
    return m_pTexture->GetHandleGPU();
}


///////////////////////////////////////////////////////////////////////////////
// TextureManager class
///////////////////////////////////////////////////////////////////////////////
TextureManager TextureManager::s_Instance = {};

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
TextureManager& TextureManager::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
TextureManager::TextureManager()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
TextureManager::~TextureManager()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool TextureManager::Init()
{
    // 初期化済み.
    if (m_Initialized)
        return true;

    ScopedLock<SpinLock> locker(m_SpinLock);

    // デバイスを取得します.
    auto pDevice = GetD3D12Device();

    for(auto i=0; i<2; ++i)
    {
        auto type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        // コマンドアロケータを生成.
        auto hr = pDevice->CreateCommandAllocator(
            type, IID_PPV_ARGS(m_CmdAllocator[i].GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommandAllocator() Failed. errcode = 0x%x", hr);
            return false;
        }

        // コマンドリスト生成.
        hr = pDevice->CreateCommandList(
            0, type, m_CmdAllocator[i].GetPtr(), nullptr, IID_PPV_ARGS(m_CmdList[i].GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommandList() Failed. errcode = 0x%x", hr);
            return false;
        }

        // いったん閉じておく.
        m_CmdList[i]->Close();
    }

    // バッファ番号初期化.
    m_BufferIndex = 0;

    // 初期化済みフラグを立てる.
    m_Initialized = true;

    // コマンドリストの記録を開始しておく.
    m_CmdList[m_BufferIndex]->Reset(m_CmdAllocator[m_BufferIndex].GetPtr(), nullptr);

    // デフォルトテクスチャを生成.
    CreateDefaultTextures();

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void TextureManager::Term()
{
    if (!m_Initialized)
        return;

    ScopedLock<SpinLock> locker(m_SpinLock);

    for(auto i=0; i<2; ++i)
    {
        // コマンドリスト破棄.
        {
            auto item = m_CmdList[i].Detach();
            if (item)
                Dispose(item);
        }

        // コマンドアロケータ破棄.
        {
            auto item = m_CmdAllocator[i].Detach();
            if (item)
                Dispose(item);
        }
    }

    // テクスチャ破棄.
    for(auto& itr : m_Textures)
    {
        auto item = itr.second;
        itr.second = nullptr;
        SafeRelease(item);
    }
    m_Textures.clear();

    m_BufferIndex = 0;

    // 初期化済みフラグを下す.
    m_Initialized = false;
}

//-----------------------------------------------------------------------------
//      生成または取得処理を行います.
//-----------------------------------------------------------------------------
TextureHolder TextureManager::GetOrCreate(const char* fullPath)
{
    // ファイルパスがnullかどうかチェック.
    if (fullPath == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return TextureHolder();
    }

    // ハッシュ値を計算.
    auto hash = CalcHash(fullPath);

    // 辞書の中から探し出す.
    ScopedLock<SpinLock> locker(m_SpinLock);
    auto itr = m_Textures.find(hash);
    if (itr != m_Textures.end())
    {
        auto pTexture = itr->second; // 見つかった場合はポインタを返却.
        pTexture->AddRef(); // 参照カウントを上げる.
        return TextureHolder(pTexture, hash);
    }

    // テクスチャバイナリをロードする.
    std::vector<uint8_t> blob;
    if (!LoadA(fullPath, blob))
    {
        ELOGA("Error : File Load Failed. path = %s", fullPath);
        return TextureHolder();
    }

    // テクスチャバイナリ取得.
    TextureBinary binary;
    binary.Load(std::move(blob));

    // テクスチャ初期化.
    auto resource = binary.GetResource();
    Texture* pTexture = nullptr;
    if (!CreateTexture(resource, &pTexture))
    {
        ELOGA("Error : Texture Init failed. path = %s", fullPath);
        return TextureHolder();
    }

    // テクスチャ登録.
    m_Textures[hash] = pTexture;

    // 生成したテクスチャを返却する.
    return TextureHolder(pTexture, hash);
}

//-----------------------------------------------------------------------------
//      削除処理.
//-----------------------------------------------------------------------------
void TextureManager::Remove(TextureHolder& holder)
{
    // 無効なら即終了.
    if (!holder.IsValid())
        return;

    if (holder.m_pTexture->GetRefCount() > 1)
    {
        // 参照カウンタを下げる.
        holder.m_pTexture->Release();
    }
    else
    {
        // 管理対象からも外す.
        ScopedLock<SpinLock> locker(m_SpinLock);

        // 削除処理.
        auto itr = m_Textures.find(holder.m_Hash);
        if (itr != m_Textures.end())
        {
            auto item = itr->second;
            itr->second = nullptr;
            m_Textures.erase(holder.m_Hash);
            SafeRelease(item);
        }
    }

    // クリア処理.
    holder.m_pTexture = nullptr;
    holder.m_Hash     = 0;
}

//-----------------------------------------------------------------------------
//      コマンドリストを入れ替えます.
//-----------------------------------------------------------------------------
ID3D12GraphicsCommandList* TextureManager::Swap()
{
    ScopedLock<SpinLock> locker(m_SpinLock);

    // 返却するコマンドリストを取得.
    auto pCmdList = m_CmdList[m_BufferIndex].GetPtr();

    // コマンドを閉じる.
    pCmdList->Close();

    // バッファ入れ替え.
    m_BufferIndex = (m_BufferIndex + 1) & 0x1;

    // 次に使うコマンドリストをリセットしておく.
    m_CmdList[m_BufferIndex]->Reset(m_CmdAllocator[m_BufferIndex].GetPtr(), nullptr);

    m_HasCommand = false;

    // コマンドが積まれているコマンドリストを返却.
    return pCmdList;
}

//-----------------------------------------------------------------------------
//      更新コマンドを持つかどうかチェックします.
//-----------------------------------------------------------------------------
bool TextureManager::HasCommand() const
{ return m_HasCommand; }

//-----------------------------------------------------------------------------
//      デフォルトテクスチャを生成します.
//-----------------------------------------------------------------------------
void TextureManager::CreateDefaultTextures()
{
    // Base Color Map.
    {
        std::vector<uint8_t> pixels;
        pixels.resize(32 * 32* 4);

        auto idx = 0u;
        for(auto i=0; i<32; ++i)
        {
            for(auto j=0; j<32; ++j)
            {
                // 黒
                if ((i < 16 && j < 16) || (i >= 16 && j >= 16))
                {
                    pixels[idx + 0] = 64;
                    pixels[idx + 1] = 64;
                    pixels[idx + 2] = 64;
                    pixels[idx + 3] = 255;
                }
                // 白.
                else
                {
                    pixels[idx + 0] = 128;
                    pixels[idx + 1] = 128;
                    pixels[idx + 2] = 128;
                    pixels[idx + 3] = 255;
                }
                idx += 4;
            }
        }

        ResSubResource subRes = {};
        subRes.Width       = 32;
        subRes.Height      = 32;
        subRes.RowPitch    = 32 * 4;
        subRes.SlicePitch  = 32 * 32 * 4;
        subRes.PixelOffset = 0;

        ResTexture res = {};
        res.Dimension           = TEXTURE_DIMENSION_2D;
        res.Width               = 32;
        res.Height              = 32;
        res.DepthOrArraySize    = 1;
        res.Format              = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        res.MipLevels           = 1;
        res.SubResources        = ArrayView(&subRes, 1);
        res.Pixels              = ArrayView(pixels.data(), pixels.size());

        Texture* pTexture = nullptr;
        if (CreateTexture(res, &pTexture))
        {
            auto hash = CalcHash("default.BaseColor");
            pTexture->SetName(L"default.BaseColor");

            // テクスチャ登録.
            m_Textures[hash] = pTexture;
        }
    }

    // Normal Map.
    {
        std::vector<uint8_t> pixels;
        pixels.resize(32 * 32* 4);

        auto idx = 0u;
        for(auto i=0; i<32; ++i)
        {
            for(auto j=0; j<32; ++j)
            {
                {
                    pixels[idx + 0] = 128;
                    pixels[idx + 1] = 128;
                    pixels[idx + 2] = 255;
                    pixels[idx + 3] = 255;
                }
                idx += 4;
            }
        }

        ResSubResource subRes = {};
        subRes.Width        = 32;
        subRes.Height       = 32;
        subRes.RowPitch     = 32 * 4;
        subRes.SlicePitch   = 32 * 32 * 4;
        subRes.PixelOffset  = 0;

        ResTexture res = {};
        res.Dimension           = TEXTURE_DIMENSION_2D;
        res.Width               = 32;
        res.Height              = 32;
        res.DepthOrArraySize    = 1;
        res.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
        res.MipLevels           = 1;
        res.SubResources        = ArrayView(&subRes, 1);
        res.Pixels              = ArrayView(pixels.data(), pixels.size());

        Texture* pTexture = nullptr;
        if (CreateTexture(res, &pTexture))
        {
            auto hash = CalcHash("default.Normal");
            pTexture->SetName(L"default.Normal");

            // テクスチャ登録.
            m_Textures[hash] = pTexture;
        }
    }

    // Occlusion/Roughness/Metalness
    {
        std::vector<uint8_t> pixels;
        pixels.resize(32 * 32* 4);

        auto idx = 0;
        for(auto i=0; i<32; ++i)
        {
            for(auto j=0; j<32; ++j)
            {
                pixels[idx + 0] = 255;     // Occlusion
                pixels[idx + 1] = 128;     // Roughness
                pixels[idx + 2] = 0;       // Metallic
                pixels[idx + 3] = 255;
                idx += 4;
            }
        }

        ResSubResource subRes = {};
        subRes.Width        = 32;
        subRes.Height       = 32;
        subRes.RowPitch     = 32 * 4;
        subRes.SlicePitch   = 32 * 32 * 4;
        subRes.PixelOffset  = 0;

        ResTexture res = {};
        res.Dimension           = TEXTURE_DIMENSION_2D;
        res.Width               = 32;
        res.Height              = 32;
        res.DepthOrArraySize    = 1;
        res.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
        res.MipLevels           = 1;
        res.SubResources        = ArrayView(&subRes, 1);
        res.Pixels              = ArrayView(pixels.data(), pixels.size());

        Texture* pTexture = nullptr;
        if (CreateTexture(res, &pTexture))
        {
            auto hash = CalcHash("default.Orm");
            pTexture->SetName(L"default.Orm");

            // テクスチャ登録.
            m_Textures[hash] = pTexture;
        }
    }

    // Transparent Black.
    {
        std::vector<uint8_t> pixels;
        pixels.resize(32 * 32* 4);

        auto idx = 0;
        for(auto i=0; i<32; ++i)
        {
            for(auto j=0; j<32; ++j)
            {
                pixels[idx + 0] = 0;
                pixels[idx + 1] = 0;
                pixels[idx + 2] = 0;
                pixels[idx + 3] = 0;
                idx += 4;
            }
        }

        ResSubResource subRes = {};
        subRes.Width        = 32;
        subRes.Height       = 32;
        subRes.RowPitch     = 32 * 4;
        subRes.SlicePitch   = 32 * 32 * 4;
        subRes.PixelOffset  = 0;

        ResTexture res = {};
        res.Dimension           = TEXTURE_DIMENSION_2D;
        res.Width               = 32;
        res.Height              = 32;
        res.DepthOrArraySize    = 1;
        res.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
        res.MipLevels           = 1;
        res.SubResources        = ArrayView(&subRes, 1);
        res.Pixels              = ArrayView(pixels.data(), pixels.size());

        Texture* pTexture = nullptr;
        if (CreateTexture(res, &pTexture))
        {
            auto hash = CalcHash("default.TransparentBlack");
            pTexture->SetName(L"default.TransparentBlack");

            // テクスチャ登録.
            m_Textures[hash] = pTexture;
        }
    }

    // Opaque Black.
    {
        std::vector<uint8_t> pixels;
        pixels.resize(32 * 32* 4);

        auto idx = 0;
        for(auto i=0; i<32; ++i)
        {
            for(auto j=0; j<32; ++j)
            {
                pixels[idx + 0] = 0;
                pixels[idx + 1] = 0;
                pixels[idx + 2] = 0;
                pixels[idx + 3] = 255;
                idx += 4;
            }
        }

        ResSubResource subRes = {};
        subRes.Width        = 32;
        subRes.Height       = 32;
        subRes.RowPitch     = 32 * 4;
        subRes.SlicePitch   = 32 * 32 * 4;
        subRes.PixelOffset  = 0;

        ResTexture res = {};
        res.Dimension           = TEXTURE_DIMENSION_2D;
        res.Width               = 32;
        res.Height              = 32;
        res.DepthOrArraySize    = 1;
        res.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
        res.MipLevels           = 1;
        res.SubResources        = ArrayView(&subRes, 1);
        res.Pixels              = ArrayView(pixels.data(), pixels.size());

        Texture* pTexture = nullptr;
        if (CreateTexture(res, &pTexture))
        {
            auto hash = CalcHash("default.OpaqueBlack");
            pTexture->SetName(L"default.OpaqueBlack");

            // テクスチャ登録.
            m_Textures[hash] = pTexture;
        }
    }

    // Transparent White.
    {
        std::vector<uint8_t> pixels;
        pixels.resize(32 * 32* 4);

        auto idx = 0;
        for(auto i=0; i<32; ++i)
        {
            for(auto j=0; j<32; ++j)
            {
                pixels[idx + 0] = 255;
                pixels[idx + 1] = 255;
                pixels[idx + 2] = 255;
                pixels[idx + 3] = 0;
                idx += 4;
            }
        }

        ResSubResource subRes = {};
        subRes.Width        = 32;
        subRes.Height       = 32;
        subRes.RowPitch     = 32 * 4;
        subRes.SlicePitch   = 32 * 32 * 4;
        subRes.PixelOffset  = 0;

        ResTexture res = {};
        res.Dimension           = TEXTURE_DIMENSION_2D;
        res.Width               = 32;
        res.Height              = 32;
        res.DepthOrArraySize    = 1;
        res.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
        res.MipLevels           = 1;
        res.SubResources        = ArrayView(&subRes, 1);
        res.Pixels              = ArrayView(pixels.data(), pixels.size());

        Texture* pTexture = nullptr;
        if (CreateTexture(res, &pTexture))
        {
            auto hash = CalcHash("default.TransparentWhite");
            pTexture->SetName(L"default.TransparentWhite");

            // テクスチャ登録.
            m_Textures[hash] = pTexture;
        }
    }

    // Opaque White.
    {
        std::vector<uint8_t> pixels;
        pixels.resize(32 * 32* 4);

        auto idx = 0;
        for(auto i=0; i<32; ++i)
        {
            for(auto j=0; j<32; ++j)
            {
                pixels[idx + 0] = 255;
                pixels[idx + 1] = 255;
                pixels[idx + 2] = 255;
                pixels[idx + 3] = 255;
                idx += 4;
            }
        }

        ResSubResource subRes = {};
        subRes.Width        = 32;
        subRes.Height       = 32;
        subRes.RowPitch     = 32 * 4;
        subRes.SlicePitch   = 32 * 32 * 4;
        subRes.PixelOffset  = 0;

        ResTexture res = {};
        res.Dimension           = TEXTURE_DIMENSION_2D;
        res.Width               = 32;
        res.Height              = 32;
        res.DepthOrArraySize    = 1;
        res.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
        res.MipLevels           = 1;
        res.SubResources        = ArrayView(&subRes, 1);
        res.Pixels              = ArrayView(pixels.data(), pixels.size());

        Texture* pTexture = nullptr;
        if (CreateTexture(res, &pTexture))
        {
            auto hash = CalcHash("default.OpaqueWhite");
            pTexture->SetName(L"default.OpaqueWhite");

            // テクスチャ登録.
            m_Textures[hash] = pTexture;
        }
    }

    // Velocity Map.
    {
        std::vector<uint8_t> pixels;
        pixels.resize(32 * 32* 4);

        auto idx = 0;
        for(auto i=0; i<32; ++i)
        {
            for(auto j=0; j<32; ++j)
            {
                pixels[idx + 0] = 128;
                pixels[idx + 1] = 128;
                pixels[idx + 2] = 0;
                pixels[idx + 3] = 255;
                idx += 4;
            }
        }

        ResSubResource subRes = {};
        subRes.Width        = 32;
        subRes.Height       = 32;
        subRes.RowPitch     = 32 * 4;
        subRes.SlicePitch   = 32 * 32 * 4;
        subRes.PixelOffset  = 0;

        ResTexture res = {};
        res.Dimension           = TEXTURE_DIMENSION_2D;
        res.Width               = 32;
        res.Height              = 32;
        res.DepthOrArraySize    = 1;
        res.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
        res.MipLevels           = 1;
        res.SubResources        = ArrayView(&subRes, 1);
        res.Pixels              = ArrayView(pixels.data(), pixels.size());

        Texture* pTexture = nullptr;
        if (CreateTexture(res, &pTexture))
        {
            auto hash = CalcHash("default.Velocity");
            pTexture->SetName(L"default.Velocity");

            // テクスチャ登録.
            m_Textures[hash] = pTexture;
        }
    }
}

//-----------------------------------------------------------------------------
//      テクスチャを生成します.
//-----------------------------------------------------------------------------
bool TextureManager::CreateTexture(ResTexture& resource, Texture** ppTexture)
{
    if (asdx::IsSupportGpuUploadHeap())
        return Texture::Create(resource, ppTexture);

    auto ret = Texture::Create(m_CmdList[m_BufferIndex].GetPtr(), resource, ppTexture);
    if (ret)
    { m_HasCommand = true; }

    return ret;
}

} // namespace asdx
