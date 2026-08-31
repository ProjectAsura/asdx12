//-----------------------------------------------------------------------------
// File : StackTest.cpp
// Desc : Stack Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxStack.h>

namespace { 

struct IntNode : public asdx::Stack<IntNode>::Node
{
    IntNode(int value)
        : Value(value)
    {}

    int Value = 0;
};

} // namespace

TEST(StackTest, Basic)
{
    IntNode node0(1);
    IntNode node1(2);
    IntNode node2(3);
    IntNode node3(4);

    asdx::Stack<IntNode> stack;

    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);

    stack.push(&node0);
    EXPECT_FALSE(stack.empty());
    EXPECT_EQ(stack.size(), 1);

    stack.push(&node1);
    EXPECT_EQ(stack.size(), 2);

    stack.push(&node2);
    EXPECT_EQ(stack.size(), 3);

    stack.push(&node3);
    EXPECT_EQ(stack.size(), 4);

    auto item = stack.pop();
    EXPECT_EQ(item->Value, 4);
    EXPECT_EQ(stack.size(), 3);

    item = stack.pop();
    EXPECT_EQ(item->Value, 3);
    EXPECT_EQ(stack.size(), 2);

    item = stack.pop();
    EXPECT_EQ(item->Value, 2);
    EXPECT_EQ(stack.size(), 1);

    item = stack.pop();
    EXPECT_EQ(item->Value, 1);
    EXPECT_EQ(stack.size(), 0);
    EXPECT_TRUE(stack.empty());

    item = stack.pop();
    EXPECT_EQ(item, nullptr);

    stack.push(&node1);
    EXPECT_EQ(stack.size(), 1);

    stack.push(&node2);
    EXPECT_EQ(stack.size(), 2);

    stack.pop();
    stack.push(&node3);
    EXPECT_EQ(stack.size(), 2);

    item = stack.pop();
    EXPECT_EQ(item->Value, 4);

    stack.clear();
    EXPECT_EQ(stack.size(), 0);
    EXPECT_TRUE(stack.empty());
}

TEST(StackTest, IgnoresNullNode)
{
    asdx::Stack<IntNode> stack;
    IntNode node(1);

    stack.push(nullptr);

    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);
    EXPECT_EQ(stack.pop(), nullptr);

    stack.push(&node);
    EXPECT_EQ(stack.pop(), &node);
    EXPECT_TRUE(stack.empty());
}

TEST(StackTest, ReusesPoppedNode)
{
    IntNode node0(1);
    IntNode node1(2);
    IntNode node2(3);

    asdx::Stack<IntNode> stack;
    stack.push(&node0);
    stack.push(&node1);
    stack.push(&node2);

    EXPECT_EQ(stack.pop(), &node2);
    stack.push(&node2);

    EXPECT_EQ(stack.size(), 3);
    EXPECT_EQ(stack.pop(), &node2);
    EXPECT_EQ(stack.pop(), &node1);
    EXPECT_EQ(stack.pop(), &node0);
    EXPECT_TRUE(stack.empty());
}

TEST(StackTest, ReusesNodesAfterClear)
{
    IntNode node0(1);
    IntNode node1(2);
    IntNode node2(3);

    asdx::Stack<IntNode> stack;
    stack.push(&node0);
    stack.push(&node1);
    stack.push(&node2);
    stack.clear();

    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0);

    stack.push(&node0);
    stack.push(&node2);
    stack.push(&node1);

    EXPECT_EQ(stack.size(), 3);
    EXPECT_EQ(stack.pop(), &node1);
    EXPECT_EQ(stack.pop(), &node2);
    EXPECT_EQ(stack.pop(), &node0);
    EXPECT_EQ(stack.pop(), nullptr);
    EXPECT_TRUE(stack.empty());
}
