//-----------------------------------------------------------------------------
// File : asdxGuiMgr.cpp
// Desc : GUI Manager.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <edit/asdxGuiMgr.h>

#ifdef ASDX_ENABLE_IMGUI
#include <fnd/asdxMisc.h>
#include <fnd/asdxLogger.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxCommandList.h>
#include <res/asdxResTexture.h>
#include <imgui.h>
#include <imgui_internal.h>


namespace {

//-----------------------------------------------------------------------------
// Global Varaibles.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/ImGuiVS.inc"
#include "../res/shaders/Compiled/ImGuiPS.inc"


///////////////////////////////////////////////////////////////////////////////
// TransformBuffer
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) TransformBuffer
{
    float WorldViewProjection[ 4 ][ 4 ];
};

//-----------------------------------------------------------------------------
//      クリップボードテキストを取得します.
//-----------------------------------------------------------------------------
const char* GetClipboardText(void*)
{
    static char* buf_local = NULL;
    if (buf_local)
    {
        ImGui::MemFree(buf_local);
        buf_local = NULL;
    }
    if (!OpenClipboard(NULL))
        return NULL;
    HANDLE wbuf_handle = GetClipboardData(CF_UNICODETEXT);
    if (wbuf_handle == NULL)
        return NULL;
    if (ImWchar* wbuf_global = (ImWchar*)GlobalLock(wbuf_handle))
    {
        int buf_len = ImTextCountUtf8BytesFromStr(wbuf_global, NULL) + 1;
        buf_local = (char*)ImGui::MemAlloc(buf_len * sizeof(char));
        ImTextStrToUtf8(buf_local, buf_len, wbuf_global, NULL);
    }
    GlobalUnlock(wbuf_handle);
    CloseClipboard();
    return buf_local;
}

//-----------------------------------------------------------------------------
//      クリップボードテキストを設定します.
//-----------------------------------------------------------------------------
void SetClipboardText(void*, const char* text)
{
    if (!OpenClipboard(NULL))
        return;
    const int wbuf_length = ImTextCountCharsFromUtf8(text, NULL) + 1;
    HGLOBAL wbuf_handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wbuf_length * sizeof(ImWchar));
    if (wbuf_handle == NULL)
        return;
    ImWchar* wbuf_global = (ImWchar*)GlobalLock(wbuf_handle);
    ImTextStrFromUtf8(wbuf_global, wbuf_length, text, NULL);
    GlobalUnlock(wbuf_handle);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, wbuf_handle);
    CloseClipboard();
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GuiMgr class
///////////////////////////////////////////////////////////////////////////////
GuiMgr GuiMgr::s_Instance;

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GuiMgr::GuiMgr()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GuiMgr::~GuiMgr()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
GuiMgr& GuiMgr::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期処理を行います.
//-----------------------------------------------------------------------------
bool GuiMgr::Init
(
    ID3D12GraphicsCommandList*  pCmdList,
    HWND                        hWnd,
    uint32_t                    width,
    uint32_t                    height,
    DXGI_FORMAT                 format,
    const char*                 fontPath
)
{
    m_LastTime = std::chrono::system_clock::now();

    m_pGuiContext = ImGui::CreateContext();

    auto& io = ImGui::GetIO();

    {
        std::string path;
        if (asdx::SearchFilePathA(fontPath, path))
        {
            auto utf8_path = asdx::ToStringUTF8(path);
            io.Fonts->AddFontFromFileTTF(utf8_path.c_str(), 12.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
        }

        uint8_t* pPixels;
        int width;
        int height;
        io.Fonts->GetTexDataAsRGBA32( &pPixels, &width, &height );

        asdx::ResSubResource subRes;
        subRes.Width       = width;
        subRes.Height      = height;
        subRes.RowPitch    = width * 4;
        subRes.SlicePitch  = width * height * 4;
        subRes.pPixels     = pPixels;

        asdx::ResTexture res;
        res.Dimension           = asdx::TEXTURE_DIMENSION_2D;
        res.Width               = width;
        res.Height              = height;
        res.DepthOrArraySize    = 1;
        res.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
        res.MipLevels           = 1;
        res.SubResourceCount    = 1;
        res.SubResources[0]     = subRes;

        if (!m_FontTexture.Init(pCmdList, res))
        {
            ELOG("Error : Texture::Init() Failed.");
            return false;
        }

        io.Fonts->TexID = (void*)m_FontTexture.GetCpuHandleSRV().ptr;
    }

    auto pDevice = GetD3D12Device();

    // ルートシグニチャの生成.
    {
        D3D12_DESCRIPTOR_RANGE range = {};
        range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.NumDescriptors                    = 1;
        range.BaseShaderRegister                = 0;
        range.RegisterSpace                     = 0;
        range.OffsetInDescriptorsFromTableStart = 0;
 
        D3D12_ROOT_PARAMETER params[2];
        params[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
        params[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
        params[0].Descriptor.ShaderRegister = 0;
        params[0].Descriptor.RegisterSpace  = 0;

        params[1].ParameterType                         = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        params[1].ShaderVisibility                      = D3D12_SHADER_VISIBILITY_PIXEL;
        params[1].DescriptorTable.NumDescriptorRanges   = 1;
        params[1].DescriptorTable.pDescriptorRanges     = &range;

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.ShaderRegister      = 0;
        sampler.RegisterSpace       = 0;
        sampler.ShaderVisibility    = D3D12_SHADER_VISIBILITY_PIXEL;
        sampler.Filter              = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU            = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV            = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW            = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.MipLODBias          = 0.0f;
        sampler.ComparisonFunc      = D3D12_COMPARISON_FUNC_ALWAYS;
        sampler.MinLOD              = 0.0f;
        sampler.MaxLOD              = FLT_MAX;
        sampler.MaxAnisotropy       = 0;
        sampler.BorderColor         = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

        auto flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

        RefPtr<ID3DBlob> pBlob;
        RefPtr<ID3DBlob> pErrorBlob;

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = 2;
        desc.pParameters        = params;
        desc.NumStaticSamplers  = 1;
        desc.pStaticSamplers    = &sampler;
        desc.Flags              = flags;

        auto hr = D3D12SerializeRootSignature(
            &desc, D3D_ROOT_SIGNATURE_VERSION_1_0, pBlob.GetAddress(), pErrorBlob.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x, msg = %s", hr, (char*)pErrorBlob->GetBufferPointer());
            return false;
        }

        hr = pDevice->CreateRootSignature(
            0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_RootSig.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateRootSigature() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // パイプラインステートの生成.
    {
        D3D12_INPUT_ELEMENT_DESC elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.AlphaToCoverageEnable                 = FALSE;
        blendDesc.IndependentBlendEnable                = FALSE;
        blendDesc.RenderTarget[0].BlendEnable           = TRUE;
        blendDesc.RenderTarget[0].LogicOpEnable         = FALSE;
        blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode                 = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode                 = D3D12_CULL_MODE_NONE;
        rasterizerDesc.FrontCounterClockwise    = FALSE;
        rasterizerDesc.DepthClipEnable          = TRUE;
        rasterizerDesc.MultisampleEnable        = FALSE;
        rasterizerDesc.AntialiasedLineEnable    = TRUE;

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable    = FALSE;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc      = D3D12_COMPARISON_FUNC_ALWAYS;
        depthStencilDesc.StencilEnable  = FALSE;
        depthStencilDesc.FrontFace.StencilDepthFailOp
            = depthStencilDesc.FrontFace.StencilDepthFailOp
            = depthStencilDesc.FrontFace.StencilPassOp
            = D3D12_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        depthStencilDesc.BackFace = depthStencilDesc.FrontFace;


        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature         = m_RootSig.GetPtr();
        desc.VS                     = { ImGuiVS, sizeof(ImGuiVS) };
        desc.PS                     = { ImGuiPS, sizeof(ImGuiPS) };
        desc.BlendState             = blendDesc;
        desc.SampleMask             = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState        = rasterizerDesc;
        desc.DepthStencilState      = depthStencilDesc;
        desc.InputLayout            = { elements, 3 };
        desc.PrimitiveTopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets       = 1;
        desc.RTVFormats[0]          = format;
        desc.DSVFormat              = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count       = 1;
        desc.SampleDesc.Quality     = 0;
        desc.Flags                  = D3D12_PIPELINE_STATE_FLAG_NONE;

        auto hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_PSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    for(auto i=0; i<2; ++i)
    {
        m_VB[i].Term();
        m_SizeVB = MaxPrimitiveCount * 4;
        if (!m_VB[i].Init(m_SizeVB * sizeof(ImDrawVert), sizeof(ImDrawVert)))
        {
            ELOG("Error : VertexBuffer::Init() Failed.");
            return false;
        }

        m_IB[i].Term();
        m_SizeIB = MaxPrimitiveCount * 6;
        if (!m_IB[i].Init(sizeof(ImDrawIdx) * m_SizeIB, true))
        {
            ELOG("Error : IndexBuffer::Init() Failed.");
            return false;
        }
    }

    m_BufferIndex = 0;

    for(auto i=0; i<2; ++i)
    {
        if (!m_CB[i].Init(sizeof(TransformBuffer)))
        {
            ELOG("Error : ConstantBuffer::Init() Failed.");
            return false;
        }
    }

    {
        io.KeyMap[ ImGuiKey_Tab ]       = VK_TAB;
        io.KeyMap[ ImGuiKey_LeftArrow ] = VK_LEFT;
        io.KeyMap[ ImGuiKey_RightArrow ]= VK_RIGHT;
        io.KeyMap[ ImGuiKey_UpArrow ]   = VK_UP;
        io.KeyMap[ ImGuiKey_DownArrow ] = VK_DOWN;
        io.KeyMap[ ImGuiKey_PageUp ]    = VK_PRIOR;
        io.KeyMap[ ImGuiKey_PageDown ]  = VK_NEXT;
        io.KeyMap[ ImGuiKey_Home ]      = VK_HOME;
        io.KeyMap[ ImGuiKey_End ]       = VK_END;
        io.KeyMap[ ImGuiKey_Delete ]    = VK_DELETE;
        io.KeyMap[ ImGuiKey_Backspace ] = VK_BACK;
        io.KeyMap[ ImGuiKey_Enter ]     = VK_RETURN;
        io.KeyMap[ ImGuiKey_Escape ]    = VK_ESCAPE;
        io.KeyMap[ ImGuiKey_A ]         = 'A';
        io.KeyMap[ ImGuiKey_C ]         = 'C';
        io.KeyMap[ ImGuiKey_V ]         = 'V';
        io.KeyMap[ ImGuiKey_X ]         = 'X';
        io.KeyMap[ ImGuiKey_Y ]         = 'Y';
        io.KeyMap[ ImGuiKey_Z ]         = 'Z';

        //io.RenderDrawListsFn    = RenderImGui;
        io.SetClipboardTextFn   = SetClipboardText;
        io.GetClipboardTextFn   = GetClipboardText;
        io.ImeWindowHandle      = hWnd;
        io.DisplaySize.x        = float( width );
        io.DisplaySize.y        = float( height );

        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        io.DeltaTime   = 1.0f / 60.0f;

        auto& style = ImGui::GetStyle();
        style.WindowRounding = 2.0f;

#if 1
        style.Colors[ ImGuiCol_Text ]                   = ImVec4( 1.000000f, 1.000000f, 1.000000f, 1.000000f );
        style.Colors[ ImGuiCol_TextDisabled ]           = ImVec4( 0.400000f, 0.400000f, 0.400000f, 1.000000f );
        style.Colors[ ImGuiCol_WindowBg ]               = ImVec4( 0.060000f, 0.060000f, 0.060000f, 0.752000f );
        style.Colors[ ImGuiCol_PopupBg ]                = ImVec4( 0.000000f, 0.000000f, 0.000000f, 0.752000f );
        style.Colors[ ImGuiCol_Border ]                 = ImVec4( 1.000000f, 1.000000f, 1.000000f, 0.312000f );
        style.Colors[ ImGuiCol_BorderShadow ]           = ImVec4( 0.000000f, 0.000000f, 0.000000f, 0.080000f );
        style.Colors[ ImGuiCol_FrameBg ]                = ImVec4( 0.800000f, 0.800000f, 0.800000f, 0.300000f );
        style.Colors[ ImGuiCol_FrameBgHovered ]         = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.320000f );
        style.Colors[ ImGuiCol_FrameBgActive ]          = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.536000f );
        style.Colors[ ImGuiCol_TitleBg ]                = ImVec4( 0.000000f, 0.250000f, 0.500000f, 0.500000f );
        style.Colors[ ImGuiCol_TitleBgCollapsed ]       = ImVec4( 0.000000f, 0.000000f, 0.500000f, 0.500000f );
        style.Colors[ ImGuiCol_TitleBgActive ]          = ImVec4( 0.000000f, 0.500000f, 1.000000f, 0.800000f );
        style.Colors[ ImGuiCol_MenuBarBg ]              = ImVec4( 0.140000f, 0.140000f, 0.140000f, 1.000000f );
        style.Colors[ ImGuiCol_ScrollbarBg ]            = ImVec4( 0.020000f, 0.020000f, 0.020000f, 0.424000f );
        style.Colors[ ImGuiCol_ScrollbarGrab ]          = ImVec4( 0.310000f, 0.310000f, 0.310000f, 1.000000f );
        style.Colors[ ImGuiCol_ScrollbarGrabHovered ]   = ImVec4( 0.410000f, 0.410000f, 0.410000f, 1.000000f );
        style.Colors[ ImGuiCol_ScrollbarGrabActive ]    = ImVec4( 0.510000f, 0.510000f, 0.510000f, 1.000000f );
        style.Colors[ ImGuiCol_CheckMark ]              = ImVec4( 0.260000f, 0.590000f, 0.980000f, 1.000000f );
        style.Colors[ ImGuiCol_SliderGrab ]             = ImVec4( 0.240000f, 0.520000f, 0.880000f, 1.000000f );
        style.Colors[ ImGuiCol_SliderGrabActive ]       = ImVec4( 0.260000f, 0.590000f, 0.980000f, 1.000000f );
        style.Colors[ ImGuiCol_Button ]                 = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.320000f );
        style.Colors[ ImGuiCol_ButtonHovered ]          = ImVec4( 0.260000f, 0.590000f, 0.980000f, 1.000000f );
        style.Colors[ ImGuiCol_ButtonActive ]           = ImVec4( 0.060000f, 0.530000f, 0.980000f, 1.000000f );
        style.Colors[ ImGuiCol_Header ]                 = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.248000f );
        style.Colors[ ImGuiCol_HeaderHovered ]          = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.640000f );
        style.Colors[ ImGuiCol_HeaderActive ]           = ImVec4( 0.260000f, 0.590000f, 0.980000f, 1.000000f );
        style.Colors[ ImGuiCol_ResizeGrip ]             = ImVec4( 0.000000f, 0.000000f, 0.000000f, 0.400000f );
        style.Colors[ ImGuiCol_ResizeGripHovered ]      = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.536000f );
        style.Colors[ ImGuiCol_ResizeGripActive ]       = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.760000f );
        style.Colors[ ImGuiCol_PlotLines ]              = ImVec4( 0.610000f, 0.610000f, 0.610000f, 1.000000f );
        style.Colors[ ImGuiCol_PlotLinesHovered ]       = ImVec4( 1.000000f, 0.430000f, 0.350000f, 1.000000f );
        style.Colors[ ImGuiCol_PlotHistogram ]          = ImVec4( 0.900000f, 0.700000f, 0.000000f, 1.000000f );
        style.Colors[ ImGuiCol_PlotHistogramHovered ]   = ImVec4( 1.000000f, 0.600000f, 0.000000f, 1.000000f );
        style.Colors[ ImGuiCol_TextSelectedBg ]         = ImVec4( 0.260000f, 0.590000f, 0.980000f, 0.280000f );
        //style.Colors[ ImGuiCol_ModalWindowDarkening ]   = ImVec4( 0.800000f, 0.800000f, 0.800000f, 0.280000f );
#endif
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GuiMgr::Term()
{
    for(auto i=0; i<2; ++i)
    {
        m_VB[i].Term();
        m_IB[i].Term();
        m_CB[i].Term();
    }
    m_FontTexture.Term();

    m_RootSig.Reset();
    m_PSO    .Reset();

    if (m_pGuiContext != nullptr)
    {
        ImGui::DestroyContext(m_pGuiContext);
        m_pGuiContext = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void GuiMgr::Update( uint32_t width, uint32_t height )
{
    auto time = std::chrono::system_clock::now();
    auto elapsedMilliSec = std::chrono::duration_cast<std::chrono::microseconds>( time - m_LastTime ).count();
    auto elapsedSec = static_cast<float>( double(elapsedMilliSec) / (1000.0 * 1000.0) );
    assert(elapsedSec > 0.0f); // ImGuiで落とされる前にチェックする.

    auto& io = ImGui::GetIO();
    io.DeltaTime     = elapsedSec;
    io.DisplaySize.x = float( width );
    io.DisplaySize.y = float( height );
    io.KeyCtrl       = ( GetKeyState( VK_CONTROL ) & 0x8000 ) != 0;
    io.KeyShift      = ( GetKeyState( VK_SHIFT )   & 0x8000 ) != 0;
    io.KeyAlt        = ( GetKeyState( VK_MENU )    & 0x8000 ) != 0;

    ImGui::NewFrame();

    m_LastTime = time;
}

//-----------------------------------------------------------------------------
//      描画処理です.
//-----------------------------------------------------------------------------
void GuiMgr::Draw(ID3D12GraphicsCommandList* pCmdList)
{
    if (pCmdList == nullptr)
    { return; }

    ImGui::Render();

    auto pDrawData = ImGui::GetDrawData();
    if (pDrawData == nullptr)
    { return; }

    if ( uint32_t( pDrawData->TotalVtxCount ) >= m_SizeVB )
    {
        auto resource = m_VB[m_BufferIndex].GetResource();
        if (resource != nullptr)
        {
            resource->AddRef();
            Dispose(resource);
        }

        m_VB[m_BufferIndex].Term();
        m_SizeVB = pDrawData->TotalVtxCount + 5000;
        if (!m_VB[m_BufferIndex].Init(m_SizeVB * sizeof(ImDrawVert), sizeof(ImDrawVert)))
        { return; }
    }

    if ( pDrawData->TotalIdxCount >= MaxPrimitiveCount * 6 )
    {
        auto resource = m_IB[m_BufferIndex].GetResource();
        if (resource != nullptr)
        {
            resource->AddRef();
            Dispose(resource);
        }

        m_IB[m_BufferIndex].Term();
        m_SizeIB = pDrawData->TotalIdxCount + 10000;
        if (!m_IB[m_BufferIndex].Init(m_SizeIB * sizeof(uint32_t), true))
        { return; }
    }

    auto pDstVtx = m_VB[m_BufferIndex].MapAs<ImDrawVert>();
    auto pDstIdx = m_IB[m_BufferIndex].MapAs<ImDrawIdx>();

    for ( auto i = 0; i < pDrawData->CmdListsCount; ++i )
    {
        const auto pCmdList = pDrawData->CmdLists[ i ];
        memcpy( pDstVtx, pCmdList->VtxBuffer.Data, pCmdList->VtxBuffer.size() * sizeof( ImDrawVert ) );
        memcpy( pDstIdx, pCmdList->IdxBuffer.Data, pCmdList->IdxBuffer.size() * sizeof( ImDrawIdx ) );
        pDstVtx += pCmdList->VtxBuffer.size();
        pDstIdx += pCmdList->IdxBuffer.size();
    }

    m_VB[m_BufferIndex].Unmap();
    m_IB[m_BufferIndex].Unmap();

    {
        float L = 0.0f;
        float R = ImGui::GetIO().DisplaySize.x;
        float B = ImGui::GetIO().DisplaySize.y;
        float T = 0.0f;

        float mvp[ 4 ][ 4 ] = {
            { 2.0f / ( R - L ),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f / ( T - B ),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { ( R + L ) / ( L - R ),  ( T + B ) / ( B - T ),    0.5f,       1.0f },
        };

        auto ptr = m_CB[m_BufferIndex].Map();
        memcpy(ptr, &mvp, sizeof(mvp));
        m_CB[m_BufferIndex].Unmap();
    }

    {
        D3D12_VIEWPORT viewport = {};
        viewport.TopLeftX   = 0.0f;
        viewport.TopLeftY   = 0.0f;
        viewport.Width      = ImGui::GetIO().DisplaySize.x;
        viewport.Height     = ImGui::GetIO().DisplaySize.y;
        viewport.MinDepth   = 0.0f;
        viewport.MaxDepth   = 1.0f;

        pCmdList->RSSetViewports(1, &viewport);
    }

    {
        auto vbv = m_VB[m_BufferIndex].GetVBV();
        auto ibv = m_IB[m_BufferIndex].GetIBV();
        pCmdList->SetGraphicsRootSignature(m_RootSig.GetPtr());
        pCmdList->SetPipelineState(m_PSO.GetPtr());
        pCmdList->SetGraphicsRootConstantBufferView(0, m_CB[m_BufferIndex].GetGpuAddress());
        pCmdList->SetGraphicsRootDescriptorTable(1, m_FontTexture.GetGpuHandleSRV());
        pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pCmdList->IASetVertexBuffers(0, 1, &vbv);
        pCmdList->IASetIndexBuffer(&ibv);
    }

    {
        int offsetVtx = 0;
        int offsetIdx = 0;
        auto changeTexture = false;

        for ( auto i = 0; i < pDrawData->CmdListsCount; ++i )
        {
            const auto pList = pDrawData->CmdLists[ i ];
            for ( auto j = 0; j < pList->CmdBuffer.size(); ++j )
            {
                const auto pCmd = &pList->CmdBuffer[ j ];
                if ( pCmd->UserCallback )
                {
                    pCmd->UserCallback( pList, pCmd );
                }
                else
                {
                    // テクスチャが渡された場合は変更.
                    if (pCmd->TextureId != nullptr)
                    {
                        D3D12_GPU_DESCRIPTOR_HANDLE handle;
                        handle.ptr = (UINT64)pCmd->TextureId;
                        pCmdList->SetGraphicsRootDescriptorTable(1, handle);
                        changeTexture = true;
                    }
                    else
                    {
                        // フォントのテクスチャに戻す.
                        if (changeTexture)
                        {
                            D3D12_GPU_DESCRIPTOR_HANDLE handle = m_FontTexture.GetGpuHandleSRV();
                            pCmdList->SetGraphicsRootDescriptorTable(1, handle);
                            changeTexture = false;
                        }
                    }

                    const D3D12_RECT rc = {
                        LONG(pCmd->ClipRect.x),
                        LONG(pCmd->ClipRect.y),
                        LONG(pCmd->ClipRect.z),
                        LONG(pCmd->ClipRect.w)
                    };

                    pCmdList->RSSetScissorRects(1, &rc);
                    pCmdList->DrawIndexedInstanced(pCmd->ElemCount, 1, offsetIdx, offsetVtx, 0);
                }
                offsetIdx += pCmd->ElemCount;
            }
            offsetVtx += pList->VtxBuffer.size();
        }
    }

    m_BufferIndex = (m_BufferIndex + 1) & 0x1;
}

//-----------------------------------------------------------------------------
//      マウスの処理です.
//-----------------------------------------------------------------------------
void GuiMgr::OnMouse( int x, int y, int wheelDelta, bool isDownL, bool isDownM, bool isDownR )
{
    auto& io = ImGui::GetIO();

    io.MousePosPrev = io.MousePos;
    io.MousePos = ImVec2( float( x ), float( y ) );
    io.MouseDown[ 0 ] = isDownL;
    io.MouseDown[ 1 ] = isDownR;
    io.MouseDown[ 2 ] = isDownM;
    io.MouseDown[ 3 ] = false;
    io.MouseDown[ 4 ] = false;
    if ( wheelDelta > 0 )
    {
        io.MouseWheel = 1.0f;
    }
    else if ( wheelDelta < 0 )
    {
        io.MouseWheel = -1.0f;
    }
    else
    {
        io.MouseWheel = 0;
    }
}

//-----------------------------------------------------------------------------
//      キーの処理です.
//-----------------------------------------------------------------------------
void GuiMgr::OnKey( bool isDown, bool isAltDown, uint32_t code )
{
    auto& io = ImGui::GetIO();

    io.KeysDown[ code ] = isDown;
    io.KeyAlt = isAltDown;
}

//-------------------------------------------------------------------------------------------------
//      タイピング処理です.
//-------------------------------------------------------------------------------------------------
void GuiMgr::OnTyping( uint32_t code )
{
    if ( code > 0 && code < 0x10000 )
    {
        auto& io = ImGui::GetIO();
        io.AddInputCharacter( ImWchar( code ) );
    }
}

} // namespace asdx
#endif//ASDX_ENABLE_IMGUI
