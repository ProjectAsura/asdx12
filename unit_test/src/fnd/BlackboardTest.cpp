//-----------------------------------------------------------------------------
// File : BlackboardTest.cpp
// Desc : Black Board Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxBlackboard.h>
#include <cstring>


namespace {

struct TestData
{
    int32_t Value  = 0;
    float   Factor = 0.0f;
};

} // namespace

TEST(BlackboardTest, DefaultConstructed)
{
    asdx::Blackboard blackboard;

    EXPECT_FALSE(blackboard.Contains("missing"));
    EXPECT_TRUE(blackboard.Get("missing").empty());
}

TEST(BlackboardTest, SetAndGetBuffer)
{
    asdx::Blackboard blackboard;
    const uint8_t source[] = { 1, 2, 3, 4 };
    uint8_t destination[sizeof(source)] = {};

    blackboard.Set("buffer", source, sizeof(source));

    ASSERT_TRUE(blackboard.Contains("buffer"));
    EXPECT_EQ(blackboard.Get("buffer").size(), sizeof(source));
    EXPECT_TRUE(blackboard.Get("buffer", destination, sizeof(destination)));
    EXPECT_EQ(memcmp(source, destination, sizeof(source)), 0);
}

TEST(BlackboardTest, SetVectorCopiesBuffer)
{
    asdx::Blackboard blackboard;
    std::vector<uint8_t> source = { 10, 20, 30 };

    blackboard.Set("vector", source);
    source[0] = 99;

    const auto& result = blackboard.Get("vector");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 10);
    EXPECT_EQ(result[1], 20);
    EXPECT_EQ(result[2], 30);
}

TEST(BlackboardTest, TypedAccess)
{
    asdx::Blackboard blackboard;
    const TestData value = { 42, 1.5f };

    blackboard.SetAs("data", value);

    EXPECT_EQ(blackboard.GetAs<TestData>("data").Value, 42);
    EXPECT_FLOAT_EQ(blackboard.GetAs<TestData>("data").Factor, 1.5f);
}

TEST(BlackboardTest, GetFailsForMissingOrDifferentSize)
{
    asdx::Blackboard blackboard;
    const uint32_t value = 123;
    uint32_t destination = 0;

    EXPECT_FALSE(blackboard.Get("missing", &destination, sizeof(destination)));

    blackboard.SetAs("value", value);

    EXPECT_FALSE(blackboard.Get("value", &destination, sizeof(destination) - 1));
    EXPECT_EQ(destination, 0u);
}

TEST(BlackboardTest, RemoveAndClear)
{
    asdx::Blackboard blackboard;

    blackboard.SetAs("first", 1);
    blackboard.SetAs("second", 2);
    blackboard.Remove("first");

    EXPECT_FALSE(blackboard.Contains("first"));
    EXPECT_TRUE(blackboard.Contains("second"));

    blackboard.Clear();

    EXPECT_FALSE(blackboard.Contains("second"));
    EXPECT_TRUE(blackboard.Get("second").empty());
}

TEST(ThreadSafeBlackboardTest, SupportsTypedAccessAndClear)
{
    asdx::ThreadSafeBlackboard blackboard;

    blackboard.SetAs("value", 1234);

    EXPECT_TRUE(blackboard.Contains("value"));
    EXPECT_EQ(blackboard.GetAs<int>("value"), 1234);

    blackboard.Clear();

    EXPECT_FALSE(blackboard.Contains("value"));
}
