//-----------------------------------------------------------------------------
// File : TokenizerTest.cpp
// Desc : Tokenizer Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxTokenizer.h>


TEST(TokenizerTest, InitializeAndTerminate)
{
    asdx::Tokenizer tokenizer;

    EXPECT_TRUE(tokenizer.Init(64));
    EXPECT_FALSE(tokenizer.IsValidToken());
    EXPECT_STREQ(tokenizer.GetAsChar(), "");

    tokenizer.Term();
    EXPECT_EQ(tokenizer.GetBuffer(), nullptr);
    EXPECT_EQ(tokenizer.GetPtr(), nullptr);
}

TEST(TokenizerTest, TokenizeWithSeparators)
{
    char buffer[] = "  alpha, beta;gamma";
    asdx::Tokenizer tokenizer;
    ASSERT_TRUE(tokenizer.Init(64));
    tokenizer.SetSeparator(" ,;");
    tokenizer.SetBuffer(buffer, sizeof(buffer));

    ASSERT_TRUE(tokenizer.IsValidToken());
    EXPECT_STREQ(tokenizer.GetAsChar(), "alpha");
    EXPECT_TRUE(tokenizer.Compare("alpha"));
    EXPECT_FALSE(tokenizer.Compare("ALPHA"));
    EXPECT_TRUE(tokenizer.CompareAsLower("ALPHA"));
    EXPECT_NE(tokenizer.Contain("ph"), nullptr);

    tokenizer.Next();
    EXPECT_STREQ(tokenizer.GetAsChar(), "beta");
    tokenizer.Next();
    EXPECT_STREQ(tokenizer.GetAsChar(), "gamma");
    tokenizer.Next();
    EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(TokenizerTest, TokenizeWithCutOffCharacters)
{
    char buffer[] = "name(value)[text]";
    asdx::Tokenizer tokenizer;
    ASSERT_TRUE(tokenizer.Init(64));
    tokenizer.SetSeparator("");
    tokenizer.SetCutOff("()[]");
    tokenizer.SetBuffer(buffer, sizeof(buffer));

    const char* expected[] = { "name", "(", "value", ")", "[", "text", "]" };
    for (const auto* value : expected)
    {
        ASSERT_TRUE(tokenizer.IsValidToken());
        EXPECT_STREQ(tokenizer.GetAsChar(), value);
        tokenizer.Next();
    }
    EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(TokenizerTest, ConvertTokenValues)
{
    char buffer[] = "-42 3.5 2.25 TRUE false 0x2A";
    asdx::Tokenizer tokenizer;
    ASSERT_TRUE(tokenizer.Init(64));
    tokenizer.SetSeparator(" ");
    tokenizer.SetBuffer(buffer, sizeof(buffer));

    EXPECT_EQ(tokenizer.GetAsInt(), -42);
    EXPECT_FLOAT_EQ(tokenizer.NextAsFloat(), 3.5f);
    EXPECT_DOUBLE_EQ(tokenizer.NextAsDouble(), 2.25);
    EXPECT_TRUE(tokenizer.NextAsBool());
    EXPECT_FALSE(tokenizer.NextAsBool());
    EXPECT_EQ(tokenizer.NextAsUint(), 42u);
}

TEST(TokenizerTest, SkipToAndSkipLine)
{
    char buffer[] = "one two target three\n  remaining";
    asdx::Tokenizer tokenizer;
    ASSERT_TRUE(tokenizer.Init(64));
    tokenizer.SetSeparator(" \t\r\n");
    tokenizer.SetBuffer(buffer, sizeof(buffer));

    tokenizer.SkipTo("target");
    EXPECT_STREQ(tokenizer.GetAsChar(), "three");

    tokenizer.SkipLine();
    EXPECT_STREQ(tokenizer.GetAsChar(), "");
    tokenizer.Next();
    EXPECT_STREQ(tokenizer.GetAsChar(), "remaining");
}
