//-----------------------------------------------------------------------------
// File : ComputeUtil.hlsli
// Desc : Compute Shader Utility Functions.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_COMPUTE_UTIL_HLSLI
#define ASDX_COMPUTE_UTIL_HLSLI

//-----------------------------------------------------------------------------
//      ビットフィールドを抽出します.
//-----------------------------------------------------------------------------
uint BitfieldExtract(uint src, uint offset, uint bits)
{
    uint mask = (1u << bits) - 1u;
    return (src >> offset) & mask;
}

//-----------------------------------------------------------------------------
//      ビットフィールドを挿入します.
//-----------------------------------------------------------------------------
uint BitfieldInsert(uint src, uint insert, uint bits)
{
    uint mask = (1u << bits) - 1u;
    return (insert & mask) | (src & (~mask));
}

//-----------------------------------------------------------------------------
//      モートンオーダーにリマップします.
//-----------------------------------------------------------------------------
uint2 RemapLane8x8(uint2 dispatchId, uint groupIndex)
{
    uint2 remappedId;
    remappedId.x = BitfieldInsert(BitfieldExtract(groupIndex, 2u, 3u), groupIndex, 1u);
    remappedId.y = BitfieldInsert(BitfieldExtract(groupIndex, 3u, 3u), BitfieldExtract(groupIndex, 1u, 2u), 2u);

    uint2 dispatchGroupId = uint2(dispatchId) / 8; // 8未満切り捨て.
    return (dispatchGroupId * 8) + remappedId;
}

#endif//ASDX_COMPUTE_UTIL_HLSLI
