//-----------------------------------------------------------------------------
// File : OctreeTest.cpp
// Desc : Octree Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#include <fnd/asdxOctree.h>

struct Object : public asdx::List<Object>::Node
{
    uint32_t      Id    = 0;
    asdx::Vector3 Min   = {};
    asdx::Vector3 Max   = {};

    Object(uint32_t id, asdx::Vector3 mini, asdx::Vector3 maxi)
    : Id (id)
    , Min(mini)
    , Max(maxi)
    { /* DO_NOTHING */ }

    asdx::Vector3 CalcSize() const
    { return asdx::Vector3::Abs(Max - Min); }

    asdx::Vector3 CalcCenter() const
    { return (Max + Min) * 0.5f; }
};

TEST(OctreeTest, Basic)
{
    Object object0(0, asdx::Vector3(-10.0f, 0.0f, -10.0f), asdx::Vector3(10.0f, 0.0f, 10.0f));
    Object object1(1, asdx::Vector3(-150.0f, -10.0f, -100.0f), asdx::Vector3(200.0f, 0.0f, 200.0f));
    Object object2(2, asdx::Vector3(-9.0f, -1.0f, -9.0f), asdx::Vector3(11.0f, 1.0f, 11.0f));

    uint8_t levels = 3;
    float cellSize = 100.0f;
    float rootSize = (cellSize * (1 << levels)) * 0.5f;

    asdx::Octree<Object> octree;

    auto mini = asdx::Vector3(-rootSize, -rootSize, -rootSize);
    auto maxi = asdx::Vector3(rootSize, rootSize, rootSize);

    octree.Init(levels, mini, maxi);
    EXPECT_EQ(octree.GetMaxLevels(), 3);
    EXPECT_FLOAT_EQ(octree.GetCellSize().x, cellSize);
    EXPECT_FLOAT_EQ(octree.GetCellSize().y, cellSize);
    EXPECT_FLOAT_EQ(octree.GetCellSize().z, cellSize);
    EXPECT_FLOAT_EQ(octree.GetRootMax().x, maxi.x);
    EXPECT_FLOAT_EQ(octree.GetRootMax().y, maxi.y);
    EXPECT_FLOAT_EQ(octree.GetRootMax().z, maxi.z);
    EXPECT_FLOAT_EQ(octree.GetRootMin().x, mini.x);
    EXPECT_FLOAT_EQ(octree.GetRootMin().y, mini.y);
    EXPECT_FLOAT_EQ(octree.GetRootMin().z, mini.z);

    uint8_t level = 0;
    level = octree.CalcLevel(object0.CalcSize());
    EXPECT_EQ(level, 3);

    level = octree.CalcLevel(object1.CalcSize());
    EXPECT_EQ(level, 2);

    level = octree.CalcLevel(object2.CalcSize());
    EXPECT_EQ(level, 3);

    uint32_t index = 0;
    index = octree.CalcHash(object0.Min, object0.Max);
    printf_s("hash : 0x%x\n", index);
    octree.Add(index, &object0);
    auto node = octree.Find(index);
    EXPECT_EQ(node->Objects.size(), 1);
    EXPECT_EQ(node->Objects.front().Id, object0.Id);

    index = octree.CalcHash(object1.Min, object1.Max);
    printf_s("hash : 0x%x\n", index);
    octree.Add(index, &object1);
    node = octree.Find(index);
    EXPECT_EQ(node->Objects.size(), 1);
    EXPECT_EQ(node->Objects.front().Id, object1.Id);

    index = octree.CalcHash(object2.Min, object2.Max);
    printf_s("hash : 0x%x\n", index);
    octree.Add(index, &object2);
    node = octree.Find(index);
    EXPECT_EQ(node->Objects.size(), 2);
    EXPECT_EQ(node->Objects.back().Id, 2);

    octree.Term();
    EXPECT_EQ(octree.GetNodeCount(), 0);
}