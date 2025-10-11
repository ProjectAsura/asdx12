//-----------------------------------------------------------------------------
// File : asdxPresetState.cpp
// Desc : Preset State.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxPresetState.h>


namespace {
namespace internal {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/SpriteVS.inc"
#include "../res/shaders/Compiled/SpritePS.inc"
#include "../res/shaders/Compiled/FullScreenVS.inc"
#include "../res/shaders/Compiled/CopyPS.inc"

} // namespace internal
} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Preset class
///////////////////////////////////////////////////////////////////////////////
const D3D12_RASTERIZER_DESC Preset::CullNone = {
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
const D3D12_RASTERIZER_DESC Preset::CullBack = {
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
const D3D12_RASTERIZER_DESC Preset::CullFront = {
    D3D12_FILL_MODE_SOLID,
    D3D12_CULL_MODE_FRONT,
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
const D3D12_RASTERIZER_DESC Preset::Wireframe = {
    D3D12_FILL_MODE_WIREFRAME,
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

const D3D12_DEPTH_STENCILOP_DESC Preset::StencilDefault = {
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_KEEP,
    D3D12_COMPARISON_FUNC_ALWAYS
};

const D3D12_DEPTH_STENCIL_DESC Preset::DepthDefault = {
    TRUE,
    D3D12_DEPTH_WRITE_MASK_ALL,
    D3D12_COMPARISON_FUNC_LESS_EQUAL,
    FALSE,
    D3D12_DEFAULT_STENCIL_READ_MASK,
    D3D12_DEFAULT_STENCIL_WRITE_MASK,
    StencilDefault,
    StencilDefault
};
const D3D12_DEPTH_STENCIL_DESC Preset::DepthNone = {
    FALSE,
    D3D12_DEPTH_WRITE_MASK_ZERO,
    D3D12_COMPARISON_FUNC_ALWAYS,
    FALSE,
    D3D12_DEFAULT_STENCIL_READ_MASK,
    D3D12_DEFAULT_STENCIL_WRITE_MASK,
    StencilDefault,
    StencilDefault
};
const D3D12_DEPTH_STENCIL_DESC Preset::DepthReadOnly = {
    TRUE,
    D3D12_DEPTH_WRITE_MASK_ZERO,
    D3D12_COMPARISON_FUNC_LESS_EQUAL,
    FALSE,
    D3D12_DEFAULT_STENCIL_READ_MASK,
    D3D12_DEFAULT_STENCIL_WRITE_MASK,
    StencilDefault,
    StencilDefault
};
const D3D12_DEPTH_STENCIL_DESC Preset::DepthWriteOnly = {
    FALSE,
    D3D12_DEPTH_WRITE_MASK_ALL,
    D3D12_COMPARISON_FUNC_LESS_EQUAL,
    FALSE,
    D3D12_DEFAULT_STENCIL_READ_MASK,
    D3D12_DEFAULT_STENCIL_WRITE_MASK,
    StencilDefault,
    StencilDefault
};

const D3D12_RENDER_TARGET_BLEND_DESC Preset::RTB_Opaque = {
    FALSE,
    FALSE,
    D3D12_BLEND_ONE,
    D3D12_BLEND_ZERO,
    D3D12_BLEND_OP_ADD,
    D3D12_BLEND_ONE,
    D3D12_BLEND_ZERO,
    D3D12_BLEND_OP_ADD,
    D3D12_LOGIC_OP_NOOP,
    D3D12_COLOR_WRITE_ENABLE_ALL
};
const D3D12_RENDER_TARGET_BLEND_DESC Preset::RTB_AlphaBlend = {
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
const D3D12_RENDER_TARGET_BLEND_DESC Preset::RTB_Additive = {
    TRUE,
    FALSE,
    D3D12_BLEND_SRC_ALPHA,
    D3D12_BLEND_ONE,
    D3D12_BLEND_OP_ADD,
    D3D12_BLEND_SRC_ALPHA,
    D3D12_BLEND_ONE,
    D3D12_BLEND_OP_ADD,
    D3D12_LOGIC_OP_NOOP,
    D3D12_COLOR_WRITE_ENABLE_ALL
};
const D3D12_RENDER_TARGET_BLEND_DESC Preset::RTB_Subtract = {
    TRUE,
    FALSE,
    D3D12_BLEND_SRC_ALPHA,
    D3D12_BLEND_ONE,
    D3D12_BLEND_OP_REV_SUBTRACT,
    D3D12_BLEND_SRC_ALPHA,
    D3D12_BLEND_ONE,
    D3D12_BLEND_OP_REV_SUBTRACT,
    D3D12_LOGIC_OP_NOOP,
    D3D12_COLOR_WRITE_ENABLE_ALL
};
const D3D12_RENDER_TARGET_BLEND_DESC Preset::RTB_Premultiplied = {
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
const D3D12_RENDER_TARGET_BLEND_DESC Preset::RTB_Multiply = {
    TRUE,
    FALSE,
    D3D12_BLEND_ZERO,
    D3D12_BLEND_SRC_COLOR,
    D3D12_BLEND_OP_ADD,
    D3D12_BLEND_ZERO,
    D3D12_BLEND_SRC_ALPHA,
    D3D12_BLEND_OP_ADD,
    D3D12_LOGIC_OP_NOOP,
    D3D12_COLOR_WRITE_ENABLE_ALL
};

const D3D12_RENDER_TARGET_BLEND_DESC Preset::RTB_Screen = {
    TRUE,
    FALSE,
    D3D12_BLEND_DEST_COLOR,
    D3D12_BLEND_ONE,
    D3D12_BLEND_OP_ADD,
    D3D12_BLEND_DEST_ALPHA,
    D3D12_BLEND_ONE,
    D3D12_BLEND_OP_ADD,
    D3D12_LOGIC_OP_NOOP,
    D3D12_COLOR_WRITE_ENABLE_ALL
};

const D3D12_BLEND_DESC Preset::Opaque = {
    FALSE,
    FALSE,
    { RTB_Opaque, RTB_Opaque, RTB_Opaque, RTB_Opaque, RTB_Opaque, RTB_Opaque, RTB_Opaque, RTB_Opaque }
};
const D3D12_BLEND_DESC Preset::AlphaBlend = {
    FALSE,
    FALSE,
    { RTB_AlphaBlend, RTB_AlphaBlend, RTB_AlphaBlend, RTB_AlphaBlend, RTB_AlphaBlend, RTB_AlphaBlend, RTB_AlphaBlend, RTB_AlphaBlend }
};
const D3D12_BLEND_DESC Preset::Additive = {
    FALSE,
    FALSE,
    { RTB_Additive, RTB_Additive, RTB_Additive, RTB_Additive, RTB_Additive, RTB_Additive, RTB_Additive, RTB_Additive }
};
const D3D12_BLEND_DESC Preset::Subtract = {
    FALSE,
    FALSE,
    { RTB_Subtract, RTB_Subtract, RTB_Subtract, RTB_Subtract, RTB_Subtract, RTB_Subtract, RTB_Subtract, RTB_Subtract }
};
const D3D12_BLEND_DESC Preset::Premultiplied = {
    FALSE,
    FALSE,
    { RTB_Premultiplied, RTB_Premultiplied, RTB_Premultiplied, RTB_Premultiplied, RTB_Premultiplied, RTB_Premultiplied, RTB_Premultiplied, RTB_Premultiplied }
};
const D3D12_BLEND_DESC Preset::Multiply = {
    FALSE,
    FALSE,
    { RTB_Multiply, RTB_Multiply, RTB_Multiply, RTB_Multiply, RTB_Multiply, RTB_Multiply, RTB_Multiply, RTB_Multiply }
};
const D3D12_BLEND_DESC Preset::Screen = {
    FALSE,
    FALSE,
    { RTB_Screen, RTB_Screen, RTB_Screen, RTB_Screen, RTB_Screen, RTB_Screen, RTB_Screen, RTB_Screen }
};

const D3D12_SHADER_BYTECODE Preset::FullScreenVS = { internal::FullScreenVS, sizeof(internal::FullScreenVS) };
const D3D12_SHADER_BYTECODE Preset::CopyPS       = { internal::CopyPS,       sizeof(internal::CopyPS) };
const D3D12_SHADER_BYTECODE Preset::SpriteVS     = { internal::SpriteVS,     sizeof(internal::SpriteVS) };
const D3D12_SHADER_BYTECODE Preset::SpritePS     = { internal::SpritePS,     sizeof(internal::SpritePS) };

#define DEF_STATIC_SAMPLER(filter, addressMode, maxAnisotropy, comparison, borderColor, shaderRegister) \
    { filter, addressMode, addressMode, addressMode, 0.0f, maxAnisotropy, comparison, borderColor, 0.0f, D3D12_FLOAT32_MAX, shaderRegister, 0, D3D12_SHADER_VISIBILITY_ALL }

const D3D12_STATIC_SAMPLER_DESC Preset::StaticSamplers[11] = {
    DEF_STATIC_SAMPLER(D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_WRAP,   0,  D3D12_COMPARISON_FUNC_NEVER,      D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 0),
    DEF_STATIC_SAMPLER(D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  0,  D3D12_COMPARISON_FUNC_NEVER,      D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 1),
    DEF_STATIC_SAMPLER(D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_MIRROR, 0,  D3D12_COMPARISON_FUNC_NEVER,      D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 2),
    DEF_STATIC_SAMPLER(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,   0,  D3D12_COMPARISON_FUNC_NEVER,      D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 3),
    DEF_STATIC_SAMPLER(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  0,  D3D12_COMPARISON_FUNC_NEVER,      D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 4),
    DEF_STATIC_SAMPLER(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, 0,  D3D12_COMPARISON_FUNC_NEVER,      D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 5),
    DEF_STATIC_SAMPLER(D3D12_FILTER_ANISOTROPIC,        D3D12_TEXTURE_ADDRESS_MODE_WRAP,   16, D3D12_COMPARISON_FUNC_NEVER,      D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 6),
    DEF_STATIC_SAMPLER(D3D12_FILTER_ANISOTROPIC,        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  16, D3D12_COMPARISON_FUNC_NEVER,      D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 7),
    DEF_STATIC_SAMPLER(D3D12_FILTER_ANISOTROPIC,        D3D12_TEXTURE_ADDRESS_MODE_MIRROR, 16, D3D12_COMPARISON_FUNC_NEVER,      D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 8),
    DEF_STATIC_SAMPLER(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_BORDER, 0,  D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,      9),
    DEF_STATIC_SAMPLER(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_BORDER, 0,  D3D12_COMPARISON_FUNC_GREATER,    D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,     10),
};
#undef DEF_STATIC_SAMPLER

const D3D12_INPUT_ELEMENT_DESC Preset::QuadElements[2] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

} // namespace asdx
