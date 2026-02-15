//-----------------------------------------------------------------------------
// File : EditMaterial.cpp
// Desc : Material For Edit Data.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "EditMaterial.h"
#include <cstdio>
#include <simdjson.h>
#include <d3d12shader.h>
#include <dxcapi.h>
#include <wrl/client.h>


template<typename T>
using RefPtr = Microsoft::WRL::ComPtr<T>;

#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File: %s, Line: %d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG

namespace {

//-----------------------------------------------------------------------------
//      ブレンドステートを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(asdx::edit::BlendState state)
{
    switch(state)
    {
    case asdx::edit::BlendState::Opaque:
    default:
        return "Opaque";

    case asdx::edit::BlendState::AlphaBlend:
        return "AlphaBlend";

    case asdx::edit::BlendState::Additive:
        return "Additive";

    case asdx::edit::BlendState::Subtract:
        return "Subtract";

    case asdx::edit::BlendState::Premultiplied:
        return "Premultiplied";

    case asdx::edit::BlendState::Multiply:
        return "Multiply";

    case asdx::edit::BlendState::Screen:
        return "Screen";
    }
}

//-----------------------------------------------------------------------------
//      文字列からブレンドステートに変換します.
//-----------------------------------------------------------------------------
asdx::edit::BlendState ToBlendState(const char* state)
{
    if (_stricmp(state, "Opaque") == 0)
    {
        return asdx::edit::BlendState::Opaque;
    }
    else if (_stricmp(state, "AlphaBlend") == 0)
    {
        return asdx::edit::BlendState::AlphaBlend;
    }
    else if (_stricmp(state, "Additive") == 0)
    {
        return asdx::edit::BlendState::Additive;
    }
    else if (_stricmp(state, "Subtract") == 0)
    {
        return asdx::edit::BlendState::Subtract;
    }
    else if (_stricmp(state, "Premultiplied") == 0)
    {
        return asdx::edit::BlendState::Premultiplied;
    }
    else if (_stricmp(state, "Multiply") == 0)
    {
        return asdx::edit::BlendState::Multiply;
    }
    else if (_stricmp(state, "Screen") == 0)
    {
        return asdx::edit::BlendState::Screen;
    }

    return asdx::edit::BlendState::Opaque;
}

//-----------------------------------------------------------------------------
//      深度ステートから文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(asdx::edit::DepthState state)
{
    switch(state)
    {
    case asdx::edit::DepthState::ReadWrite:
        return "ReadWrite";

    case asdx::edit::DepthState::ReadOnly:
        return "ReadOnly";

    case asdx::edit::DepthState::WriteOnly:
        return "WriteOnly";

    case asdx::edit::DepthState::None:
        return "None";
    }

    return "ReadWrite";
}

//-----------------------------------------------------------------------------
//      文字列から深度ステートに変換します.
//-----------------------------------------------------------------------------
asdx::edit::DepthState ToDepthState(const char* state)
{
    if (_stricmp(state, "ReadWrite") == 0)
    {
        return asdx::edit::DepthState::ReadWrite;
    }
    else if (_stricmp(state, "ReadOnly") == 0)
    {
        return asdx::edit::DepthState::ReadOnly;
    }
    else if (_stricmp(state, "WriteOnly") == 0)
    {
        return asdx::edit::DepthState::WriteOnly;
    }
    else if (_stricmp(state, "None") == 0)
    {
        return asdx::edit::DepthState::None;
    }

    return asdx::edit::DepthState::ReadWrite;
}

//-----------------------------------------------------------------------------
//      ラスタライザーステートを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(asdx::edit::RasterizerState state)
{
    switch(state)
    {
    case asdx::edit::RasterizerState::CullNone:
        return "CullNone";

    case asdx::edit::RasterizerState::CullBack:
        return "CullBack";

    case asdx::edit::RasterizerState::CullFront:
        return "CullFront";

    case asdx::edit::RasterizerState::Wireframe:
        return "Wireframe";
    }

    return "CullNone";
}

//-----------------------------------------------------------------------------
//      文字列からラスタライザーステートに変換します.
//-----------------------------------------------------------------------------
asdx::edit::RasterizerState ToRasterizerState(const char* state)
{
    if (_stricmp(state, "CullNone") == 0)
    {
        return asdx::edit::RasterizerState::CullNone;
    }
    else if (_stricmp(state, "CullBack") == 0)
    {
        return asdx::edit::RasterizerState::CullBack;
    }
    else if (_stricmp(state, "CullFront") == 0)
    {
        return asdx::edit::RasterizerState::CullFront;
    }
    else if (_stricmp(state, "Wireframe") == 0)
    {
        return asdx::edit::RasterizerState::Wireframe;
    }

    return asdx::edit::RasterizerState::CullNone;
}

//-----------------------------------------------------------------------------
//      シェーダリフレクションを生成します.
//-----------------------------------------------------------------------------
HRESULT CreateShaderReflectionOld(const void* pData, size_t size, ID3D12ShaderReflection** ppResult)
{
    // DirectX ShaderCompiler before March 2020.

    const uint32_t kDFCC_DXIL  = DXC_FOURCC('D', 'X', 'I', 'L');

    RefPtr<IDxcLibrary> pLibrary;
    auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(pLibrary.GetAddressOf()));
    if (FAILED(hr))
    {
        ELOG("Error : DxcCreateInstance() Failed. errcode = 0x%x", hr);
        return hr;
    }

    RefPtr<IDxcBlobEncoding> blobEncoding;
    hr = pLibrary->CreateBlobWithEncodingOnHeapCopy(pData, UINT32(size), CP_ACP, blobEncoding.GetAddressOf());
    if (FAILED(hr))
    {
        ELOG("Error : IDxcLibrary::CreateBlobWithEncodingOnHeapCopy() Faield. errcode = 0x%x", hr);
        return hr;
    }

    RefPtr<IDxcContainerReflection> containerReflection;
    hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(containerReflection.GetAddressOf()));
    if (FAILED(hr))
    {
        ELOG("Error : DxcCreateInstance() Failed. errcode = 0x%x", hr);
        return hr;
    }

    uint32_t shaderIdx = 0;
    hr = containerReflection->Load(blobEncoding.Get());
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

bool CreateShaderReflection(const void* pData, size_t size, ID3D12ShaderReflection** ppReflection)
{
    RefPtr<IDxcUtils> pUtil;
    auto hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtil.GetAddressOf()));
    if (FAILED(hr))
    {
        ELOG("Error : DxcCreateInstance() Failed. errcode = 0x%x", hr);
        return false;
    }

    DxcBuffer buf = {};
    buf.Ptr      = pData;
    buf.Size     = size;
    buf.Encoding = CP_UTF8;

    hr = pUtil->CreateReflection(&buf, IID_PPV_ARGS(ppReflection));
    if (FAILED(hr))
    {
        if (hr == E_NOINTERFACE)
        {
            hr = CreateShaderReflectionOld(pData, size, ppReflection);
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
}

asdx::edit::MaterialParamType GetMaterialParamType(const D3D12_SHADER_TYPE_DESC& desc)
{
    switch(desc.Type)
    {
    case D3D_SVT_BOOL:
        {
            if (desc.Class == D3D_SVC_VECTOR)
            {
                if (desc.Columns == 1)
                    return asdx::edit::MaterialParamType::Bool;

                else if (desc.Columns == 2)
                    return asdx::edit::MaterialParamType::Bool2;

                else if (desc.Columns == 3)
                    return asdx::edit::MaterialParamType::Bool3;

                else if (desc.Columns == 4)
                    return asdx::edit::MaterialParamType::Bool4;
            }

            return asdx::edit::MaterialParamType::Bool;
        }

    case D3D_SVT_INT:
        {
            if (desc.Class == D3D_SVC_VECTOR)
            {
                if (desc.Columns == 1)
                    return asdx::edit::MaterialParamType::Int;

                else if (desc.Columns == 2)
                    return asdx::edit::MaterialParamType::Int2;

                else if (desc.Columns == 3)
                    return asdx::edit::MaterialParamType::Int3;

                else if (desc.Columns == 4)
                    return asdx::edit::MaterialParamType::Int4;
            }

            return asdx::edit::MaterialParamType::Int;
        }


    case D3D_SVT_UINT:
        {
            if (desc.Class == D3D_SVC_VECTOR)
            {
                if (desc.Columns == 1)
                    return asdx::edit::MaterialParamType::Uint;

                else if (desc.Columns == 2)
                    return asdx::edit::MaterialParamType::Uint2;

                else if (desc.Columns == 3)
                    return asdx::edit::MaterialParamType::Uint3;

                else if (desc.Columns == 4)
                    return asdx::edit::MaterialParamType::Uint4;
            }

            return asdx::edit::MaterialParamType::Uint;
        }

    case D3D_SVT_FLOAT:
        {
            if (desc.Class == D3D_SVC_VECTOR)
            {
                if (desc.Columns == 1)
                    return asdx::edit::MaterialParamType::Float;

                else if (desc.Columns == 2)
                    return asdx::edit::MaterialParamType::Float2;

                else if (desc.Columns == 3)
                    return asdx::edit::MaterialParamType::Float3;

                else if (desc.Columns == 4)
                    return asdx::edit::MaterialParamType::Float4;
            }

            return asdx::edit::MaterialParamType::Float;
        }
    }

    return asdx::edit::MaterialParamType::Float;
}

} // namespace

namespace asdx::edit {

//-----------------------------------------------------------------------------
//      マテリアルを json ファイルに保存します.
//-----------------------------------------------------------------------------
bool SaveToJson(const char* path, const Material& material)
{
    FILE* fp = nullptr;
    auto err = fopen_s(&fp, path, "w");
    if (err != 0)
    {
        ELOG("Error : File Open Failed. path = %s", path);
        return false;
    }

    fprintf_s(fp, "{\n");
    fprintf_s(fp, "    \"Name\": \"%s\",\n", material.Name.c_str());
    fprintf_s(fp, "    \"State\": {\n");
    fprintf_s(fp, "         \"Blend\": \"%s\",\n", ToString(material.State.Blend));
    fprintf_s(fp, "         \"Depth\": \"%s\",\n", ToString(material.State.Depth));
    fprintf_s(fp, "         \"Rasterizer\": \"%s\",\n", ToString(material.State.Rasterizer));
    fprintf_s(fp, "         \"UserFlag\": %u,\n", material.State.UserFlag);
    fprintf_s(fp, "     }");
    if (!material.Textures.empty())
    {
        fprintf_s(fp, ",\n");
        fprintf_s(fp, "    \"Textures\": [\n");
        for(size_t i=0; i<material.Textures.size(); ++i)
        {
            auto& tex = material.Textures[i];
            fprintf_s(fp, "        {\n");
            fprintf_s(fp, "             \"Name\": \"%s\",\n", tex.Name.c_str());
            fprintf_s(fp, "             \"Path\": \"%s\"\n", tex.Path.c_str());
            fprintf_s(fp, "        }");

            if (i != material.Textures.size() - 1)
            { fprintf_s(fp, ",\n"); }
            else
            { fprintf_s(fp, "\n"); }
        }

        fprintf_s(fp, "     ]");
    }
    if (!material.Buffers.empty())
    {
        fprintf_s(fp, ",\n");
        fprintf_s(fp, "    \"Buffers\": [\n");
        for(size_t i=0; i<material.Buffers.size(); ++i)
        {
            auto& buf = material.Buffers[i];
            fprintf_s(fp, "        {\n");
            fprintf_s(fp, "            \"Name\": \"%s\"", buf.Name.c_str());
            if (!buf.Buffer.empty())
            {
                fprintf_s(fp, ",\n");
                fprintf_s(fp, "            \"Buffer\": [\n");
                for(size_t j=0; j<buf.Buffer.size(); ++j)
                {
                    fprintf(fp, "                %u", buf.Buffer[j]);

                    if (j != buf.Buffer.size() - 1)
                    { fprintf_s(fp, ",\n"); }
                    else
                    { fprintf_s(fp, "\n"); }
                }

                fprintf_s(fp, "            ]");
            }


            if (!buf.Params.empty())
            {
                fprintf_s(fp, ",\n");

                fprintf_s(fp, "             \"Params\": [\n");
                for(size_t j=0; j<buf.Params.size(); ++j)
                {
                    auto& param = buf.Params[j];
                    fprintf_s(fp, "                  {\n");
                    fprintf_s(fp, "                       \"Name\": \"%s\",\n", param.Name.c_str());
                    fprintf_s(fp, "                       \"Type\": %u,\n", (uint8_t)param.Type);
                    fprintf_s(fp, "                       \"Offset\": %u,\n", param.Offset);
                    fprintf_s(fp, "                       \"ArraySize\": %u,\n", param.ArraySize);
                    fprintf_s(fp, "                  }");

                    if (j != buf.Params.size() - 1)
                    { fprintf_s(fp, ",\n"); }
                    else
                    { fprintf_s(fp, "\n"); }
                }
                fprintf_s(fp, "             ]\n");
            }

            fprintf_s(fp, "        }");

            if (i != material.Buffers.size() - 1)
            { fprintf_s(fp, ",\n"); }
            else
            { fprintf_s(fp, "\n"); }
        }
        fprintf_s(fp, "    ]");

        // 次のデータ無いのでここで改行.
        fprintf_s(fp, "\n");
    }

    fprintf_s(fp, "}\n");

    return true;
}

//-----------------------------------------------------------------------------
//      json ファイルからマテリアルを読込します.
//-----------------------------------------------------------------------------
bool LoadFromJson(const char* path, Material& material)
{
    simdjson::ondemand::parser parser;

    auto json = simdjson::padded_string::load(path);
    if (json.error() != simdjson::SUCCESS)
    {
        ELOG("Error : File Load Failed. path = %s", path);
        return false;
    }

    auto doc = parser.iterate(json);
    if (json.error() != simdjson::SUCCESS)
    {
        ELOG("Error : simdjson parser error.");
        return false;
    }

    auto name = doc["Name"];
    if (name.error() == simdjson::SUCCESS)
    {
        std::string_view str;
        name.get(str);
        material.Name = str;
    }

    auto state = doc["State"];
    if (state.error() == simdjson::SUCCESS)
    {
        auto blend = state["Blend"];
        if (blend.error() == simdjson::SUCCESS)
        {
            std::string_view str;
            blend.get(str);
            material.State.Blend = ToBlendState(str.data());
        }

        auto depth = state["Depth"];
        if (depth.error() == simdjson::SUCCESS)
        {
            std::string_view str;
            depth.get(str);
            material.State.Depth = ToDepthState(str.data());
        }

        auto rasterizer = state["Rasterizer"];
        if (rasterizer.error() == simdjson::SUCCESS)
        {
            std::string_view str;
            rasterizer.get(str);
            material.State.Rasterizer = ToRasterizerState(str.data());
        }

        auto userFlag = state["UserFlag"];
        if (userFlag.error() == simdjson::SUCCESS)
        {
            material.State.UserFlag = uint8_t(userFlag.get_uint64());
        }
    }

    auto textures = doc["Textures"];
    if (textures.error() == simdjson::SUCCESS)
    {
        for(auto tex : textures.get_array())
        {
            MaterialTexture item = {};

            auto name = tex["Name"];
            if (name.error() == simdjson::SUCCESS)
            {
                std::string_view str;
                name.get(str);
                item.Name = str.data();
            }

            auto path = tex["Path"];
            if (path.error() == simdjson::SUCCESS)
            {
                std::string_view str;
                path.get(str);
                item.Path = str.data();
            }

            material.Textures.emplace_back(item);
        }
    }

    auto buffers = doc["Buffers"];
    if (buffers.error() == simdjson::SUCCESS)
    {
        for(auto buf : buffers.get_array())
        {
            MaterialBuffer item = {};

            auto name = buf["Name"];
            if (name.error() == simdjson::SUCCESS)
            {
                std::string_view str;
                name.get(str);
                item.Name = str.data();
            }

            auto blob = buf["Buffer"];
            if (blob.error() == simdjson::SUCCESS)
            {
                for(auto val : blob)
                {
                    item.Buffer.push_back(uint8_t(val.get_uint64()));
                }
            }

            auto params = buf["Params"];
            if (params.error() == simdjson::SUCCESS)
            {
                for(auto param : params.get_array())
                {
                    MaterialParam p = {};
                    p.Type      = MaterialParamType::Float;
                    p.ArraySize = 1;

                    auto tag = param["Name"];
                    if (tag.error() == simdjson::SUCCESS)
                    {
                        std::string_view str;
                        tag.get(str);
                        p.Name = str.data();
                    }

                    auto type = param["Type"];
                    if (type.error() == simdjson::SUCCESS)
                    {
                        p.Type = (MaterialParamType)(uint8_t(type.get_uint64()));
                    }

                    auto offset = param["Offset"];
                    if (offset.error() == simdjson::SUCCESS)
                    {
                        p.Offset = uint32_t(type.get_uint64());
                    }

                    auto arraySize = param["ArraySize"];
                    if (arraySize.error() == simdjson::SUCCESS)
                    {
                        p.ArraySize = uint32_t(type.get_uint64());
                    }

                    item.Params.emplace_back(p);
                }
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      シェーダからマテリアルを初期化します.
//-----------------------------------------------------------------------------
bool InitFromShader(const char* path, Material& material)
{
    // コンパイル済みファイルを前提とします.
    FILE* fp = nullptr;
    auto err = fopen_s(&fp, path, "rb");
    if (err != 0)
    {
        ELOG("Error : File Open Failed. path = %s", path);
        return false;
    }

    auto curPos = ftell(fp);
    fseek(fp, SEEK_END, 0);
    auto endPos = ftell(fp);
    fseek(fp, SEEK_SET, 0);

    auto size = endPos - curPos;
    std::vector<uint8_t> blob;
    blob.resize(size);

    fread(blob.data(), size, 1, fp);
    fclose(fp);

    material.PixelShader = path;

    return InitFromShader(blob.data(), blob.size(), material);
}

//-----------------------------------------------------------------------------
//      シェーダからマテリアルを初期化します.
//-----------------------------------------------------------------------------
bool InitFromShader(const void* buffer, size_t bufferSize, Material& material)
{
    RefPtr<ID3D12ShaderReflection> pReflection;
    if (!CreateShaderReflection(buffer, bufferSize, pReflection.GetAddressOf()))
    {
        ELOG("CreateShaderReflection() Failed.");
        return false;
    }

    D3D12_SHADER_DESC desc = {};
    auto hr = pReflection->GetDesc(&desc);
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Reflection::GetDesc() Failed. errcode = 0x%x", hr);
        return false;
    }

    // 名前に "Material" とつくものを対象とする.

    // バッファデータを解析.
    for(auto i=0u; i<desc.ConstantBuffers; ++i)
    {
        auto pConstantBuffer = pReflection->GetConstantBufferByIndex(i);
        if (pConstantBuffer == nullptr)
            continue;

        D3D12_SHADER_BUFFER_DESC bufDesc = {};
        hr = pConstantBuffer->GetDesc(&bufDesc);
        if (FAILED(hr))
            continue;

        if (strstr(bufDesc.Name, "Material") == nullptr)
            continue;

        MaterialBuffer matBuffer = {};
        matBuffer.Name = bufDesc.Name;
        matBuffer.Buffer.resize(bufDesc.Size);

        for(auto j=0u; j<bufDesc.Variables; ++j)
        {
            auto pVariable = pConstantBuffer->GetVariableByIndex(j);
            if (pVariable == nullptr)
                continue;

            D3D12_SHADER_VARIABLE_DESC varDesc = {};
            hr = pVariable->GetDesc(&varDesc);
            if (FAILED(hr))
                continue;

            MaterialParam matParam = {};
            matParam.Name   = varDesc.Name;
            matParam.Offset = varDesc.StartOffset;

            if (varDesc.DefaultValue != nullptr)
            {
                memcpy(matBuffer.Buffer.data() + varDesc.StartOffset, varDesc.DefaultValue, varDesc.Size);
            }

            auto pVarType = pVariable->GetType();
            assert(pVarType != nullptr);

            D3D12_SHADER_TYPE_DESC typeDesc = {};
            hr = pVarType->GetDesc(&typeDesc);
            assert(SUCCEEDED(hr));

            matParam.ArraySize = typeDesc.Elements;
            matParam.Type      = GetMaterialParamType(typeDesc);

            matBuffer.Params.emplace_back(matParam);
        }

        material.Buffers.emplace_back(matBuffer);
    }

    // テクスチャを解析.
    for(auto i=0u; i<desc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC inputDesc = {};
        hr = pReflection->GetResourceBindingDesc(i, &inputDesc);
        if (FAILED(hr))
            continue;

        if (strstr(inputDesc.Name, "Material") == nullptr)
            continue;

        // テクスチャ以外は無視.
        if (inputDesc.Type != D3D_SIT_TEXTURE)
            continue;

        MaterialTexture matTex = {};
        matTex.Name = inputDesc.Name;

        material.Textures.emplace_back(matTex);
    }

    return true;
}

} // namespace asdx::edit
