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


TEST(FunctionTest, Basic)
{
    {
        asdx::Function<void()> f;

        EXPECT_TRUE(f == nullptr);
        EXPECT_TRUE(nullptr == f);
        EXPECT_FALSE((bool)f);

        f = []() {
            EXPECT_TRUE(true);
        };
        f();

        EXPECT_TRUE((bool)f);

        EXPECT_TRUE(f != nullptr);
        EXPECT_TRUE(nullptr != f);
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
        struct FakePass
        {
            asdx::Action<void> action;
        };

        FakePass pass = {};
        pass.action = []() {
            EXPECT_TRUE(true);
        };

        pass.action();
    }

    {
        bool called = false;
        bool destructed = false;

        {
            struct Functor
            {
                bool* pCalled     = nullptr;
                bool* pDestructed = nullptr;

                Functor(bool* called, bool* destructed)
                : pCalled(called)
                , pDestructed(destructed)
                {}

                ~Functor()
                { *pDestructed = true; }

                void operator() () { *pCalled = true; }
            };

            asdx::Function<void()> f = Functor(&called, &destructed);
            f();
        }
        EXPECT_TRUE(called);
        EXPECT_TRUE(destructed);
    }

    {
        bool called = false;
        asdx::Function<void()> f = [&]() { called = true; };

        EXPECT_FALSE(called);

        auto&& f2 = std::move(f);
        f2();
        EXPECT_TRUE(called);

        called = false;
        asdx::Function<void()> f3(std::move(f2));
        EXPECT_FALSE(called);
        f3();
        EXPECT_TRUE(called);
    }
}

TEST(FunctionTest, Thread)
{
    {
        bool called = false;
        asdx::Function<void()> func = [&](){ called = true; };
        EXPECT_FALSE(called);
        std::thread thread(func);
        thread.join();
        EXPECT_TRUE(called);
    }
}
