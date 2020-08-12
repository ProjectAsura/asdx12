//-----------------------------------------------------------------------------
// File : Math.hlsli
// Desc : Math Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_SHADOW_HLSLI
#define ASDX_SHADOW_HLSLI


//-----------------------------------------------------------------------------
//      プロシージャルグリッドを描画します.
//-----------------------------------------------------------------------------
float3 DrawGrid
(
    float3 baseColor,         // 基本色.
    float2 shadowCoord,       // シャドウマップテクスチャ座標.
    float  gridLineWidth,     // グリッド線の太さ(ピクセル単位).
    float3 gridColor,         // グリッド線の色.
    float  spacing            // グリッド間隔(ピクセル単位).
)
{
    float2 st = shadowCoord / spacing;
    float2 dx = float2(ddx_fine(st.x), ddy_fine(st.x));
    float2 dy = float2(ddx_fine(st.y), ddy_fine(st.y));
    float2 m  = frac(st);

    if ( m.x < gridLineWidth * length(dx) 
      || m.y < gridLineWidth * length(dy))
    { return gridColor; }
    else
    { return baseColor; }
}

//-----------------------------------------------------------------------------
//      テクスチャを使って誤差を可視化します.
//-----------------------------------------------------------------------------
float3 VisualizeError
(
    Texture2D       errorMap,      // 誤差を表す色が格納されたテクスチャ.
    SamplerState    errorSmp,      // クランプサンプラー.
    float2          texcoord,      // シャドウマップテクスチャ座標.
    float2          map_size       // シャドウマップサイズ.
)
{
    float2 ds = map_size.x * ddx_fine(texcoord);
    float2 dt = map_size.y * ddy_fine(texcoord);
    float s = 0.1f; // [0, 10.0]の値を[0, 1]にマッピングするので0.1倍.
    float error = max(length(ds + dt), length(ds - dt)) * s;
    return errorMap.Sample(errorSmp, error).rgb;
}


#endif
