//-----------------------------------------------------------------------------
// File : asdxOctree.h
// Desc : Octree
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxBit.h>
#include <fnd/asdxList.h>
#include <fnd/asdxMath.h>
#include <unordered_map>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Octree class
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class Octree
{
public:
    struct Node
    {
        List<T>     Objects;
    };

    void Init(uint8_t maxLevels, const Vector3& rootMin, const Vector3& rootMax)
    {
        m_RootMin   = rootMin;
        m_MaxLevels = maxLevels;

        auto size = Vector3::Abs(rootMax - rootMin);
        m_CellSize = size / ((float)(1 << maxLevels));

        m_NodeCount = ((1 << ((maxLevels + 1) * 3)) - 1) / 7;
        m_Nodes = new Node[m_NodeCount];
    }

    void Term()
    {
        if (m_Nodes)
        {
            for(auto i=0u; i<m_NodeCount; ++i)
            { m_Nodes[i].Objects.clear(); }

            delete[] m_Nodes;
            m_Nodes = nullptr;
        }
        m_NodeCount = 0;
        m_CellSize  = {};
        m_RootMin   = {};
        m_MaxLevels = 0;
    }

    void Add(uint32_t index, T* object)
    {
        assert(index < m_NodeCount);
        m_Nodes[index].Objects.push_back(object);
    }

    void Remove(uint32_t index, T* object)
    {
        assert(index < m_NodeCount);
        m_Nodes[index].Objects.earse(object);
    }

    Node& Get(uint32_t index)
    {
        assert(index < m_NodeCount);
        return m_Nodes[index];
    }

    const Node& Get(uint32_t index) const
    {
        assert(index < m_NodeCount);
        return m_Nodes[index];
    }

    Node* GetPtr(uint32_t index)
    {
        assert(index < m_NodeCount);
        return &m_Nodes[index];
    }

    const Node* GetPtr(uint32_t index) const
    {
        assert(index < m_NodeCount);
        return &m_Nodes[index];
    }

    uint8_t CalcLevel(const Vector3& size)
    {
        auto cellSize    = Max3(m_CellSize);
        auto sizeMax     = Max3(size);
        auto levelOffset = 0u;

        if (cellSize <= sizeMax)
            levelOffset = Log2(uint32_t(sizeMax / cellSize));

        if (m_MaxLevels < levelOffset)
            return 0;   // ルート.

        return m_MaxLevels - levelOffset;
    }

    uint32_t CalcIndex(const Vector3& mini, const Vector3& maxi)
    {
        // 所属空間を求める.
        auto rhs   = GetPointCode(maxi);
        auto level = CalcLevel(maxi - mini);
        auto shift = m_MaxLevels - level;
        auto code  = rhs >> (shift * 3);

        // 線形配列の番号に直す.
        auto offset = ((1 << (level * 3)) - 1) / 7;
        return code + offset;
    }

    static uint32_t CalcParentCode(uint32_t childCode)
    { return childCode >> 3; }

    static uint32_t CalcChildCode(uint32_t parentCode, uint8_t childIndex)
    { return (parentCode << 3) | childIndex; }

    uint8_t GetMaxLevels() const
    { return m_MaxLevels; }

    Vector3 GetCellSize() const
    { return m_CellSize; }

    uint32_t GetNodeCount() const
    { return m_NodeCount; }

private:
    Node*       m_Nodes     = nullptr;  // ノード.
    uint32_t    m_NodeCount = 0;        // ノード数.
    uint8_t     m_MaxLevels = 0;        // レベル数.
    Vector3     m_RootMin   = {};       // ルートレベルの最小値.
    Vector3     m_CellSize  = {};       // 末端のセルサイズ.

    static float Max3(const Vector3& size)
    { return Max<float>(size.x, Max<float>(size.y, size.z)); }

    static uint32_t Log2(uint32_t value)
    { return 31 - CountZeroL(value); }

    uint32_t GetPointCode(const Vector3& p)
    {
        return MortonOrder3(
            uint32_t((p.x - m_RootMin.x) / m_CellSize.x),
            uint32_t((p.y - m_RootMin.y) / m_CellSize.y),
            uint32_t((p.z - m_RootMin.z) / m_CellSize.z)
        );
    }
};

} // namespace asdx
