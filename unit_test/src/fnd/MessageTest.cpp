//-----------------------------------------------------------------------------
// File : MessageTest.cpp
// Desc : Message Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMessage.h>


namespace {

struct TestData
{
    int32_t Value = 0;
    float Factor = 0.0f;
};

struct TestListener : public asdx::IMessageListener
{
    void OnMessage(const asdx::Message& msg) override
    {
        ++MessageCount;
        LastType = msg.GetType();
        LastSize = msg.GetSize();

        if (msg.GetSize() == sizeof(TestData))
        {
            LastData = *msg.GetAs<TestData>();
        }
    }

    int MessageCount = 0;
    uint32_t LastType = 0;
    uint64_t LastSize = 0;
    TestData LastData;
};

} // namespace

TEST(MessageTest, MessageAccessors)
{
    const TestData data = { 42, 1.5f };
    asdx::Message msg(7, &data, sizeof(data));

    EXPECT_EQ(msg.GetType(), 7u);
    EXPECT_EQ(msg.GetSize(), sizeof(data));
    EXPECT_EQ(msg.GetBuffer(), &data);
    EXPECT_EQ(msg.GetAs<TestData>(), &data);
}

TEST(MessageTest, TypedMessageStoresValue)
{
    const TestData data = { 123, 2.5f };
    asdx::TypedMessage<TestData> msg(11, data);

    EXPECT_EQ(msg.GetType(), 11u);
    EXPECT_EQ(msg.GetSize(), sizeof(TestData));
    ASSERT_NE(msg.GetBuffer(), nullptr);
    EXPECT_EQ(msg.GetAs<TestData>()->Value, 123);
    EXPECT_FLOAT_EQ(msg.GetAs<TestData>()->Factor, 2.5f);
}

TEST(MessageTest, BroadcastsCopiedMessage)
{
    auto& manager = asdx::MessageManager::Instance();
    ASSERT_TRUE(manager.Init(1024));

    TestListener listener;
    manager.AddListener(&listener);

    TestData data = { 5, 3.0f };
    asdx::Message msg(21, &data, sizeof(data));
    manager.EnqueueMessage(msg);

    data.Value = 99;
    manager.Broadcast();

    EXPECT_EQ(listener.MessageCount, 1);
    EXPECT_EQ(listener.LastType, 21u);
    EXPECT_EQ(listener.LastSize, sizeof(TestData));
    EXPECT_EQ(listener.LastData.Value, 5);
    EXPECT_FLOAT_EQ(listener.LastData.Factor, 3.0f);

    manager.Term();
}

TEST(MessageTest, BroadcastsMessageWithoutBuffer)
{
    auto& manager = asdx::MessageManager::Instance();
    ASSERT_TRUE(manager.Init(256));

    TestListener listener;
    manager.AddListener(&listener);
    manager.EnqueueMessage(asdx::Message(31));
    manager.Broadcast();

    EXPECT_EQ(listener.MessageCount, 1);
    EXPECT_EQ(listener.LastType, 31u);
    EXPECT_EQ(listener.LastSize, 0u);

    manager.Term();
}

TEST(MessageTest, ListenerOperations)
{
    auto& manager = asdx::MessageManager::Instance();
    ASSERT_TRUE(manager.Init(512));

    TestListener listener1;
    TestListener listener2;
    manager.AddListener(&listener1);
    manager.AddListener(&listener2);
    manager.RemoveListener(&listener1);

    manager.EnqueueMessage(asdx::Message(41));
    manager.Broadcast();

    EXPECT_EQ(listener1.MessageCount, 0);
    EXPECT_EQ(listener2.MessageCount, 1);

    manager.Clear();
    manager.EnqueueMessage(asdx::Message(42));
    manager.Broadcast();
    EXPECT_EQ(listener2.MessageCount, 1);

    manager.Term();
}

