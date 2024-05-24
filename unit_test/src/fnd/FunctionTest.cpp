//-----------------------------------------------------------------------------
// File : FunctionTest.cpp
// Desc : Function Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxFunction.h>
#include <cstdio>


TEST(FunctionTest, Basic)
{
    {
        asdx::Function<void()> f;

        f = []() {
            printf_s("Called\n");
            EXPECT_TRUE(true);
        };
        f();
    }

    {
        struct Data
        {
            int a = 1;
            int b = 2;
        };

        asdx::Function<int(int)> f;

        Data data;
        Data* ptr = &data;

        f = [ptr](int val) {
            return ptr->a + ptr->b + val;
        };

        EXPECT_EQ(f(1), 4);
    }
}
