//-------------------------------------------------------------------------------------------------
// File : FxParser.cpp
// Desc : Shader Effect Parser
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <gfx/asdxFxParser.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <cstdio>
#include <cassert>
#include <fstream>


namespace asdx {

//-----------------------------------------------------------------------------
//      シェーダタイプに対応する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(SHADER_TYPE value)
{
    switch(value)
    {
        case SHADER_TYPE_VERTEX:
            return "vertex";

        case SHADER_TYPE_GEOMETRY:
            return "geometry";

        case SHADER_TYPE_DOMAIN:
            return "domain";

        case SHADER_TYPE_HULL:
            return "hull";

        case SHADER_TYPE_PIXEL:
            return "pixel";

        case SHADER_TYPE_COMPUTE:
            return "compute";

        case SHADER_TYPE_AMPLIFICATION:
            return "amplification";

        case SHADER_TYPE_MESH:
            return "mesh";
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
//      ポリゴンモードに対応する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_FILL_MODE mode)
{
    switch(mode)
    {
    default:
    case D3D12_FILL_MODE_SOLID:
        return "solid";

    case D3D12_FILL_MODE_WIREFRAME:
        return "wireframe";
    }
}

//-----------------------------------------------------------------------------
//      カリングタイプに対応する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_CULL_MODE type)
{
    switch(type)
    {
    default:
    case D3D12_CULL_MODE_NONE:
        return "none";

    case D3D12_CULL_MODE_FRONT:
        return "front";

    case D3D12_CULL_MODE_BACK:
        return "back";
    }
}

//-----------------------------------------------------------------------------
//      ブレンドタイプに対応する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_BLEND type)
{
    switch(type)
    {
    default:
    case D3D12_BLEND_ZERO:
        return "zero";

    case D3D12_BLEND_ONE:
        return "one";

    case D3D12_BLEND_SRC_COLOR:
        return "src_color";

    case D3D12_BLEND_INV_SRC_COLOR:
        return "inv_src_color";

    case D3D12_BLEND_SRC_ALPHA:
        return "src_alpha";

    case D3D12_BLEND_INV_SRC_ALPHA:
        return "inv_src_alpha";

    case D3D12_BLEND_DEST_ALPHA:
        return "dst_alpha";

    case D3D12_BLEND_INV_DEST_ALPHA:
        return "inv_dst_alpha";

    case D3D12_BLEND_DEST_COLOR:
        return "dst_color";

    case D3D12_BLEND_INV_DEST_COLOR:
        return "inv_dst_color";
    }
}

//-----------------------------------------------------------------------------
//      フィルタモードに対応する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(FILTER_MODE type)
{
    switch(type)
    {
    default:
    case FILTER_MODE_NEAREST:
        return "nearest";

    case FILTER_MODE_LINEAR:
        return "linear";
    }
}

//-----------------------------------------------------------------------------
//      ミップマップモードに対する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(MIPMAP_MODE type)
{
    switch(type)
    {
    default:
    case MIPMAP_MODE_NEAREST:
        return "nearest";

    case MIPMAP_MODE_LINEAR:
        return "linear";

    case MIPMAP_MODE_NONE:
        return "none";
    }
}

//-----------------------------------------------------------------------------
//      アドレスモードに対応する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_TEXTURE_ADDRESS_MODE type)
{
    switch(type)
    {
    default:
    case D3D12_TEXTURE_ADDRESS_MODE_WRAP:
        return "wrap";

    case D3D12_TEXTURE_ADDRESS_MODE_CLAMP:
        return "clamp";

    case D3D12_TEXTURE_ADDRESS_MODE_MIRROR:
        return "mirror";

    case D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE:
        return "mirror_once";

    case D3D12_TEXTURE_ADDRESS_MODE_BORDER:
        return "border";
    }
}

//-----------------------------------------------------------------------------
//      ボーダーカラーに対応する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_STATIC_BORDER_COLOR type)
{
    switch(type)
    {
    default:
    case D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK:
        return "transparent_black";

    case D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK:
        return "opaque_black";

    case D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE:
        return "opaque_white";
    }
}

//-----------------------------------------------------------------------------
//      コンペアタイプに対応する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_COMPARISON_FUNC type)
{
    switch(type)
    {
    default:
    case D3D12_COMPARISON_FUNC_NEVER:
        return "never";

    case D3D12_COMPARISON_FUNC_LESS:
        return "less";

    case D3D12_COMPARISON_FUNC_EQUAL:
        return "equal";

    case D3D12_COMPARISON_FUNC_LESS_EQUAL:
        return "less_equal";

    case D3D12_COMPARISON_FUNC_GREATER:
        return "greater";

    case D3D12_COMPARISON_FUNC_NOT_EQUAL:
        return "not_equal";

    case D3D12_COMPARISON_FUNC_GREATER_EQUAL:
        return "greater_equal";

    case D3D12_COMPARISON_FUNC_ALWAYS:
        return "always";
    }
}

//-----------------------------------------------------------------------------
//      ステンシル操作タイプに対応する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_STENCIL_OP type)
{
    switch(type)
    {
    default:
    case D3D12_STENCIL_OP_KEEP:
        return "keep";

    case D3D12_STENCIL_OP_ZERO:
        return "zero";

    case D3D12_STENCIL_OP_REPLACE:
        return "replace";

    case D3D12_STENCIL_OP_INCR_SAT:
        return "incr_sat";

    case D3D12_STENCIL_OP_DECR_SAT:
        return "decr_sat";

    case D3D12_STENCIL_OP_INVERT:
        return "invert";

    case D3D12_STENCIL_OP_INCR:
        return "incr";

    case D3D12_STENCIL_OP_DECR:
        return "decr";
    }
}

//-----------------------------------------------------------------------------
//      深度書き込みマスクに対応する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_DEPTH_WRITE_MASK type)
{
    switch(type)
    {
    default:
    case D3D12_DEPTH_WRITE_MASK_ZERO:
        return "zero";

    case D3D12_DEPTH_WRITE_MASK_ALL:
        return "all";
    }
}

//-----------------------------------------------------------------------------
//      ブレンド操作タイプに対応する文字列を返却します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_BLEND_OP type)
{
    switch(type)
    {
    default:
    case D3D12_BLEND_OP_ADD:
        return "add";

    case D3D12_BLEND_OP_SUBTRACT:
        return "sub";

    case D3D12_BLEND_OP_REV_SUBTRACT:
        return "rev_sub";

    case D3D12_BLEND_OP_MIN:
        return "min";

    case D3D12_BLEND_OP_MAX:
        return "max";
    }
}

//-----------------------------------------------------------------------------
//      POLYGON_MODE型を解析します.
//-----------------------------------------------------------------------------
D3D12_FILL_MODE ParsePolygonMode(const char* value)
{
    auto result = D3D12_FILL_MODE_SOLID;

    if (_stricmp(value, "WIREFRAME") == 0)
    { result = D3D12_FILL_MODE_WIREFRAME; }
    else if (_stricmp(value, "SOLID") == 0)
    { result = D3D12_FILL_MODE_SOLID; }

    return result;
}

//-----------------------------------------------------------------------------
//      BLEND_TYPE型を解析します.
//-----------------------------------------------------------------------------
D3D12_BLEND ParseBlendType(const char* value)
{
    auto type = D3D12_BLEND_ZERO;

    if (_stricmp(value, "ZERO") == 0)
    { type = D3D12_BLEND_ZERO; }
    else if (_stricmp(value, "ONE") == 0)
    { type = D3D12_BLEND_ONE; }
    else if (_stricmp(value, "SRC_COLOR") == 0)
    { type = D3D12_BLEND_SRC_COLOR; }
    else if (_stricmp(value, "INV_SRC_COLOR") == 0)
    { type = D3D12_BLEND_INV_SRC_COLOR; }
    else if (_stricmp(value, "SRC_ALPHA") == 0)
    { type = D3D12_BLEND_SRC_ALPHA; }
    else if (_stricmp(value, "INV_SRC_ALPHA") == 0)
    { type = D3D12_BLEND_INV_SRC_ALPHA; }
    else if (_stricmp(value, "DST_ALPHA") == 0)
    { type = D3D12_BLEND_DEST_ALPHA; }
    else if (_stricmp(value, "INV_DST_ALPHA") == 0)
    { type = D3D12_BLEND_INV_DEST_ALPHA; }
    else if (_stricmp(value, "DST_COLOR") == 0)
    { type = D3D12_BLEND_DEST_COLOR; }
    else if (_stricmp(value, "INV_DST_COLOR") == 0)
    { type = D3D12_BLEND_INV_DEST_COLOR; }

    return type;
}

//-----------------------------------------------------------------------------
//      FILTER_MODE型を解析します.
//-----------------------------------------------------------------------------
FILTER_MODE ParseFilterMode(const char* value)
{
    FILTER_MODE result = FILTER_MODE_NEAREST;

    if (_stricmp(value, "NEAREST") == 0)
    { result = FILTER_MODE_NEAREST; }
    else if (_stricmp(value, "LINEAR") == 0)
    { result = FILTER_MODE_LINEAR; }

    return result;
}

//-----------------------------------------------------------------------------
//      MIPMAP_MODE型を解析します.
//-----------------------------------------------------------------------------
MIPMAP_MODE ParseMipmapMode(const char* value)
{
    MIPMAP_MODE result = MIPMAP_MODE_NEAREST;

    if (_stricmp(value, "NEAREST") == 0)
    { result = MIPMAP_MODE_NEAREST; }
    else if (_stricmp(value, "LINEAR") == 0)
    { result = MIPMAP_MODE_LINEAR; }
    else if (_stricmp(value, "NONE") == 0)
    { result = MIPMAP_MODE_NONE; }

    return result;
}

//-----------------------------------------------------------------------------
//      ADDRESS_MODE型を解析します.
//-----------------------------------------------------------------------------
D3D12_TEXTURE_ADDRESS_MODE ParseAddressMode(const char* value)
{
    auto result = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    if (_stricmp(value, "CLAMP") == 0)
    { result = D3D12_TEXTURE_ADDRESS_MODE_CLAMP; }
    else if (_stricmp(value, "WRAP") == 0)
    { result = D3D12_TEXTURE_ADDRESS_MODE_WRAP; }
    else if (_stricmp(value, "MIRROR") == 0)
    { result = D3D12_TEXTURE_ADDRESS_MODE_MIRROR; }
    else if (_stricmp(value, "MIRROR_ONCE") == 0)
    { result = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE; }
    else if (_stricmp(value, "BORDER") == 0)
    { result = D3D12_TEXTURE_ADDRESS_MODE_BORDER; }

    return result;
}

//-----------------------------------------------------------------------------
//      BORDER_COLOR型を解析します.
//-----------------------------------------------------------------------------
D3D12_STATIC_BORDER_COLOR ParseBorderColor(const char* value)
{
    auto result = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;

    if (_stricmp(value, "TRANSPARENT_BLACK") == 0)
    { result = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK; }
    else if (_stricmp(value, "OPAQUE_BLACK") == 0)
    { result = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK; }
    else if (_stricmp(value, "OAPQUE_WHITE") == 0)
    { result = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; }

    return result;
}

//-----------------------------------------------------------------------------
//      CULL_TYPE型を解析します.
//-----------------------------------------------------------------------------
D3D12_CULL_MODE ParseCullType(const char* value)
{
    auto result = D3D12_CULL_MODE_NONE;

    if (_stricmp(value, "NONE") == 0)
    { result = D3D12_CULL_MODE_NONE; }
    else if (_stricmp(value, "FRONT") == 0)
    { result = D3D12_CULL_MODE_FRONT; }
    else if (_stricmp(value, "BACK") == 0)
    { result = D3D12_CULL_MODE_BACK; }

    return result;
}

//-----------------------------------------------------------------------------
//      COMPARE_TYPE型を解析します.
//-----------------------------------------------------------------------------
D3D12_COMPARISON_FUNC ParseCompareType(const char* value)
{
    auto result = D3D12_COMPARISON_FUNC_NEVER;

    if (_stricmp(value, "NEVER") == 0)
    { result = D3D12_COMPARISON_FUNC_NEVER; }
    else if (_stricmp(value, "LESS") == 0)
    { result = D3D12_COMPARISON_FUNC_LESS; }
    else if (_stricmp(value, "EQUAL") == 0)
    { result = D3D12_COMPARISON_FUNC_EQUAL; }
    else if (_stricmp(value, "LEQUAL") == 0)
    { result = D3D12_COMPARISON_FUNC_LESS_EQUAL; }
    else if (_stricmp(value, "GREATER") == 0)
    { result = D3D12_COMPARISON_FUNC_GREATER; }
    else if (_stricmp(value, "NEQUAL") == 0)
    { result = D3D12_COMPARISON_FUNC_NOT_EQUAL; }
    else if (_stricmp(value, "GEQUAL") == 0)
    { result = D3D12_COMPARISON_FUNC_GREATER_EQUAL; }

    return result;
}

//-----------------------------------------------------------------------------
//      STENCIL_OP_TYPE型を解析します.
//-----------------------------------------------------------------------------
D3D12_STENCIL_OP ParseStencilOpType(const char* value)
{
    auto result = D3D12_STENCIL_OP_KEEP;

    if (_stricmp(value, "KEEP") == 0)
    { result = D3D12_STENCIL_OP_KEEP; }
    else if (_stricmp(value, "ZERO") == 0)
    { result = D3D12_STENCIL_OP_ZERO; }
    else if (_stricmp(value, "REPLACE") == 0)
    { result = D3D12_STENCIL_OP_REPLACE; }
    else if (_stricmp(value, "INCR_SAT") == 0)
    { result = D3D12_STENCIL_OP_INCR_SAT; }
    else if (_stricmp(value, "DECR_SAT") == 0)
    { result = D3D12_STENCIL_OP_DECR_SAT; }
    else if (_stricmp(value, "INVERT") == 0)
    { result = D3D12_STENCIL_OP_INVERT; }
    else if (_stricmp(value, "INCR") == 0)
    { result = D3D12_STENCIL_OP_INCR; }
    else if (_stricmp(value, "DECR") == 0)
    { result = D3D12_STENCIL_OP_DECR; }

    return result;
}

//-----------------------------------------------------------------------------
//      DEPTH_WRITE_MASK型を解析します.
//-----------------------------------------------------------------------------
D3D12_DEPTH_WRITE_MASK ParseDepthWriteMask(const char* value)
{
    auto result = D3D12_DEPTH_WRITE_MASK_ALL;

    if (_stricmp(value, "ALL") == 0)
    { result = D3D12_DEPTH_WRITE_MASK_ALL; }
    else if (_stricmp(value, "ZERO") == 0)
    { result = D3D12_DEPTH_WRITE_MASK_ZERO; }

    return result;
}

//-----------------------------------------------------------------------------
//      BLEND_OP_TYPE型を解析します.
//-----------------------------------------------------------------------------
D3D12_BLEND_OP ParseBlendOpType(const char* value)
{
    auto result = D3D12_BLEND_OP_ADD;

    if (_stricmp(value, "ADD") == 0)
    { result = D3D12_BLEND_OP_ADD; }
    else if (_stricmp(value, "SUB") == 0)
    { result = D3D12_BLEND_OP_SUBTRACT; }
    else if (_stricmp(value, "REV_SUB") == 0)
    { result = D3D12_BLEND_OP_REV_SUBTRACT; }
    else if (_stricmp(value, "MIN") == 0)
    { result = D3D12_BLEND_OP_MIN; }
    else if (_stricmp(value, "MAX") == 0)
    { result = D3D12_BLEND_OP_MAX; }

    return result;
}

//-----------------------------------------------------------------------------
//      文字列を置換します.
//-----------------------------------------------------------------------------
std::string Replace
(
    const std::string& input,
    const std::string& pattern,
    const std::string& replace,
    bool& hit
)
{
    std::string result = input;
    auto pos = result.find(pattern);
    hit = false;

    while (pos != std::string::npos)
    {
        result.replace(pos, pattern.length(), replace);
        pos = result.find(pattern, pos + replace.length());
        hit = true;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      ファイルをロードします.
//-----------------------------------------------------------------------------
bool LoadFile(const char* filename, std::string& result)
{
    FILE* pFile = nullptr;
    auto err = fopen_s(&pFile, filename, "rb");
    if (err != 0 || pFile == nullptr)
    {
        ELOG("Erorr : File Open Failed. path = %s", filename);
        return false;
    }

    auto cur_pos = ftell(pFile);
    fseek(pFile, 0, SEEK_END);
    auto end_pos = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    auto size = size_t(end_pos) - size_t(cur_pos);
    auto pBuffer = new char[size + 1];
    fread(pBuffer, size, 1, pFile);
    pBuffer[size] = '\0';
    fclose(pFile);

    result = pBuffer;
    delete[] pBuffer;

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// FxParser class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
FxParser::FxParser()
: m_Tokenizer    ()
, m_Technieues   ()
, m_ShaderCounter(0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
FxParser::~FxParser()
{ Clear(); }

//-----------------------------------------------------------------------------
//      クリアします.
//-----------------------------------------------------------------------------
void FxParser::Clear()
{
    m_Tokenizer.Term();
    m_Technieues.clear();
    m_Technieues.shrink_to_fit();

    m_Shaders.clear();
    m_Defines.clear();
    m_BlendStates.clear();
    m_RasterizerStates.clear();
    m_DepthStencilStates.clear();
    m_ConstantBuffers.clear();
    m_Structures.clear();
    m_Resources.clear();
    m_Properties.Values.clear();
    m_Properties.Textures.clear();
    m_ShaderCounter = 0;
    m_DirPaths.clear();
    m_DirPaths.shrink_to_fit();
    m_Includes.clear();
    m_Includes.shrink_to_fit();
    m_Expanded.clear();
    m_SourceCode.clear();
}

//-----------------------------------------------------------------------------
//      解析します.
//-----------------------------------------------------------------------------
bool FxParser::Parse(const char* filename)
{
    if (!Load(filename))
    {
        ELOG( "Error : File Load Failed. filename = %s", filename );
        return false;
    }

    if (!m_Tokenizer.Init(2048))
    {
        ELOG( "Error : Tokenizer Initialize failed." );
        return false;
    }

    m_SourceCode.clear();
    m_SourceCode.reserve( m_Expanded.size() );

    m_Tokenizer.SetSeparator( " \t\r\n,\"" );
    m_Tokenizer.SetCutOff( "{}()=#<>;" );
    m_Tokenizer.SetBuffer( const_cast<char*>(m_Expanded.c_str()), m_Expanded.size() );

    auto cur = m_Tokenizer.GetBuffer();

    while(!m_Tokenizer.IsEnd())
    {
        bool output = true;

        // プリプロセッサ系.
        if (m_Tokenizer.Compare("#"))
        {
            ParsePreprocessor();
        }
        // テクニック.
        else if (m_Tokenizer.CompareAsLower("technique"))
        {
            output = false;
            ParseTechnique();
        }
        // 定数バッファ.
        else if (m_Tokenizer.CompareAsLower("cbuffer"))
        {
            ParseConstantBuffer();
        }
        // 構造体.
        else if (m_Tokenizer.CompareAsLower("struct"))
        {
            ParseStruct();
        }
        // プロパティ.
        else if (m_Tokenizer.CompareAsLower("properties"))
        {
            auto ptr = m_Tokenizer.GetPtr();
            auto size = (ptr - cur) - strlen("properties");
            if (size > 0)
            {
                m_SourceCode.append(cur, size);
                cur = ptr;
            }

            ParseProperties();
            output = false;
        }
        else if (
           m_Tokenizer.CompareAsLower("Texture1D")
        || m_Tokenizer.CompareAsLower("Texture1DArray")
        || m_Tokenizer.CompareAsLower("Texture2D")
        || m_Tokenizer.CompareAsLower("Texture2DArray")
        || m_Tokenizer.CompareAsLower("Texture2DMS")
        || m_Tokenizer.CompareAsLower("Texture2DMSArray")
        || m_Tokenizer.CompareAsLower("Texture3D")
        || m_Tokenizer.CompareAsLower("TextureCube")
        || m_Tokenizer.CompareAsLower("TextureCubeArray")
        || m_Tokenizer.CompareAsLower("Buffer")
        || m_Tokenizer.CompareAsLower("ByteAddressBuffer")
        || m_Tokenizer.CompareAsLower("StructuredBuffer")
        || m_Tokenizer.CompareAsLower("RWTexture1D")
        || m_Tokenizer.CompareAsLower("RWTexture1DArray")
        || m_Tokenizer.CompareAsLower("RWTexture2D")
        || m_Tokenizer.CompareAsLower("RWTexture2DArray")
        || m_Tokenizer.CompareAsLower("RWTexture3D")
        || m_Tokenizer.CompareAsLower("RWBuffer")
        || m_Tokenizer.CompareAsLower("RWByteAddressBuffer")
        || m_Tokenizer.CompareAsLower("RWStructuredBuffer")
        || m_Tokenizer.CompareAsLower("SamplerState")
        || m_Tokenizer.CompareAsLower("SamplerComparisonState"))
        {
            ParseResource();
        }
        // シェーダ.
        else if (
            m_Tokenizer.CompareAsLower("vertexshader") 
         || m_Tokenizer.CompareAsLower("pixelshader")
         || m_Tokenizer.CompareAsLower("geometryshader")
         || m_Tokenizer.CompareAsLower("domainshader")
         || m_Tokenizer.CompareAsLower("hullshader") 
         || m_Tokenizer.CompareAsLower("computeshader")
         || m_Tokenizer.CompareAsLower("amplificationshader")
         || m_Tokenizer.CompareAsLower("meshshader")
        )
        {
            output = false;
            ParseShader();
        }
        else if (m_Tokenizer.CompareAsLower("BlendState"))
        {
            output = false;
            ParseBlendState();
        }
        else if (m_Tokenizer.CompareAsLower("RasterizerState"))
        {
            output = false;
            ParseRasterizerState();
        }
        else if ((m_Tokenizer.CompareAsLower("DepthStencilState")))
        {
            output = false;
            ParseDepthStencilState();
        }

        auto ptr = m_Tokenizer.GetPtr();

        // ソースコード文字列に追加.
        if (output && !m_Tokenizer.IsEnd())
        {
            auto size = (ptr - cur);

        #if 0 // デバッグのための視認用.
            for(auto i=0; i<size; ++i)
            { putchar(cur[i]); }
        #endif

            if (size > 0)
            { m_SourceCode.append(cur, size); }
        }

        cur = ptr;

        // 次のトークンを取得.
        m_Tokenizer.Next();
    }

    // 最後が出力されないので追加.
    {
        auto ptr = m_Tokenizer.GetPtr();
        auto size = (ptr - cur);
        if (size > 0)
        { m_SourceCode.append(cur, size); }
    }

    // 一時データを削除.
    m_Shaders.clear();

    // 破棄処理.
    m_Tokenizer.Term();

    return true;
}

//-----------------------------------------------------------------------------
//      ファイルを読み込みます.
//-----------------------------------------------------------------------------
bool FxParser::Load(const char* filename)
{
    auto dir = GetDirectoryPathA(filename);
    m_DirPaths.push_back(dir);

    // インクルード情報を収集.
    if (!CorrectIncludes(filename))
    { return false; }

    for(size_t i=0; i<m_Includes.size(); ++i)
    { LoadFile(m_Includes[i].FindPath.c_str(), m_Includes[i].Code); }

    m_Expanded.clear();

    if (!LoadFile(filename, m_Expanded))
    { return false; }

    for(;;)
    {
        if (!Expand(m_Expanded))
        { break; }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      シェーダを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParseShader()
{
    // シェーダタイプを取得.
    SHADER_TYPE type = GetShaderType();
    m_Tokenizer.Next();

    std::string variable;
    std::string entryPoint;
    std::string profile;

    // テクニック内からの呼び出し.
    if (m_Tokenizer.Compare("="))
    {
        // 適当な変数名を付ける.
        char name[256] = {};
        sprintf_s(name, "Shader_%d", m_ShaderCounter);
        variable = name;

        m_Tokenizer.Next();
    }
    // テクニック外からの呼び出し.
    else
    {
        // 変数名取得.
        variable = std::string(m_Tokenizer.GetAsChar());
        m_Tokenizer.Next();
        assert(m_Tokenizer.Compare("="));
        m_Tokenizer.Next();
    }

    m_ShaderCounter++;
    assert(m_Tokenizer.Compare("compile"));

    // プロファイル名を取得.
    profile = std::string(m_Tokenizer.NextAsChar());
   
    // エントリーポイント名を取得.
    entryPoint = std::string(m_Tokenizer.NextAsChar());

    m_Tokenizer.Next();
    assert(m_Tokenizer.Compare("("));

    // シェーダデータを設定.
    Shader data = {};
    data.EntryPoint = entryPoint;
    data.Profile    = profile;

    m_Tokenizer.Next();
    while(!m_Tokenizer.IsEnd())
    {
        if (m_Tokenizer.Compare(")"))
        { break; }
       
        // メソッド引数を追加.
        data.Arguments.push_back( std::string(m_Tokenizer.GetAsChar()) );

        m_Tokenizer.Next();
    }

    // シェーダを登録.
    if (m_Shaders.find(variable) == m_Shaders.end() )
    { m_Shaders[variable] = data; }
}

//-----------------------------------------------------------------------------
//      テクニックを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParseTechnique()
{
    m_Tokenizer.Next();

    // テクニック名を取得.
    auto name = std::string(m_Tokenizer.GetAsChar());

    // テクニックブロック開始.
    m_Tokenizer.Next();
    assert(m_Tokenizer.Compare("{"));

    // テクニック名を設定.
    Technique technique = {};
    technique.Name = name;

    while(!m_Tokenizer.IsEnd())
    {
        // テクニックブロック終了.
        if (m_Tokenizer.Compare("}"))
        {
            break;
        }
        else if (m_Tokenizer.Compare("pass"))
        {
            // パスを解析.
            ParsePass(technique);
        }

        // 次のトークンを取得.
        m_Tokenizer.Next();
    }

    // テクニックを登録.
    m_Technieues.push_back(technique);
}

//-----------------------------------------------------------------------------
//      パスを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParsePass(Technique& technique)
{
    m_Tokenizer.Next();

    // パス名を取得.
    auto name = std::string(m_Tokenizer.GetAsChar());

    // パスブロック開始.
    m_Tokenizer.Next();
    assert(m_Tokenizer.Compare("{"));
    int blockCount = 1;

    m_Tokenizer.Next();

    // パス名を設定.
    Pass pass = {};
    pass.Name = name;

    while(!m_Tokenizer.IsEnd())
    {
        // パスブロック終了.
        if (m_Tokenizer.Compare("}"))
        {
            blockCount--;
            if (blockCount == 0)
            { break; }
        }
        else if (m_Tokenizer.Compare("{"))
        {
            blockCount++;
        }
        else if (m_Tokenizer.CompareAsLower("RasterizerState"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            m_Tokenizer.Next();

            // 変数名を取得.
            name = std::string(m_Tokenizer.GetAsChar());

            // 変数名からステートを引っ張ってくる.
            auto itr = m_RasterizerStates.find(name);

            if (itr != m_RasterizerStates.end())
            {
                // 発見できたら，パスにステートを登録.
                pass.RasterizerState = itr->first;
            }
        }
        else if (m_Tokenizer.CompareAsLower("DepthStencilState"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            m_Tokenizer.Next();

            // 変数名を取得.
            name = std::string(m_Tokenizer.GetAsChar());

            // 変数名からステートを引っ張ってくる.
            auto itr = m_DepthStencilStates.find(name);

            if (itr != m_DepthStencilStates.end())
            {
                // 発見できたら，パスにステートを登録.
                pass.DepthStencilState = itr->first;
            }
        }
        else if (m_Tokenizer.CompareAsLower("BlendState"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            m_Tokenizer.Next();

            // 変数名を取得.
            name = std::string(m_Tokenizer.GetAsChar());

            // 変数名からステートを引っ張ってくる.
            auto itr = m_BlendStates.find(name);

            if (itr != m_BlendStates.end())
            {
                // 発見できたら，パスにステートを登録.
                pass.BlendState = itr->first;
            }
        }
        // シェーダデータ.
        else if (
            m_Tokenizer.CompareAsLower("VertexShader")
         || m_Tokenizer.CompareAsLower("PixelShader")
         || m_Tokenizer.CompareAsLower("GeometryShader")
         || m_Tokenizer.CompareAsLower("DomainShader")
         || m_Tokenizer.CompareAsLower("HullShader")
         || m_Tokenizer.CompareAsLower("ComputeShader")
         || m_Tokenizer.CompareAsLower("AmplificationShader")
         || m_Tokenizer.CompareAsLower("MeshShader"))
        {
            // シェーダタイプを取得.
            Shader shader = {};
            shader.Type = GetShaderType();

            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            m_Tokenizer.Next();

            // 変数で設定されていない場合.
            if (m_Tokenizer.Compare("compile"))
            {
                // シェーダプロファイル名を取得.
                shader.Profile      = std::string(m_Tokenizer.NextAsChar());

                // エントリーポイント名を取得.
                shader.EntryPoint   = std::string(m_Tokenizer.NextAsChar());

                // メソッド引数開始.
                m_Tokenizer.Next();
                assert(m_Tokenizer.Compare("("));

                int blankCounter = 1;
                m_Tokenizer.Next();
                while(!m_Tokenizer.IsEnd())
                {
                    // メソッド引数終了.
                    if (m_Tokenizer.Compare(")"))
                    {
                        blankCounter--;
                        if (blankCounter == 0)
                        { break; }
                    }
                    else if (m_Tokenizer.Compare("("))
                    {
                        blankCounter++;
                        m_Tokenizer.Next();
                        continue;
                    }

                    // 引数を追加.
                    shader.Arguments.push_back(m_Tokenizer.GetAsChar());

                    // 次のトークンを取得.
                    m_Tokenizer.Next();
                }

                // パスにシェーダを登録.
                pass.Shaders.push_back(shader);
            }
            // 変数で設定されている場合.
            else
            {
                if (m_Tokenizer.Compare("("))
                { m_Tokenizer.Next(); }

                // 変数名を取得.
                name = std::string(m_Tokenizer.GetAsChar());

                // 変数名からシェーダを引っ張ってくる.
                auto itr = m_Shaders.find(name);

                if (itr != m_Shaders.end())
                {
                    // 発見できたら，パスにシェーダを登録.
                    pass.Shaders.push_back( itr->second );
                }
            }
        }

        // 次のトークンを取得.
        m_Tokenizer.Next();
    }

    // テクニックにパスを登録.
    technique.Pass.push_back(pass);
}

//-----------------------------------------------------------------------------
//      プリプロセッサを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParsePreprocessor()
{
    // 次のトークンを取得.
    m_Tokenizer.Next();

    if (m_Tokenizer.Compare("define"))
    {
        auto tag = std::string(m_Tokenizer.NextAsChar());
        auto val = std::string(m_Tokenizer.NextAsChar());
        m_Defines[tag] = val;
    }
    else if (m_Tokenizer.Compare("elif"))
    {
    }
    else if (m_Tokenizer.Compare("else"))
    {
    }
    else if (m_Tokenizer.Compare("endif"))
    {
    }
    else if (m_Tokenizer.Compare("error"))
    {
    }
    else if (m_Tokenizer.Compare("if"))
    {
    }
    else if (m_Tokenizer.Compare("ifdef"))
    {
    }
    else if (m_Tokenizer.Compare("ifndef"))
    {
    }
    else if (m_Tokenizer.Compare("include"))
    {
    }
    else if (m_Tokenizer.Compare("line"))
    {
    }
    else if (m_Tokenizer.Compare("pragma"))
    {
    }
    else if (m_Tokenizer.Compare("undef"))
    {
        auto tag = std::string(m_Tokenizer.NextAsChar());
        m_Defines.erase(tag);
    }
}

//-----------------------------------------------------------------------------
//      ブレンドステートを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParseBlendState()
{
    m_Tokenizer.Next();

    // ステート名を取得.
    auto name = std::string(m_Tokenizer.GetAsChar());

    // ステートブロック開始.
    m_Tokenizer.Next();
    assert(m_Tokenizer.Compare("{"));
    int blockCount = 1;

    D3D12_BLEND_DESC state = {};
    state.AlphaToCoverageEnable         = FALSE;
    state.IndependentBlendEnable        = FALSE;
    state.RenderTarget[0].LogicOpEnable = FALSE;
    state.RenderTarget[0].LogicOp       = D3D12_LOGIC_OP_NOOP;

    m_Tokenizer.Next();
    while(!m_Tokenizer.IsEnd())
    {
        // パスブロック終了.
        if (m_Tokenizer.Compare("}"))
        {
            blockCount--;
            if (blockCount == 0)
            { break; }
        }
        else if (m_Tokenizer.Compare("{"))
        {
            blockCount++;
        }
        else if (m_Tokenizer.CompareAsLower("AlphaToCoverageEnable"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.AlphaToCoverageEnable = m_Tokenizer.NextAsBool();
        }
        else if (m_Tokenizer.CompareAsLower("BlendEnable"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.RenderTarget[0].BlendEnable = m_Tokenizer.NextAsBool();
        }
        else if (m_Tokenizer.CompareAsLower("SrcBlend"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            auto value = m_Tokenizer.NextAsChar();
            state.RenderTarget[0].SrcBlend = ParseBlendType(value);
        }
        else if (m_Tokenizer.CompareAsLower("DstBlend"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            auto value = m_Tokenizer.NextAsChar();
            state.RenderTarget[0].DestBlend = ParseBlendType(value);
        }
        else if (m_Tokenizer.CompareAsLower("BlendOp"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            auto value = m_Tokenizer.NextAsChar();
            state.RenderTarget[0].BlendOp = ParseBlendOpType(value);
        }
        else if (m_Tokenizer.CompareAsLower("SrcBlendAlpha"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            auto value = m_Tokenizer.NextAsChar();
            state.RenderTarget[0].SrcBlendAlpha = ParseBlendType(value);
        }
        else if (m_Tokenizer.CompareAsLower("DstBlendAlpha"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            auto value = m_Tokenizer.NextAsChar();
            state.RenderTarget[0].DestBlendAlpha = ParseBlendType(value);
        }
        else if (m_Tokenizer.CompareAsLower("BlendOpAlpha"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            auto value = m_Tokenizer.NextAsChar();
            state.RenderTarget[0].BlendOpAlpha = ParseBlendOpType(value);
        }
        else if (m_Tokenizer.CompareAsLower("RenderTargetWriteMask"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.RenderTarget[0].RenderTargetWriteMask = m_Tokenizer.NextAsUint();
        }

        // 次のトークンを取得.
        m_Tokenizer.Next();
    }

    if (m_BlendStates.find(name) == m_BlendStates.end() )
    { m_BlendStates[name] = state; }
}

//-----------------------------------------------------------------------------
//      ラスタライザーステートを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParseRasterizerState()
{
    m_Tokenizer.Next();

    // ステート名を取得.
    auto name = std::string(m_Tokenizer.GetAsChar());

    // ステートブロック開始.
    m_Tokenizer.Next();
    assert(m_Tokenizer.Compare("{"));
    int blockCount = 1;

    D3D12_RASTERIZER_DESC state = {};

    m_Tokenizer.Next();
    while(!m_Tokenizer.IsEnd())
    {
        // パスブロック終了.
        if (m_Tokenizer.Compare("}"))
        {
            blockCount--;
            if (blockCount == 0)
            { break; }
        }
        else if (m_Tokenizer.Compare("{"))
        {
            blockCount++;
        }
        else if (m_Tokenizer.CompareAsLower("PolygonMode"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.FillMode = ParsePolygonMode(m_Tokenizer.NextAsChar());
        }
        else if (m_Tokenizer.CompareAsLower("CullMode"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.CullMode = ParseCullType(m_Tokenizer.NextAsChar());
        }
        else if (m_Tokenizer.CompareAsLower("FrontCCW"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.FrontCounterClockwise = m_Tokenizer.NextAsBool();
        }
        else if (m_Tokenizer.CompareAsLower("DepthBias"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.DepthBias = uint32_t(m_Tokenizer.NextAsInt());
        }
        else if (m_Tokenizer.CompareAsLower("DepthBiasClamp"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.DepthBiasClamp = m_Tokenizer.NextAsFloat();
        }
        else if (m_Tokenizer.CompareAsLower("SlopeScaledDepthBias"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.SlopeScaledDepthBias = m_Tokenizer.NextAsFloat();
        }
        else if (m_Tokenizer.CompareAsLower("DepthClipEnable"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.DepthClipEnable = m_Tokenizer.NextAsBool();
        }
        else if (m_Tokenizer.CompareAsLower("EnableConservativeRaster"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.ConservativeRaster = m_Tokenizer.NextAsBool() ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        }

        m_Tokenizer.Next();
    }

    if (m_RasterizerStates.find(name) == m_RasterizerStates.end())
    { m_RasterizerStates[name] = state; }
}

//-----------------------------------------------------------------------------
//      深度ステンシルステートを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParseDepthStencilState()
{
    m_Tokenizer.Next();

    // ステート名を取得.
    auto name = std::string(m_Tokenizer.GetAsChar());

    // ステートブロック開始.
    m_Tokenizer.Next();
    assert(m_Tokenizer.Compare("{"));
    int blockCount = 1;

    D3D12_DEPTH_STENCIL_DESC state = {};

    m_Tokenizer.Next();
    while(!m_Tokenizer.IsEnd())
    {
        // パスブロック終了.
        if (m_Tokenizer.Compare("}"))
        {
            blockCount--;
            if (blockCount == 0)
            { break; }
        }
        else if (m_Tokenizer.Compare("{"))
        {
            blockCount++;
        }
        else if (m_Tokenizer.CompareAsLower("DepthEnable"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.DepthEnable = m_Tokenizer.NextAsBool();
        }
        else if (m_Tokenizer.CompareAsLower("DepthWriteMask"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.DepthWriteMask = ParseDepthWriteMask(m_Tokenizer.NextAsChar());
        }
        else if (m_Tokenizer.CompareAsLower("DepthFunc"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.DepthFunc = ParseCompareType(m_Tokenizer.NextAsChar());
        }
        else if (m_Tokenizer.CompareAsLower("StencilEnable"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.StencilEnable = m_Tokenizer.NextAsBool();
        }
        else if (m_Tokenizer.CompareAsLower("StencilReadMask"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.StencilReadMask = uint8_t(m_Tokenizer.NextAsInt());
        }
        else if (m_Tokenizer.CompareAsLower("StencilWriteMask"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.StencilWriteMask = uint8_t(m_Tokenizer.NextAsInt());
        }
        else if (m_Tokenizer.CompareAsLower("FrontFaceStencilFail"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.FrontFace.StencilFailOp = ParseStencilOpType(m_Tokenizer.NextAsChar());
        }
        else if (m_Tokenizer.CompareAsLower("FrontFaceStencilDepthFail"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.FrontFace.StencilDepthFailOp = ParseStencilOpType(m_Tokenizer.NextAsChar());
        }
        else if (m_Tokenizer.CompareAsLower("FrontFaceStencilPass"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.FrontFace.StencilPassOp = ParseStencilOpType(m_Tokenizer.NextAsChar());
        }
        else if (m_Tokenizer.CompareAsLower("FrontFaceStencilFunc"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.FrontFace.StencilFunc = ParseCompareType(m_Tokenizer.NextAsChar());
        }
        else if (m_Tokenizer.CompareAsLower("BackFaceStencilFail"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.BackFace.StencilFailOp = ParseStencilOpType(m_Tokenizer.NextAsChar());
        }
        else if (m_Tokenizer.CompareAsLower("BackFaceStencilDepthFail"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.BackFace.StencilDepthFailOp = ParseStencilOpType(m_Tokenizer.NextAsChar());
        }
        else if (m_Tokenizer.CompareAsLower("BackFaceStencilPass"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.BackFace.StencilPassOp = ParseStencilOpType(m_Tokenizer.NextAsChar());
        }
        else if (m_Tokenizer.CompareAsLower("BackFaceStencilFunc"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("="));

            state.BackFace.StencilFunc = ParseCompareType(m_Tokenizer.NextAsChar());
        }

        m_Tokenizer.Next();
    }

    if (m_DepthStencilStates.find(name) == m_DepthStencilStates.end())
    { m_DepthStencilStates[name] = state; }
}

//-----------------------------------------------------------------------------
//      定数バッファを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParseConstantBuffer()
{
    m_Tokenizer.Next();

    // 定数バッファ名を取得.
    auto name = std::string(m_Tokenizer.GetAsChar());

    // 定数バッファ名を設定.
    ConstantBuffer buffer = {};
    buffer.Name     = name;
    buffer.Register = -1;

    m_Tokenizer.Next();
    if (m_Tokenizer.Compare(":"))
    {
        m_Tokenizer.Next();
        assert(m_Tokenizer.CompareAsLower("register"));
        m_Tokenizer.Next(); // register
        assert(m_Tokenizer.Compare("("));
        m_Tokenizer.Next(); // "("
        auto regStr = std::string(m_Tokenizer.GetAsChar());
        auto regNo  = std::stoi(regStr.substr(1));
        buffer.Register = uint32_t(regNo);
        m_Tokenizer.Next(); // bxx
        assert(m_Tokenizer.Compare(")"));
        m_Tokenizer.Next(); // ")"
    }

    assert(m_Tokenizer.Compare("{"));

    TYPE_MODIFIER modifier = TYPE_MODIFIER_NONE;

    while(!m_Tokenizer.IsEnd())
    {
        // テクニックブロック終了.
        if (m_Tokenizer.Compare("}"))
        {
            break;
        }
        else if (m_Tokenizer.Compare("float"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float1x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT1x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float1x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT1x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float1x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT1x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float2x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT2x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float2x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT2x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float2x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT2x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float2x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT2x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float3x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT3x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float3x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT3x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float3x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT3x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float3x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT3x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float4x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT4x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float4x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT4x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float4x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT4x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("float4x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_FLOAT4x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int1x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT1x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int1x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT1x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int1x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT1x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int2x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT2x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int2x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT2x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int2x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT2x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int2x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT2x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int3x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT3x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int3x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT3x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int3x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT3x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int3x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT3x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int4x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT4x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int4x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT4x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int4x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT4x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("int4x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_INT4x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint1x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT1x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint1x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT1x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint1x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT1x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint2x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT2x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint2x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT2x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint2x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT2x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint2x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT2x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint3x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT3x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint3x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT3x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint3x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT3x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint3x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT3x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint4x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT4x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint4x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT4x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint4x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT4x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("uint4x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_UINT4x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool1x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL1x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool1x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL1x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool1x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL1x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool2x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL2x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool2x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL2x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool2x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL2x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool2x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL2x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool3x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL3x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool3x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL3x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool3x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL3x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool3x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL3x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool4x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL4x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool4x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL4x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool4x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL4x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("bool4x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_BOOL4x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double1x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE1x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double1x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE1x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double1x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE1x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double2x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE2x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double2x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE2x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double2x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE2x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double2x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE2x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double3x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE3x1, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double3x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE3x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double3x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE3x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double3x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE3x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double4x1"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double4x2"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE4x2, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double4x3"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE4x3, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("double4x4"))
        {
            ParseConstantBufferMember(MEMBER_TYPE_DOUBLE4x4, buffer, modifier);
        }
        else if (m_Tokenizer.Compare("row_major"))
        {
            modifier = TYPE_MODIFIER_ROW_MAJOR;
            m_Tokenizer.Next();
        }
        else if (m_Tokenizer.Compare("colum_major"))
        {
            modifier = TYPE_MODIFIER_COLUMN_MAJOR;
            m_Tokenizer.Next();
        }
        else
        {
            auto name = m_Tokenizer.GetAsChar();
            if (m_Structures.find(name) != m_Structures.end())
            {
                ParseConstantBufferMember(MEMBER_TYPE_STRUCT, buffer, modifier);
            }
            else
            {
                // 次のトークンを取得.
                m_Tokenizer.Next();
            }
        }
    }

    if (m_ConstantBuffers.find(name) == m_ConstantBuffers.end())
    { m_ConstantBuffers[name] = buffer; }
}

//-----------------------------------------------------------------------------
//      定数バッファのメンバーを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParseConstantBufferMember
(
    MEMBER_TYPE     type,
    ConstantBuffer& buffer,
    TYPE_MODIFIER&  modifier
)
{
    Member member = {};
    member.Type         = type;
    member.Modifier     = modifier;
    member.PackOffset   = -1;

    auto name = std::string(m_Tokenizer.NextAsChar());
    auto pos = name.find(";");
    auto end = false;
    if (pos != std::string::npos)
    {
        end = true;
        name = name.substr(0, pos);
    }

    member.Name = name;

    modifier = TYPE_MODIFIER_NONE;

    if (end)
    {
        buffer.Members.push_back(member);
        return;
    }

    m_Tokenizer.Next();
    if (m_Tokenizer.Compare("packoffset"))
    {
        auto offsetStr  = std::string(m_Tokenizer.GetAsChar());
        pos = offsetStr.find(";");
        auto offset     = std::stoi(offsetStr.substr(1, pos));
        member.PackOffset = uint32_t(offset);
    }

    buffer.Members.push_back(member);
}

//-----------------------------------------------------------------------------
//      構造体を解析します.
//-----------------------------------------------------------------------------
void FxParser::ParseStruct()
{
    m_Tokenizer.Next();

    // 構造体名を取得.
    auto name = std::string(m_Tokenizer.GetAsChar());

    // 構造体名を設定.
    Structure structure;
    structure.Name = name;

    m_Tokenizer.Next();

    assert(m_Tokenizer.Compare("{"));

    TYPE_MODIFIER modifier = TYPE_MODIFIER_NONE;

    while(!m_Tokenizer.IsEnd())
    {
        // 構造体ブロック終了.
        if (m_Tokenizer.Compare("}"))
        {
            break;
        }
        else if (m_Tokenizer.Compare("float"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float1"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float1x2"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT1x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float1x3"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT1x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float1x4"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT1x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float2"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float2x1"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT2x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float2x2"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT2x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float2x3"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT2x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float2x4"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT2x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float3"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float3x1"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT3x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float3x2"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT3x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float3x3"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT3x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float3x4"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT3x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float4"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float4x1"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT4x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float4x2"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT4x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float4x3"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT4x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("float4x4"))
        {
            ParseStructMember(MEMBER_TYPE_FLOAT4x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int"))
        {
            ParseStructMember(MEMBER_TYPE_INT, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int1"))
        {
            ParseStructMember(MEMBER_TYPE_INT, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int1x2"))
        {
            ParseStructMember(MEMBER_TYPE_INT1x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int1x3"))
        {
            ParseStructMember(MEMBER_TYPE_INT1x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int1x4"))
        {
            ParseStructMember(MEMBER_TYPE_INT1x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int2"))
        {
            ParseStructMember(MEMBER_TYPE_INT2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int2x1"))
        {
            ParseStructMember(MEMBER_TYPE_INT2x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int2x2"))
        {
            ParseStructMember(MEMBER_TYPE_INT2x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int2x3"))
        {
            ParseStructMember(MEMBER_TYPE_INT2x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int2x4"))
        {
            ParseStructMember(MEMBER_TYPE_INT2x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int3"))
        {
            ParseStructMember(MEMBER_TYPE_INT3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int3x1"))
        {
            ParseStructMember(MEMBER_TYPE_INT3x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int3x2"))
        {
            ParseStructMember(MEMBER_TYPE_INT3x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int3x3"))
        {
            ParseStructMember(MEMBER_TYPE_INT3x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int3x4"))
        {
            ParseStructMember(MEMBER_TYPE_INT3x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int4"))
        {
            ParseStructMember(MEMBER_TYPE_INT4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int4x1"))
        {
            ParseStructMember(MEMBER_TYPE_INT4x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int4x2"))
        {
            ParseStructMember(MEMBER_TYPE_INT4x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int4x3"))
        {
            ParseStructMember(MEMBER_TYPE_INT4x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("int4x4"))
        {
            ParseStructMember(MEMBER_TYPE_INT4x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint"))
        {
            ParseStructMember(MEMBER_TYPE_UINT, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint1"))
        {
            ParseStructMember(MEMBER_TYPE_UINT, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint1x2"))
        {
            ParseStructMember(MEMBER_TYPE_UINT1x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint1x3"))
        {
            ParseStructMember(MEMBER_TYPE_UINT1x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint1x4"))
        {
            ParseStructMember(MEMBER_TYPE_UINT1x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint2"))
        {
            ParseStructMember(MEMBER_TYPE_UINT2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint2x1"))
        {
            ParseStructMember(MEMBER_TYPE_UINT2x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint2x2"))
        {
            ParseStructMember(MEMBER_TYPE_UINT2x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint2x3"))
        {
            ParseStructMember(MEMBER_TYPE_UINT2x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint2x4"))
        {
            ParseStructMember(MEMBER_TYPE_UINT2x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint3"))
        {
            ParseStructMember(MEMBER_TYPE_UINT3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint3x1"))
        {
            ParseStructMember(MEMBER_TYPE_UINT3x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint3x2"))
        {
            ParseStructMember(MEMBER_TYPE_UINT3x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint3x3"))
        {
            ParseStructMember(MEMBER_TYPE_UINT3x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint3x4"))
        {
            ParseStructMember(MEMBER_TYPE_UINT3x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint4"))
        {
            ParseStructMember(MEMBER_TYPE_UINT4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint4x1"))
        {
            ParseStructMember(MEMBER_TYPE_UINT4x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint4x2"))
        {
            ParseStructMember(MEMBER_TYPE_UINT4x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint4x3"))
        {
            ParseStructMember(MEMBER_TYPE_UINT4x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("uint4x4"))
        {
            ParseStructMember(MEMBER_TYPE_UINT4x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool1"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool1x2"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL1x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool1x3"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL1x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool1x4"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL1x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool2"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool2x1"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL2x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool2x2"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL2x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool2x3"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL2x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool2x4"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL2x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool3"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool3x1"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL3x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool3x2"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL3x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool3x3"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL3x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool3x4"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL3x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool4"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool4x1"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL4x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool4x2"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL4x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool4x3"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL4x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("bool4x4"))
        {
            ParseStructMember(MEMBER_TYPE_BOOL4x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double1"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double1x2"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE1x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double1x3"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE1x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double1x4"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE1x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double2"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double2x1"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE2x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double2x2"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE2x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double2x3"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE2x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double2x4"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE2x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double3"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double3x1"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE3x1, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double3x2"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE3x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double3x3"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE3x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double3x4"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE3x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double4"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double4x1"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double4x2"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE4x2, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double4x3"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE4x3, structure, modifier);
        }
        else if (m_Tokenizer.Compare("double4x4"))
        {
            ParseStructMember(MEMBER_TYPE_DOUBLE4x4, structure, modifier);
        }
        else if (m_Tokenizer.Compare("row_major"))
        {
            modifier = TYPE_MODIFIER_ROW_MAJOR;
            m_Tokenizer.Next();
        }
        else if (m_Tokenizer.Compare("colum_major"))
        {
            modifier = TYPE_MODIFIER_COLUMN_MAJOR;
            m_Tokenizer.Next();
        }
        else 
        {
            auto name = m_Tokenizer.GetAsChar();
            if (m_Structures.find(name) != m_Structures.end())
            {
                ParseStructMember(MEMBER_TYPE_STRUCT, structure, modifier);
            }
            else
            {
                // 次のトークンを取得.
                m_Tokenizer.Next();
            }
        }
    }

    if (m_Structures.find(name) == m_Structures.end())
    { m_Structures[name] = structure; }
}

//-----------------------------------------------------------------------------
//      構造体のメンバーを解析します
//-----------------------------------------------------------------------------
void FxParser::ParseStructMember
(
    MEMBER_TYPE     type,
    Structure&      structure,
    TYPE_MODIFIER&  modifier
)
{
    Member member = {};
    member.Type         = type;
    member.Modifier     = modifier;
    member.PackOffset   = -1;

    auto name = std::string(m_Tokenizer.NextAsChar());
    auto pos = name.find(";");
    bool end = false;
    if (pos != std::string::npos)
    {
        end = true;
        name = name.substr(0, pos);
    }

    member.Name = name;
    modifier = TYPE_MODIFIER_NONE;

    if (end)
    {
        structure.Members.push_back(member);
        return;
    }

    m_Tokenizer.Next();
    if (m_Tokenizer.Compare(":"))
    {
        auto semantics = std::string(m_Tokenizer.NextAsChar());
        pos = semantics.find(";");
        semantics = semantics.substr(0, pos);
    }

    structure.Members.push_back(member);
}

//-----------------------------------------------------------------------------
//      プロパティを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParseProperties()
{
    /*
        Properties
        {
            bool    value1("flag") = false;
            float   value1("alpha", 0.1, range(0.0f, 1.0f)) = 1.0f;
            float2  value2("uv_offset", 0.01f) = float2(0.0f, 0.0f);
            float3  value3("color_scale", 0.01f) = float3(1.0f, 1.0f, 1.0f);
            float4  value4("test", 1.0f) = float4(0.0f, 0.0f, 0.0f, 0.0f);
            color3  value5("emissive") = color3(0.0f, 0.0f, 0.0f);
            color4  value6("base_color") = color4(0.0f, 0.0f, 0.0f, 1.0f);
            map1D   myTexture1("noise", false) = "texture_file_name";
            map2D   myTexture2("roughness") = "white";
            map3D   myTexture3("volume", true) = "white";
            mapCube mayTexture4("ibl_diffuse", false) = "black";
        };
    
    */
    m_Tokenizer.Next();

    assert(m_Tokenizer.Compare("{"));
    int count = 1;
    m_Tokenizer.Next();

    uint32_t bufferSize = 0;
    uint32_t offset = 0;


    while(!m_Tokenizer.IsEnd())
    {
        // ブロック終了.
        if (m_Tokenizer.Compare("}"))
        {
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(";"));
            count--;
            if (count == 0)
            { break; }
        }
        // ブロック開始.
        else if (m_Tokenizer.Compare("{"))
        {
            count++;
        }

        if (m_Tokenizer.CompareAsLower("bool"))
        {
            // 次のフォーマット.
            // bool name("display") = default;

            auto name = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("("));
            auto display_tag = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("="));
            auto defValue = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(";"));

            ValueProperty prop;
            prop.Name           = name;
            prop.DisplayTag     = display_tag;
            prop.Type           = PROPERTY_TYPE_BOOL;
            prop.Offset         = offset;
            prop.Step           = 0;
            prop.Min            = 0.0f;
            prop.Max            = 0.0f;
            prop.DefaultValue0  = defValue;

            m_Properties.Values.push_back(prop);

            offset += sizeof(int); // シェーダ上で bool は 4byteであるため.
        }
        else if (m_Tokenizer.CompareAsLower("int"))
        {
            // 次のフォーマット
            // int name("display", step, range(min, max)) = default;

            auto name = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("("));
            auto display_tag = std::string(m_Tokenizer.NextAsChar());
            auto step = m_Tokenizer.NextAsFloat();
            m_Tokenizer.Next();

            float mini = 0.0f;
            float maxi = 0.0f;
            if (m_Tokenizer.CompareAsLower("range"))
            {
                m_Tokenizer.Next();
                assert(m_Tokenizer.Compare("("));
                mini = m_Tokenizer.NextAsFloat();
                maxi = m_Tokenizer.NextAsFloat();
                m_Tokenizer.Next();
                assert(m_Tokenizer.Compare(")"));
                m_Tokenizer.Next();
            }

            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("="));
            auto defValue = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(";"));

            ValueProperty prop;
            prop.Name           = name;
            prop.DisplayTag     = display_tag;
            prop.Type           = PROPERTY_TYPE_INT;
            prop.Offset         = offset;
            prop.Step           = step;
            prop.Min            = mini;
            prop.Max            = maxi;
            prop.DefaultValue0  = defValue;

            m_Properties.Values.push_back(prop);

            offset += sizeof(int);
        }
        else if (m_Tokenizer.CompareAsLower("float"))
        {
            // 次のフォーマット
            // float name("display", step, range(min, max)) = default;

            auto name = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("("));
            auto display_tag = std::string(m_Tokenizer.NextAsChar());
            auto step = m_Tokenizer.NextAsFloat();
            m_Tokenizer.Next();

            float mini = 0.0f;
            float maxi = 0.0f;
            if (m_Tokenizer.CompareAsLower("range"))
            {
                m_Tokenizer.Next();
                assert(m_Tokenizer.Compare("("));
                mini = m_Tokenizer.NextAsFloat();
                maxi = m_Tokenizer.NextAsFloat();
                m_Tokenizer.Next();
                assert(m_Tokenizer.Compare(")"));
                m_Tokenizer.Next();
            }

            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("="));
            auto defValue = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(";"));
            m_Tokenizer.Next();


            ValueProperty prop;
            prop.Name           = name;
            prop.DisplayTag     = display_tag;
            prop.Type           = PROPERTY_TYPE_FLOAT;
            prop.Offset         = offset;
            prop.Step           = step;
            prop.Min            = mini;
            prop.Max            = maxi;
            prop.DefaultValue0  = defValue;

            m_Properties.Values.push_back(prop);

            offset += sizeof(float);
        }
        else if (m_Tokenizer.CompareAsLower("float2"))
        {
            // 次のフォーマット
            // float2 name("display", step, range(min, max)) = float2(default.x, default.y);

            auto name = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("("));
            auto display_tag = std::string(m_Tokenizer.NextAsChar());
            auto step = m_Tokenizer.NextAsFloat();
            m_Tokenizer.Next();

            float mini = 0.0f;
            float maxi = 0.0f;
            if (m_Tokenizer.CompareAsLower("range"))
            {
                m_Tokenizer.Next();
                assert(m_Tokenizer.Compare("("));
                mini = m_Tokenizer.NextAsFloat();
                maxi = m_Tokenizer.NextAsFloat();
                m_Tokenizer.Next();
                assert(m_Tokenizer.Compare(")"));
                m_Tokenizer.Next();
            }

            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("="));
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("float2"));
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("("));
            auto defValueX = std::string(m_Tokenizer.NextAsChar());
            auto defValueY = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(";"));
            m_Tokenizer.Next();

            ValueProperty prop;
            prop.Name           = name;
            prop.DisplayTag     = display_tag;
            prop.Type           = PROPERTY_TYPE_FLOAT2;
            prop.Offset         = offset;
            prop.Step           = step;
            prop.Min            = mini;
            prop.Max            = maxi;
            prop.DefaultValue0  = defValueX;
            prop.DefaultValue1  = defValueY;

            m_Properties.Values.push_back(prop);

            offset += (sizeof(float) * 2);
        }
        else if (m_Tokenizer.CompareAsLower("float3"))
        {
            // 次のフォーマット
            // float3 name("display", step, range(min, max)) = float3(default.x, default.y, default.z);

            auto name = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("("));
            auto display_tag = std::string(m_Tokenizer.NextAsChar());
            auto step = m_Tokenizer.NextAsFloat();
            m_Tokenizer.Next();

            float mini = 0.0f;
            float maxi = 0.0f;
            if (m_Tokenizer.CompareAsLower("range"))
            {
                m_Tokenizer.Next();
                assert(m_Tokenizer.Compare("("));
                mini = m_Tokenizer.NextAsFloat();
                maxi = m_Tokenizer.NextAsFloat();
                m_Tokenizer.Next();
                assert(m_Tokenizer.Compare(")"));
                m_Tokenizer.Next();
            }

            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("="));
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("float3"));
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("("));
            auto defValueX = std::string(m_Tokenizer.NextAsChar());
            auto defValueY = std::string(m_Tokenizer.NextAsChar());
            auto defValueZ = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(";"));
            m_Tokenizer.Next();

            ValueProperty prop;
            prop.Name           = name;
            prop.DisplayTag     = display_tag;
            prop.Type           = PROPERTY_TYPE_FLOAT3;
            prop.Offset         = offset;
            prop.Step           = step;
            prop.Min            = mini;
            prop.Max            = maxi;
            prop.DefaultValue0  = defValueX;
            prop.DefaultValue1  = defValueY;
            prop.DefaultValue2  = defValueZ;

            m_Properties.Values.push_back(prop);

            offset += (sizeof(float) * 3);
        }
        else if (m_Tokenizer.CompareAsLower("float4"))
        {
            // 次のフォーマット
            // float4 name("display", step, range(min, max)) = float4(default.x, default.y, default.z, default.w);

            auto name = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("("));
            auto display_tag = std::string(m_Tokenizer.NextAsChar());
            auto step = m_Tokenizer.NextAsFloat();
            m_Tokenizer.Next();

            float mini = 0.0f;
            float maxi = 0.0f;
            if (m_Tokenizer.CompareAsLower("range"))
            {
                m_Tokenizer.Next();
                assert(m_Tokenizer.Compare("("));
                mini = m_Tokenizer.NextAsFloat();
                maxi = m_Tokenizer.NextAsFloat();
                m_Tokenizer.Next();
                assert(m_Tokenizer.Compare(")"));
                m_Tokenizer.Next();
            }

            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("="));
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("float4"));
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("("));
            auto defValueX = std::string(m_Tokenizer.NextAsChar());
            auto defValueY = std::string(m_Tokenizer.NextAsChar());
            auto defValueZ = std::string(m_Tokenizer.NextAsChar());
            auto defValueW = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(";"));
            m_Tokenizer.Next();

            ValueProperty prop;
            prop.Name           = name;
            prop.DisplayTag     = display_tag;
            prop.Type           = PROPERTY_TYPE_FLOAT4;
            prop.Offset         = offset;
            prop.Step           = step;
            prop.Min            = mini;
            prop.Max            = maxi;
            prop.DefaultValue0  = defValueX;
            prop.DefaultValue1  = defValueY;
            prop.DefaultValue2  = defValueZ;
            prop.DefaultValue3  = defValueW;

            m_Properties.Values.push_back(prop);

            offset += (sizeof(float) * 4);
        }
        else if (m_Tokenizer.CompareAsLower("color3"))
        {
            // 次のフォーマット
            // color3 name("display") = color3(default.r, default.g, default.b);

            auto name = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("("));
            auto display_tag = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("="));
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("color3"));
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("("));
            auto defValueX = std::string(m_Tokenizer.NextAsChar());
            auto defValueY = std::string(m_Tokenizer.NextAsChar());
            auto defValueZ = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();

            ValueProperty prop;
            prop.Name           = name;
            prop.DisplayTag     = display_tag;
            prop.Type           = PROPERTY_TYPE_COLOR3;
            prop.Offset         = offset;
            prop.Step           = 0.0f;
            prop.Min            = 0.0f;
            prop.Max            = 0.0f;
            prop.DefaultValue0  = defValueX;
            prop.DefaultValue1  = defValueY;
            prop.DefaultValue2  = defValueZ;

            m_Properties.Values.push_back(prop);

            offset += (sizeof(float) * 3);
        }
        else if (m_Tokenizer.CompareAsLower("color4"))
        {
            // 次のフォーマット
            // color4 name("display") = color4(default.r, default.g, default.b, default.a);

            auto name = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("("));
            auto display_tag = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("="));
            m_Tokenizer.Next();

            assert(m_Tokenizer.Compare("color4"));
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare("("));
            auto defValueX = std::string(m_Tokenizer.NextAsChar());
            auto defValueY = std::string(m_Tokenizer.NextAsChar());
            auto defValueZ = std::string(m_Tokenizer.NextAsChar());
            auto defValueW = std::string(m_Tokenizer.NextAsChar());
            m_Tokenizer.Next();
            assert(m_Tokenizer.Compare(")"));
            m_Tokenizer.Next();

            ValueProperty prop;
            prop.Name           = name;
            prop.DisplayTag     = display_tag;
            prop.Type           = PROPERTY_TYPE_COLOR4;
            prop.Offset         = offset;
            prop.Step           = 0.0f;
            prop.Min            = 0.0f;
            prop.Max            = 0.0f;
            prop.DefaultValue0  = defValueX;
            prop.DefaultValue1  = defValueY;
            prop.DefaultValue2  = defValueZ;
            prop.DefaultValue3  = defValueW;

            m_Properties.Values.push_back(prop);

            offset += (sizeof(float) * 4);
        }
        else if (m_Tokenizer.CompareAsLower("map1d"))
        {
            ParseTextureProperty(PROPERTY_TYPE_TEXTURE1D);
        }
        else if (m_Tokenizer.CompareAsLower("map1darray"))
        {
            ParseTextureProperty(PROPERTY_TYPE_TEXTURE1D_ARRAY);
        }
        else if (m_Tokenizer.CompareAsLower("map2d"))
        {
            ParseTextureProperty(PROPERTY_TYPE_TEXTURE2D);
        }
        else if (m_Tokenizer.CompareAsLower("map2darray"))
        {
            ParseTextureProperty(PROPERTY_TYPE_TEXTURE2D_ARRAY);
        }
        else if (m_Tokenizer.CompareAsLower("map3d"))
        {
            ParseTextureProperty(PROPERTY_TYPE_TEXTURE3D);
        }
        else if (m_Tokenizer.CompareAsLower("mapcube"))
        {
            ParseTextureProperty(PROPERTY_TYPE_TEXTURECUBE);
        }
        else if (m_Tokenizer.CompareAsLower("mapcubearray"))
        {
            ParseTextureProperty(PROPERTY_TYPE_TEXTURECUBE_ARRAY);
        }
        else
        {
            m_Tokenizer.Next();
        }
    }

    // メモリ最適化.
    m_Properties.Values.shrink_to_fit();
    m_Properties.Textures.shrink_to_fit();

    m_Properties.BufferSize = offset;

    if (!m_Properties.Values.empty())
    {
        m_SourceCode += "cbuffer CbProperties\n";
        m_SourceCode += "{\n";

        for(auto& prop : m_Properties.Values)
        {
            m_SourceCode += "    ";
            switch(prop.Type)
            {
            case PROPERTY_TYPE_BOOL:
                {
                    m_SourceCode += "int";
                    m_SourceCode += " ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_INT:
                {
                    m_SourceCode += "int";
                    m_SourceCode += " ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_FLOAT:
                {
                    m_SourceCode += "float";
                    m_SourceCode += " ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_FLOAT2:
                {
                    m_SourceCode += "float2";
                    m_SourceCode += " ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_FLOAT3:
                {
                    m_SourceCode += "float3";
                    m_SourceCode += " ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_FLOAT4:
                {
                    m_SourceCode += "float4";
                    m_SourceCode += " ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_COLOR3:
                {
                    m_SourceCode += "float3";
                    m_SourceCode += " ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_COLOR4:
                {
                    m_SourceCode += "float4";
                    m_SourceCode += " ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;
            }
        }

        m_SourceCode += "};\n\n";
    }

    if (!m_Properties.Textures.empty())
    {
        for(auto& prop : m_Properties.Textures)
        {
            switch(prop.Type)
            {
            case PROPERTY_TYPE_TEXTURE1D:
                {
                    m_SourceCode += "Texture1D ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_TEXTURE1D_ARRAY:
                {
                    m_SourceCode += "Texture1DArray ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_TEXTURE2D:
                {
                    m_SourceCode += "Texture2D ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_TEXTURE2D_ARRAY:
                {
                    m_SourceCode += "Texture2DArray ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_TEXTURE3D:
                {
                    m_SourceCode += "Texture3D ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_TEXTURECUBE:
                {
                    m_SourceCode += "TextureCube ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;

            case PROPERTY_TYPE_TEXTURECUBE_ARRAY:
                {
                    m_SourceCode += "TextureCubeArray ";
                    m_SourceCode += prop.Name;
                    m_SourceCode += ";";
                    m_SourceCode += "    //";
                    m_SourceCode += prop.DisplayTag;
                    m_SourceCode += "\n";
                }
                break;
            }
        }
        m_SourceCode += "\n";
    }
}

//-----------------------------------------------------------------------------
//      テクスチャプロパティを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParseTextureProperty(PROPERTY_TYPE type)
{
    // 次のフォーマット.
    // mapXXX name("display", srgb) = default;
    auto srgb = false;

    auto name = std::string(m_Tokenizer.NextAsChar());
    m_Tokenizer.Next();
    assert(m_Tokenizer.Compare("("));
    auto display_tag = std::string(m_Tokenizer.NextAsChar());

    m_Tokenizer.Next();
    if (!m_Tokenizer.Compare(")"))
    {
        srgb = m_Tokenizer.GetAsBool();
        m_Tokenizer.Next();
    }
    assert(m_Tokenizer.Compare(")"));

    m_Tokenizer.Next();
    assert(m_Tokenizer.Compare("="));

    auto defValue = std::string(m_Tokenizer.NextAsChar());
    m_Tokenizer.Next();
    assert(m_Tokenizer.Compare(";"));
    m_Tokenizer.Next();

    TextureProperty prop;
    prop.Name           = name;
    prop.DisplayTag     = display_tag;
    prop.Type           = type;
    prop.EnableSRGB     = srgb;
    prop.DefaultValue   = defValue;

    m_Properties.Textures.push_back(prop);
}

//-----------------------------------------------------------------------------
//      リソースを解析します.
//-----------------------------------------------------------------------------
void FxParser::ParseResource()
{
    Resource res = {};

    if (m_Tokenizer.CompareAsLower("Texture1D"))
    {
        ParseResourceDetail(RESOURCE_TYPE_TEXTURE1D);
    }
    else if (m_Tokenizer.CompareAsLower("Texture1DArray"))
    {
        ParseResourceDetail(RESOURCE_TYPE_TEXTURE1DARRAY);
    }
    else if (m_Tokenizer.CompareAsLower("Texture2D"))
    {
        ParseResourceDetail(RESOURCE_TYPE_TEXTURE2D);
    }
    else if (m_Tokenizer.CompareAsLower("Texture2DArray"))
    {
        ParseResourceDetail(RESOURCE_TYPE_TEXTURE2DARRAY);
    }
    else if (m_Tokenizer.CompareAsLower("Texture2DMS"))
    {
        ParseResourceDetail(RESOURCE_TYPE_TEXTURE2DMS);
    }
    else if (m_Tokenizer.CompareAsLower("Texture2DMSArray"))
    {
        ParseResourceDetail(RESOURCE_TYPE_TEXTURE2DMSARRAY);
    }
    else if (m_Tokenizer.CompareAsLower("Texture3D"))
    {
        ParseResourceDetail(RESOURCE_TYPE_TEXTURE3D);
    }
    else if (m_Tokenizer.CompareAsLower("TextureCube"))
    {
        ParseResourceDetail(RESOURCE_TYPE_TEXTURECUBE);
    }
    else if (m_Tokenizer.CompareAsLower("TextureCubeArray"))
    {
        ParseResourceDetail(RESOURCE_TYPE_TEXTURECUBEARRAY);
    }
    else if (m_Tokenizer.CompareAsLower("Buffer"))
    {
        ParseResourceDetail(RESOURCE_TYPE_BUFFER);
    }
    else if (m_Tokenizer.CompareAsLower("ByteAddressBuffer"))
    {
        ParseResourceDetail(RESOURCE_TYPE_BYTEADDRESS_BUFFER);
    }
    else if (m_Tokenizer.CompareAsLower("StructuredBuffer"))
    {
        ParseResourceDetail(RESOURCE_TYPE_STRUCTURED_BUFFER);
    }
    else if (m_Tokenizer.CompareAsLower("RWTexture1D"))
    {
        ParseResourceDetail(RESOURCE_TYPE_RWTEXTURE1D);
    }
    else if (m_Tokenizer.CompareAsLower("RWTexture1DArray"))
    {
        ParseResourceDetail(RESOURCE_TYPE_RWTEXTURE1DARRAY);
    }
    else if (m_Tokenizer.CompareAsLower("RWTexture2D"))
    {
        ParseResourceDetail(RESOURCE_TYPE_RWTEXTURE2D);
    }
    else if (m_Tokenizer.CompareAsLower("RWTexture2DArray"))
    {
        ParseResourceDetail(RESOURCE_TYPE_RWTEXTURE2DARRAY);
    }
    else if (m_Tokenizer.CompareAsLower("RWTexture3D"))
    {
        ParseResourceDetail(RESOURCE_TYPE_RWTEXTURE3D);
    }
    else if (m_Tokenizer.CompareAsLower("RWBuffer"))
    {
        ParseResourceDetail(RESOURCE_TYPE_RWBUFFER);
    }
    else if (m_Tokenizer.CompareAsLower("RWByteAddressBuffer"))
    {
        ParseResourceDetail(RESOURCE_TYPE_RWBYTEADDRESS_BUFFER);
    }
    else if (m_Tokenizer.CompareAsLower("RWStructuredBuffer"))
    {
        ParseResourceDetail(RESOURCE_TYPE_RWSTRUCTURED_BUFFER);
    }
    else if (m_Tokenizer.CompareAsLower("SamplerState"))
    {
        ParseResourceDetail(RESOURCE_TYPE_SAMPLER_STATE);
    }
    else if (m_Tokenizer.CompareAsLower("SamplerComparisonState"))
    {
        ParseResourceDetail(RESOURCE_TYPE_SAMPLER_COMPRISON_STATE);
    }
}

//-----------------------------------------------------------------------------
//      リソース解析の実態.
//-----------------------------------------------------------------------------
void FxParser::ParseResourceDetail(RESOURCE_TYPE type)
{
    m_Tokenizer.Next();

    MEMBER_TYPE dataType = MEMBER_TYPE_UNKNOWN;

    if (m_Tokenizer.Compare("<"))
    {
        m_Tokenizer.Next();

        if (m_Tokenizer.CompareAsLower("float"))
        {
            dataType = MEMBER_TYPE_FLOAT;
        }
        else if (m_Tokenizer.CompareAsLower("float2"))
        {
            dataType = MEMBER_TYPE_FLOAT2;
        }
        else if (m_Tokenizer.CompareAsLower("float3"))
        {
            dataType = MEMBER_TYPE_FLOAT3;
        }
        else if (m_Tokenizer.CompareAsLower("float4"))
        {
            dataType = MEMBER_TYPE_FLOAT4;
        }
        else if (m_Tokenizer.CompareAsLower("double"))
        {
            dataType = MEMBER_TYPE_DOUBLE;
        }
        else if (m_Tokenizer.CompareAsLower("double2"))
        {
            dataType = MEMBER_TYPE_DOUBLE2;
        }
        else if (m_Tokenizer.CompareAsLower("double3"))
        {
            dataType = MEMBER_TYPE_DOUBLE3;
        }
        else if (m_Tokenizer.CompareAsLower("double4"))
        {
            dataType = MEMBER_TYPE_DOUBLE4;
        }
        else if (m_Tokenizer.CompareAsLower("int"))
        {
            dataType = MEMBER_TYPE_INT;
        }
        else if (m_Tokenizer.CompareAsLower("int2"))
        {
            dataType = MEMBER_TYPE_INT2;
        }
        else if (m_Tokenizer.CompareAsLower("int3"))
        {
            dataType = MEMBER_TYPE_INT3;
        }
        else if (m_Tokenizer.CompareAsLower("int4"))
        {
            dataType = MEMBER_TYPE_INT4;
        }
        else if (m_Tokenizer.CompareAsLower("uint"))
        {
            dataType = MEMBER_TYPE_UINT;
        }
        else if (m_Tokenizer.CompareAsLower("uint2"))
        {
            dataType = MEMBER_TYPE_UINT2;
        }
        else if (m_Tokenizer.CompareAsLower("uint3"))
        {
            dataType = MEMBER_TYPE_UINT3;
        }
        else if (m_Tokenizer.CompareAsLower("uint4"))
        {
            dataType = MEMBER_TYPE_UINT4;
        }
        else
        {
            auto name = m_Tokenizer.GetAsChar();
            if (m_Structures.find(name) != m_Structures.end())
            {
                dataType = MEMBER_TYPE_STRUCT;
            }
        }

        m_Tokenizer.Next();
        assert(m_Tokenizer.Compare(">"));
        m_Tokenizer.Next();
    }

    auto name = std::string(m_Tokenizer.GetAsChar());
    auto pos = name.find(";");
    auto end = false;
    if (pos != std::string::npos)
    {
        name = name.substr(0, pos);
        end  = true;
    }

    Resource res = {};
    res.Name            = name;
    res.ResourceType    = type;
    res.Register        = -1;
    res.DataType        = dataType;

    if (end)
    {
        if (m_Resources.find(name) == m_Resources.end())
        {
            m_Resources[name] = res;
            return;
        }
    }

    m_Tokenizer.Next();

    if (m_Tokenizer.Compare(":"))
    {
        m_Tokenizer.Next();
        assert(m_Tokenizer.CompareAsLower("register"));
        m_Tokenizer.Next(); // register
        assert(m_Tokenizer.Compare("("));
        m_Tokenizer.Next(); // "("
        auto reg = std::string(m_Tokenizer.GetAsChar());
        auto idx = std::stoi(reg.substr(1));
        res.Register = uint32_t(idx);
        m_Tokenizer.Next(); // txx
        assert(m_Tokenizer.Compare(")"));
        m_Tokenizer.Next(); // ")"
    }

    if (m_Resources.find(name) == m_Resources.end())
    {
        m_Resources[name] = res;
    }
}

//-----------------------------------------------------------------------------
//      トークンをシェーダタイプとして取得します.
//-----------------------------------------------------------------------------
SHADER_TYPE FxParser::GetShaderType()
{
    SHADER_TYPE type = SHADER_TYPE(-1);

    if (m_Tokenizer.CompareAsLower("vertexshader"))
    {
        type = SHADER_TYPE_VERTEX;
    }
    else if (m_Tokenizer.CompareAsLower("pixelshader"))
    {
        type = SHADER_TYPE_PIXEL;
    }
    else if (m_Tokenizer.CompareAsLower("geometryshader"))
    {
        type = SHADER_TYPE_GEOMETRY;
    }
    else if (m_Tokenizer.CompareAsLower("domainshader"))
    {
        type = SHADER_TYPE_DOMAIN;
    }
    else if (m_Tokenizer.CompareAsLower("hullshader"))
    {
        type = SHADER_TYPE_HULL;
    }
    else if (m_Tokenizer.CompareAsLower("computeshader"))
    {
        type = SHADER_TYPE_COMPUTE;
    }
    else if (m_Tokenizer.CompareAsLower("amplificationshader"))
    {
        type = SHADER_TYPE_AMPLIFICATION;
    }
    else if (m_Tokenizer.CompareAsLower("meshshader"))
    {
        type = SHADER_TYPE_MESH;
    }

    return type;
}

//-----------------------------------------------------------------------------
//      ソースコードを取得します.
//-----------------------------------------------------------------------------
const char* FxParser::GetSourceCode() const
{ return m_SourceCode.c_str(); }

//-----------------------------------------------------------------------------
//      ソースコードサイズを取得します.
//-----------------------------------------------------------------------------
size_t FxParser::GetSourceCodeSize() const
{ return m_SourceCode.size(); }

//-----------------------------------------------------------------------------
//      ブレンドステートを取得します.
//-----------------------------------------------------------------------------
const std::map<std::string, D3D12_BLEND_DESC>& FxParser::GetBlendStates() const
{ return m_BlendStates; }

//-----------------------------------------------------------------------------
//      ラスタライザーステートを取得します.
//-----------------------------------------------------------------------------
const std::map<std::string, D3D12_RASTERIZER_DESC>& FxParser::GetRasterizerStates() const
{ return m_RasterizerStates; }

//-----------------------------------------------------------------------------
//      深度ステンシルステートを取得します.
//-----------------------------------------------------------------------------
const std::map<std::string, D3D12_DEPTH_STENCIL_DESC>& FxParser::GetDepthStencilStates() const
{ return m_DepthStencilStates; }

//-----------------------------------------------------------------------------
//      定数バッファを取得します.
//-----------------------------------------------------------------------------
const std::map<std::string, FxParser::ConstantBuffer>& FxParser::GetConstantBuffers() const
{ return m_ConstantBuffers; }

//-----------------------------------------------------------------------------
//      構造体を取得します.
//-----------------------------------------------------------------------------
const std::map<std::string, FxParser::Structure>& FxParser::GetStructures() const
{ return m_Structures; }

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
const std::map<std::string, FxParser::Resource>& FxParser::GetResources() const
{ return m_Resources; }

//-----------------------------------------------------------------------------
//      テクニックを取得します.
//-----------------------------------------------------------------------------
const std::vector<FxParser::Technique>& FxParser::GetTechniques() const
{ return m_Technieues; }

//-----------------------------------------------------------------------------
//      プロパティを取得します.
//-----------------------------------------------------------------------------
const FxParser::Properties& FxParser::GetProperties() const
{ return m_Properties; }

//-----------------------------------------------------------------------------
//      インクルード文を収集します.
//-----------------------------------------------------------------------------
bool FxParser::CorrectIncludes(const char* filename)
{
    std::ifstream stream;
    stream.open(filename, std::ios::in);

    if (!stream.is_open())
    {
        return false;
    }

    std::string line;

    for (;;)
    {
        if (!stream)
        { break; }

        if (stream.eof())
        { break; }

        stream >> line;

        auto pos = line.find("#include");
        if (pos != std::string::npos)
        {
            stream >> line;

            Info info;
            info.IncludeFile = "#include ";
            info.IncludeFile += line;

            line = Replace(line, "\"", "");
            line = Replace(line, "<", "");
            line = Replace(line, ">", "");
            line = Replace(line, "\r\n", "");

            info.FindPath    = line;

            for (size_t i = 0; i < m_DirPaths.size(); ++i)
            {
                auto path = m_DirPaths[i] + "\\" + line;
                if (CorrectIncludes(path.c_str()))
                {
                    info.FindPath = path;
                    break;
                }
            }

            // ここで追加.
            m_Includes.push_back(info);
        }
    }

    stream.close();
    return true;
}

//-----------------------------------------------------------------------------
//      インクルード文を展開します.
//-----------------------------------------------------------------------------
bool FxParser::Expand(std::string& inout)
{
    bool find = false;

    for (size_t i = 0; i < m_Includes.size(); ++i)
    {
        bool hit = false;
        inout = Replace(inout, m_Includes[i].IncludeFile, m_Includes[i].Code, hit);
        if (hit)
        { find = true; }
    }

    inout = Replace(inout, "\r\n", "\n");

    return find;
}

} // namespace asura
