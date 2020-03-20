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

} // namespace /* anonymouus */


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ColorTarget class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ColorTarget::ColorTarget()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ColorTarget::~ColorTarget()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool ColorTarget::Init(GraphicsDevice& device, const TargetDesc* pDesc, bool isSRGB)
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

        hr = device.GetDevice6()->CreateCommittedResource( 
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            pDesc->InitState,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource()" );
            return false;
        }
    }

    {
        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        desc.Format             = ( isSRGB ) ? GetSRGBFormat(pDesc->Format) : pDesc->Format;
        desc.Texture2D.MipSlice = 0;

        m_pDescriptorRTV = device.AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        if (m_pDescriptorRTV == nullptr)
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }

        device.GetDevice6()->CreateRenderTargetView( m_pResource.GetPtr(), &desc, m_pDescriptorRTV->GetHandleCPU() );
    }

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format                     = pDesc->Format;
        desc.Texture2D.MipLevels        = pDesc->MipLevels;
        desc.Texture2D.MostDetailedMip  = 0;
        desc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        m_pDescriptorSRV = device.AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        if (m_pDescriptorSRV == nullptr)
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }

        device.GetDevice6()->CreateShaderResourceView( m_pResource.GetPtr(), &desc, m_pDescriptorRTV->GetHandleCPU() );
    }

    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool ColorTarget::Init(GraphicsDevice& device, IDXGISwapChain* pSwapChain, uint32_t backBufferIndex, bool isSRGB)
{
    HRESULT hr = S_OK;

    hr = pSwapChain->GetBuffer( backBufferIndex, IID_PPV_ARGS(m_pResource.GetAddress()));
    if ( FAILED( hr ) )
    {
        ELOG( "Error : IDXGISwapChain::GetBuffer() Failed." );
        return false;
    }

    auto desc = m_pResource->GetDesc();
    if ( desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D )
    {
        ELOG( "Error : Invalid Resource Dimension. ");
        return false;
    }

    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format              = desc.Format;

        if ( desc.DepthOrArraySize > 1 )
        {
            if ( desc.MipLevels <= 1 )
            {
                rtvDesc.Texture2DMSArray.ArraySize       = desc.DepthOrArraySize;
                rtvDesc.Texture2DMSArray.FirstArraySlice = 0;
            }
            else
            {
                rtvDesc.Texture2DArray.ArraySize        = desc.DepthOrArraySize;
                rtvDesc.Texture2DArray.FirstArraySlice  = 0;
                rtvDesc.Texture2DArray.MipSlice         = 0;
                rtvDesc.Texture2DArray.PlaneSlice       = 0;
            }
        }
        else
        {
            if ( desc.MipLevels <= 1 )
            {
                rtvDesc.Texture2D.MipSlice = 0;
            }
        }

        m_pDescriptorRTV = device.AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        if (m_pDescriptorRTV == nullptr)
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }
        device.GetDevice6()->CreateRenderTargetView( m_pResource.GetPtr(), nullptr, m_pDescriptorRTV->GetHandleCPU() );
    }

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                    = ( isSRGB ) ? GetSRGBFormat( desc.Format) : desc.Format;
        srvDesc.Texture2D.MipLevels       = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        m_pDescriptorSRV = device.AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        if (m_pDescriptorSRV == nullptr)
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }
        device.GetDevice6()->CreateShaderResourceView( m_pResource.GetPtr(), &srvDesc, m_pDescriptorSRV->GetHandleCPU() );
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ColorTarget::Term()
{
    m_pDescriptorRTV.Reset();
    m_pDescriptorSRV.Reset();
    m_pResource.Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* ColorTarget::GetResource() const
{ return m_pResource.GetPtr(); }

//-----------------------------------------------------------------------------
//      レンダーターゲットビュー用ディスクリプタを取得します.
//-----------------------------------------------------------------------------
const Descriptor* ColorTarget::GetRTV() const
{ return m_pDescriptorRTV.GetPtr(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビュー用ディスクリプタを取得します.
//-----------------------------------------------------------------------------
const Descriptor* ColorTarget::GetSRV() const
{ return m_pDescriptorSRV.GetPtr(); }


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
bool DepthTarget::Init(GraphicsDevice& device, const TargetDesc* pDesc)
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
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        };

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format               = pDesc->Format;
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        hr = device.GetDevice6()->CreateCommittedResource( 
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            pDesc->InitState,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource()" );
            return false;
        }
    }

    {
        D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
        desc.Format        = pDesc->Format;
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        desc.Flags         = D3D12_DSV_FLAG_NONE;

        m_pDescriptorDSV = device.AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        if (m_pDescriptorDSV == nullptr)
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }
        device.GetDevice6()->CreateDepthStencilView(m_pResource.GetPtr(), &desc, m_pDescriptorDSV->GetHandleCPU());
    }

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format                    = pDesc->Format;
        desc.Texture2D.MipLevels       = pDesc->MipLevels;
        desc.Texture2D.MostDetailedMip = 0;
        desc.Shader4ComponentMapping  = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        m_pDescriptorSRV = device.AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        if (m_pDescriptorSRV == nullptr)
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }
        device.GetDevice6()->CreateShaderResourceView(m_pResource.GetPtr(), &desc, m_pDescriptorSRV->GetHandleCPU());
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void DepthTarget::Term()
{
    m_pDescriptorDSV.Reset();
    m_pDescriptorSRV.Reset();
    m_pResource.Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* DepthTarget::GetResource() const
{ return m_pResource.GetPtr(); }

//-----------------------------------------------------------------------------
//      深度ステンシルビュー用ディスクリプターを取得します.
//-----------------------------------------------------------------------------
const Descriptor* DepthTarget::GetDSV() const
{ return m_pDescriptorDSV.GetPtr(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビュー用ディスクリプターを取得します.
//-----------------------------------------------------------------------------
const Descriptor* DepthTarget::GetSRV() const
{ return m_pDescriptorSRV.GetPtr(); }

} // namespace asdx
