//-----------------------------------------------------------------------------
// File : ListTest.cpp
// Desc : List Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxList.h>

struct IntValue : asdx::List<IntValue>::Node
{
    IntValue(int value)
    : Value(value)
    {}

    int Value = 0;
};

static bool operator < (const IntValue& lhs, const IntValue& rhs)
{ return lhs.Value < rhs.Value; }

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