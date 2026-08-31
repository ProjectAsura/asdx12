//-----------------------------------------------------------------------------
// File : ListTest.cpp
// Desc : List Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxList.h>

namespace {

struct IntValue : asdx::List<IntValue>::Node
{
    IntValue(int value)
    : Value(value)
    {}

    int Value = 0;
};

static bool operator < (const IntValue& lhs, const IntValue& rhs)
{ return lhs.Value < rhs.Value; }

} // namespace

TEST(ListTest, Basic)
{
    IntValue node0(0);
    IntValue node1(1);
    IntValue node2(2);
    IntValue node3(3);

    asdx::List<IntValue> list;
    EXPECT_TRUE(list.empty());

    list.push_back(&node3);

    EXPECT_EQ(list.size(), 1);
    EXPECT_FALSE(list.empty());

    list.push_back(&node1);
    list.push_back(&node0);
    list.push_back(&node2);

    EXPECT_EQ(list.size(), 4);

    {
        int check[] = {3, 1, 0, 2};
        int idx = 0;
        for(auto& itr : list)
        {
            EXPECT_EQ(itr.Value, check[idx]);
            idx++;
        }
        EXPECT_EQ(idx, 4);
    }

    list.sort();

    {
        int check[] = {0, 1, 2, 3};
        int idx = 0;
        for(auto& itr : list)
        {
            EXPECT_EQ(itr.Value, check[idx]);
            idx++;
        }
        EXPECT_EQ(idx, 4);
        EXPECT_EQ(list.size(), 4);
    }

    list.clear();
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);

    list.push_back(&node0);
    list.insert(&node0, &node1);
    list.insert(&node1, &node2);

    {
        int check[] = { 2, 1, 0 };
        int idx = 0;
        for(auto& itr : list)
        {
            EXPECT_EQ(itr.Value, check[idx]);
            idx++;
        }
        EXPECT_EQ(idx, 3);
        EXPECT_EQ(list.size(), 3);
    }

    list.erase(&node1);

    {
        int check[] = { 2, 0 };
        int idx = 0;
        for(auto& itr : list)
        {
            EXPECT_EQ(itr.Value, check[idx]);
            idx++;
        }
        EXPECT_EQ(list.size(), 2);
    }

    list.erase(&node2);
    EXPECT_EQ(list.size(), 1);
    list.erase(&node0);
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.empty());

    list.push_front(&node3);
    list.push_front(&node2);
    list.push_front(&node1);
    list.push_front(&node0);
    EXPECT_EQ(list.size(), 4);

    {
        int check[] = { 0, 1, 2, 3 };
        int idx = 0;
        for(auto& itr : list)
        {
            EXPECT_EQ(itr.Value, check[idx]);
            idx++;
        }
    }

    {
        int check[] = { 3, 2, 1, 0 };
        int idx = 0;

        auto itr = list.rbegin();
        while(itr != list.rend())
        {
            EXPECT_EQ(itr->Value, check[idx]);
            idx++;
            itr++;
        }
    }

    list.pop_front();
    EXPECT_EQ(list.front().Value, 1);
    EXPECT_EQ(list.back().Value, 3);
    list.pop_back();
    EXPECT_EQ(list.back().Value, 2);

    list.clear();
}

TEST(ListTest, RangeOperations)
{
    IntValue sourceNode0(1);
    IntValue sourceNode1(2);
    IntValue sourceNode2(3);
    IntValue destinationNode0(0);
    IntValue destinationNode1(4);

    asdx::List<IntValue> source;
    source.push_back(&sourceNode0);
    source.push_back(&sourceNode1);
    source.push_back(&sourceNode2);

    asdx::List<IntValue> destination;
    destination.push_back(&destinationNode0);
    destination.push_back(&destinationNode1);

    auto insertPosition = destination.begin();
    ++insertPosition;
    destination.insert(insertPosition, source.begin(), source.end());

    EXPECT_TRUE(source.empty());
    EXPECT_EQ(destination.size(), 5);

    int expected[] = {0, 1, 2, 3, 4};
    int index = 0;
    for (auto& value : destination)
    {
        EXPECT_EQ(value.Value, expected[index]);
        index++;
    }

    auto first = destination.begin();
    ++first;
    auto last = destination.end();
    --last;
    auto next = destination.erase(first, last);
    EXPECT_EQ(next->Value, 4);
    EXPECT_EQ(destination.size(), 2);

    int remaining[] = {0, 4};
    index = 0;
    for (auto& value : destination)
    {
        EXPECT_EQ(value.Value, remaining[index]);
        index++;
    }
}

TEST(ListTest, CustomSortAndForeach)
{
    IntValue node0(1);
    IntValue node1(3);
    IntValue node2(2);

    asdx::List<IntValue> list;
    list.push_back(&node0);
    list.push_back(&node1);
    list.push_back(&node2);

    list.sort([](const IntValue& lhs, const IntValue& rhs)
    { return lhs.Value > rhs.Value; });

    int values[] = {3, 2, 1};
    int index = 0;
    list.foreach([&](IntValue& value)
    {
        EXPECT_EQ(value.Value, values[index]);
        value.Value += 10;
        index++;
    });

    int reverseValues[] = {11, 12, 13};
    index = 0;
    list.reverse_foreach([&](IntValue& value)
    {
        EXPECT_EQ(value.Value, reverseValues[index]);
        index++;
    });
}

TEST(ListTest, ConstIterators)
{
    IntValue node0(0);
    IntValue node1(1);
    IntValue node2(2);

    asdx::List<IntValue> list;
    list.push_back(&node0);
    list.push_back(&node1);
    list.push_back(&node2);

    const auto& constList = list;

    int index = 0;
    for (auto itr = constList.cbegin(); itr != constList.cend(); ++itr)
    {
        EXPECT_EQ(itr->Value, index);
        index++;
    }
    EXPECT_EQ(index, 3);

    index = 2;
    for (auto itr = constList.crbegin(); itr != constList.crend(); ++itr)
    {
        EXPECT_EQ(itr->Value, index);
        index--;
    }
    EXPECT_EQ(index, -1);
}

TEST(ListTest, NodeDestructionUnlinks)
{
    asdx::List<IntValue> list;

    {
        IntValue node(1);
        list.push_back(&node);
        EXPECT_EQ(list.size(), 1);
    }

    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
}
