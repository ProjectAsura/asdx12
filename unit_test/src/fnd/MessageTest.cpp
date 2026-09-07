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

TEST(MessageTest, SendsMessage)
{
    asdx::MessageHandler handler;

    TestListener listener;
    handler += &listener;

    const TestData data = { 5, 3.0f };
    const asdx::Message msg(21, &data, sizeof(data));
    handler.Send(msg);

    EXPECT_EQ(listener.MessageCount, 1);
    EXPECT_EQ(listener.LastType, 21u);
    EXPECT_EQ(listener.LastSize, sizeof(TestData));
    EXPECT_EQ(listener.LastData.Value, 5);
    EXPECT_FLOAT_EQ(listener.LastData.Factor, 3.0f);

}

TEST(MessageTest, SendsMessageWithoutBuffer)
{
    asdx::MessageHandler handler;

    TestListener listener;
    handler += &listener;
    handler.Send(asdx::Message(31));

    EXPECT_EQ(listener.MessageCount, 1);
    EXPECT_EQ(listener.LastType, 31u);
    EXPECT_EQ(listener.LastSize, 0u);

}

TEST(MessageTest, ListenerOperations)
{
    asdx::MessageHandler handler;

    TestListener listener1;
    TestListener listener2;
    handler += &listener1;
    handler += &listener2;
    handler -= &listener1;

    handler.Send(asdx::Message(41));

    EXPECT_EQ(listener1.MessageCount, 0);
    EXPECT_EQ(listener2.MessageCount, 1);

    handler -= &listener2;
    handler.Send(asdx::Message(42));
    EXPECT_EQ(listener2.MessageCount, 1);
}

