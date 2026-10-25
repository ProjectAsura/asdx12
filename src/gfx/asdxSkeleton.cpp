//----------------------------------------------------------------------------
// File : asdxSkeleton.cpp
// Desc : Skeleton Data.
// Copyright(c) Project Asura.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstring>
#include <cassert>
#include <gfx/asdxSkeleton.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Skeleton class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Skeleton::Skeleton()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Skeleton::~Skeleton()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
void Skeleton::Init(const ArrayView<ResBone>& bones)
{
    if (bones.empty())
        return;

    auto count = bones.size();
    m_Names       .resize(count);
    m_Parents     .resize(count);
    m_BindPoses   .resize(count);
    m_InvBindPoses.resize(count);
    m_Children    .resize(count);

    for(size_t i=0; i<count; ++i)
    {
        m_Names[i].assign(bones[i].Name.c_str(), bones[i].Name.size());
        m_Parents[i] = bones[i].Parent;
        m_BindPoses[i] = bones[i].BindPose;
        m_InvBindPoses[i] = bones[i].InvBindPose;

        auto size = bones[i].Children.size();
        m_Children[i].resize(size);
        for(size_t j=0; j<size; ++j)
            m_Children[i][j] = bones[i].Children[j];
    }
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Skeleton::Term()
{
    m_Names         .clear();
    m_Parents       .clear();
    m_BindPoses     .clear();
    m_InvBindPoses  .clear();

    for(size_t i=0; i<m_Children.size(); ++i)
        m_Children[i].clear();
    m_Children.clear();
}

//-----------------------------------------------------------------------------
//      ボーンを持つかどうかチェックします.
//-----------------------------------------------------------------------------
bool Skeleton::HasBone() const
{ return !m_BindPoses.empty(); }

//-----------------------------------------------------------------------------
//      ボーン数を取得します.
//-----------------------------------------------------------------------------
size_t Skeleton::GetBoneCount() const
{ return m_BindPoses.size(); }

//-----------------------------------------------------------------------------
//      ボーンを取得します.
//-----------------------------------------------------------------------------
ResBone Skeleton::GetBone(size_t index) const
{
    assert(index < GetBoneCount());
    ResBone result;
    result.Name         = StringView(m_Names[index].c_str());
    result.Parent       = m_Parents[index];
    result.BindPose     = m_BindPoses[index];
    result.InvBindPose  = m_InvBindPoses[index];
    result.Children     = ArrayView<BoneId>(m_Children[index].data(), m_Children[index].size());
    return result;
}

//-----------------------------------------------------------------------------
//      ボーン名を検索します.
//-----------------------------------------------------------------------------
bool Skeleton::FindBone(const char* name, size_t& index) const
{
    for(size_t i=0; i<m_Names.size(); ++i)
    {
        if (strcmp(m_Names[i].c_str(), name) == 0)
        {
            index = uint32_t(i);
            return true;
        }
    }

    index = SIZE_MAX;
    return false;
}

} // namespace asdx
