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
#include <fnd/asdxLogger.h>
#include <fnd/asdxPath.h>
#include <gfx/asdxDevice.h>
#include <res/asdxResTexture.h>
#include <imgui.h>
#include <imgui_internal.h>


namespace {

//-----------------------------------------------------------------------------
// Global Varaibles.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/ImGuiVS.inc"
#include "../res/shaders/Compiled/ImGuiPS.inc"
#include "YasashisaGothic.h"


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

ImGuiKey ToImGuiKey(uint32_t code)
{
    switch (code)
    {
        case VK_TAB: return ImGuiKey_Tab;
        case VK_LEFT: return ImGuiKey_LeftArrow;
        case VK_RIGHT: return ImGuiKey_RightArrow;
        case VK_UP: return ImGuiKey_UpArrow;
        case VK_DOWN: return ImGuiKey_DownArrow;
        case VK_PRIOR: return ImGuiKey_PageUp;
        case VK_NEXT: return ImGuiKey_PageDown;
        case VK_HOME: return ImGuiKey_Home;
        case VK_END: return ImGuiKey_End;
        case VK_INSERT: return ImGuiKey_Insert;
        case VK_DELETE: return ImGuiKey_Delete;
        case VK_BACK: return ImGuiKey_Backspace;
        case VK_SPACE: return ImGuiKey_Space;
        case VK_RETURN: return ImGuiKey_Enter;
        case VK_ESCAPE: return ImGuiKey_Escape;
        //case VK_OEM_7: return ImGuiKey_Apostrophe;
        case VK_OEM_COMMA: return ImGuiKey_Comma;
        //case VK_OEM_MINUS: return ImGuiKey_Minus;
        case VK_OEM_PERIOD: return ImGuiKey_Period;
        //case VK_OEM_2: return ImGuiKey_Slash;
        //case VK_OEM_1: return ImGuiKey_Semicolon;
        //case VK_OEM_PLUS: return ImGuiKey_Equal;
        //case VK_OEM_4: return ImGuiKey_LeftBracket;
        //case VK_OEM_5: return ImGuiKey_Backslash;
        //case VK_OEM_6: return ImGuiKey_RightBracket;
        //case VK_OEM_3: return ImGuiKey_GraveAccent;
        case VK_CAPITAL: return ImGuiKey_CapsLock;
        case VK_SCROLL: return ImGuiKey_ScrollLock;
        case VK_NUMLOCK: return ImGuiKey_NumLock;
        case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
        case VK_PAUSE: return ImGuiKey_Pause;
        case VK_NUMPAD0: return ImGuiKey_Keypad0;
        case VK_NUMPAD1: return ImGuiKey_Keypad1;
        case VK_NUMPAD2: return ImGuiKey_Keypad2;
        case VK_NUMPAD3: return ImGuiKey_Keypad3;
        case VK_NUMPAD4: return ImGuiKey_Keypad4;
        case VK_NUMPAD5: return ImGuiKey_Keypad5;
        case VK_NUMPAD6: return ImGuiKey_Keypad6;
        case VK_NUMPAD7: return ImGuiKey_Keypad7;
        case VK_NUMPAD8: return ImGuiKey_Keypad8;
        case VK_NUMPAD9: return ImGuiKey_Keypad9;
        case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
        case VK_DIVIDE: return ImGuiKey_KeypadDivide;
        case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case VK_ADD: return ImGuiKey_KeypadAdd;
        case VK_LSHIFT: return ImGuiKey_LeftShift;
        case VK_LCONTROL: return ImGuiKey_LeftCtrl;
        case VK_LMENU: return ImGuiKey_LeftAlt;
        case VK_LWIN: return ImGuiKey_LeftSuper;
        case VK_RSHIFT: return ImGuiKey_RightShift;
        case VK_RCONTROL: return ImGuiKey_RightCtrl;
        case VK_RMENU: return ImGuiKey_RightAlt;
        case VK_RWIN: return ImGuiKey_RightSuper;
        case VK_APPS: return ImGuiKey_Menu;
        case '0': return ImGuiKey_0;
        case '1': return ImGuiKey_1;
        case '2': return ImGuiKey_2;
        case '3': return ImGuiKey_3;
        case '4': return ImGuiKey_4;
        case '5': return ImGuiKey_5;
        case '6': return ImGuiKey_6;
        case '7': return ImGuiKey_7;
        case '8': return ImGuiKey_8;
        case '9': return ImGuiKey_9;
        case 'A': return ImGuiKey_A;
        case 'B': return ImGuiKey_B;
        case 'C': return ImGuiKey_C;
        case 'D': return ImGuiKey_D;
        case 'E': return ImGuiKey_E;
        case 'F': return ImGuiKey_F;
        case 'G': return ImGuiKey_G;
        case 'H': return ImGuiKey_H;
        case 'I': return ImGuiKey_I;
        case 'J': return ImGuiKey_J;
        case 'K': return ImGuiKey_K;
        case 'L': return ImGuiKey_L;
        case 'M': return ImGuiKey_M;
        case 'N': return ImGuiKey_N;
        case 'O': return ImGuiKey_O;
        case 'P': return ImGuiKey_P;
        case 'Q': return ImGuiKey_Q;
        case 'R': return ImGuiKey_R;
        case 'S': return ImGuiKey_S;
        case 'T': return ImGuiKey_T;
        case 'U': return ImGuiKey_U;
        case 'V': return ImGuiKey_V;
        case 'W': return ImGuiKey_W;
        case 'X': return ImGuiKey_X;
        case 'Y': return ImGuiKey_Y;
        case 'Z': return ImGuiKey_Z;
        case VK_F1: return ImGuiKey_F1;
        case VK_F2: return ImGuiKey_F2;
        case VK_F3: return ImGuiKey_F3;
        case VK_F4: return ImGuiKey_F4;
        case VK_F5: return ImGuiKey_F5;
        case VK_F6: return ImGuiKey_F6;
        case VK_F7: return ImGuiKey_F7;
        case VK_F8: return ImGuiKey_F8;
        case VK_F9: return ImGuiKey_F9;
        case VK_F10: return ImGuiKey_F10;
        case VK_F11: return ImGuiKey_F11;
        case VK_F12: return ImGuiKey_F12;
        case VK_F13: return ImGuiKey_F13;
        case VK_F14: return ImGuiKey_F14;
        case VK_F15: return ImGuiKey_F15;
        case VK_F16: return ImGuiKey_F16;
        case VK_F17: return ImGuiKey_F17;
        case VK_F18: return ImGuiKey_F18;
        case VK_F19: return ImGuiKey_F19;
        case VK_F20: return ImGuiKey_F20;
        case VK_F21: return ImGuiKey_F21;
        case VK_F22: return ImGuiKey_F22;
        case VK_F23: return ImGuiKey_F23;
        case VK_F24: return ImGuiKey_F24;
        case VK_BROWSER_BACK: return ImGuiKey_AppBack;
        case VK_BROWSER_FORWARD: return ImGuiKey_AppForward;
        default: break;
    }

    return ImGuiKey_None;
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
        fs::path path;
        if (fontPath != nullptr && asdx::SearchFilePath(fontPath, path))
        {
            auto utf8_path = path.string();
            io.Fonts->AddFontFromFileTTF(
                utf8_path.c_str(),
                12.0f,
                nullptr,
                io.Fonts->GetGlyphRangesJapanese());
        }
        else
        {
            io.Fonts->AddFontFromMemoryCompressedTTF(
                YasashisaGothic_compressed_data,
                YasashisaGothic_compressed_size,
                12.0f,
                nullptr,
                io.Fonts->GetGlyphRangesJapanese());
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

        io.Fonts->SetTexID((ImTextureID)m_FontTexture.GetGpuHandleSRV().ptr);
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
        io.SetClipboardTextFn   = SetClipboardText;
        io.GetClipboardTextFn   = GetClipboardText;
        io.DisplaySize.x        = float(width);
        io.DisplaySize.y        = float(height);

        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        io.DeltaTime   = 1.0f / 60.0f;  // 0.0f以外の値になっていればいい.

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
    io.DisplaySize.x = float(width);
    io.DisplaySize.y = float(height);
    io.KeyCtrl       = !!(GetKeyState(VK_CONTROL) & 0x8000);
    io.KeyShift      = !!(GetKeyState(VK_SHIFT)   & 0x8000);
    io.KeyAlt        = !!(GetKeyState(VK_MENU)    & 0x8000);

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
                    auto textureId = pCmd->GetTexID();
                    if (textureId != ImTextureID_Invalid)
                    {
                        D3D12_GPU_DESCRIPTOR_HANDLE handle = {};
                        handle.ptr = textureId;
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
        io.MouseWheel = 0.0f;
    }
}

//-----------------------------------------------------------------------------
//      キーの処理です.
//-----------------------------------------------------------------------------
void GuiMgr::OnKey(uint32_t code, bool isDown, bool isAltDown)
{
    auto& io = ImGui::GetIO();
    auto key = ToImGuiKey(code);
    io.AddKeyEvent(key, isDown);
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
