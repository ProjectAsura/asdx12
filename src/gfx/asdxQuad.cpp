//-----------------------------------------------------------------------------
// File : asdxQuad.cpp
// Desc : Full Screen Quad.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxQuad.h>
#include <fnd/asdxMath.h>
#include <fnd/asdxLogger.h>


namespace {

#include "../res/shaders/Compiled/FullScreenVS.inc"

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const D3D12_INPUT_ELEMENT_DESC kElements[] = {
    { "POSITION", 0, DXGI_FORMAT_R16G16_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT, 0, 4, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

///////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////
struct Vertex 
{
    uint32_t Position; //!< 位置座標.
    uint32_t TexCoord; //!< テクスチャ座標.

    Vertex(float x, float y, float u, float v)
    : Position(asdx::EncodeHalf2(asdx::Vector2(x, y)).u)
    , TexCoord(asdx::EncodeHalf2(asdx::Vector2(u, v)).u)
    { /* DO_NOTHING */}
};

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Quad class
///////////////////////////////////////////////////////////////////////////////
const D3D12_INPUT_LAYOUT_DESC Quad::InputLayout = { kElements, 2 };
Quad Quad::s_Instance;

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Quad::Quad()
: m_Init(false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Quad::~Quad()
{ Term(); }

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
Quad& Quad::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化済みかどうか?
//-----------------------------------------------------------------------------
bool Quad::IsInit() const
{ return m_Init; }

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool Quad::Init(ID3D12Device* pDevice)
{
    Vertex vertices[] = {
        Vertex(-1.0f,  1.0f, 0.0f,  0.0f),
        Vertex( 3.0f,  1.0f, 2.0f,  0.0f),
        Vertex(-1.0f, -3.0f, 0.0f,  2.0f)
    };

    auto size   = uint64_t(sizeof(vertices));
    auto stride = uint32_t(sizeof(vertices[0]));

    if (!m_VB.Init(size, stride))
    {
        ELOG("Error : VertexBuffer::Init() Failed.");
        return false;
    }

    auto ptr = m_VB.Map<Vertex>();
    memcpy(ptr, vertices, size);
    m_VB.Unmap();

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理です.
//-----------------------------------------------------------------------------
void Quad::Term()
{ m_VB.Term(); }

//-----------------------------------------------------------------------------
//      描画処理です.
//-----------------------------------------------------------------------------
void Quad::Draw(ID3D12GraphicsCommandList* pCmd)
{
    auto vbv = m_VB.GetView();
    pCmd->IASetVertexBuffers(0, 1, &vbv);
    pCmd->IASetIndexBuffer(nullptr);
    pCmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCmd->DrawInstanced(3, 1, 0, 0);
}

//-----------------------------------------------------------------------------
//      頂点シェーダを取得します.
//-----------------------------------------------------------------------------
D3D12_SHADER_BYTECODE Quad::GetVS()
{
    D3D12_SHADER_BYTECODE result;
    result.pShaderBytecode = FullScreenVS;
    result.BytecodeLength = sizeof(FullScreenVS);
    return result;
}

} // namespace asdx
