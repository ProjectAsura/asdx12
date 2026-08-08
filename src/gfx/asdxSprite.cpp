//-----------------------------------------------------------------------------
// File : asdxSprite.cpp
// Desc : Sprite Renderer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxSprite.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxScopedMarker.h>
#include "D3D12MemAlloc.h"


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxSpriteVS.inc"
#include "../res/shaders/Compiled/asdxSpritePS.inc"

static constexpr uint32_t kVertexCountPerSprite = 4;
static constexpr uint32_t kIndexCountPerSprite  = 6;

static const D3D12_INPUT_ELEMENT_DESC kElements[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR"   , 0, DXGI_FORMAT_R8G8B8A8_UNORM , 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

const D3D12_RASTERIZER_DESC kCullBack = {
    D3D12_FILL_MODE_SOLID,
    D3D12_CULL_MODE_BACK,
    FALSE,
    D3D12_DEFAULT_DEPTH_BIAS,
    D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
    D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
    TRUE,
    FALSE,
    FALSE,
    0,
    D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
};

const D3D12_DEPTH_STENCILOP_DESC kStencilDefault = {
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_KEEP,
    D3D12_COMPARISON_FUNC_ALWAYS
};

const D3D12_DEPTH_STENCIL_DESC kDepthNone = {
    FALSE,
    D3D12_DEPTH_WRITE_MASK_ZERO,
    D3D12_COMPARISON_FUNC_ALWAYS,
    FALSE,
    D3D12_DEFAULT_STENCIL_READ_MASK,
    D3D12_DEFAULT_STENCIL_WRITE_MASK,
    kStencilDefault,
    kStencilDefault
};

const D3D12_DEPTH_STENCIL_DESC kDepthDefault = {
    TRUE,
    D3D12_DEPTH_WRITE_MASK_ALL,
    D3D12_COMPARISON_FUNC_LESS_EQUAL,
    FALSE,
    D3D12_DEFAULT_STENCIL_READ_MASK,
    D3D12_DEFAULT_STENCIL_WRITE_MASK,
    kStencilDefault,
    kStencilDefault
};

const D3D12_RENDER_TARGET_BLEND_DESC kRTB_AlphaBlend = {
    TRUE,
    FALSE,
    D3D12_BLEND_SRC_ALPHA,
    D3D12_BLEND_INV_SRC_ALPHA,
    D3D12_BLEND_OP_ADD,
    D3D12_BLEND_SRC_ALPHA,
    D3D12_BLEND_INV_SRC_ALPHA,
    D3D12_BLEND_OP_ADD,
    D3D12_LOGIC_OP_NOOP,
    D3D12_COLOR_WRITE_ENABLE_ALL
};

const D3D12_RENDER_TARGET_BLEND_DESC kRTB_Premultiplied = {
    TRUE,
    FALSE,
    D3D12_BLEND_ONE,
    D3D12_BLEND_INV_SRC_ALPHA,
    D3D12_BLEND_OP_ADD,
    D3D12_BLEND_ONE,
    D3D12_BLEND_INV_SRC_ALPHA,
    D3D12_BLEND_OP_ADD,
    D3D12_LOGIC_OP_NOOP,
    D3D12_COLOR_WRITE_ENABLE_ALL
};

const D3D12_BLEND_DESC kAlphaBlend = {
    FALSE,
    FALSE,
    { kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend }
};

const D3D12_BLEND_DESC kPremultiplied = {
    FALSE,
    FALSE,
    { kRTB_Premultiplied, kRTB_Premultiplied, kRTB_Premultiplied, kRTB_Premultiplied, kRTB_Premultiplied, kRTB_Premultiplied, kRTB_Premultiplied, kRTB_Premultiplied }
};

//-----------------------------------------------------------------------------
//      回転と平行移動を適用します.
//-----------------------------------------------------------------------------
asdx::Vector2 TransformPos(const asdx::Vector2& p, float s, float c, float tx, float ty)
{
    return asdx::Vector2(
        (p.x * c - p.y * s) + tx,
        (p.x * s + p.y * c) + ty);
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SpriteRenderer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SpriteRenderer::SpriteRenderer()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SpriteRenderer::~SpriteRenderer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SpriteRenderer::Init
(
    uint32_t        w,
    uint32_t        h,
    uint32_t        maxSpriteCount,
    uint32_t        maxBatchCount,
    DXGI_FORMAT     rtvFormat,
    DXGI_FORMAT     dsvFormat
)
{
    auto pDevice = GetD3D12Device();

    D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16 = {};
    bool gpuUploadHeapSupported = false;
    if (SUCCEEDED(pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(options16))))
    { gpuUploadHeapSupported = options16.GPUUploadHeapSupported; }

    auto heapType = gpuUploadHeapSupported 
        ? D3D12_HEAP_TYPE_GPU_UPLOAD 
        : D3D12_HEAP_TYPE_UPLOAD;

    auto allocator = GetD3D12MA();

    for(auto i=0; i<2; ++i)
    {
        // 頂点バッファ生成.
        {
            auto count = maxSpriteCount * kVertexCountPerSprite;

            D3D12_HEAP_PROPERTIES props = {};
            props.Type                  = heapType;
            props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
            props.VisibleNodeMask       = 1;
            props.CreationNodeMask      = 1;

            D3D12_RESOURCE_DESC desc = {};
            desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
            desc.Width              = sizeof(Vertex) * count;
            desc.Height             = 1;
            desc.DepthOrArraySize   = 1;
            desc.Format             = DXGI_FORMAT_UNKNOWN;
            desc.MipLevels          = 1;
            desc.SampleDesc.Count   = 1;
            desc.SampleDesc.Quality = 0;
            desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

            auto state = D3D12_RESOURCE_STATE_COMMON;
            auto flags = D3D12_HEAP_FLAG_NONE;

            if (allocator != nullptr)
            {
                D3D12MA::ALLOCATION_DESC allocDesc = {};
                allocDesc.HeapType = heapType;

                D3D12MA::Allocation* allocation = nullptr;

                auto hr = allocator->CreateResource(
                    &allocDesc,
                    &desc,
                    state,
                    nullptr,
                    &allocation,
                    IID_PPV_ARGS(m_VB[i].GetAddress()));
                if (FAILED(hr))
                {
                    ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
                    return false;
                }

                m_AllocationVB[i].Attach(allocation);
            }
            else
            {
                auto hr = pDevice->CreateCommittedResource(
                    &props, flags, &desc, state, nullptr, IID_PPV_ARGS(m_VB[i].GetAddress()));
                if (FAILED(hr))
                {
                    ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
                    return false;
                }
            }

            auto hr = m_VB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_pVertices[i]));
            if (FAILED(hr))
            {
                ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
                return false;
            }
        }
    }

    // インデックスバッファ生成.
    {
        auto count = maxSpriteCount * kIndexCountPerSprite;

        D3D12_HEAP_PROPERTIES props = {};
        props.Type                  = heapType;
        props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
        props.VisibleNodeMask       = 1;
        props.CreationNodeMask      = 1;

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width              = sizeof(uint32_t) * count;
        desc.Height             = 1;
        desc.DepthOrArraySize   = 1;
        desc.Format             = DXGI_FORMAT_UNKNOWN;
        desc.MipLevels          = 1;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

        auto state = D3D12_RESOURCE_STATE_COMMON;
        auto flags = D3D12_HEAP_FLAG_NONE;

        if (allocator != nullptr)
        {
            D3D12MA::ALLOCATION_DESC allocDesc = {};
            allocDesc.HeapType = heapType;

            D3D12MA::Allocation* allocation = nullptr;

            auto hr = allocator->CreateResource(
                &allocDesc,
                &desc,
                state,
                nullptr,
                &allocation,
                IID_PPV_ARGS(m_IB.GetAddress()));
            if (FAILED(hr))
            {
                ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
                return false;
            }

            m_AllocationIB.Attach(allocation);
        }
        else
        {
            auto hr = pDevice->CreateCommittedResource(
                &props, flags, &desc, state, nullptr, IID_PPV_ARGS(m_IB.GetAddress()));
            if (FAILED(hr))
            {
                ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
                return false;
            }
        }

        uint32_t* pIndices = nullptr;
        auto hr = m_IB->Map(0, nullptr, reinterpret_cast<void**>(&pIndices));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
            return false;
        }

        uint32_t j=0u;
        for(auto i=0u; i<count && j<count; i+=kVertexCountPerSprite)
        {
            pIndices[j + 0] = i + 0;
            pIndices[j + 1] = i + 1;
            pIndices[j + 2] = i + 2;

            pIndices[j + 3] = i + 1;
            pIndices[j + 4] = i + 3;
            pIndices[j + 5] = i + 2;

            j += kIndexCountPerSprite;
        }

        m_IB->Unmap(0, nullptr);
    }

    // ルートシグニチャの生成.
    {
        D3D12_DESCRIPTOR_RANGE range[3] = {};
        range[0].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[0].NumDescriptors                     = 1;
        range[0].BaseShaderRegister                 = 0;
        range[0].RegisterSpace                      = 0;
        range[0].OffsetInDescriptorsFromTableStart  = 0;

        range[1].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        range[1].NumDescriptors                     = 1;
        range[1].BaseShaderRegister                 = 0;
        range[1].RegisterSpace                      = 0;
        range[1].OffsetInDescriptorsFromTableStart  = 0;

        range[2].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[2].NumDescriptors                     = 1;
        range[2].BaseShaderRegister                 = 1;
        range[2].RegisterSpace                      = 0;
        range[2].OffsetInDescriptorsFromTableStart  = 0;

        D3D12_ROOT_PARAMETER param[5] = {};
        param[0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.Num32BitValues   = 16;
        param[0].Constants.ShaderRegister   = 0;
        param[0].Constants.RegisterSpace    = 0;
        param[0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_VERTEX;

        param[1].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges    = 1;
        param[1].DescriptorTable.pDescriptorRanges      = &range[0];
        param[1].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_PIXEL;

        param[2].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[2].DescriptorTable.NumDescriptorRanges    = 1;
        param[2].DescriptorTable.pDescriptorRanges      = &range[1];
        param[2].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_PIXEL;

        // フォント用定数.
        param[3].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[3].Constants.Num32BitValues   = 4;
        param[3].Constants.ShaderRegister   = 1;
        param[3].Constants.RegisterSpace    = 0;
        param[3].ShaderVisibility           = D3D12_SHADER_VISIBILITY_PIXEL;

        // テクスチャ.
        param[4].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[4].DescriptorTable.NumDescriptorRanges    = 1;
        param[4].DescriptorTable.pDescriptorRanges      = &range[2];
        param[4].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(param);
        desc.pParameters        = param;
        desc.NumStaticSamplers  = 0;
        desc.pStaticSamplers    = nullptr;
        desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        RefPtr<ID3DBlob> blob;
        RefPtr<ID3DBlob> errorBlob;
        auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddress(), errorBlob.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x", hr);
            if (errorBlob.GetPtr() != nullptr)
            {
                ELOG("Error : Msg = %s", reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
            }
            return false;
        }

        hr = pDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_RootSig.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // パイプラインステート生成.
    {
        m_ColorFormat = rtvFormat;
        m_DepthFormat = dsvFormat;

        D3D12_SHADER_BYTECODE ps = { asdxSpritePS, sizeof(asdxSpritePS) };
        if (!CreatePipelineState(pDevice, ps, false, m_DefaultState.GetAddress()))
        { return false; }

        m_pCurrentState = m_DefaultState.GetPtr();
    }

    // バッチメモリ確保.
    {
        m_BatchCount     = 0;
        m_MaxSpriteCount = maxSpriteCount;
        m_MaxBatchCount  = maxBatchCount;

        m_Batches.resize(maxBatchCount);
    }

    // スクリーンサイズ設定.
    SetScreenSize(w, h);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SpriteRenderer::Term()
{
    for(auto i=0; i<2; ++i)
    {
        if (m_VB[i].GetPtr() != nullptr)
        {
            m_VB[i]->Unmap(0, nullptr);
        }

        m_VB[i].Reset();
        m_AllocationVB[i].Reset();
        m_pVertices[i] = nullptr;
    }
    m_IB.Reset();
    m_AllocationIB.Reset();

    m_RootSig.Reset();
    m_DefaultState.Reset();

    m_Batches.clear();
    m_Batches.shrink_to_fit();

    m_SpriteCount   = 0;
    m_BatchCount    = 0;
    m_SubmitCount   = 0;
    m_Color         = { 255, 255, 255, 255 };
    m_HandleSRV     = {};
    m_HandleSampler = {};
    m_ScreenSize    = Vector2(0.0f, 0.0f);
}

//-----------------------------------------------------------------------------
//      リセット処理です.
//-----------------------------------------------------------------------------
void SpriteRenderer::Reset()
{
    m_SpriteCount = 0;
    m_BatchCount  = 0;
    m_SubmitCount = 0;

    if (!m_Batches.empty())
    {
        m_Batches[0].IndexCount  = 0;
        m_Batches[0].IndexOffset = 0;
        m_Batches[0].SRV         = {};
        m_Batches[0].Sampler     = {};
        m_Batches[0].pState      = m_DefaultState.GetPtr();
        memset(m_Batches[0].Param, 0, sizeof(m_Batches[0].Param));
    }

    m_HandleSRV     = {};
    m_HandleSampler = {};
    m_Color         = { 255, 255, 255, 255 };
    m_pCurrentState = m_DefaultState.GetPtr();
    memset(m_Param, 0, sizeof(m_Param));

    m_BufferIndex = (m_BufferIndex + 1) & 0x1;
}

//-----------------------------------------------------------------------------
//      バッチを変更します.
//-----------------------------------------------------------------------------
void SpriteRenderer::ChangeBatch
(
    ID3D12PipelineState*        pPipelineState,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSampler
)
{
    // 無効な引数なら設定しない.
    if (handleSRV.ptr == 0 || handleSampler.ptr == 0 || pPipelineState == nullptr)
    { return; }

    // 同じ状態ならバッチは変えない.
    if (m_HandleSRV.ptr == handleSRV.ptr && m_HandleSampler.ptr == handleSampler.ptr && m_pCurrentState == pPipelineState)
    { return; }
 
    m_HandleSRV     = handleSRV;
    m_HandleSampler = handleSampler;
    m_pCurrentState = pPipelineState;

    auto index = m_BatchCount;
    m_BatchCount++;
    m_Batches[index].IndexCount  = 0;
    m_Batches[index].IndexOffset = m_SpriteCount * kIndexCountPerSprite;
    m_Batches[index].SRV         = m_HandleSRV;
    m_Batches[index].Sampler     = m_HandleSampler;
    m_Batches[index].pState      = pPipelineState;
    memcpy(m_Batches[index].Param, &m_Param, sizeof(m_Param));
}

//-----------------------------------------------------------------------------
//      パイプラインステートを変更します.
//-----------------------------------------------------------------------------
void SpriteRenderer::SetPipelineState(ID3D12PipelineState* pPipelineState)
{
    assert(m_HandleSRV    .ptr != 0);
    assert(m_HandleSampler.ptr != 0);
    ChangeBatch(pPipelineState, m_HandleSRV, m_HandleSampler);
}

//-----------------------------------------------------------------------------
//      テクスチャを設定します.
//-----------------------------------------------------------------------------
void SpriteRenderer::SetTexture(D3D12_GPU_DESCRIPTOR_HANDLE handleSRV, D3D12_GPU_DESCRIPTOR_HANDLE handleSampler)
{
    assert(m_pCurrentState != nullptr);
    ChangeBatch(m_pCurrentState, handleSRV, handleSampler);
}

//-----------------------------------------------------------------------------
//      カラーを設定します.
//-----------------------------------------------------------------------------
void SpriteRenderer::SetColor(float r, float g, float b, float a)
{
    m_Color.R = Clamp<uint8_t>(uint8_t(r * 255.0f), 0, 255);
    m_Color.G = Clamp<uint8_t>(uint8_t(g * 255.0f), 0, 255);
    m_Color.B = Clamp<uint8_t>(uint8_t(b * 255.0f), 0, 255);
    m_Color.A = Clamp<uint8_t>(uint8_t(a * 255.0f), 0, 255);
}

//-----------------------------------------------------------------------------
//      カラーを設定します.
//-----------------------------------------------------------------------------
void SpriteRenderer::SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    m_Color.R = r;
    m_Color.G = g;
    m_Color.B = b;
    m_Color.A = a;
}

//-----------------------------------------------------------------------------
//      スクリーンサイズを設定します.
//-----------------------------------------------------------------------------
void SpriteRenderer::SetScreenSize(uint32_t w, uint32_t h)
{
    m_ScreenSize.x = float(w);
    m_ScreenSize.y = float(h);

    float sx = (m_ScreenSize.x > 0.0f) ? 2.0f / m_ScreenSize.x : 0.0f;
    float sy = (m_ScreenSize.y > 0.0f) ? 2.0f / m_ScreenSize.y : 0.0f;

    m_Transform = Matrix(
          sx,   0.0f,   0.0f,   0.0f,
        0.0f,    -sy,   0.0f,   0.0f,
        0.0f,   0.0f,   1.0f,   0.0f,
       -1.0f,   1.0f,   0.0f,   1.0f);
}

//-----------------------------------------------------------------------------
//      スプライトを追加します.
//-----------------------------------------------------------------------------
void SpriteRenderer::Add(int x, int y, int w, int h, int layer, const Vector2& uv0, const Vector2& uv1)
{
    if (m_SpriteCount + 1 > m_MaxSpriteCount)
    { return; }

    if (m_BatchCount == 0 || m_pVertices == nullptr)
    { return; }

    // 画面外ならカリング.
    if (((x + w) < 0 || x > m_ScreenSize.x || (y + h) < 0 || y > m_ScreenSize.y))
    { return; }
 
    auto& batch = m_Batches[m_BatchCount - 1];
    batch.IndexCount += kIndexCountPerSprite;

    auto pHead     = m_pVertices[m_BufferIndex];
    auto pVertices = &pHead[m_SpriteCount * kVertexCountPerSprite];

    // テクスチャ座標
    float u0 = uv0.x;
    float u1 = uv1.x;
    float v0 = uv0.y;
    float v1 = uv1.y;

    float d = float(layer);

    // 位置座標.
    float x0 = float(x);
    float x1 = float(x + w);
    float y0 = float(y);
    float y1 = float(y + h);

    // Vertex : 0
    pVertices[ 0 ].Position.x = x0;
    pVertices[ 0 ].Position.y = y0;
    pVertices[ 0 ].Position.z = d;
    pVertices[ 0 ].Color      = m_Color;
    pVertices[ 0 ].TexCoord.x = u0;
    pVertices[ 0 ].TexCoord.y = v0;

    // Vertex : 1
    pVertices[ 1 ].Position.x = x1;
    pVertices[ 1 ].Position.y = y0;
    pVertices[ 1 ].Position.z = d;
    pVertices[ 1 ].Color      = m_Color;
    pVertices[ 1 ].TexCoord.x = u1;
    pVertices[ 1 ].TexCoord.y = v0;

    // Vertex : 2
    pVertices[ 2 ].Position.x = x0;
    pVertices[ 2 ].Position.y = y1;
    pVertices[ 2 ].Position.z = d;
    pVertices[ 2 ].Color      = m_Color;
    pVertices[ 2 ].TexCoord.x = u0;
    pVertices[ 2 ].TexCoord.y = v1;

    // Vertex : 3
    pVertices[ 3 ].Position.x = x1;
    pVertices[ 3 ].Position.y = y1;
    pVertices[ 3 ].Position.z = d;
    pVertices[ 3 ].Color      = m_Color;
    pVertices[ 3 ].TexCoord.x = u1;
    pVertices[ 3 ].TexCoord.y = v1;

    // スプライト数をカウントアップします.
    m_SpriteCount++;
}

//-----------------------------------------------------------------------------
//      スプライトを追加します.
//-----------------------------------------------------------------------------
void SpriteRenderer::Add(int x, int y, int w, int h, int layer, float rad, const Vector2& uv0, const Vector2& uv1)
{
    if (m_SpriteCount + 1 > m_MaxSpriteCount)
    { return; }

    if (m_BatchCount == 0 || m_pVertices == nullptr)
    { return; }

    // 画面外ならカリング.
    if (((x + w) < 0 || x > m_ScreenSize.x || (y + h) < 0 || y > m_ScreenSize.y))
    { return; }
 
    auto& batch = m_Batches[m_BatchCount - 1];
    batch.IndexCount += kIndexCountPerSprite;

    auto pHead     = m_pVertices[m_BufferIndex];
    auto pVertices = &pHead[m_SpriteCount * kVertexCountPerSprite];

    // テクスチャ座標
    float u0 = uv0.x;
    float u1 = uv1.x;
    float v0 = uv0.y;
    float v1 = uv1.y;

    float d = float(layer);

    float hw = w * 0.5f;
    float hh = h * 0.5f;

    float x0 = float(-hw);
    float x1 = float( hw);
    float y0 = float(-hh);
    float y1 = float( hh);

    float s = sinf(rad);
    float c = cosf(rad);

    asdx::Vector2 p0(x0, y0);
    asdx::Vector2 p1(x1, y0);
    asdx::Vector2 p2(x0, y1);
    asdx::Vector2 p3(x1, y1);

    // 回転中心.
    float cx = x + hw;
    float cy = y + hh;

    // 変換を適用.
    p0 = TransformPos(p0, s, c, cx, cy);
    p1 = TransformPos(p1, s, c, cx, cy);
    p2 = TransformPos(p2, s, c, cx, cy);
    p3 = TransformPos(p3, s, c, cx, cy);

    // Vertex : 0
    pVertices[ 0 ].Position.x = p0.x;
    pVertices[ 0 ].Position.y = p0.y;
    pVertices[ 0 ].Position.z = d;
    pVertices[ 0 ].Color      = m_Color;
    pVertices[ 0 ].TexCoord.x = u0;
    pVertices[ 0 ].TexCoord.y = v0;

    // Vertex : 1
    pVertices[ 1 ].Position.x = p1.x;
    pVertices[ 1 ].Position.y = p1.y;
    pVertices[ 1 ].Position.z = d;
    pVertices[ 1 ].Color      = m_Color;
    pVertices[ 1 ].TexCoord.x = u1;
    pVertices[ 1 ].TexCoord.y = v0;

    // Vertex : 2
    pVertices[ 2 ].Position.x = p2.x;
    pVertices[ 2 ].Position.y = p2.y;
    pVertices[ 2 ].Position.z = d;
    pVertices[ 2 ].Color      = m_Color;
    pVertices[ 2 ].TexCoord.x = u0;
    pVertices[ 2 ].TexCoord.y = v1;

    // Vertex : 3
    pVertices[ 3 ].Position.x = p3.x;
    pVertices[ 3 ].Position.y = p3.y;
    pVertices[ 3 ].Position.z = d;
    pVertices[ 3 ].Color      = m_Color;
    pVertices[ 3 ].TexCoord.x = u1;
    pVertices[ 3 ].TexCoord.y = v1;

    // スプライト数をカウントアップします.
    m_SpriteCount++;
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void SpriteRenderer::Draw(ID3D12GraphicsCommandList* pCmdList)
{
    if (m_SpriteCount == 0 || pCmdList == nullptr)
        return;

    ASDX_SCOPED_MARKER(pCmdList, SpriteDraw);

    D3D12_VERTEX_BUFFER_VIEW vbv = {};
    vbv.BufferLocation  = m_VB[m_BufferIndex]->GetGPUVirtualAddress();
    vbv.SizeInBytes     = UINT(m_VB[m_BufferIndex]->GetDesc().Width);
    vbv.StrideInBytes   = sizeof(Vertex);

    D3D12_INDEX_BUFFER_VIEW ibv = {};
    ibv.BufferLocation = m_IB->GetGPUVirtualAddress();
    ibv.SizeInBytes    = UINT(m_IB->GetDesc().Width);
    ibv.Format         = DXGI_FORMAT_R32_UINT;

    pCmdList->IASetVertexBuffers(0, 1, &vbv);
    pCmdList->IASetIndexBuffer(&ibv);
    pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCmdList->SetGraphicsRoot32BitConstants(CBV0, 16, &m_Transform, 0);

    for(auto i=m_SubmitCount; i<m_BatchCount; ++i)
    {
        auto& batch = m_Batches[i];
        pCmdList->SetPipelineState(batch.pState);
        pCmdList->SetGraphicsRootDescriptorTable(SRV0, batch.SRV);
        pCmdList->SetGraphicsRootDescriptorTable(Sampler0, batch.Sampler);
        pCmdList->SetGraphicsRoot32BitConstants(CBV1, 4, batch.Param, 0);
        pCmdList->DrawIndexedInstanced(batch.IndexCount, 1, batch.IndexOffset, 0, 0);
    }
    m_SubmitCount = m_BatchCount;
}

//-----------------------------------------------------------------------------
//      スクリーンサイズを取得します.
//-----------------------------------------------------------------------------
const Vector2& SpriteRenderer::GetScreenSize() const
{ return m_ScreenSize; }

//-----------------------------------------------------------------------------
//      カラーを取得します.
//-----------------------------------------------------------------------------
Vector4 SpriteRenderer::GetColor() const
{
    auto inv = 1.0f / 255.0f;
    return Vector4(
        Saturate(m_Color.R * inv),
        Saturate(m_Color.G * inv),
        Saturate(m_Color.B * inv),
        Saturate(m_Color.A * inv));
}

//-----------------------------------------------------------------------------
//      スプライト描画用パイプラインステートを生成します.
//-----------------------------------------------------------------------------
bool SpriteRenderer::CreatePipelineState
(
    ID3D12Device*                   pDevice,
    const D3D12_SHADER_BYTECODE&    pixelShader,
    bool                            preMultipliedAlpha,
    ID3D12PipelineState**           ppResult
)
{
    if (pDevice == nullptr || ppResult == nullptr)
    { return false; }

    // パイプラインステート生成.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_RootSig.GetPtr();
        desc.VS                             = { asdxSpriteVS, sizeof(asdxSpriteVS) };
        desc.PS                             = pixelShader;
        desc.BlendState                     = (preMultipliedAlpha) ? kPremultiplied : kAlphaBlend;
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = kCullBack;
        desc.DepthStencilState              = (m_DepthFormat == DXGI_FORMAT_UNKNOWN) ? kDepthNone : kDepthDefault;
        desc.InputLayout.NumElements        = _countof(kElements);
        desc.InputLayout.pInputElementDescs = kElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = m_ColorFormat;
        desc.DSVFormat                      = m_DepthFormat;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        auto hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(ppResult));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ユーザーパラメータを設定します.
//-----------------------------------------------------------------------------
void SpriteRenderer::SetParam(uint32_t count, const void* param, uint32_t destOffset)
{
    assert(param != nullptr);
    assert(count <= 4);
    memcpy(m_Param + destOffset, param, sizeof(uint32_t) * count);
}

//-----------------------------------------------------------------------------
//      デフォルトのパイプラインステートを取得します.
//-----------------------------------------------------------------------------
ID3D12PipelineState* SpriteRenderer::GetDefaultState() const
{ return m_DefaultState.GetPtr(); }

//-----------------------------------------------------------------------------
//      現在のパイプラインステートを取得します.
//-----------------------------------------------------------------------------
ID3D12PipelineState* SpriteRenderer::GetCurrentState() const
{ return m_pCurrentState; }

//-----------------------------------------------------------------------------
//      シェーダリソースビューハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE SpriteRenderer::GetHandleSRV() const
{ return m_HandleSRV; }

//-----------------------------------------------------------------------------
//      サンプラーハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE SpriteRenderer::GetHandleSampler() const
{ return m_HandleSampler; }

//-----------------------------------------------------------------------------
//      ルートシグニチャを取得します.
//-----------------------------------------------------------------------------
ID3D12RootSignature* SpriteRenderer::GetRootSignature() const
{ return m_RootSig.GetPtr(); }

//-----------------------------------------------------------------------------
//      9 slices を描画します.
//-----------------------------------------------------------------------------
void Draw9Slices
(
    SpriteRenderer& renderer,
    int x,
    int y,
    int w,
    int h,
    int l,
    int r,
    int t,
    int b,
    int texW,
    int texH,
    int texL,
    int texR,
    int texT,
    int texB
)
{
    float u0 = 0.0f;
    float u1 = float(texL) / float(texW);
    float u2 = float(texR) / float(texW);
    float u3 = 1.0f;

    float v0 = 0.0f;
    float v1 = float(texT) / float(texH);
    float v2 = float(texB) / float(texH);
    float v3 = 1.0f;

    int x0 = x;
    int x1 = x + l;
    int x2 = x + r;

    int y0 = y;
    int y1 = y + t;
    int y2 = y + b;

    int w0 = l;
    int w1 = (r - l);
    int w2 = (w - r);

    int h0 = t;
    int h1 = (b - t);
    int h2 = (h - b);

    renderer.Add(x0, y0, w0, h0, asdx::Vector2(u0, v0), asdx::Vector2(u1, v1));
    renderer.Add(x1, y0, w1, h0, asdx::Vector2(u1, v0), asdx::Vector2(u2, v1));
    renderer.Add(x2, y0, w2, h0, asdx::Vector2(u2, v0), asdx::Vector2(u3, v1));

    renderer.Add(x0, y1, w0, h1, asdx::Vector2(u0, v1), asdx::Vector2(u1, v2));
    renderer.Add(x1, y1, w1, h1, asdx::Vector2(u1, v1), asdx::Vector2(u2, v2));
    renderer.Add(x2, y1, w2, h1, asdx::Vector2(u2, v1), asdx::Vector2(u3, v2));

    renderer.Add(x0, y2, w0, h2, asdx::Vector2(u0, v2), asdx::Vector2(u1, v3));
    renderer.Add(x1, y2, w1, h2, asdx::Vector2(u1, v2), asdx::Vector2(u2, v3));
    renderer.Add(x2, y2, w2, h2, asdx::Vector2(u2, v2), asdx::Vector2(u3, v3));
}

//------------------------------------------------------------------------------
//      9 slices を描画します.
//------------------------------------------------------------------------------
void Draw9Slices(
    SpriteRenderer& renderer,
    int x,
    int y,
    int w,
    int h,
    int texW,
    int texH,
    int texSX,
    int texSY)
{
    Draw9Slices(
        renderer,
        x, y, w, h,
        texSX, w - texSX,
        texSY, h - texSY,
        texW, texH,
        texSX, texW - texSX,
        texSY, texH - texSY);
}

//-----------------------------------------------------------------------------
//      9 slices を描画します.
//-----------------------------------------------------------------------------
void Draw9Slices
(
    SpriteRenderer& renderer,
    int x,
    int y,
    int w,
    int h,
    int texW,
    int texH
)
{
    int gridW = texW / 3;
    int gridH = texH / 3;

    Draw9Slices(renderer, x, y, w, h, texW, texH, gridW, gridH);
}

} // namespace asdx
