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
        uint32_t    MortonCode;
        List<T>     Objects;
    };

    void Setup(uint8_t maxLevels, const Vector3& mini, const Vector3& maxi)
    {
        auto size   = Vector3::Abs(maxi - mini);
        m_CellSize  = Max3(size);
        m_MaxLevels = maxLevels;
    }

    void Add(uint32_t code, T* object)
    {
        auto node = Find(code);
        if (node == nullptr)
        {
            Node newNode = {};
            newNode.MortonCode = code;
            newNode.Objects.push_back(object);

            m_Nodes[code] = newNode;
        }
        else
        {
            node->Objects.push_back(object);
        }
    }

    void Remove(uint32_t code, T* object)
    {
        auto node = Find(code);
        if (node == nullptr)
            return;

        node->Objects.erase(object);
    }

    Node* Find(uint32_t code)
    {
        const auto itr = m_Nodes.find(code);
        return (itr == m_Nodes.end()) ? nullptr : &(*itr);
    }

    template<typename Action>
    void ForEach(Node* node, Action action)
    {
        if (node == nullptr)
            return;

        action(node);

        for(uint8_t i=0; i<8; ++i)
        {
            const auto childCode = CalcChildCode(node->MortonCode, i);
            auto child = Find(childCode);
            ForEach(child);
        }
    }

    uint8_t CalcLevel(const Vector3& size)
    {
        auto sizeMax     = Max3(size);
        auto levelOffset = 0u;

        if (m_MaxSize <= sizeMax)
            levelOffset = 31 - CountZeroL(uint32_t(sizeMax / m_MaxSize));

        if (m_MaxLevels < levelOffset)
            return 0;   // ルート.

        return m_MaxLevels - levelOffset;
    }

    uint32_t CalcNode(const Vector3& pos, uint8_t level)
    {
        uint32_t currSize = m_CellSize << (m_MaxLevels - level);
        uint32_t rootSize = m_CellSize << m_MaxLevels;
        uint32_t maxCount = 1u << level;

        uint32_t levelPosX = uint32_t(pos.x + rootSize / 2) / currSize;
        uint32_t levelPosY = uint32_t(pos.y + rootSize / 2) / currSize;
        uint32_t levelPosZ = uint32_t(pos.z + rootSize / 2) / currSize;

        if ((maxCount <= levelPosX) || (maxCount <= levelPosY) || (maxCount <= levelPosZ))
            return ~0; // 範囲外.

        return MortonOrder3(levelPosX, levelPosY, levelPosZ);
    }

    uint32_t CalcMortonCode(const Vector3& mini, const Vector3& maxi)
    {
        auto size  = Vector3::Abs(maxi - mini);
        auto pos   = (maxi + mini) * 0.5f;
        auto level = CalcLevel(size);
        auto code  = CalcNode(pos, level);
        return code;
    }

    static uint32_t CalcParentCode(uint32_t childCode)
    { return childCode >> 3; }

    static uint32_t CalcChildCode(uint32_t parentCode, uint8_t childIndex)
    { return (parentCode << 3) | childIndex; }

private:
    std::unordered_map<uint32_t, Node>  m_Nodes;        // ハッシュテーブル.
    uint8_t                             m_MaxLevels;    // レベル数.
    float                               m_CellSize;     // 末端セルのサイズの最大辺長.

    static float Max3(const Vector3& size)
    { return Max<float>(size.x, Max<float>(size.y, size.z)); }
};


} // namespace asdx
