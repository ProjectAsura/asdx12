//-----------------------------------------------------------------------------
// File : asdxTarget.cpp
// Desc : Target Wrapper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxTarget.h>
#include <asdxLogger.h>
#include <asdxGraphicsDevice.h>


namespace /* anonymous */ {

//-----------------------------------------------------------------------------
//      sRGBフォーマットに変換します.
//-----------------------------------------------------------------------------
DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT value)
{
    DXGI_FORMAT result = value;

    switch( value )
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        { result = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_BC1_UNORM:
        { result = DXGI_FORMAT_BC1_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_BC2_UNORM:
        { result = DXGI_FORMAT_BC2_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_BC3_UNORM:
        { result = DXGI_FORMAT_BC3_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
        { result = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
        { result = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_BC7_UNORM:
        { result = DXGI_FORMAT_BC7_UNORM_SRGB; }
        break;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      深度フォーマットからリソースフォーマットに変換します.
//-----------------------------------------------------------------------------
DXGI_FORMAT GetResourceFormat(DXGI_FORMAT value, bool isStencil)
{
    DXGI_FORMAT result = value;

    switch(value)
    {
    case DXGI_FORMAT_D16_UNORM:
        { result = DXGI_FORMAT_R16_UNORM; }
        break;

    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        {
            if (!isStencil)
                result = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            else
                result = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
        }
        break;

    case DXGI_FORMAT_D32_FLOAT:
        { result = DXGI_FORMAT_R32_FLOAT; }
        break;

    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        {
            if (!isStencil)
                result = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            else
                result = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
        }
        break;
    }

    return result;
}

} // namespace /* anonymouus */


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ColorTarget class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ColorTarget::ColorTarget()
: m_IsSRGB(false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ColorTarget::~ColorTarget()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool ColorTarget::Init(const TargetDesc* pDesc, bool isSRGB)
{
    HRESULT hr = S_OK;

    {
        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1
        };

        D3D12_RESOURCE_DESC desc = {
            pDesc->Dimension,
            pDesc->Alignment,
            pDesc->Width,
            pDesc->Height,
            pDesc->DepthOrArraySize,
            pDesc->MipLevels,
            pDesc->Format,
            pDesc->SampleDesc,
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        };

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format = pDesc->Format,
        clearValue.Color[0] = 1.0f;
        clearValue.Color[1] = 1.0f;
        clearValue.Color[2] = 1.0f;
        clearValue.Color[3] = 1.0f;

        hr = GetD3D12Device()->CreateCommittedResource( 
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            pDesc->InitState,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr );
            return false;
        }

        m_pResource->SetName(L"asdxColorTarget");
    }

#if ASDX_IS_SCARLETT
    auto mostDetailedMip = (pDesc->MipLevels - 1);
#else
    auto mostDetailedMip = 0u;
#endif

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    D3D12_RENDER_TARGET_VIEW_DESC   rtv_desc = {};
    rtv_desc.Format = ( isSRGB ) ? GetSRGBFormat(pDesc->Format) : pDesc->Format;
    srv_desc.Format = ( isSRGB ) ? GetSRGBFormat(pDesc->Format) : pDesc->Format; 
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    if ( pDesc->DepthOrArraySize > 1 )
    {
        if ( pDesc->MipLevels <= 1 )
        {
            rtv_desc.Texture2DMSArray.ArraySize       = pDesc->DepthOrArraySize;
            rtv_desc.Texture2DMSArray.FirstArraySlice = 0;
            rtv_desc.ViewDimension                    = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;

            srv_desc.Texture2DMSArray.ArraySize       = pDesc->DepthOrArraySize;
            srv_desc.Texture2DMSArray.FirstArraySlice = 0;
            srv_desc.ViewDimension                    = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
        }
        else
        {
            rtv_desc.Texture2DArray.ArraySize        = pDesc->DepthOrArraySize;
            rtv_desc.Texture2DArray.FirstArraySlice  = 0;
            rtv_desc.Texture2DArray.MipSlice         = 0;
            rtv_desc.Texture2DArray.PlaneSlice       = 0;
            rtv_desc.ViewDimension                   = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

            srv_desc.Texture2DArray.ArraySize           = pDesc->DepthOrArraySize;
            srv_desc.Texture2DArray.FirstArraySlice     = 0;
            srv_desc.Texture2DArray.MipLevels           = 0;
            srv_desc.Texture2DArray.PlaneSlice          = 0;
            srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
            srv_desc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
            srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        }
    }
    else
    {
        if ( pDesc->MipLevels <= 1 )
        {
            rtv_desc.Texture2D.MipSlice = 0;
            rtv_desc.ViewDimension      = D3D12_RTV_DIMENSION_TEXTURE2DMS;

            srv_desc.Texture2D.MipLevels            = pDesc->MipLevels;
            srv_desc.Texture2D.MostDetailedMip      = mostDetailedMip;
            srv_desc.Texture2D.PlaneSlice           = 0;
            srv_desc.Texture2D.ResourceMinLODClamp  = 0.0f;
            srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2D;
        }
        else
        {
            rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
    }

    if (!CreateRenderTargetView(m_pResource.GetPtr(), &rtv_desc, m_pRTV.GetAddress()))
    {
        ELOG("Error : CreateRenderTargetView() Failed.");
        return false;
    }

    if (!CreateShaderResourceView(m_pResource.GetPtr(), &srv_desc, m_pSRV.GetAddress()))
    {
        ELOG("Error : CreateShaderResourceView() Failed.");
        return false;
    }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    m_IsSRGB = isSRGB;

    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool ColorTarget::Init
(
    IDXGISwapChain* pSwapChain,
    uint32_t        backBufferIndex,
    bool            isSRGB
)
{
    HRESULT hr = S_OK;

    hr = pSwapChain->GetBuffer( backBufferIndex, IID_PPV_ARGS(m_pResource.GetAddress()));
    if ( FAILED( hr ) )
    {
        ELOG( "Error : IDXGISwapChain::GetBuffer() Failed. errcode = 0x%x", hr );
        return false;
    }

    auto desc = m_pResource->GetDesc();
    if ( desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D )
    {
        ELOG( "Error : Invalid Resource Dimension. ");
        return false;
    }

#if ASDX_IS_SCARLETT
    auto mostDetailedMip = (pDesc->MipLevels - 1);
#else
    auto mostDetailedMip = 0u;
#endif

    auto format = desc.Format;
    if (isSRGB)
    { format = GetSRGBFormat(desc.Format); }

    D3D12_RENDER_TARGET_VIEW_DESC   rtv_desc = {};
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    rtv_desc.Format = desc.Format;
    srv_desc.Format = desc.Format;
    srv_desc.Shader4ComponentMapping =D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if ( desc.DepthOrArraySize > 1 )
    {
        if ( desc.SampleDesc.Count >= 1 )
        {
            rtv_desc.Texture2DMSArray.ArraySize       = desc.DepthOrArraySize;
            rtv_desc.Texture2DMSArray.FirstArraySlice = 0;
            rtv_desc.ViewDimension                    = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;

            srv_desc.Texture2DMSArray.ArraySize       = desc.DepthOrArraySize;
            srv_desc.Texture2DMSArray.FirstArraySlice = 0;
            srv_desc.ViewDimension                    = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
        }
        else
        {
            rtv_desc.Texture2DArray.ArraySize        = desc.DepthOrArraySize;
            rtv_desc.Texture2DArray.FirstArraySlice  = 0;
            rtv_desc.Texture2DArray.MipSlice         = 0;
            rtv_desc.Texture2DArray.PlaneSlice       = 0;
            rtv_desc.ViewDimension                   = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

            srv_desc.Texture2DArray.ArraySize           = desc.DepthOrArraySize;
            srv_desc.Texture2DArray.FirstArraySlice     = 0;
            srv_desc.Texture2DArray.MipLevels           = desc.MipLevels;
            srv_desc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
            srv_desc.Texture2DArray.PlaneSlice          = 0;
            srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
        }
    }
    else
    {
        if ( desc.SampleDesc.Count <= 1 )
        {
            rtv_desc.Texture2D.MipSlice     = 0;
            rtv_desc.Texture2D.PlaneSlice   = 0;
            rtv_desc.ViewDimension          = D3D12_RTV_DIMENSION_TEXTURE2D;
                
            srv_desc.Texture2D.MipLevels            = desc.MipLevels;
            srv_desc.Texture2D.MostDetailedMip      = mostDetailedMip;
            srv_desc.Texture2D.PlaneSlice           = 0;
            srv_desc.Texture2D.ResourceMinLODClamp  = 0.0f;
            srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2D;
        }
        else
        {
            rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
    }

    if (!CreateRenderTargetView(m_pResource.GetPtr(), &rtv_desc, m_pRTV.GetAddress()))
    {
        ELOG("Error : CreateRenderTargetView() Failed.");
        return false;
    }

    if (!CreateShaderResourceView(m_pResource.GetPtr(), &srv_desc, m_pSRV.GetAddress()))
    {
        ELOG("Error : CreateShaderResourceView() Failed.");
        return false;
    }

    m_IsSRGB = true;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ColorTarget::Term()
{
    m_pSRV.Reset();
    m_pRTV.Reset();
    m_pResource.Reset();

    memset(&m_Desc, 0, sizeof(m_Desc));
    m_IsSRGB = false;
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
bool ColorTarget::Resize(uint32_t width, uint32_t height)
{
    auto desc = m_Desc;
    auto srgb = m_IsSRGB;
    Term();

    desc.Width  = width;
    desc.Height = height;
    return Init(&desc, srgb);
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* ColorTarget::GetResource() const
{ return m_pResource.GetPtr(); }

//-----------------------------------------------------------------------------
//      レンダーターゲットビューを取得します.
//-----------------------------------------------------------------------------
const IRenderTargetView* ColorTarget::GetRTV() const
{ return m_pRTV.GetPtr(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-----------------------------------------------------------------------------
const IShaderResourceView* ColorTarget::GetSRV() const
{ return m_pSRV.GetPtr(); }

//-----------------------------------------------------------------------------
//      構成設定を取得します.
//-----------------------------------------------------------------------------
TargetDesc ColorTarget::GetDesc() const
{ return m_Desc; }

//-----------------------------------------------------------------------------
//      sRGBフラグを取得します.
//-----------------------------------------------------------------------------
bool ColorTarget::IsSRGB() const
{ return m_IsSRGB; }


///////////////////////////////////////////////////////////////////////////////
// DepthTarget class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
DepthTarget::DepthTarget()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
DepthTarget::~DepthTarget()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool DepthTarget::Init(const TargetDesc* pDesc)
{
    HRESULT hr = S_OK;

    auto format = GetResourceFormat(pDesc->Format, false);
    {
        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1
        };

        D3D12_RESOURCE_DESC desc = {
            pDesc->Dimension,
            pDesc->Alignment,
            pDesc->Width,
            pDesc->Height,
            pDesc->DepthOrArraySize,
            pDesc->MipLevels,
            pDesc->Format,
            pDesc->SampleDesc,
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        };

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format               = pDesc->Format;
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        hr = GetD3D12Device()->CreateCommittedResource( 
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            pDesc->InitState,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr );
            return false;
        }

        m_pResource->SetName(L"asdxDepthTarget");
    }

    {
        D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
        desc.Format        = pDesc->Format;
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        desc.Flags         = D3D12_DSV_FLAG_NONE;

        if (!CreateDepthStencilView(m_pResource.GetPtr(), &desc, m_pDSV.GetAddress()))
        {
            ELOG("Error : CreateDepthStencilView() Failed.");
            return false;
        }
    }

    {
    #if ASDX_IS_SCARLETT
        auto mostDetailedMip = (pDesc->MipLevels - 1);
    #else
        auto mostDetailedMip = 0u;
    #endif

        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.ViewDimension              = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Format                     = format;
        desc.Texture2D.MipLevels        = pDesc->MipLevels;
        desc.Texture2D.MostDetailedMip  = mostDetailedMip;
        desc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (!CreateShaderResourceView(m_pResource.GetPtr(), &desc, m_pSRV.GetAddress()))
        {
            ELOG("Error : CreateShaderResourceView() Failed.");
            return false;
        }
    }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void DepthTarget::Term()
{
    m_pDSV.Reset();
    m_pSRV.Reset();
    m_pResource.Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* DepthTarget::GetResource() const
{ return m_pResource.GetPtr(); }

//-----------------------------------------------------------------------------
//      深度ステンシルビューを取得します.
//-----------------------------------------------------------------------------
const IDepthStencilView* DepthTarget::GetDSV() const
{ return m_pDSV.GetPtr(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-----------------------------------------------------------------------------
const IShaderResourceView* DepthTarget::GetSRV() const
{ return m_pSRV.GetPtr(); }

} // namespace asdx
