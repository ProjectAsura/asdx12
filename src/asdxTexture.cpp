//-----------------------------------------------------------------------------
// File : asdxTexture.cpp
// Desc : Texture.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxTexture.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Texture class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Texture::Texture()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Texture::~Texture()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Texture::Init
(
    GraphicsDevice&     device,
    const ResTexture&   resource,
    ID3D12Resource**    ppUpload
)
{
    auto pD3DDevice = device.GetDevice();

    auto dimension  = D3D12_RESOURCE_DIMENSION_UNKNOWN;
    auto isCube     = false;
    auto depth      = 1;
    auto format     = DXGI_FORMAT(resource.Format);

#if ASDX_IS_SCARLETT
    auto mostDetailedMip = resourc.MipMapCount - 1;
#else
    auto mostDetailedMip = 0u;
#endif

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    {
        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1
        };

        switch(resource.Dimension)
        {
        case TEXTURE_DIMENSION_1D:
            {
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                if (resource.SurfaceCount > 1)
                {
                    viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Format                             = format;
                    viewDesc.Texture1DArray.ArraySize           = resource.SurfaceCount;
                    viewDesc.Texture1DArray.FirstArraySlice     = 0;
                    viewDesc.Texture1DArray.MipLevels           = resource.MipMapCount;
                    viewDesc.Texture1DArray.MostDetailedMip     = mostDetailedMip;
                    viewDesc.Texture1DArray.ResourceMinLODClamp = 0;
                }
                else
                {
                    viewDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1D;
                    viewDesc.Format                         = format;
                    viewDesc.Texture1D.MipLevels            = resource.MipMapCount;
                    viewDesc.Texture1D.MostDetailedMip      = mostDetailedMip;
                    viewDesc.Texture1D.ResourceMinLODClamp  = 0;
                }
            }
            break;

        case TEXTURE_DIMENSION_2D:
            {
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                if (resource.SurfaceCount > 1)
                {
                    if (resource.MipMapCount > 1)
                    {
                        viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                        viewDesc.Format                             = format;
                        viewDesc.Texture2DMSArray.ArraySize         = resource.SurfaceCount;
                        viewDesc.Texture2DMSArray.FirstArraySlice   = 0;
                    }
                    else
                    {
                        viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Format                             = format;
                        viewDesc.Texture2DArray.ArraySize           = resource.SurfaceCount;
                        viewDesc.Texture2DArray.FirstArraySlice     = 0;
                        viewDesc.Texture2DArray.MipLevels           = resource.MipMapCount;
                        viewDesc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
                        viewDesc.Texture2DArray.PlaneSlice          = 0;
                        viewDesc.Texture2DArray.ResourceMinLODClamp = 0;
                    }
                }
                else
                {
                    if (resource.MipMapCount > 1)
                    {
                        viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Format                             = format;
                        viewDesc.Texture2DArray.ArraySize           = resource.SurfaceCount;
                        viewDesc.Texture2DArray.FirstArraySlice     = 0;
                        viewDesc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
                        viewDesc.Texture2DArray.PlaneSlice          = 0;
                        viewDesc.Texture2DArray.ResourceMinLODClamp = 0;
                    }
                    else
                    {
                        viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2D;
                        viewDesc.Format                             = format;
                        viewDesc.Texture2D.MipLevels                = resource.MipMapCount;
                        viewDesc.Texture2D.MostDetailedMip          = mostDetailedMip;
                        viewDesc.Texture2D.PlaneSlice               = 0;
                        viewDesc.Texture2D.ResourceMinLODClamp      = 0;
                    }
                }
            }
            break;

        case TEXTURE_DIMENSION_3D:
            {
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                depth = resource.Depth;
            }
            break;

        case TEXTURE_DIMENSION_CUBE:
            {
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                depth = resource.SurfaceCount;
            }
            break;
        }

        D3D12_RESOURCE_DESC desc = {
            dimension,
            0,
            resource.Width,
            resource.Height,
            depth,
            resource.MipMapCount,
            format,
            { 1, 0 },
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_NONE
        };

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = desc.Format;
        clearValue.Color[0] = 1.0f;
        clearValue.Color[1] = 1.0f;
        clearValue.Color[2] = 1.0f;
        clearValue.Color[3] = 1.0f;

        auto hr = pD3DDevice->CreateCommittedResource(
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommitedResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_pResource->SetName(L"asdxTexture");
    }

    // シェーダリソースビューの生成.
    {
        auto ret = device.AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_pDescriptor.GetAddress());
        if (!ret)
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }

        pD3DDevice->CreateShaderResourceView(m_pResource.GetPtr(), &viewDesc, m_pDescriptor->GetHandleCPU());
    }

    // アップロードリソースを生成.
    if (ppUpload != nullptr)
    {
        ID3D12Resource* pUpload = nullptr;

        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1
        };

        D3D12_RESOURCE_DESC desc = {
            dimension,
            0,
            resource.Width,
            resource.Height,
            depth,
            resource.MipMapCount,
            format,
            { 1, 0 },
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_NONE
        };

        auto hr = pD3DDevice->CreateCommittedResource(
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&pUpload));
        if (!FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
            return false;
        }

    #if ASDX_IS_SCARLETT
        auto dstIdx = 0;

        for(auto j=resource.MipMapCount - 1; j>=0 j--)
        {
            for(auto i=0; i<resource.SurfaceCount; ++i)
            {
                auto srcIdx = j * resource.SurfaceCount + i;
                uint8_t* ptr = nullptr;
                hr = pUpload->Map(dstIdx, nullptr, reinterpret_cast<void**>(&ptr));
                if (FAILED(hr))
                { continue; }

                memcpy(ptr, resource.pResource[srcIdx].pPixel, resource.pResource[srcIdx].SlicePitch);

                pUpload->Umap(dstIdx, nullptr);
                dstIdx++;
            }
        }
    #else
        auto count = resource.MipMapCount * resource.SurfaceCount;

        for(auto i=0; i<count; ++i)
        {
            uint8_t* ptr = nullptr;
            hr = pUpload->Map(i, nullptr, reinterpret_cast<void**>(&ptr));
            if (FAILED(hr))
            { continue; }

            memcpy(ptr, resource.pResources[i].pPixels, resource.pResources[i].SlicePitch);

            pUpload->Unmap(i, nullptr);
        }
    #endif
    }


    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Texture::Term()
{
    m_pDescriptor.Reset();
    m_pResource  .Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* Texture::GetResource() const
{ return m_pResource.GetPtr(); }

//-----------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-----------------------------------------------------------------------------
const Descriptor* Texture::GetDescriptor() const
{ return m_pDescriptor.GetPtr(); }

} // namespace asdx
