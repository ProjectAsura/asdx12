//-----------------------------------------------------------------------------
// File : asdxShaderCompiler.cpp
// Desc : Shader Compiler.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <cassert>
#include <atomic>
#include <asdxMisc.h>
#include <asdxShaderCompiler.h>
#include <d3d12.h>
#ifdef ASDX_ENABLE_DXC
    #include <dxcapi.h>
#else
    #include <d3dcompiler.h>
#endif//ASDX_ENABLE_DXC


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Blob class
///////////////////////////////////////////////////////////////////////////////
class Blob : public IBlob
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    size_t m_Size = 0;
    void*  m_Buffer = nullptr;

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //      コンストラクタです.
    //-------------------------------------------------------------------------
    Blob()
    : m_Count(1)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //      デストラクタです.
    //-------------------------------------------------------------------------
    ~Blob()
    {
        if (m_Buffer)
        {
            free(m_Buffer);
            m_Buffer = nullptr;
        }
        m_Size = 0;
    }

    //-------------------------------------------------------------------------
    //      メモリを確保します.
    //-------------------------------------------------------------------------
    bool Alloc(size_t size)
    {
        m_Buffer = malloc(size);
        if (m_Buffer == nullptr)
        { return false; }

        m_Size = size;
        return true;
    }

    //-------------------------------------------------------------------------
    //      参照カウントを上げます.
    //-------------------------------------------------------------------------
    void AddRef() override
    { m_Count++; }

    //-------------------------------------------------------------------------
    //      参照カウントを下げます.
    //-------------------------------------------------------------------------
    void Release() override
    {
        m_Count--;
        if (m_Count == 0)
        { delete this; }
    }

    //-------------------------------------------------------------------------
    //      参照カウントを取得します.
    //-------------------------------------------------------------------------
    uint32_t GetCount() const override
    { return m_Count; }

    //-------------------------------------------------------------------------
    //      バッファサイズを取得します.
    //-------------------------------------------------------------------------
    size_t GetBufferSize() override
    { return m_Size; }

    //-------------------------------------------------------------------------
    //      バッファポインタを取得します.
    //-------------------------------------------------------------------------
    void* GetBufferPointer() override
    { return m_Buffer; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<uint32_t> m_Count;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

//-----------------------------------------------------------------------------
//      バイナリラージオブジェクトを生成します.
//-----------------------------------------------------------------------------
bool CreateBlob(size_t size, IBlob** ppResult)
{
    auto blob = new Blob();
    if (!blob->Alloc(size))
    {
        delete blob;
        return false;
    }

    *ppResult = blob;
    return true;
}

//-----------------------------------------------------------------------------
//      バイナリラージオブジェクトに読み込みます.
//-----------------------------------------------------------------------------
bool ReadFileToBlob(const char* filename, IBlob** ppResult)
{
    FILE* pFile;
    auto err = fopen_s(&pFile, filename, "rb");
    if (err != 0)
    {
        return false;
    }

    auto prevpos = ftell(pFile);
    fseek(pFile, 0, SEEK_END);
    auto currpos = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    auto size = uint64_t(currpos) - uint64_t(prevpos);
    auto ptr = malloc(size);
    if (ptr == nullptr)
    {
        fclose(pFile);
        return false;
    }

    fread(ptr, size, 1, pFile);
    fclose(pFile);

    auto blob = new Blob();
    blob->m_Buffer = ptr;
    blob->m_Size = size;

    *ppResult = blob;
    return true;
}

//-----------------------------------------------------------------------------
//      バイナリラージオブジェクトを書き出します.
//-----------------------------------------------------------------------------
bool WriteBlobToFile(IBlob* pBlob, const char* filename)
{
    FILE* pFile;
    auto err = fopen_s(&pFile, filename, "wb");
    if (err != 0)
    {
        return false;
    }

    fwrite(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), 1, pFile);
    fclose(pFile);

    return true;
}

//-----------------------------------------------------------------------------
//      シェーダコンパイルします.
//-----------------------------------------------------------------------------
bool CompileFromFile
(
    const wchar_t*  filename,
    const char*     entryPoint,
    const char*     shaderModel,
    IBlob**         ppResult
)
{
#ifdef ASDX_ENABLE_DXC
    FILE* pFile = nullptr;
    std::vector<uint8_t> buffer;
    auto err = _wfopen_s(&pFile, filename, L"rb");
    if (err != 0)
    {
        return false;
    }

    auto cur = ftell(pFile);
    fseek(pFile, 0, SEEK_END);
    auto end = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    auto size = end - cur;
    buffer.resize(end - cur + 1);
    fread(buffer.data(), size, 1, pFile);
    fclose(pFile);


    HRESULT hr = S_OK;
    RefPtr<IDxcCompiler> pCompiler;
    RefPtr<IDxcLibrary> pLibrary;
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.GetAddress()));
    DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(pLibrary.GetAddress()));

    RefPtr<IDxcBlobEncoding> pSource;
    hr = pLibrary->CreateBlobWithEncodingFromPinned(buffer.data(), UINT32(buffer.size()), CP_ACP, pSource.GetAddress());
    if (FAILED(hr))
    {
        return false;
    }

    LPCWSTR args[] = {
#if defined(DEBUG) || defined(_DEBUG)
        L"/Zi",
        L"/O0",
#else
        L"/O2"
#endif
    };

    auto ep = asdx::ToStringW(entryPoint);
    auto sm = asdx::ToStringW(shaderModel);

    RefPtr<IDxcOperationResult> pResults;
    pCompiler->Compile(
        pSource.GetPtr(),
        filename,
        ep.c_str(),
        sm.c_str(),
        args,
        _countof(args),
        nullptr,
        0,
        nullptr,
        pResults.GetAddress());

    HRESULT ret;
    pResults->GetStatus(&ret);
    if (FAILED(ret))
    {
        RefPtr<IDxcBlobEncoding> pErrorBlob;
        pResults->GetErrorBuffer(pErrorBlob.GetAddress());
        wprintf(L"Compilation Failed. errcode = 0x%x, msg = %lS",
            ret, reinterpret_cast<const wchar_t*>(pErrorBlob->GetBufferPointer()));
        
        return false;
    }

    RefPtr<IDxcBlob> pShader;
    pResults->GetResult(pShader.GetAddress());

    auto blob = new Blob();
    blob->m_Buffer = malloc(pShader->GetBufferSize());
    blob->m_Size   = pShader->GetBufferSize();

    memcpy(blob->m_Buffer, pShader->GetBufferPointer(), pShader->GetBufferSize());
    *ppResult = blob;

    return true;
#else
    RefPtr<ID3DBlob> pShader;
    RefPtr<ID3DBlob> pError;
    auto hr = D3DCompileFromFile(
        filename,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flags,
        0,
        pShader.GetAddress(),
        pError.GetAddress());
    if (FAILED(hr))
    {
        return false;
    }

    auto blob = new Blob();
    blob->m_Buffer = malloc(pShader->GetBufferSize());
    blob->m_Size = pShader->GetBufferSize();
    assert(blob->m_Buffer != nullptr);
    memcpy(blob->m_Buffer, pShader->GetBufferPointer(), pShader->GetBufferSize());
    *ppResult = blob;

    return true;
#endif
}

//-----------------------------------------------------------------------------
//      シェーダコンパイルします.
//-----------------------------------------------------------------------------
bool CompileFromFile
(
    const wchar_t*  filename,
    const wchar_t** compileArgs,
    uint32_t        countCompileArgs,
    IBlob**         ppResult
)
{
#ifdef ASDX_ENABLE_DXC
    #if 1
        HRESULT hr = S_OK;
        RefPtr<IDxcUtils> pUtils;
        RefPtr<IDxcCompiler3> pCompiler;
        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddress()));
        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.GetAddress()));

        RefPtr<IDxcIncludeHandler> pIncludeHandler;
        hr = pUtils->CreateDefaultIncludeHandler(pIncludeHandler.GetAddress());
        if (FAILED(hr))
        { return false; }

        RefPtr<IDxcBlobEncoding> pSource;
        pUtils->LoadFile(filename, nullptr, pSource.GetAddress());

        BOOL known;
        UINT32 codePage;
        pSource->GetEncoding(&known, &codePage);

        DxcBuffer source;
        source.Ptr = pSource->GetBufferPointer();
        source.Size = pSource->GetBufferSize();
        source.Encoding = (known == TRUE) ? codePage : DXC_CP_ACP;

        RefPtr<IDxcResult> pResults;
        pCompiler->Compile(
            &source,
            compileArgs,
            countCompileArgs,
            pIncludeHandler.GetPtr(),
            IID_PPV_ARGS(pResults.GetAddress()));

        RefPtr<IDxcBlobUtf8> pErrors;
        pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(pErrors.GetAddress()), nullptr);
        if (pErrors.GetPtr() != nullptr && pErrors->GetStringLength() != 0)
        {
            wprintf(L"Warnings and Errors:\n%S\n", pErrors->GetStringPointer());
        }

        HRESULT ret;
        pResults->GetStatus(&ret);
        if (FAILED(ret))
        {
            wprintf(L"Compilation Failed. errcode = 0x%x", ret);
            return false;
        }

        RefPtr<IDxcBlob> pShader;
        RefPtr<IDxcBlobUtf16> pShaderName;
        pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(pShader.GetAddress()), pShaderName.GetAddress());

        auto blob = new Blob();
        blob->m_Buffer = malloc(pShader->GetBufferSize());
        blob->m_Size = pShader->GetBufferSize();

        memcpy(blob->m_Buffer, pShader->GetBufferPointer(), pShader->GetBufferSize());
        *ppResult = blob;

        return true;
    #else
        std::string entryPoint = "main";
        std::string shaderModel = "vs_5_0";
        UINT flags = 0;

        for(auto i=0u; i<countCompileArgs; ++i)
        {
            // エントリーポイント.
            if (wcscmp(compileArgs[i], L"-E") == 0)
            {
                i++;
                entryPoint = asdx::ToStringA(compileArgs[i]);
            }
            // シェーダプロファイル.
            else if (wcscmp(compileArgs[i], L"-T") == 0)
            {
                i++;
                shaderModel = asdx::ToStringA(compileArgs[i]);
            }
        }

        return CompileFromFile(filename, entryPoint.c_str(), shaderModel.c_str(), ppResult);
    #endif
#else
    std::string entryPoint = "main";
    std::string shaderModel = "vs_5_0";
    UINT flags = 0;

    for(auto i=0u; i<countCompileArgs; ++i)
    {
        // エントリーポイント.
        if (wcscmp(compileArgs[i], L"-E") == 0)
        {
            i++;
            entryPoint = asdx::ToStringA(compileArgs[i]);
        }
        // シェーダプロファイル.
        else if (wcscmp(compileArgs[i], L"-T") == 0)
        {
            i++;
            shaderModel = asdx::ToStringA(compileArgs[i]);
        }
    }

    return CompileFromFile(filename, entryPoint.c_str(), shaderModel.c_str(), ppResult);
#endif
}

} // namespace asdx