//-----------------------------------------------------------------------------
// File : asdxLine.cpp
// Desc : Line Renderer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxLine.h>
#include <gfx/asdxDevice.h>
#include "D3D12MemAlloc.h"


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxLineVS.inc"
#include "../res/shaders/Compiled/asdxLinePS.inc"

static const D3D12_INPUT_ELEMENT_DESC kElements[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR"   , 0, DXGI_FORMAT_R8G8B8A8_UNORM , 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

const D3D12_RASTERIZER_DESC kCullNone = {
    D3D12_FILL_MODE_SOLID,
    D3D12_CULL_MODE_NONE,
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

const D3D12_DEPTH_STENCIL_DESC kDepthRead = {
    TRUE,
    D3D12_DEPTH_WRITE_MASK_ZERO,
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

const D3D12_BLEND_DESC kAlphaBlend = {
    FALSE,
    FALSE,
    { kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend, kRTB_AlphaBlend }
};

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// LineRenderer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
LineRenderer::LineRenderer()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
LineRenderer::~LineRenderer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool LineRenderer::Init(uint32_t maxLineCount, DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat)
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
            auto count = maxLineCount * 2;

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

    // ルートシグニチャの生成.
    {
        D3D12_ROOT_PARAMETER param[1] = {};
        param[0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_CBV;
        param[0].Descriptor.ShaderRegister  = 0;
        param[0].Descriptor.RegisterSpace   = 0;
        param[0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_VERTEX;

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
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_RootSig.GetPtr();
        desc.VS                             = { asdxLineVS, sizeof(asdxLineVS) };
        desc.PS                             = { asdxLinePS, sizeof(asdxLinePS) };
        desc.BlendState                     = kAlphaBlend;
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = kCullNone;
        desc.DepthStencilState              = (dsvFormat == DXGI_FORMAT_UNKNOWN) ? kDepthNone : kDepthRead;
        desc.InputLayout.NumElements        = _countof(kElements);
        desc.InputLayout.pInputElementDescs = kElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = rtvFormat;
        desc.DSVFormat                      = dsvFormat;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        auto hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_PSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    m_MaxLineCount = maxLineCount;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void LineRenderer::Term()
{
    for(auto i=0; i<2; ++i)
    {
        m_VB[i].Reset();
        m_AllocationVB[i].Reset();
        m_pVertices[i] = nullptr;
    }

    m_LineCount    = 0;
    m_SubmitCount  = 0;
    m_MaxLineCount = 0;

    m_PSO    .Reset();
    m_RootSig.Reset();
}

//-----------------------------------------------------------------------------
//      リセット処理を行います.
//-----------------------------------------------------------------------------
void LineRenderer::Reset()
{
    m_SubmitCount = 0;
    m_LineCount   = 0;

    m_BufferIndex = (m_BufferIndex + 1) & 0x1;
}

//-----------------------------------------------------------------------------
//      線分を追加します.
//-----------------------------------------------------------------------------
void LineRenderer::Add(const Vector3& v0, const Vector3& v1, const Vector4& c0, const Vector4& c1)
{
    if (m_LineCount + 1 >= m_MaxLineCount)
        return;

    auto& Vertex0 = m_pVertices[m_BufferIndex][m_LineCount * 2 + 0];
    auto& Vertex1 = m_pVertices[m_BufferIndex][m_LineCount * 2 + 1];

    Vertex0.Position = v0;
    Vertex0.Color    = Unorm4(c0.x, c0.y, c0.z, c0.w);

    Vertex1.Position = v1;
    Vertex1.Color    = Unorm4(c1.x, c1.y, c1.z, c1.w);
    
    m_LineCount++;
}

//-----------------------------------------------------------------------------
//      パイプラインステートを設定します.
//-----------------------------------------------------------------------------
void LineRenderer::SetPipelineState(ID3D12GraphicsCommandList* pCmd, ID3D12PipelineState* pPipelineState)
{
    auto pso = (pPipelineState != nullptr) ? pPipelineState : m_PSO.GetPtr();
    pCmd->SetGraphicsRootSignature(m_RootSig.GetPtr());
    pCmd->SetPipelineState(pso);
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void LineRenderer::Draw(ID3D12GraphicsCommandList* pCmd)
{
    D3D12_VERTEX_BUFFER_VIEW vbv = {};
    vbv.BufferLocation = m_VB[m_BufferIndex]->GetGPUVirtualAddress() + m_SubmitCount * sizeof(Vertex) * 2;
    vbv.SizeInBytes    = m_LineCount * sizeof(Vertex) * 2;
    vbv.StrideInBytes  = sizeof(Vertex);

    pCmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    pCmd->IASetVertexBuffers(0, 1, &vbv);
    pCmd->DrawInstanced(m_LineCount * 2, 1, 0, 0);

    m_SubmitCount += m_LineCount;
}

///////////////////////////////////////////////////////////////////////////////
// Functions.
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      ワイヤーフレームのボックスを描画します.
//-----------------------------------------------------------------------------
void DrawWireBox(LineRenderer& renderer, const Vector3& mini, const Vector3& maxi, const Vector4& color)
{
    renderer.Add(Vector3(mini.x, mini.y, mini.z), Vector3(maxi.x, mini.y, mini.z), color);
    renderer.Add(Vector3(maxi.x, mini.y, mini.z), Vector3(maxi.x, maxi.y, mini.z), color);
    renderer.Add(Vector3(maxi.x, maxi.y, mini.z), Vector3(mini.x, maxi.y, mini.z), color);
    renderer.Add(Vector3(mini.x, maxi.y, mini.z), Vector3(mini.x, mini.y, mini.z), color);
    renderer.Add(Vector3(mini.x, mini.y, maxi.z), Vector3(maxi.x, mini.y, maxi.z), color);
    renderer.Add(Vector3(maxi.x, mini.y, maxi.z), Vector3(maxi.x, maxi.y, maxi.z), color);
    renderer.Add(Vector3(maxi.x, maxi.y, maxi.z), Vector3(mini.x, maxi.y, maxi.z), color);
    renderer.Add(Vector3(mini.x, maxi.y, maxi.z), Vector3(mini.x, mini.y, maxi.z), color);
    renderer.Add(Vector3(mini.x, mini.y, mini.z), Vector3(mini.x, mini.y, maxi.z), color);
    renderer.Add(Vector3(maxi.x, mini.y, mini.z), Vector3(maxi.x, mini.y, maxi.z), color);
    renderer.Add(Vector3(maxi.x, maxi.y, mini.z), Vector3(maxi.x, maxi.y, maxi.z), color);
    renderer.Add(Vector3(mini.x, maxi.y, mini.z), Vector3(mini.x, maxi.y, maxi.z), color);
}

//-----------------------------------------------------------------------------
//      ワイヤーフレームの球を描画します.
//-----------------------------------------------------------------------------
void DrawWireSphere(LineRenderer& renderer, const Vector3& center, float radius, const Vector4& color, uint32_t segments)
{
    const float step = asdx::F_PI * 2.0f / static_cast<float>(segments);
    for (auto i = 0u; i < segments; ++i)
    {
        auto theta0 = step * (i + 0);
        auto theta1 = step * (i + 1);
        auto ct0    = cosf(theta0);
        auto st0    = sinf(theta0);
        auto ct1    = cosf(theta1);
        auto st1    = sinf(theta1);

        // XY平面
        renderer.Add(
            Vector3(center.x + radius * ct0, center.y + radius * st0, center.z),
            Vector3(center.x + radius * ct1, center.y + radius * st1, center.z),
            color);
        // YZ平面
        renderer.Add(
            Vector3(center.x, center.y + radius * ct0, center.z + radius * st0),
            Vector3(center.x, center.y + radius * ct1, center.z + radius * st1),
            color);
        // ZX平面
        renderer.Add(
            Vector3(center.x + radius * st0, center.y, center.z + radius * ct0),
            Vector3(center.x + radius * st1, center.y, center.z + radius * ct1),
            color);
    }
}

//-----------------------------------------------------------------------------
//      ワイヤーフレームの半球を描画します.
//-----------------------------------------------------------------------------
void DrawWireHemisphere(LineRenderer& renderer, const Vector3& center, float radius, const Vector3& upward, const Vector4& color, uint32_t segments)
{
    auto N = Vector3::Normalize(upward);
    Vector3 T;
    Vector3 B;
    CalcONB(N, T, B);

    const float step = asdx::F_PI / static_cast<float>(segments);
    for (auto i = 0u; i < segments; ++i)
    {
        auto phi0 = step * (i + 0);
        auto phi1 = step * (i + 1);
        auto cp0  = cosf(phi0);
        auto sp0  = sinf(phi0);
        auto cp1  = cosf(phi1);
        auto sp1  = sinf(phi1);

        for (auto j = 0u; j < segments * 2; ++j)
        {
            auto theta0 = step * j;
            auto theta1 = step * (j + 1);
            auto ct0    = cosf(theta0);
            auto st0    = sinf(theta0);
            auto ct1    = cosf(theta1);
            auto st1    = sinf(theta1);
            Vector3 p0  = center + radius * (N * cp0 + (T * st0 + B * ct0) * sp0);
            Vector3 p1  = center + radius * (N * cp0 + (T * st1 + B * ct1) * sp0);
            Vector3 p2  = center + radius * (N * cp1 + (T * st0 + B * ct0) * sp1);

            renderer.Add(p0, p1, color);
            renderer.Add(p0, p2, color);
        }
    }
}

//-----------------------------------------------------------------------------
//      ワイヤーフレームの円錐を描画します.
//-----------------------------------------------------------------------------
void DrawWireCone(LineRenderer& renderer, const Vector3& apex, const Vector3& baseCenter, float baseRadius, const Vector4& color, uint32_t segments)
{
    Vector3 axis = Vector3::Normalize(baseCenter - apex);
    Vector3 T;
    Vector3 B;
    CalcONB(axis, T, B);

    const float step = asdx::F_PI * 2.0f / static_cast<float>(segments);
    for (auto i = 0u; i < segments; ++i)
    {
        auto theta0 = step * (i + 0);
        auto theta1 = step * (i + 1);
        auto ct0    = cosf(theta0);
        auto st0    = sinf(theta0);
        auto ct1    = cosf(theta1);
        auto st1    = sinf(theta1);
        Vector3 p0  = baseCenter + baseRadius * (T * ct0 + B * st0);
        Vector3 p1  = baseCenter + baseRadius * (T * ct1 + B * st1);

        renderer.Add(apex, p0, color);
        renderer.Add(p0, p1, color);
    }
}

//-----------------------------------------------------------------------------
//      ワイヤーフレームの四角錐を描画します.
//-----------------------------------------------------------------------------
void DrawWirePyramid(LineRenderer& renderer, const Vector3& apex, const Vector3& baseCenter, float baseSize, const Vector4& color)
{
    Vector3 axis = Vector3::Normalize(baseCenter - apex);
    Vector3 T;
    Vector3 B;
    CalcONB(axis, T, B);
 
    Vector3 p0 = baseCenter + baseSize * ( T +  B);
    Vector3 p1 = baseCenter + baseSize * (-T +  B);
    Vector3 p2 = baseCenter + baseSize * (-T + -B);
    Vector3 p3 = baseCenter + baseSize * ( T + -B);
    
    renderer.Add(apex, p0, color);
    renderer.Add(apex, p1, color);
    renderer.Add(apex, p2, color);
    renderer.Add(apex, p3, color);
    renderer.Add(p0, p1, color);
    renderer.Add(p1, p2, color);
    renderer.Add(p2, p3, color);
    renderer.Add(p3, p0, color);
}

//-----------------------------------------------------------------------------
//      ワイヤーフレームの円柱を描画します.
//-----------------------------------------------------------------------------
void DrawWireCylinder(LineRenderer& renderer, const Vector3& baseCenter, const Vector3& topCenter, float radius, const Vector4& color, uint32_t segments)
{
    Vector3 axis = Vector3::Normalize(topCenter - baseCenter);
    Vector3 T;
    Vector3 B;
    CalcONB(axis, T, B);
 
    const float step = asdx::F_PI * 2.0f / static_cast<float>(segments);
    for (auto i = 0u; i < segments; ++i)
    {
        auto theta0 = step * (i + 0);
        auto theta1 = step * (i + 1);
        auto ct0    = cosf(theta0);
        auto st0    = sinf(theta0);
        auto ct1    = cosf(theta1);
        auto st1    = sinf(theta1);
        Vector3 p0  = baseCenter + radius * (T * ct0 + B * st0);
        Vector3 p1  = baseCenter + radius * (T * ct1 + B * st1);
        Vector3 p2  = topCenter  + radius * (T * ct0 + B * st0);
        Vector3 p3  = topCenter  + radius * (T * ct1 + B * st1);
        renderer.Add(p0, p1, color);
        renderer.Add(p2, p3, color);
        renderer.Add(p0, p2, color);
    }
}

//-----------------------------------------------------------------------------
//      ワイヤーフレームの平面を描画します.
//-----------------------------------------------------------------------------
void DrawWirePlane(LineRenderer& renderer, const Vector3& center, const Vector3& normal, float size, const Vector4& color)
{
    Vector3 N = Vector3::Normalize(normal);
    Vector3 T;
    Vector3 B;
    CalcONB(N, T, B);

    Vector3 p0 = center + size * ( T +  B);
    Vector3 p1 = center + size * (-T +  B);
    Vector3 p2 = center + size * (-T + -B);
    Vector3 p3 = center + size * ( T + -B);

    renderer.Add(p0, p1, color);
    renderer.Add(p1, p2, color);
    renderer.Add(p2, p3, color);
    renderer.Add(p3, p0, color);
}

//-----------------------------------------------------------------------------
//      ワイヤーフレームのカプセルを描画します.
//-----------------------------------------------------------------------------
void DrawWireCapsule(LineRenderer& renderer, const Vector3& baseCenter, const Vector3& topCenter, float radius, const Vector4& color, uint32_t segments)
{
    DrawWireCylinder  (renderer, baseCenter, topCenter, radius, color, segments);
    DrawWireHemisphere(renderer, baseCenter, radius, topCenter  - baseCenter, color, segments);
    DrawWireHemisphere(renderer, topCenter,  radius, baseCenter - topCenter,  color, segments);
}

//-----------------------------------------------------------------------------
//      ワイヤーフレームの円盤を描画します.
//-----------------------------------------------------------------------------
void DrawWireDisk(LineRenderer& renderer, const Vector3& center, const Vector3& normal, float radius, const Vector4& color, uint32_t segments)
{
    Vector3 N = Vector3::Normalize(normal);
    Vector3 T;
    Vector3 B;
    CalcONB(N, T, B);

    const float step = asdx::F_PI * 2.0f / static_cast<float>(segments);
    for (auto i = 0u; i < segments; ++i)
    {
        auto theta0 = step * i;
        auto theta1 = step * (i + 1);
        auto ct0    = cosf(theta0);
        auto st0    = sinf(theta0);
        auto ct1    = cosf(theta1);
        auto st1    = sinf(theta1);
        Vector3 p0  = center + radius * (T * ct0 + B * st0);
        Vector3 p1  = center + radius * (T * ct1 + B * st1);

        renderer.Add(p0, p1, color);
    }
}

//-----------------------------------------------------------------------------
//      ワイヤーフレームの扇形を描画します.
//-----------------------------------------------------------------------------
void DrawWireFan(LineRenderer& renderer, const Vector3& center, const Vector3& normal, float radius, float angle, const Vector4& color, uint32_t segments)
{
    Vector3 N = Vector3::Normalize(normal);
    Vector3 T;
    Vector3 B;
    CalcONB(N, T, B);

    const float step = angle / static_cast<float>(segments);
    for (auto i = 0u; i < segments; ++i)
    {
        auto theta0 = step * (i + 0) - angle * 0.5f;
        auto theta1 = step * (i + 1) - angle * 0.5f;
        auto ct0    = cosf(theta0);
        auto st0    = sinf(theta0);
        auto ct1    = cosf(theta1);
        auto st1    = sinf(theta1);
        Vector3 p0  = center + radius * (T * ct0 + B * st0);
        Vector3 p1  = center + radius * (T * ct1 + B * st1);

        renderer.Add(center, p0, color);
        renderer.Add(p0, p1, color);
    }
}

//-----------------------------------------------------------------------------
//      ワイヤーフレームのボーンを描画します.
//-----------------------------------------------------------------------------
void DrawWireBone(LineRenderer& renderer, const Vector3& start, const Vector3& end, const Vector4& color)
{
    auto length = Vector3::Distance(start, end);
    if (length <= 1e-6f || isnan(length))
        return;

    auto size = length * 0.0625f;    // 1/16 サイズ.

    Vector3 N = Vector3::Normalize(end - start);
    Vector3 T;
    Vector3 B;
    CalcONB(N, T, B);
 
    Vector3 p0 = start + size * ( T +  B + N);
    Vector3 p1 = start + size * (-T +  B + N);
    Vector3 p2 = start + size * (-T + -B + N);
    Vector3 p3 = start + size * ( T + -B + N);

    renderer.Add(p0, p1, color);
    renderer.Add(p1, p2, color);
    renderer.Add(p2, p3, color);
    renderer.Add(p3, p0, color);

    renderer.Add(start, p0, color);
    renderer.Add(start, p1, color);
    renderer.Add(start, p2, color);
    renderer.Add(start, p3, color);

    renderer.Add(end, p0, color);
    renderer.Add(end, p1, color);
    renderer.Add(end, p2, color);
    renderer.Add(end, p3, color);
}

} // namespace asdx
