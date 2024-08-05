//-----------------------------------------------------------------------------
// File : DisposerTest.cpp
// Desc : Disposer Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxDisposer.h>
#include <atomic>

static std::atomic<uint32_t> g_Disposed = 0;

class TestObject
{
public:
    static TestObject* Create()
    { return new TestObject(); }

    void Release()
    {
        m_RefCount--;
        if (m_RefCount == 0)
        { delete this; }
    }

    void AddRef()
    { m_RefCount++; }

    uint32_t GetRefCount() const
    { return m_RefCount; }

private:
    std::atomic<uint32_t> m_RefCount = 1;

    TestObject()
    { /* DO_NOTHING */ }

    ~TestObject()
    { g_Disposed++; }
};

TEST(DisposerTest, Basic)
{
    asdx::Disposer<TestObject> disposer(4);
    EXPECT_EQ(disposer.GetCount(), 0);
    EXPECT_TRUE(disposer.Empty());

    for(auto i=0; i<4; ++i)
    {
        auto object = TestObject::Create();
        disposer.Push(object);
    }

    EXPECT_EQ(disposer.GetCount(), 4);
    EXPECT_FALSE(disposer.Empty());

    disposer.FrameSync();

    for(auto i=0; i<3; ++i)
    {
        auto object = TestObject::Create();
        disposer.Push(object);
    }
    EXPECT_EQ(disposer.GetCount(), 7);
    EXPECT_FALSE(disposer.Empty());
    disposer.FrameSync();

    for(auto i=0; i<5; ++i)
    {
        auto object = TestObject::Create();
        disposer.Push(object);
    }

    EXPECT_EQ(disposer.GetCount(), 12);
    EXPECT_FALSE(disposer.Empty());
    disposer.FrameSync();

    for(auto i=0; i<2; ++i)
    {
        auto object = TestObject::Create();
        disposer.Push(object);
    }

    EXPECT_EQ(disposer.GetCount(), 14);
    EXPECT_FALSE(disposer.Empty());
    disposer.FrameSync();
    EXPECT_EQ(disposer.GetCount(), 10);
    EXPECT_EQ(g_Disposed, 4);

    disposer.FrameSync();
    EXPECT_EQ(disposer.GetCount(), 7);
    EXPECT_EQ(g_Disposed, 7);

    disposer.FrameSync();
    EXPECT_EQ(disposer.GetCount(), 2);
    EXPECT_EQ(g_Disposed, 12);

    disposer.FrameSync();
    EXPECT_EQ(disposer.GetCount(), 0);
    EXPECT_TRUE(disposer.Empty());
    EXPECT_EQ(g_Disposed, 14);
}
