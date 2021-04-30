﻿//-----------------------------------------------------------------------------
// File : asdxEntity.cpp
// Desc : Entity
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fw/asdxEntity.h>
#include <fnd/asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Entity class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンポーネントを追加します.
//-----------------------------------------------------------------------------
bool Entity::Add(uint32_t id, IComponent* item)
{
    if (m_Components.find(id) != m_Components.end())
    { return false; }

    m_Components[id] = item;
    return true;
}

//-----------------------------------------------------------------------------
//      コンポーネントを削除します.
//-----------------------------------------------------------------------------
bool Entity::Remove(uint32_t id)
{ return m_Components.erase(id) > 0; }

//-----------------------------------------------------------------------------
//      コンポーネントを取得します.
//-----------------------------------------------------------------------------
IComponent* Entity::Get(uint32_t id) const
{
    if (m_Components.find(id) == m_Components.end())
    { return nullptr; }

    return m_Components.at(id);
}

//-----------------------------------------------------------------------------
//      更新処理を行ないます.
//-----------------------------------------------------------------------------
void Entity::Update()
{
    for(auto& itr : m_Components)
    { itr.second->Update(); }
}

//-----------------------------------------------------------------------------
//      描画処理を行ないます.
//-----------------------------------------------------------------------------
void Entity::Draw()
{
    for(auto& itr : m_Components)
    { itr.second->Draw(); }
}

} // namespace asdx
