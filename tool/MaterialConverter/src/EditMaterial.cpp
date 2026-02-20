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
    fprintf_s(fp, "         \"BlendState\": \"%s\",\n", ToString(material.BlendState));
    fprintf_s(fp, "         \"DepthState\": \"%s\",\n", ToString(material.DepthState));
    fprintf_s(fp, "         \"RasterizerState\": \"%s\",\n", ToString(material.RasterizerState));
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
    if (!material.Buffer.empty())
    {
        fprintf_s(fp, ",\n");
        fprintf_s(fp, "    \"Buffer\": {\n");
        fprintf_s(fp, "        \"Size\": %zu,\n", material.Buffer.size());
        fprintf_s(fp, "        \"Data\": [\n");
        for(size_t i=0; i<material.Buffer.size(); ++i)
        {
            fprintf_s(fp, "        %u", material.Buffer[i]);
            if (i != material.Buffer.size() - 1)
            {
                fprintf_s(fp, ",\n");
            }
            else
            {
                fprintf_s(fp, "\n");
            }
        }
        fprintf_s(fp, "        ]\n");
        fprintf_s(fp, "    }");
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

    auto blend = doc["BlendState"];
    if (blend.error() == simdjson::SUCCESS)
    {
        std::string_view str;
        blend.get(str);
        material.BlendState = ToBlendState(str.data());
    }

    auto depth = doc["Depth"];
    if (depth.error() == simdjson::SUCCESS)
    {
        std::string_view str;
        depth.get(str);
        material.DepthState = ToDepthState(str.data());
    }

    auto rasterizer = doc["Rasterizer"];
    if (rasterizer.error() == simdjson::SUCCESS)
    {
        std::string_view str;
        rasterizer.get(str);
        material.RasterizerState = ToRasterizerState(str.data());
    }

    auto buffer = doc["Buffer"];
    if (buffer.error() == simdjson::SUCCESS)
    {
        auto size = buffer["Size"];
        if (size.error() == simdjson::SUCCESS)
        {
            material.Buffer.resize(size.get_uint64());
        }

        auto data = buffer["Data"];
        if (data.error() == simdjson::SUCCESS)
        {
            auto i=0u;
            for(auto val : data.get_array())
            {
                material.Buffer[i] = uint8_t(val.get_uint64());
                i++;
            }
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


    return true;
}

//-----------------------------------------------------------------------------
//      マテリアルを初期化します.
//-----------------------------------------------------------------------------
bool InitMaterial(MaterialKind kind, Material& material)
{
    auto ret = false;
    switch(kind)
    {
    case MaterialKind::Lambert:
        {
            material.Kind = uint32_t(kind);

            material.BlendState      = BlendState::Opaque;
            material.DepthState      = DepthState::ReadWrite;
            material.RasterizerState = RasterizerState::CullBack;

            ParamLambert param = {};
            material.Buffer.resize(sizeof(param));
            memcpy(material.Buffer.data(), &param, sizeof(param));

            material.Textures.resize(3);
            material.Textures[0].Name = "BaseColorMap";
            material.Textures[1].Name = "NormalMap";
            material.Textures[2].Name = "EmissiveMap";
            ret = true;
        }
        break;

    case MaterialKind::GGX:
        {
            material.Kind = uint32_t(kind);

            material.BlendState      = BlendState::Opaque;
            material.DepthState      = DepthState::ReadWrite;
            material.RasterizerState = RasterizerState::CullBack;

            ParamGGX param = {};
            material.Buffer.resize(sizeof(param));
            memcpy(material.Buffer.data(), &param, sizeof(param));

            material.Textures.resize(4);
            material.Textures[0].Name = "BaseColorMap";
            material.Textures[1].Name = "NormalMap";
            material.Textures[2].Name = "OrmMap";
            material.Textures[3].Name = "EmissiveMap";
            ret = true;
        }
        break;

    case MaterialKind::Anisotropy:
        {
            material.Kind = uint32_t(kind);

            material.BlendState      = BlendState::Opaque;
            material.DepthState      = DepthState::ReadWrite;
            material.RasterizerState = RasterizerState::CullBack;

            ParamAnisotropy param = {};
            material.Buffer.resize(sizeof(param));
            memcpy(material.Buffer.data(), &param, sizeof(param));

            material.Textures.resize(4);
            material.Textures[0].Name = "BaseColorMap";
            material.Textures[1].Name = "NormalMap";
            material.Textures[2].Name = "OrmMap";
            material.Textures[3].Name = "EmissiveMap";
            ret = true;
        }
        break;

    case MaterialKind::ClearCoat:
        {
            material.Kind = uint32_t(kind);

            material.BlendState      = BlendState::Opaque;
            material.DepthState      = DepthState::ReadWrite;
            material.RasterizerState = RasterizerState::CullBack;

            ParamClearCoat param = {};
            material.Buffer.resize(sizeof(param));
            memcpy(material.Buffer.data(), &param, sizeof(param));

            material.Textures.resize(7);
            material.Textures[0].Name = "BaseColorMap";
            material.Textures[1].Name = "NormalMap";
            material.Textures[2].Name = "OrmMap";
            material.Textures[3].Name = "ClearCoatMap";
            material.Textures[4].Name = "ClearCoatRoughnessMap";
            material.Textures[5].Name = "ClearCoatNormalMap";
            material.Textures[6].Name = "EmissiveMap";
            ret = true;
        }
        break;

    case MaterialKind::Sheen:
        {
            material.Kind = uint32_t(kind);

            material.BlendState      = BlendState::Opaque;
            material.DepthState      = DepthState::ReadWrite;
            material.RasterizerState = RasterizerState::CullBack;

            ParamSheen param = {};
            material.Buffer.resize(sizeof(param));
            memcpy(material.Buffer.data(), &param, sizeof(param));

            material.Textures.resize(6);
            material.Textures[0].Name = "BaseColorMap";
            material.Textures[1].Name = "NormalMap";
            material.Textures[2].Name = "OrmMap";
            material.Textures[3].Name = "SheenColorMap";
            material.Textures[4].Name = "SheenRoughnessMap";
            material.Textures[5].Name = "EmissiveMap";
            ret = true;
        }
        break;

    case MaterialKind::Iridescence:
        {
            material.Kind = uint32_t(kind);

            material.BlendState      = BlendState::Opaque;
            material.DepthState      = DepthState::ReadWrite;
            material.RasterizerState = RasterizerState::CullBack;

            ParamIridescence param = {};
            material.Buffer.resize(sizeof(param));
            memcpy(material.Buffer.data(), &param, sizeof(param));

            material.Textures.resize(6);
            material.Textures[0].Name = "BaseColorMap";
            material.Textures[1].Name = "NormalMap";
            material.Textures[2].Name = "OrmMap";
            material.Textures[3].Name = "IridescenceMap";
            material.Textures[4].Name = "IridescenceThicknessMap";
            material.Textures[5].Name = "EmissiveMap";
            ret = true;
        }
        break;

    case MaterialKind::Transmission:
        {
            material.Kind = uint32_t(kind);

            material.BlendState      = BlendState::AlphaBlend;
            material.DepthState      = DepthState::ReadOnly;
            material.RasterizerState = RasterizerState::CullNone;

            ParamIridescence param = {};
            material.Buffer.resize(sizeof(param));
            memcpy(material.Buffer.data(), &param, sizeof(param));

            material.Textures.resize(5);
            material.Textures[0].Name = "BaseColorMap";
            material.Textures[1].Name = "NormalMap";
            material.Textures[2].Name = "OrmMap";
            material.Textures[3].Name = "TransmissionMap";
            material.Textures[4].Name = "EmissiveMap";
            ret = true;
        }
        break;

    default:
        break;
    }

    return ret;
}


} // namespace asdx::edit
