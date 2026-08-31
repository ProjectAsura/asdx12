//-----------------------------------------------------------------------------
// File : MiscTest.cpp
// Desc : Miscellaneous Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMisc.h>

namespace
{
    int CompareInt(int lhs, int rhs)
    { return (lhs > rhs) - (lhs < rhs); }

    bool LessInt(int lhs, int rhs)
    { return lhs < rhs; }

    bool GreaterInt(int lhs, int rhs)
    { return lhs > rhs; }
}


TEST(MiscTest, Cast)
{
    const int value = 42;
    EXPECT_EQ(asdx::scast<long>(value), 42L);
    EXPECT_EQ(asdx::rcast<const int*>(&value), &value);
}

#if _HAS_CXX20
TEST(MiscTest, Utf8StringPrimitives)
{
    const char8_t source[] = u8"asura";
    EXPECT_EQ(asdx::u8len(source), 5);
    EXPECT_EQ(asdx::u8len(nullptr), 0);

    char8_t copied[8] = {};
    asdx::u8cpy(copied, source);
    EXPECT_EQ(asdx::u8cmp(copied, source), 0);

    char8_t truncated[4] = {};
    asdx::u8cpy(truncated, source);
    EXPECT_EQ(asdx::u8cmp(truncated, u8"asu"), 0);

    EXPECT_LT(asdx::u8cmp(u8"abc", u8"abd"), 0);
    EXPECT_GT(asdx::u8cmp(u8"abd", u8"abc"), 0);
    EXPECT_EQ(asdx::u8cmp(u8"same", u8"same"), 0);
}
#endif

TEST(MiscTest, StringConversion)
{
    EXPECT_EQ(asdx::ToStringW("asura"), L"asura");
    EXPECT_EQ(asdx::ToStringA(L"asura"), "asura");
}

TEST(MiscTest, Split)
{
    const auto values = asdx::Split("one,two,three", ',');
    ASSERT_EQ(values.size(), 3);
    EXPECT_EQ(values[0], "one");
    EXPECT_EQ(values[1], "two");
    EXPECT_EQ(values[2], "three");

    const auto empty = asdx::Split("", ',');
    EXPECT_TRUE(empty.empty());

    const auto wide = asdx::Split(L"one::two::", L':');
    ASSERT_EQ(wide.size(), 4);
    EXPECT_EQ(wide[0], L"one");
    EXPECT_EQ(wide[1], L"");
    EXPECT_EQ(wide[2], L"two");
    EXPECT_EQ(wide[3], L"");
}

TEST(MiscTest, PathAndReplacement)
{
    EXPECT_EQ(asdx::ToSlash("a\\b\\c"), "a/b/c");
    EXPECT_EQ(asdx::ToSlash("abc"), "abc");
    EXPECT_EQ(asdx::Replace("one two two", "two", "3"), "one 3 3");
    EXPECT_EQ(asdx::Replace("abc", "", "x"), "abc");
}

TEST(MiscTest, ToLower)
{
    EXPECT_EQ(asdx::ToLowerA("AbC 123"), "abc 123");
    EXPECT_EQ(asdx::ToLowerW(L"XyZ 123"), L"xyz 123");
}

TEST(MiscTest, GetEnv)
{
    ASSERT_EQ(_putenv_s("ASDX_MISC_TEST", "value"), 0);
    EXPECT_EQ(asdx::GetEnv("ASDX_MISC_TEST"), "value");
    EXPECT_EQ(asdx::GetEnv("ASDX_MISC_TEST_NOT_DEFINED"), "");
    ASSERT_EQ(_putenv_s("ASDX_MISC_TEST", ""), 0);
}

TEST(MiscTest, BinarySearch)
{
    const int values[] = { 1, 3, 5, 7, 9 };

    EXPECT_EQ(asdx::BinarySearch(values, 5, 1, CompareInt), 0);
    EXPECT_EQ(asdx::BinarySearch(values, 5, 7, CompareInt), 3);
    EXPECT_EQ(asdx::BinarySearch(values, 5, 9, CompareInt), 4);
    EXPECT_EQ(asdx::BinarySearch(values, 5, 4, CompareInt), SIZE_MAX);
    EXPECT_EQ(asdx::BinarySearch(values, 0, 1, CompareInt), SIZE_MAX);
    EXPECT_EQ(asdx::BinarySearch(values, 5, 5), 2);

    EXPECT_EQ(asdx::LowerBound(values, 5, 4, LessInt), 2);
    EXPECT_EQ(asdx::LowerBound(values, 5, 5, LessInt), 2);
    EXPECT_EQ(asdx::UpperBound(values, 5, 5, GreaterInt), 3);
    EXPECT_EQ(asdx::UpperBound(values, 5, 9, GreaterInt), 5);
}

#if _HAS_CXX20
TEST(MiscTest, Utf8StringOperations)
{
    const std::u8string input = u8"A\\B";
    EXPECT_EQ(asdx::ToSlash(input), u8"A/B");
    EXPECT_EQ(asdx::Replace(u8"a-b-b", u8"b", u8"x"), u8"a-x-x");
    EXPECT_EQ(asdx::ToLowerUTF8(u8"AbC"), u8"abc");

    const auto values = asdx::Split(u8"one,two", u8',');
    ASSERT_EQ(values.size(), 2);
    EXPECT_EQ(values[0], u8"one");
    EXPECT_EQ(values[1], u8"two");
}
#endif
