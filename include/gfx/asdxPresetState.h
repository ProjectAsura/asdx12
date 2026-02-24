//-----------------------------------------------------------------------------
// File : asdxPresetState.h
// Desc : Preset State.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d12.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Preset class
///////////////////////////////////////////////////////////////////////////////
class Preset
{
public:
    static const D3D12_RASTERIZER_DESC CullNone;    //!< カリング無し.
    static const D3D12_RASTERIZER_DESC CullBack;    //!< 背面カリング. 
    static const D3D12_RASTERIZER_DESC CullFront;   //!< 前面カリング.
    static const D3D12_RASTERIZER_DESC Wireframe;   //!< ワイヤーフレーム.

    static const D3D12_DEPTH_STENCILOP_DESC StencilDefault; //!< ステンシルデフォルト.

    static const D3D12_DEPTH_STENCIL_DESC DepthReadWrite;   //!< 深度テストON ・書き込み有り.
    static const D3D12_DEPTH_STENCIL_DESC DepthReadOnly;    //!< 深度テストON ・書き込み無し.
    static const D3D12_DEPTH_STENCIL_DESC DepthWriteOnly;   //!< 深度テストOFF・書き込み有り.
    static const D3D12_DEPTH_STENCIL_DESC DepthNone;        //!< 深度テストOFF・書き込み無し.
    static const D3D12_DEPTH_STENCIL_DESC ReverseDepthReadWrite;   //!< ReverseZ, 深度テストON ・書き込み有り.
    static const D3D12_DEPTH_STENCIL_DESC ReverseDepthReadOnly;    //!< ReverseZ, 深度テストON ・書き込み無し.
    static const D3D12_DEPTH_STENCIL_DESC ReverseDepthWriteOnly;   //!< ReverseZ, 深度テストOFF・書き込み有り.


    static const D3D12_RENDER_TARGET_BLEND_DESC RTB_Opaque;         //!< 不透明.
    static const D3D12_RENDER_TARGET_BLEND_DESC RTB_AlphaBlend;     //!< アルファブレンド.
    static const D3D12_RENDER_TARGET_BLEND_DESC RTB_Additive;       //!< 加算.
    static const D3D12_RENDER_TARGET_BLEND_DESC RTB_Subtract;       //!< 減算.
    static const D3D12_RENDER_TARGET_BLEND_DESC RTB_Premultiplied;  //!< 事前乗算済みアルファ.
    static const D3D12_RENDER_TARGET_BLEND_DESC RTB_Multiply;       //!< 乗算.
    static const D3D12_RENDER_TARGET_BLEND_DESC RTB_Screen;         //!< スクリーン.
    static const D3D12_RENDER_TARGET_BLEND_DESC RTB_Min;            //!< 最小値ブレンド.
    static const D3D12_RENDER_TARGET_BLEND_DESC RTB_Max;            //!< 最大値ブレンド.

    static const D3D12_BLEND_DESC Opaque;           //!< 不透明.
    static const D3D12_BLEND_DESC AlphaBlend;       //!< アルファブレンド.
    static const D3D12_BLEND_DESC Additive;         //!< 加算.
    static const D3D12_BLEND_DESC Subtract;         //!< 減算.
    static const D3D12_BLEND_DESC Premultiplied;    //!< 事前乗算済みアルファ.
    static const D3D12_BLEND_DESC Multiply;         //!< 乗算.
    static const D3D12_BLEND_DESC Screen;           //!< スクリーン.
    static const D3D12_BLEND_DESC BlendMin;         //!< 最小値ブレンド.
    static const D3D12_BLEND_DESC BlendMax;         //!< 最大値ブレンド.

    static const D3D12_SHADER_BYTECODE FullScreenVS;    //!< フルスクリーン用頂点シェーダ.
    static const D3D12_SHADER_BYTECODE CopyPS;          //!< コピー用ピクセルシェーダ.
    static const D3D12_SHADER_BYTECODE SpriteVS;        //!< スプライト用頂点シェーダ.
    static const D3D12_SHADER_BYTECODE SpritePS;        //!< スプライト用ピクセルシェーダ.

    static const D3D12_STATIC_SAMPLER_DESC StaticSamplers[11];  //!< スタティックサンプラー.
    static const D3D12_INPUT_ELEMENT_DESC  QuadElements[2];     //!< フルスクリーン矩形用入力レイアウト.
};

} // namespace asdx
