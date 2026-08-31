//-----------------------------------------------------------------------------
// File : QueueTest.cpp
// Desc : Queue Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxQueue.h>

namespace {

struct IntNode : public asdx::Queue<IntNode>::Node
{
    IntNode(int value)
        : Value(value)
    {}

    int Value = 0;
};

} // namespace

TEST(QueueTest, Basic)
{
    IntNode node0(1);
    IntNode node1(2);
    IntNode node2(3);
    IntNode node3(4);

    asdx::Queue<IntNode> queue;

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);

    queue.push(&node0);
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);

    queue.push(&node1);
    EXPECT_EQ(queue.size(), 2);

    queue.push(&node2);
    EXPECT_EQ(queue.size(), 3);

    queue.push(&node3);
    EXPECT_EQ(queue.size(), 4);

    auto item = queue.pop();
    EXPECT_EQ(item->Value, 1);
    EXPECT_EQ(queue.size(), 3);

    item = queue.pop();
    EXPECT_EQ(item->Value, 2);
    EXPECT_EQ(queue.size(), 2);

    item = queue.pop();
    EXPECT_EQ(item->Value, 3);
    EXPECT_EQ(queue.size(), 1);

    item = queue.pop();
    EXPECT_EQ(item->Value, 4);
    EXPECT_EQ(queue.size(), 0);
    EXPECT_TRUE(queue.empty());

    item = queue.pop();
    EXPECT_EQ(item, nullptr);

    queue.push(&node1); // 2
    EXPECT_EQ(queue.size(), 1);

    queue.push(&node2); // 3
    EXPECT_EQ(queue.size(), 2);

    queue.pop(); // 2
    queue.push(&node3); // 4
    EXPECT_EQ(queue.size(), 2);

    item = queue.pop(); // 3
    EXPECT_EQ(item->Value, 3);

    queue.clear();
    EXPECT_EQ(queue.size(), 0);
    EXPECT_TRUE(queue.empty());
}

TEST(QueueTest, IgnoresNullNode)
{
    asdx::Queue<IntNode> queue;
    IntNode node(1);

    queue.push(nullptr);

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    EXPECT_EQ(queue.pop(), nullptr);

    queue.push(&node);
    EXPECT_EQ(queue.pop(), &node);
    EXPECT_TRUE(queue.empty());
}

TEST(QueueTest, ReusesPoppedNode)
{
    IntNode node0(1);
    IntNode node1(2);
    IntNode node2(3);

    asdx::Queue<IntNode> queue;
    queue.push(&node0);
    queue.push(&node1);
    queue.push(&node2);

    EXPECT_EQ(queue.pop(), &node0);
    queue.push(&node0);

    EXPECT_EQ(queue.size(), 3);
    EXPECT_EQ(queue.pop(), &node1);
    EXPECT_EQ(queue.pop(), &node2);
    EXPECT_EQ(queue.pop(), &node0);
    EXPECT_TRUE(queue.empty());
}

TEST(QueueTest, ReusesNodesAfterClear)
{
    IntNode node0(1);
    IntNode node1(2);
    IntNode node2(3);

    asdx::Queue<IntNode> queue;
    queue.push(&node0);
    queue.push(&node1);
    queue.push(&node2);
    queue.clear();

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);

    queue.push(&node2);
    queue.push(&node0);
    queue.push(&node1);

    EXPECT_EQ(queue.size(), 3);
    EXPECT_EQ(queue.pop(), &node2);
    EXPECT_EQ(queue.pop(), &node0);
    EXPECT_EQ(queue.pop(), &node1);
    EXPECT_EQ(queue.pop(), nullptr);
    EXPECT_TRUE(queue.empty());
}
