//-----------------------------------------------------------------------------
// File : ArrayViewTest.cpp
// Desc : ArrayView Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxArrayView.h>


TEST(ArrayViewTest, DefaultConstructed)
{
    asdx::ArrayView<int> view;

    EXPECT_TRUE(view.empty());
    EXPECT_EQ(view.size(), 0);
    EXPECT_EQ(view.data(), nullptr);
}

TEST(ArrayViewTest, ConstructedFromArray)
{
    const int values[] = { 1, 2, 3, 4 };
    const asdx::ArrayView<int> view(values, 4);

    EXPECT_FALSE(view.empty());
    EXPECT_EQ(view.size(), 4);
    EXPECT_EQ(view.data(), values);
    EXPECT_EQ(view.front(), 1);
    EXPECT_EQ(view.back(), 4);
}

TEST(ArrayViewTest, ElementAccess)
{
    const int values[] = { 10, 20, 30, 40 };
    const asdx::ArrayView<int> view(values, 4);

    EXPECT_EQ(view.at(0), 10);
    EXPECT_EQ(view.at(1), 20);
    EXPECT_EQ(view.at(2), 30);
    EXPECT_EQ(view.at(3), 40);

    for(size_t i = 0; i < view.size(); ++i)
    {
        EXPECT_EQ(view[i], values[i]);
    }
}

TEST(ArrayViewTest, EmptyRange)
{
    const int value = 42;
    const asdx::ArrayView<int> view(&value, 0);

    EXPECT_TRUE(view.empty());
    EXPECT_EQ(view.size(), 0);
    EXPECT_EQ(view.data(), &value);
}

