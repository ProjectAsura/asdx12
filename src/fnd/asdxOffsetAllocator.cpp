//-----------------------------------------------------------------------------
// File : asdxOffsetAllocator.h
// Desc : Offset Allocator.
//-----------------------------------------------------------------------------
// Original Code Written by Sebastian Aaltonen.
// See. https://github.com/sebbbi/OffsetAllocator/blob/main/LICENSE


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <fnd/asdxOffsetAllocator.h>

#if _HAS_CXX20
  #include <bit>      // for std::countl_zero, std::countr_zero
#elif defined(_MSC_VER)
  #include <intrin.h> // for _BitScanReverse, _BitScanForward
#endif//_HAS_CXX20


namespace {

//------------------------------------------------------------------------------
// Constant Values.
//------------------------------------------------------------------------------
static constexpr uint32_t TOP_BINS_INDEX_SHIFT  = 3;
static constexpr uint32_t LEAF_BINS_INDEX_MASK  = 0x7;
static constexpr uint32_t MANTISSA_BITS         = 3;
static constexpr uint32_t MANTISSA_VALUE        = 1 << MANTISSA_BITS;
static constexpr uint32_t MANTISSA_MASK         = MANTISSA_VALUE - 1;


//-----------------------------------------------------------------------------
//      左から連続した0のビットを数えます.
//-----------------------------------------------------------------------------
inline uint32_t CountZeroL(uint32_t v)
{
#if _HAS_CXX20
    return std::countl_zero<uint32_t>(v);
#elif defined(_MSC_VER)
    unsigned long retVal;
    _BitScanReverse(&retVal, v);
    return 31 - retVal;
#else
    return __builtin_clz(v);
#endif
}

//-----------------------------------------------------------------------------
//      右から連続した0のビットを数えます.
//-----------------------------------------------------------------------------
inline uint32_t CountZeroR(uint32_t v)
{
#if _HAS_CXX20
    return std::countr_zero<uint32_t>(v);
#elif defined(_MSC_VER)
    unsigned long retVal;
    _BitScanForward(&retVal, v);
    return retVal;
#else
    return __builtin_ctz(v);
#endif
}

//-----------------------------------------------------------------------------
//      丸め上げ処理.
//-----------------------------------------------------------------------------
uint32_t uintToFloatRoundUp(uint32_t size)
{
    // ビンサイズは浮動小数点 (指数 + 仮数) 分布 (区分的線形対数近似) に従います。
    // これにより、各クラスサイズの平均オーバーヘッドパーセンテージが同じになることが保証されます。
    auto exp      = 0u;
    auto mantissa = 0u;

    if (size < MANTISSA_VALUE)
    {
        mantissa = size;
    }
    else
    {
        auto leadingZeros  = CountZeroL(size);
        auto highestSetBit = 31u - leadingZeros;

        auto mantissaStartBit = highestSetBit - MANTISSA_BITS;
        exp = mantissaStartBit + 1;
        mantissa = (size >> mantissaStartBit) & MANTISSA_MASK;

        auto lowBitsMask = (1u << mantissaStartBit) - 1u;

        if ((size & lowBitsMask) != 0)
            mantissa++;
    }

    return (exp << MANTISSA_BITS) + mantissa;
}

//-----------------------------------------------------------------------------
//      丸め下げ処理.
//-----------------------------------------------------------------------------
uint32_t uintToFloatRoundDown(uint32_t size)
{
    auto exp      = 0u;
    auto mantissa = 0u;

    if (size < MANTISSA_VALUE)
    {
        mantissa = size;
    }
    else
    {
        auto leadingZeros  = CountZeroL(size);
        auto highestSetBit = 31u - leadingZeros;

        auto mantissaStartBit = highestSetBit - MANTISSA_BITS;
        exp = mantissaStartBit + 1;
        mantissa = (size >> mantissaStartBit) & MANTISSA_MASK;
    }

    return (exp << MANTISSA_BITS) | mantissa;
}

//-----------------------------------------------------------------------------
//      設定の為の最下位ビットを求めます.
//-----------------------------------------------------------------------------
uint32_t findLowestSetBitAfter(uint32_t bitMask, uint32_t startBitIndex)
{
    uint32_t beforeIndex = (1u << startBitIndex) - 1u;
    uint32_t afterIndex  = ~beforeIndex;
    uint32_t bitsAfter   = bitMask & afterIndex;
    if (bitsAfter == 0)
        return asdx::OffsetHolder::INVALID_ID;

    return CountZeroR(bitsAfter);
}

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// OffsetAllocator class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
OffsetAllocator::~OffsetAllocator()
{
    // 解放漏れチェック.
    assert(m_Nodes     == nullptr);
    assert(m_FreeNodes == nullptr);
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool OffsetAllocator::Init(uint32_t size, uint32_t maxAllocCount)
{
    if (size == 0 || maxAllocCount == 0)
        return false;

    // 初期化済みなら解放処理をする.
    if (m_Init)
        Term();

    m_MaxSize       = size;
    m_MaxCount      = maxAllocCount;
    m_FreeSize      = 0;
    m_UsedBinsTop   = 0;
    m_FreeOffset    = m_MaxCount - 1u;

    for (uint32_t i = 0 ; i < TOP_BINS_COUNT; i++)
        m_UsedBins[i] = 0;

    for (uint32_t i = 0 ; i < LEAF_BINS_COUNT; i++)
        m_BinIndices[i] = Node::UNUSED;

    // メモリ確保.
    m_Nodes     = new Node    [m_MaxCount];
    m_FreeNodes = new uint32_t[m_MaxCount];

    // フリーリストはスタックなので，[0] が最初にポップするようにノードを逆順に配置します.
    for (uint32_t i = 0; i < m_MaxCount; i++)
        m_FreeNodes[i] = m_MaxCount - i - 1;

    // 開始状態: ストレージ全体を1つの大きなノードとしてアルゴリズムが残りを分割し, 小さなノードとして戻します.
    InsertNode(m_MaxSize, 0);

    // 初期化済みフラグを立てる.
    m_Init = true;

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void OffsetAllocator::Term()
{
    if (!m_Init)
    { return; }

    if (m_Nodes)
    {
        delete[] m_Nodes;
        m_Nodes = nullptr;
    }

    if (m_FreeNodes)
    {
        delete[] m_FreeNodes;
        m_FreeNodes = nullptr;
    }

    m_MaxSize       = 0;
    m_MaxCount      = 0;
    m_FreeSize      = 0;
    m_UsedBinsTop   = 0;
    m_FreeOffset    = 0;
    m_Init          = false;
}

//-----------------------------------------------------------------------------
//      確保処理です.
//-----------------------------------------------------------------------------
OffsetHolder OffsetAllocator::Alloc(uint32_t size)
{
    // 割り当て不可能なら無効なホルダーを返す.
    if (m_FreeOffset == 0)
        return OffsetHolder();

    // alloc >= bin になるようにビン番号に切り上げます。
    // サイズに適合する最小ビン番号を与えます.
    auto minBinId = uintToFloatRoundUp(size);

    uint32_t minTopBinId  = minBinId >> TOP_BINS_INDEX_SHIFT;
    uint32_t minLeafBinId = minBinId & LEAF_BINS_INDEX_MASK;

    auto topBinId  = minTopBinId;
    auto leafBinId = OffsetHolder::INVALID_ID;

    // トップビンが存在する場合は、そのリーフビンをスキャンします. これは失敗する可能性があります
    if (m_UsedBinsTop & (1 << topBinId))
        leafBinId = findLowestSetBitAfter(m_UsedBins[topBinId], minLeafBinId);
    
    // トップビンにスペースが見つからなかった場合は、+1 からトップビンを検索します.
    if (leafBinId == OffsetHolder::INVALID_ID)
    {
        topBinId = findLowestSetBitAfter(m_UsedBinsTop, minTopBinId + 1);

        // 割り当て不可能なら無効なホルダーを返す.
        if (topBinId == OffsetHolder::INVALID_ID)
           return OffsetHolder();

        // 上部のビンが切り上げられているため、ここではすべてのリーフビンが alloc に適合します. ビット 0 からリーフ検索を開始します.
        // 注: 最上位ビットが設定されているため、少なくとも1つのリーフビットが設定されているため、この検索は失敗しません.
        leafBinId = CountZeroR(m_UsedBins[topBinId]);
    }

    uint32_t binId = (topBinId << TOP_BINS_INDEX_SHIFT) | leafBinId;

    // ビンのトップノードをポップします. Bin to = node.next.
    auto  nodeId = m_BinIndices[binId];
    auto& node   = m_Nodes[nodeId];

    auto  nodeTotalSize = node.DataSize;

    node.DataSize = size;
    node.Used     = true;

    m_BinIndices[binId] = node.BinListNext;
    if (node.BinListNext != Node::UNUSED)
        m_Nodes[node.BinListNext].BinListPrev = Node::UNUSED;

    m_FreeSize -= nodeTotalSize;

    // ビンが空か？
    if (m_BinIndices[binId] == Node::UNUSED)
    {
        // リーフビンマスクビットを削除.
        m_UsedBins[topBinId] &= ~(1 << leafBinId);

        // 全てのリーフビンが空かどうか?
        if (m_UsedBins[topBinId] == 0)
            m_UsedBinsTop &= ~(1 << topBinId);
    }

    // リマインダー N 個の要素を下のビンにプッシュバックします.
    auto reminderSize = nodeTotalSize - size;
    if (reminderSize > 0)
    {
        auto newuint32_t = InsertNode(reminderSize, node.DataOffset + size);

        // ノードを隣り合わせてリンクし、両方が空いている場合に後でマージできるようにします.
        // そして、古い次の隣接ノードを更新して、新しいノード (中央) を指すようにします.
        if (node.NeighborNext != Node::UNUSED)
            m_Nodes[node.NeighborNext].NeighborPrev = newuint32_t;

        m_Nodes[newuint32_t].NeighborPrev = nodeId;
        m_Nodes[newuint32_t].NeighborNext = node.NeighborNext;
        node.NeighborNext = newuint32_t;
    }

    return OffsetHolder(node.DataOffset, size, nodeId);
}

//-----------------------------------------------------------------------------
//      解放処理です.
//-----------------------------------------------------------------------------
void OffsetAllocator::Free(OffsetHolder& holder)
{
    assert(holder.m_NodeId != OffsetHolder::INVALID_ID);
    if (!m_Nodes)
        return;

    auto  nodeId = holder.m_NodeId;
    auto& node   = m_Nodes[nodeId];

    assert(node.Used == true);

    auto offset = node.DataOffset;
    auto size   = node.DataSize;

    if (node.NeighborPrev != Node::UNUSED && !m_Nodes[node.NeighborPrev].Used)
    {
        // 前の (連続した) フリーノード: オフセットを前のノードのオフセットに変更します.
        auto& prevNode = m_Nodes[node.NeighborPrev];
        offset = prevNode.DataOffset;
        size  += prevNode.DataSize;

        // ビンリンクリストからノードを削除し、フリーリストに追加します.
        RemoveNode(node.NeighborPrev);

        assert(prevNode.NeighborNext == nodeId);
        node.NeighborPrev = prevNode.NeighborPrev;
    }
        
    if (node.NeighborNext != Node::UNUSED && !m_Nodes[node.NeighborNext].Used)
    {
        // 次の (連続する) 空きノード: オフセットは同じままです.
        auto& nextNode = m_Nodes[node.NeighborNext];
        size += nextNode.DataSize;

        // ビンリンクリストからノードを削除し、フリーリストに追加します.
        RemoveNode(node.NeighborNext);

        assert(nextNode.NeighborPrev == nodeId);
        node.NeighborNext = nextNode.NeighborNext;
    }

    auto neighborNext = node.NeighborNext;
    auto neighborPrev = node.NeighborPrev;

    // フリーリストに削除されたノードを挿入.
    m_FreeNodes[++m_FreeOffset] = nodeId;

    // ビンに(結合された)フリーノードを挿入.
    auto combinedId = InsertNode(size, offset);

    // 新しく結合されたで隣接を接続.
    if (neighborNext != Node::UNUSED)
    {
        m_Nodes[combinedId]  .NeighborNext = neighborNext;
        m_Nodes[neighborNext].NeighborPrev = combinedId;
    }
    if (neighborPrev != Node::UNUSED)
    {
        m_Nodes[combinedId]  .NeighborPrev = neighborPrev;
        m_Nodes[neighborPrev].NeighborNext = combinedId;
    }
}

//-----------------------------------------------------------------------------
//      ビンにノードを挿入します.
//-----------------------------------------------------------------------------
uint32_t OffsetAllocator::InsertNode(uint32_t size, uint32_t dataOffset)
{
    // bin >= alloc になるように, ビン番号をに丸め下げします.
    auto binId = uintToFloatRoundDown(size);

    uint32_t topBinId  = binId >> TOP_BINS_INDEX_SHIFT;
    uint32_t leafBinId = binId & LEAF_BINS_INDEX_MASK;

    // 以前はビンが空だったか？
    if (m_BinIndices[binId] == Node::UNUSED)
    {
        // ビンマスクビットを設定.
        m_UsedBins[topBinId] |= 1 << leafBinId;
        m_UsedBinsTop        |= 1 << topBinId;
    }

    // フリーリストノードを取得し、ビンのリンクリストの先頭に挿入します (next = old top).
    auto topNodeId  = m_BinIndices[binId];
    auto nodeId     = m_FreeNodes[m_FreeOffset--];
    m_Nodes[nodeId] = Node(dataOffset, size, topNodeId);

    if (topNodeId != Node::UNUSED)
        m_Nodes[topNodeId].BinListPrev = nodeId;

    m_BinIndices[binId] = nodeId;
    m_FreeSize += size;

    return nodeId;
}

//-----------------------------------------------------------------------------
//      ビンからノードを削除します.
//-----------------------------------------------------------------------------
void OffsetAllocator::RemoveNode(uint32_t nodeId)
{
    auto &node = m_Nodes[nodeId];

    if (node.BinListPrev != Node::UNUSED)
    {
        m_Nodes[node.BinListPrev].BinListNext = node.BinListNext;
        if (node.BinListNext != Node::UNUSED)
            m_Nodes[node.BinListNext].BinListPrev = node.BinListPrev;
    }
    else
    {
        // bin >= alloc になるように、ビン番号をに丸め下げします.
        auto binId = uintToFloatRoundDown(node.DataSize);

        uint32_t topBinId  = binId >> TOP_BINS_INDEX_SHIFT;
        uint32_t leafBinId = binId & LEAF_BINS_INDEX_MASK;

        m_BinIndices[binId] = node.BinListNext;
        if (node.BinListNext != Node::UNUSED)
            m_Nodes[node.BinListNext].BinListPrev = Node::UNUSED;

        // ビンが空か?
        if (m_BinIndices[binId] == Node::UNUSED)
        {
            // リーフビンマスクビットを削除.
            m_UsedBins[topBinId] &= ~(1 << leafBinId);

            // 全てのリーフビンが空か?
            if (m_UsedBins[topBinId] == 0)
                m_UsedBinsTop &= ~(1 << topBinId);
        }
    }

    // フリーリストにノードを挿入.
    m_FreeNodes[++m_FreeOffset] = nodeId;
    m_FreeSize -= node.DataSize;
}

} // namespace asdx
