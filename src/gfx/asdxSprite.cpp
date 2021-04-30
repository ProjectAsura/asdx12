//-----------------------------------------------------------------------------
// File : asdxSprite.cpp
// Desc : Sprite System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxSprite.h>
#include <gfx/asdxGraphicsSystem.h>
#include <core/asdxLogger.h>


namespace asdx {

#include "../res/shaders/Compiled/SpriteVS.inc"
#include "../res/shaders/Compiled/SpritePS.inc"

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const Vector2    kZero               = Vector2(0.0f, 0.0f);
static const Vector2    kOne                = Vector2(1.0f, 1.0f);
static const uint32_t   kVertexPerSprite    = 4;
static const uint32_t   kIndexPerSprite     = 6;


///////////////////////////////////////////////////////////////////////////////
// ROOT_PARAM
///////////////////////////////////////////////////////////////////////////////
enum ROOT_PARAM
{
    ROOT_PARAM_TRANSFORM    = 0,
    ROOT_PARAM_COLOR        = 1,
    ROOT_PARAM_SRV          = 2,
};


///////////////////////////////////////////////////////////////////////////////
// SpriteSystem class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SpriteSystem::SpriteSystem()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SpriteSystem::~SpriteSystem()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SpriteSystem::Init
(
    uint32_t    maxSpriteCount,
    float       width,
    float       height,
    DXGI_FORMAT colorFormat,
    DXGI_FORMAT depthFormat
)
{
    auto pDevice = GetD3D12Device();
    if (pDevice == nullptr || maxSpriteCount == 0)
    {
        ELOG("Error : Invalid Argument");
        return false;
    }

    // 頂点バッファ生成.
    auto vertexCount = maxSpriteCount * kVertexPerSprite;
    if (!m_VB.Init(sizeof(Vertex) * vertexCount, sizeof(Vertex)))
    {
        ELOGA("Error : VertexBuffer::Init() Failed.");
        return false;
    }

    // インデックスバッファ生成.
    auto indexCount = maxSpriteCount * kIndexPerSprite;
    if (!m_IB.Init(sizeof(uint32_t) * indexCount))
    {
        ELOGA("Error : IndexBuffer::Init() Failed.");
        return true;
    }

    // インデックス設定.
    {
        auto indices = m_IB.Map<uint32_t>();
        auto idx = 0;
        for(auto i=0u; i<vertexCount; i+=kVertexPerSprite)
        {
            indices[idx] = i + 0; idx++;
            indices[idx] = i + 1; idx++;
            indices[idx] = i + 2; idx++;

            indices[idx] = i + 1; idx++;
            indices[idx] = i + 3; idx++;
            indices[idx] = i + 2; idx++;
        }
        m_IB.Unmap();
    }

    // ルートシグニチャ生成.
    {
        RANGE_SRV srvRange(0);

        D3D12_ROOT_PARAMETER params[3] = {
            PARAM_CONSTANT(D3D12_SHADER_VISIBILITY_VERTEX, 16, 0 ),
            PARAM_CONSTANT(D3D12_SHADER_VISIBILITY_PIXEL, 4, 1),
            PARAM_TABLE(D3D12_SHADER_VISIBILITY_PIXEL, 1, &srvRange)
        };

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(params);
        desc.NumStaticSamplers  = 0;
        desc.pParameters        = params;
        desc.pStaticSamplers    = nullptr;
        desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        if (!m_RootSig.Init(pDevice, &desc))
        {
            ELOGA("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // パイプラインステート生成.
    {
        D3D12_INPUT_ELEMENT_DESC elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSig.GetPtr();
        desc.VS                 = { SpriteVS, sizeof(SpriteVS) };
        desc.PS                 = { SpritePS, sizeof(SpritePS) };
        desc.BlendState         = BLEND_DESC(BLEND_STATE_OPAQUE);
        desc.SampleMask         = UINT_MAX;
        desc.RasterizerState    = RASTERIZER_DESC(RASTERIZER_STATE_CULL_NONE);
        desc.DepthStencilState  = DEPTH_STENCIL_DESC(DEPTH_STATE_DEFAULT);
        desc.NumRenderTargets   = 1;
        desc.RTVFormats[0]      = colorFormat;
        desc.DSVFormat          = depthFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;

        if (!m_PSO.Init(pDevice, &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    m_MaxSpriteCount = maxSpriteCount;
    m_ScreenSize.x   = width;
    m_ScreenSize.y   = height;
    m_pSRV           = nullptr;
    m_pCmd           = nullptr;
    m_CurSpriteCount = 0;
    m_PreSpriteCount = 0;
    m_Color          = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SpriteSystem::Term()
{
    m_VB     .Term();
    m_IB     .Term();
    m_RootSig.Term();
    m_PSO    .Term();
    m_pSRV           = nullptr;
    m_CurSpriteCount = 0;
    m_PreSpriteCount = 0;
}

//-----------------------------------------------------------------------------
//      描画処理を開始します.
//-----------------------------------------------------------------------------
void SpriteSystem::Begin(ID3D12GraphicsCommandList* pCmd)
{
    m_pCmd = pCmd;

    if (m_pCmd == nullptr)
    { return; }

    m_pCmd->SetGraphicsRootSignature(m_RootSig.GetPtr());
    m_pCmd->SetPipelineState(m_PSO.GetPtr());
    m_pCmd->SetGraphicsRoot32BitConstants(ROOT_PARAM_TRANSFORM, 16, &m_Transform._11, 0);
    m_pCmd->SetGraphicsRoot32BitConstants(ROOT_PARAM_COLOR, 4, &m_Color.x, 0);

    if (m_pSRV != nullptr)
    { m_pCmd->SetGraphicsRootDescriptorTable(ROOT_PARAM_SRV, m_pSRV->GetHandleGPU()); }
}

//-----------------------------------------------------------------------------
//      描画処理を終了します.
//-----------------------------------------------------------------------------
void SpriteSystem::End()
{
    // ドローコールを発行.
    MakeCmd();

    // スプライトカウントをリセット.
    m_CurSpriteCount = 0;
    m_PreSpriteCount = 0;
}

//-----------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-----------------------------------------------------------------------------
void SpriteSystem::SetSRV(IShaderResourceView* pSRV)
{
    m_pSRV = pSRV;

    // ドローコール発行.
    MakeCmd();
}

//-----------------------------------------------------------------------------
//      スクリーンサイズを設定します.
//-----------------------------------------------------------------------------
void SpriteSystem::SetScreenSize(float width, float height)
{ SetScreenSize(Vector2(width, height)); }

//-----------------------------------------------------------------------------
//      スクリーンサイズを設定します.
//-----------------------------------------------------------------------------
void SpriteSystem::SetScreenSize(const Vector2& size)
{
    m_ScreenSize = size;

    float xScale = (m_ScreenSize.x > 0.0f) ? 2.0f / m_ScreenSize.x : 0.0f;
    float yScale = (m_ScreenSize.y > 0.0f) ? 2.0f / m_ScreenSize.y : 0.0f;

    m_Transform = asdx::Matrix(
        xScale,     0.0f,   0.0f,   0.0f,
         0.0f,   -yScale,   0.0f,   0.0f,
         0.0f,      0.0f,   1.0f,   0.0f,
        -1.0f,      1.0f,   0.0f,   1.0f );

    if (m_pCmd == nullptr)
    { return; }

    m_pCmd->SetGraphicsRoot32BitConstants(ROOT_PARAM_TRANSFORM, 16, &m_Transform._11, 0);
}

//-----------------------------------------------------------------------------
//      カラーを設定します.
//-----------------------------------------------------------------------------
void SpriteSystem::SetColor(float r, float g, float b, float a)
{ SetColor(Vector4(r, g, b, a)); }

//-----------------------------------------------------------------------------
//      カラーを設定します.
//-----------------------------------------------------------------------------
void SpriteSystem::SetColor(const Vector4& color)
{
    m_Color = color; 

    if (m_pCmd == nullptr)
    { return; }

    m_pCmd->SetGraphicsRoot32BitConstants(ROOT_PARAM_COLOR, 4, &m_Color.x, 0);
}

//-----------------------------------------------------------------------------
//      スプライトを登録します.
//-----------------------------------------------------------------------------
void SpriteSystem::Draw(int x, int y, int w, int h)
{ Draw(x, y, w, h, kZero, kOne, 0); }

//-----------------------------------------------------------------------------
//      スプライトを登録します.
//-----------------------------------------------------------------------------
void SpriteSystem::Draw(int x, int y, int w, int h, int layerDepth)
{ Draw(x, y, w, h, kZero, kOne, layerDepth); }

//-----------------------------------------------------------------------------
//      スプライトを登録します.
//-----------------------------------------------------------------------------
void SpriteSystem::Draw(int x, int y, int w, int h, const Vector2& uv0, const Vector2& uv1)
{ Draw(x, y, w, h, uv0, uv1, 0); }

//-----------------------------------------------------------------------------
//      スプライトを登録します.
//-----------------------------------------------------------------------------
void SpriteSystem::Draw(int x, int y, int w, int h, const Vector2& uv0, const Vector2& uv1, int layerDepth)
{ Draw(Vector2(float(x), float(y)), Vector2(float(w), float(h)), uv0, uv1, layerDepth); }

//-----------------------------------------------------------------------------
//      スプライトを登録します.
//-----------------------------------------------------------------------------
void SpriteSystem::Draw(const Vector2& pos, const Vector2& size)
{ Draw(pos, size, kZero, kOne, 0); }

//-----------------------------------------------------------------------------
//      スプライトを登録します.
//-----------------------------------------------------------------------------
void SpriteSystem::Draw(const Vector2& pos, const Vector2& size, int layerDepth)
{ Draw(pos, size, kZero, kOne, layerDepth); }

//-----------------------------------------------------------------------------
//      スプライトを登録します.
//-----------------------------------------------------------------------------
void SpriteSystem::Draw(const Vector2& pos, const Vector2& size, const Vector2& uv0, const Vector2& uv1)
{ Draw(pos, size, uv0, uv1, 0); }

//-----------------------------------------------------------------------------
//      スプライトを登録します.
//-----------------------------------------------------------------------------
void SpriteSystem::Draw(const Vector2& pos, const Vector2& size, const Vector2& uv0, const Vector2& uv1, int layerDepth)
{
    if (m_Vertices.empty())
    { return; }

    // 最大スプライト数を超えないかチェック.
    if (m_CurSpriteCount + 1 > m_MaxSpriteCount)
    { return; }

    auto depth    = float(layerDepth);
    auto index    = m_CurSpriteCount * kVertexPerSprite;
    auto vertices = &m_Vertices[index];

    auto x0 = pos.x;
    auto x1 = pos.x + size.x;
    auto y0 = pos.y;
    auto y1 = pos.y + size.y;

    // Vertex : 0
    vertices[0].Position = Vector3(x0, y0, depth);
    vertices[0].TexCoord = Vector2(uv0.x, uv1.y);

    // Vertex : 1
    vertices[1].Position = Vector3(x1, y0, depth);
    vertices[1].TexCoord = Vector2(uv1.x, uv1.y);

    // Vertex : 2
    vertices[2].Position = Vector3(x0, y1, depth);
    vertices[2].TexCoord = Vector2(uv0.x, uv0.y);

    // Vertex : 3
    vertices[3].Position = Vector3(x1, y1, depth);
    vertices[2].TexCoord = Vector2(uv1.x, uv0.y);

    // スプライト数をカウントアップします.
    m_CurSpriteCount++;
}

//-----------------------------------------------------------------------------
//      スクリーンサイズを取得します.
//-----------------------------------------------------------------------------
Vector2 SpriteSystem::GetScreenSize() const
{ return m_ScreenSize; }

//-----------------------------------------------------------------------------
//      乗算カラーを取得します.
//-----------------------------------------------------------------------------
Vector4 SpriteSystem::GetColor() const
{ return m_Color; }

//-----------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-----------------------------------------------------------------------------
IShaderResourceView* SpriteSystem::GetSRV() const
{ return m_pSRV; }

//-----------------------------------------------------------------------------
//      ドローコールを発行します.
//-----------------------------------------------------------------------------
void SpriteSystem::MakeCmd()
{
    if (m_pCmd == nullptr)
    { return; }

    auto count = m_CurSpriteCount - m_PreSpriteCount;
    if (count == 0)
    { return; }

    auto vertexCount = kVertexPerSprite * count;

    auto ptr    = m_VB.Map<uint8_t>();
    auto offset = m_CurSpriteCount * kVertexPerSprite;
    ptr += offset;
    memcpy(ptr, &m_Vertices[m_PreSpriteCount], sizeof(Vertex) * vertexCount);
    m_VB.Unmap();

    auto indexCount = UINT(count * kIndexPerSprite);
    auto startIndex = UINT(m_PreSpriteCount * kIndexPerSprite);
    m_pCmd->DrawIndexedInstanced(indexCount, 1, startIndex, 0, 0);

    m_PreSpriteCount = m_CurSpriteCount;
}

} // namespace asdx
