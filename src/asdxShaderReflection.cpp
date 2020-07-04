//-----------------------------------------------------------------------------
// File : asdxShaderReflection.cpp
// Desc : Shader Reflection.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxShaderReflection.h>
#include <asdxLogger.h>

#ifdef ASDX_ENABLE_DXC
    #include <dxcapi.h>
#else
    #include <d3dcompiler.h>
#endif//ASDX_ENABLE_DXC


#define ASDX_FOURCC(ch0, ch1, ch2, ch3) (                            \
  (uint32_t)(uint8_t)(ch0)        | (uint32_t)(uint8_t)(ch1) << 8  | \
  (uint32_t)(uint8_t)(ch2) << 16  | (uint32_t)(uint8_t)(ch3) << 24   \
)

namespace {

//-----------------------------------------------------------------------------
//      シェーダリフレクションを生成します.
//-----------------------------------------------------------------------------
HRESULT CreateShaderReflectionOld(const void* pData, uint32_t size, ID3D12ShaderReflection** ppResult)
{
    // DirectX ShaderCompiler before March 2020.

    const uint32_t kDFCC_DXIL  = ASDX_FOURCC('D', 'X', 'I', 'L');

    asdx::RefPtr<IDxcLibrary> pLibrary;
    auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(pLibrary.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : DxcCreateInstance() Failed. errcode = 0x%x", hr);
        return hr;
    }

    asdx::RefPtr<IDxcBlobEncoding> blobEncoding;
    hr = pLibrary->CreateBlobWithEncodingOnHeapCopy(pData, size, CP_ACP, blobEncoding.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : IDxcLibrary::CreateBlobWithEncodingOnHeapCopy() Faield. errcode = 0x%x", hr);
        return hr;
    }

    asdx::RefPtr<IDxcContainerReflection> containerReflection;
    hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(containerReflection.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : DxcCreateInstance() Failed. errcode = 0x%x", hr);
        return hr;
    }

    uint32_t shaderIdx = 0;
    hr = containerReflection->Load(blobEncoding.GetPtr());
    if (FAILED(hr))
    {
        ELOG("Error : IDxcContainerReflection::Load() Failed. errcode = 0x%x", hr);
        return hr;
    }

    hr = containerReflection->FindFirstPartKind(kDFCC_DXIL, &shaderIdx);
    if (FAILED(hr))
    {
        ELOG("Error : IDxcContainerReflection::FindFirstPartKind() Failed. errcode = 0x%x", hr);
        return hr;
    }

    return containerReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(ppResult));
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ShaderReflection class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ShaderReflection::ShaderReflection()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ShaderReflection::~ShaderReflection()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ShaderReflection::Init(const void* pData, uint32_t size)
{
#ifdef ASDX_ENABLE_DXC
    asdx::RefPtr<IDxcUtils> pUtil;
    auto hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtil.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : DxcCreateInstance() Failed. errcode = 0x%x", hr);
        return false;
    }

    DxcBuffer buf = {};
    buf.Ptr      = pData;
    buf.Size     = SIZE_T(size);
    buf.Encoding = CP_ACP;

    hr = pUtil->CreateReflection(&buf, IID_PPV_ARGS(m_pReflection.GetAddress()));
    if (FAILED(hr))
    {
        if (hr == E_NOINTERFACE)
        {
            hr = CreateShaderReflectionOld(pData, size, m_pReflection.GetAddress());
            if (FAILED(hr))
            {
                ELOG("Error : CreateShaderReflectionOld() Failed.");
                return false;
            }
        }
        else
        {
            ELOG("Error : IDxcutils::CreateReflection() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
#else
    auto hr = D3DReflect(pBinary, binarySize, IID_PPV_ARGS(m_pReflection.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : D3DReflect() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
#endif
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ShaderReflection::Term()
{ m_pReflection.Reset(); }

//-----------------------------------------------------------------------------
//      シェーダリフレクションを取得します.
//-----------------------------------------------------------------------------
ID3D12ShaderReflection* ShaderReflection::GetPtr() const
{ return m_pReflection.GetPtr(); }

//-----------------------------------------------------------------------------
//      アロー演算子です.
//-----------------------------------------------------------------------------
ID3D12ShaderReflection* ShaderReflection::operator ->()
{ return m_pReflection.GetPtr(); }

} // namespace asdx
