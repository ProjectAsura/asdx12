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
uint2 RemapLane4x4(uint2 dispatchId, uint groupIndex)
{
    uint2 remappedId;
    remappedId.x = BitfieldInsert(BitfieldExtract(groupIndex, 1u, 2u), groupIndex, 0u);
    remappedId.y = BitfieldInsert(BitfieldExtract(groupIndex, 2u, 2u), BitfieldExtract(groupIndex, 0u, 1u), 1u);

    uint2 dispatchGroupId = uint2(dispatchId) / 4;  // 4未満切り捨て.
    return (dispatchGroupId * 4) + remappedId;
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

//-----------------------------------------------------------------------------
//      モートンオーダーにリマップします.
//-----------------------------------------------------------------------------
uint2 RemapLane16x16(uint2 dispatchId, uint groupIndex)
{
    // 16x16 は 4x4 サブブロックごとにビットを抽出して組み合わせ
    uint2 remappedId;
    remappedId.x = BitfieldInsert(BitfieldExtract(groupIndex, 2u, 2u), BitfieldExtract(groupIndex, 0u, 2u), 0u);
    remappedId.y = BitfieldInsert(BitfieldExtract(groupIndex, 4u, 2u), BitfieldExtract(groupIndex, 2u, 2u), 2u);

    uint2 dispatchGroupId = uint2(dispatchId) / 16; // 16未満切り捨て.
    return (dispatchGroupId * 16) + remappedId;
}

#endif//ASDX_COMPUTE_UTIL_HLSLI
