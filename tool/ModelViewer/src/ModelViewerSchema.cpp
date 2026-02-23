//-----------------------------------------------------------------------------
// File : ModelViewerSchema.cpp
// Desc : Material Schema Loader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <ModelViewer.h>
#include <simdjson.h>
#include <fnd/asdxLogger.h>
#include <gfx/asdxMaterial.h>
#include <gfx/asdxShaderCompiler.h>


//-----------------------------------------------------------------------------
//      jsonファイルからロードします.
//-----------------------------------------------------------------------------
bool ModelViewer::LoadMaterialSchema(const char* path)
{
    if (path == nullptr)
        return false;

    struct PsoDesc
    {
        viewer::MaterialBlendState      BlendState;
        viewer::MaterialDepthState      DepthState;
        viewer::MaterialRasterizerState RasterizerState;
        std::string                     ShaderPath;
    };

    std::vector<PsoDesc> psoDescs;
    std::vector<asdx::MaterialSchema::KindDef> kinds;

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

    auto materials = doc["Materials"];
    if (materials.error() != simdjson::SUCCESS)
    {
        ELOG("Error : Materials Not found.");
        return false;
    }

    for(auto material : materials.get_array())
    {
        asdx::MaterialSchema::KindDef def = {};

        auto kindName = doc["KindName"];
        if (kindName.error() == simdjson::SUCCESS)
        {
            def.KindName = kindName.get_string().value();
        }

        auto kindId = doc["KindId"];
        if (kindId.error() == simdjson::SUCCESS)
        {
            def.KindId = uint32_t(kindId.get_uint64().value());
        }

        auto pso = doc["PipelineState"];
        if (pso.error() == simdjson::SUCCESS)
        {
            PsoDesc desc = {};

            auto blendState = pso["BlendState"];
            if (blendState.error() == simdjson::SUCCESS)
            {
                auto tag = blendState.get_string().value();
                if (tag == "Opaque")
                {
                    desc.BlendState = viewer::MaterialBlendState::Opaque;
                }
                else if (tag == "AlphaBlend")
                {
                    desc.BlendState = viewer::MaterialBlendState::AlphaBlend;
                }
                else if (tag == "Additive")
                {
                    desc.BlendState = viewer::MaterialBlendState::Additive;
                }
                else if (tag == "Subtract")
                {
                    desc.BlendState = viewer::MaterialBlendState::Subtract;
                }
                else if (tag == "Premultiplied")
                {
                    desc.BlendState = viewer::MaterialBlendState::Premultiplied;
                }
                else if (tag == "Multiply")
                {
                    desc.BlendState = viewer::MaterialBlendState::Multiply;
                }
                else if (tag == "Screen")
                {
                    desc.BlendState = viewer::MaterialBlendState::Screen;
                }
            }

            auto depthState = pso["DepthState"];
            if (depthState.error() == simdjson::SUCCESS)
            {
                auto tag = depthState.get_string().value();
                if (tag == "ReadWrite")
                {
                    desc.DepthState = viewer::MaterialDepthState::ReadWrite;
                }
                else if (tag == "ReadOnly")
                {
                    desc.DepthState = viewer::MaterialDepthState::ReadOnly;
                }
                else if (tag == "WriteOnly")
                {
                    desc.DepthState = viewer::MaterialDepthState::WriteOnly;
                }
                else if (tag == "None")
                {
                    desc.DepthState = viewer::MaterialDepthState::None;
                }
            }

            auto rasterizerState = pso["RasterizerState"];
            if (rasterizerState.error() == simdjson::SUCCESS)
            {
                auto tag = rasterizerState.get_string().value();
                if (tag == "CullNone")
                {
                    desc.RasterizerState = viewer::MaterialRasterizerState::CullNone;
                }
                else if (tag == "CullBack")
                {
                    desc.RasterizerState = viewer::MaterialRasterizerState::CullBack;
                }
                else if (tag == "CullFront")
                {
                    desc.RasterizerState = viewer::MaterialRasterizerState::CullFront;
                }
                else if (tag == "Wireframe")
                {
                    desc.RasterizerState = viewer::MaterialRasterizerState::Wireframe;
                }
            }

            auto shader = pso["Shader"];
            if (shader.error() == simdjson::SUCCESS)
            {
                auto path = shader.get_string().value();
                desc.ShaderPath = path;
            }

            psoDescs.emplace_back(desc);
        }

        auto bufSize = doc["BufferSize"];
        if (bufSize.error() == simdjson::SUCCESS)
        {
            def.BufferSize = uint32_t(bufSize.get_uint64().value());
        }

        auto params = doc["Params"];
        if (params.error() == simdjson::SUCCESS)
        {
            for(auto param : params.get_array())
            {
                asdx::MaterialSchema::ParamDef paramDef = {};

                auto name = param["Name"];
                if (name.error() == simdjson::SUCCESS)
                {
                    paramDef.Name = name.get_string().value();
                }

                auto offset = param["Offset"];
                if (offset.error() == simdjson::SUCCESS)
                {
                    paramDef.Offset = uint32_t(offset.get_uint64().value());
                }

                auto defaultValue = param["Default"];
                if (defaultValue.error() == simdjson::SUCCESS)
                {
                    paramDef.Default = float(defaultValue.get_double().value());
                }

                def.Params.emplace_back(paramDef);
            }
        }

        auto textures = doc["Textures"];
        if (textures.error() == simdjson::SUCCESS)
        {
            for(auto texture : textures.get_array())
            {
                asdx::MaterialSchema::TextureDef texDef = {};

                auto name = texture["Name"];
                if (name.error() == simdjson::SUCCESS)
                {
                    texDef.Name = name.get_string().value();
                }

                auto offset = texture["Index"];
                if (offset.error() == simdjson::SUCCESS)
                {
                    texDef.Index = uint32_t(offset.get_uint64().value());
                }

                auto defaultPath = texture["Default"];
                if (defaultPath.error() == simdjson::SUCCESS)
                {
                    texDef.Default = defaultPath.get_string().value();
                }

                def.Textures.emplace_back(texDef);
            }
        }

        kinds.emplace_back(def);
    }

    std::vector<std::string> includeDirs;
    includeDirs.push_back("../../../res/shaders");
    includeDirs.push_back("../res/shaders");

    auto count = kinds.size();
    m_PipelineStates.reserve(count);
    for(size_t i=0; i<count; ++i)
    {
        const auto& info = psoDescs[i];

        // シェーダをコンパイル.
        std::vector<uint8_t> blob;
        if (!asdx::CompileFromFileA(info.ShaderPath.c_str(), includeDirs, "main", "ps_6_6", blob))
        {
            ELOG("Error : PixelShader Compile Failed. path = %s", info.ShaderPath.c_str());
            return false;
        }

        D3D12_SHADER_BYTECODE pixelShader;
        pixelShader.pShaderBytecode = blob.data();
        pixelShader.BytecodeLength  = blob.size();

        // パイプラインステートを生成.
        if (!CreateModelPipelineState(pixelShader, info.BlendState, info.DepthState, info.RasterizerState, m_PipelineStates[i]))
        {
            ELOG("Error : CreateModelPipelineState() Failed. Index = %u", i);
            return false;
        }
    }

    // スキーマを再初期化.
    asdx::MaterialSchema::Term();
    asdx::MaterialSchema::Init(kinds);

    // 差し替え.
    m_Kinds = std::move(kinds);

    return true;
}
