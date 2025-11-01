//-----------------------------------------------------------------------------
// File : JobSystem.cpp
// Desc : Job System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxJob.h>
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

enum JOB_ID
{
    JOB_ID_01,
    JOB_ID_02,
    JOB_ID_03,
    JOB_ID_04,
    JOB_ID_05,
    JOB_ID_06,
    JOB_ID_07,
    JOB_ID_08,
    JOB_ID_09,
    JOB_ID_10,

    COUNT_OF_JOB_ID
};

struct TestListener1 : public asdx::JobListener
{
    void OnRun(uint32_t jobId) override
    {
        if (jobId < COUNT_OF_JOB_ID)
        { Called[jobId] = true; }
    }

    bool Called[COUNT_OF_JOB_ID] = {};
};

struct TestListener2 : public asdx::JobListener
{
    void OnRun(uint32_t jobId) override
    {
        if (jobId < COUNT_OF_JOB_ID)
        { Called[jobId] = true; }
    }

    bool Called[COUNT_OF_JOB_ID] = {};
};

struct TestListener3 : public asdx::JobListener
{
    void OnRun(uint32_t jobId) override
    {
        if (jobId < COUNT_OF_JOB_ID)
        { Called[jobId] = true; }
    }

    bool Called[COUNT_OF_JOB_ID] = {};
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

    for(auto i=0; i<COUNT_OF_JOB_ID; ++i)
    {
        EXPECT_FALSE(listener1.Called[i]);
        EXPECT_FALSE(listener2.Called[i]);
        EXPECT_FALSE(listener3.Called[i]);
    }

    auto job1  = asdx::Job(JOB_ID_01, SYNC_POINT_FRAME_START, SYNC_POINT_01, &listener1);
    auto job2  = asdx::Job(JOB_ID_02, SYNC_POINT_01, SYNC_POINT_02, &listener1);
    auto job3  = asdx::Job(JOB_ID_03, SYNC_POINT_02, SYNC_POINT_03, &listener2);
    auto job4  = asdx::Job(JOB_ID_04, SYNC_POINT_03, SYNC_POINT_04, &listener3);
    auto job5  = asdx::Job(JOB_ID_05, SYNC_POINT_04, SYNC_POINT_FRAME_END, &listener3);
    auto job6  = asdx::Job(JOB_ID_06, SYNC_POINT_04, SYNC_POINT_05, &listener2);
    auto job7  = asdx::Job(JOB_ID_07, SYNC_POINT_05, SYNC_POINT_07, &listener1);
    auto job8  = asdx::Job(JOB_ID_08, SYNC_POINT_05, SYNC_POINT_06, &listener2);
    auto job9  = asdx::Job(JOB_ID_09, SYNC_POINT_06, SYNC_POINT_07, &listener2);
    auto job10 = asdx::Job(JOB_ID_10, SYNC_POINT_06, SYNC_POINT_07, &listener3);

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

    EXPECT_TRUE(listener1.Called[JOB_ID_01]);
    EXPECT_TRUE(listener1.Called[JOB_ID_02]);
    EXPECT_TRUE(listener2.Called[JOB_ID_03]);
    EXPECT_TRUE(listener3.Called[JOB_ID_04]);
    EXPECT_TRUE(listener3.Called[JOB_ID_05]);
    EXPECT_TRUE(listener2.Called[JOB_ID_06]);
    EXPECT_TRUE(listener1.Called[JOB_ID_07]);
    EXPECT_TRUE(listener2.Called[JOB_ID_08]);
    EXPECT_TRUE(listener2.Called[JOB_ID_09]);
    EXPECT_TRUE(listener3.Called[JOB_ID_10]);

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