//-----------------------------------------------------------------------------
// File : JobSystem.cpp
// Desc : Job System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxJobSystem.h>
#include <cstdio>

enum SyncPoint
{
    SYNC_POINT_FRAME_START,

    SYNC_POINT_01,
    SYNC_POINT_02,
    SYNC_POINT_03,
    SYNC_POINT_04,
    SYNC_POINT_05,
    SYNC_POINT_06,
    SYNC_POINT_07,
    SYNC_POINT_08,
    SYNC_POINT_09,
    SYNC_POINT_10,

    SYNC_POINT_FRAME_END,
    COUNT_OF_SYNC_POINT,
};

struct TestListener1 : public asdx::JobListener
{
    void OnRun(uint32_t jobId, void* pUserData) override
    {
        printf_s("TestListener1::OnRun() id = %u\n", jobId);
    }
};

struct TestListener2 : public asdx::JobListener
{
    void OnRun(uint32_t jobId, void* pUserData) override
    {
        printf_s("TestListener2::OnRun() id = %u\n", jobId);
    }
};

struct TestListener3 : public asdx::JobListener
{
    void OnRun(uint32_t jobId, void* pUserData) override
    {
        printf_s("TestListener3::OnRun() id = %u\n", jobId);
    }
};


TEST(JobSystemTest, Basic)
{
    auto ret = asdx::InitJobSystem(COUNT_OF_SYNC_POINT, 4);
    EXPECT_TRUE(ret);

    auto pJobSystem = asdx::GetJobSystem();
    EXPECT_TRUE(pJobSystem != nullptr);

    auto listener1 = TestListener1();
    auto listener2 = TestListener2();
    auto listener3 = TestListener3();

    auto job1  = asdx::Job( 1, SYNC_POINT_FRAME_START, SYNC_POINT_01, &listener1);
    auto job2  = asdx::Job( 2, SYNC_POINT_01, SYNC_POINT_02, &listener1);
    auto job3  = asdx::Job( 3, SYNC_POINT_02, SYNC_POINT_03, &listener2);
    auto job4  = asdx::Job( 4, SYNC_POINT_03, SYNC_POINT_04, &listener3);
    auto job5  = asdx::Job( 5, SYNC_POINT_04, SYNC_POINT_FRAME_END, &listener3);
    auto job6  = asdx::Job( 6, SYNC_POINT_04, SYNC_POINT_05, &listener2);
    auto job7  = asdx::Job( 7, SYNC_POINT_05, SYNC_POINT_07, &listener1);
    auto job8  = asdx::Job( 8, SYNC_POINT_05, SYNC_POINT_06, &listener2);
    auto job9  = asdx::Job( 9, SYNC_POINT_06, SYNC_POINT_07, &listener2);
    auto job10 = asdx::Job(10, SYNC_POINT_06, SYNC_POINT_07, &listener3);

    EXPECT_TRUE(pJobSystem->Add(job1));
    EXPECT_TRUE(pJobSystem->Add(job2));
    EXPECT_TRUE(pJobSystem->Add(job3));
    EXPECT_TRUE(pJobSystem->Add(job4));
    EXPECT_TRUE(pJobSystem->Add(job5));
    EXPECT_TRUE(pJobSystem->Add(job6));
    EXPECT_TRUE(pJobSystem->Add(job7));
    EXPECT_TRUE(pJobSystem->Add(job8));
    EXPECT_TRUE(pJobSystem->Add(job9));
    EXPECT_TRUE(pJobSystem->Add(job10));

    pJobSystem->Run();

    EXPECT_TRUE(pJobSystem->Remove(job1));
    EXPECT_TRUE(pJobSystem->Remove(job2));
    EXPECT_TRUE(pJobSystem->Remove(job3));
    EXPECT_TRUE(pJobSystem->Remove(job4));
    EXPECT_TRUE(pJobSystem->Remove(job5));
    EXPECT_TRUE(pJobSystem->Remove(job6));
    EXPECT_TRUE(pJobSystem->Remove(job7));
    EXPECT_TRUE(pJobSystem->Remove(job8));
    EXPECT_TRUE(pJobSystem->Remove(job9));
    EXPECT_TRUE(pJobSystem->Remove(job10));

    asdx::TermJobSystem();
    EXPECT_TRUE(asdx::GetJobSystem() == nullptr);
}