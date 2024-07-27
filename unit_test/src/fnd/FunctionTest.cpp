//-----------------------------------------------------------------------------
// File : FunctionTest.cpp
// Desc : Function Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxFunction.h>
#include <thread>

struct FakePass
{
    asdx::Action<void> action;
};

TEST(FunctionTest, Basic)
{
    {
        asdx::Function<void()> f;

        f = []() {
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

    {
        FakePass pass = {};
        pass.action = []() {
            EXPECT_TRUE(true);
        };

        pass.action();
    }
}

TEST(FunctionTest, Thread)
{
    bool called = false;
    asdx::Function<void()> func = [&](){ called = true; };
    EXPECT_FALSE(called);
    std::thread thread(func);
    thread.join();
    EXPECT_TRUE(called);
}
