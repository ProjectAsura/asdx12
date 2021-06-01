﻿//-----------------------------------------------------------------------------
// File : asdxList.h
// Desc : Double-Linked List.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// List class
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class List
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    class Node;
    typedef Node ListNode;

    ///////////////////////////////////////////////////////////////////////////
    // Node class
    ///////////////////////////////////////////////////////////////////////////
    class Node
    {
        //=====================================================================
        // list of friend classes and methods.
        //=====================================================================
        friend class List<T>;

    public:
        //=====================================================================
        // public variables.
        //=====================================================================
        /* NOTHING */

        //=====================================================================
        // public methods.
        //=====================================================================

        //---------------------------------------------------------------------
        //! @brief      コンストラクタです.
        //---------------------------------------------------------------------
        Node()
        : m_Next(nullptr)
        , m_Prev(nullptr)
        { /* DO_NOTHING */ }

        //---------------------------------------------------------------------
        //! @brief      デストラクタです.
        //---------------------------------------------------------------------
        ~Node()
        {
            auto prev = m_Prev;
            auto next = m_Next;

            if (prev != nullptr)
            { prev->ListNode::m_Next = next; }

            if (next != nullptr)
            { next->ListNode::m_Prev = prev; }

            m_Prev = nullptr;
            m_Next = nullptr;
        }

        //---------------------------------------------------------------------
        //! @brief      次のノードを取得します.
        //---------------------------------------------------------------------
        T* GetNext()
        { return m_Next; }

        //---------------------------------------------------------------------
        //! @brief      前のノードを取得します.
        //---------------------------------------------------------------------
        T* GetPrev()
        { return m_Prev; }

        //---------------------------------------------------------------------
        //! @brief      次のノードを取得します.
        //---------------------------------------------------------------------
        const T* GetNext() const
        { return m_Next; }

        //---------------------------------------------------------------------
        //! @brief      前のノードを取得します.
        //---------------------------------------------------------------------
        const T* GetPrev() const
        { return m_Prev; }

        //---------------------------------------------------------------------
        //! @brief      次のノードを持つかチェックします.
        //---------------------------------------------------------------------
        bool HasNext() const
        { return m_Next != nullptr; }

        //---------------------------------------------------------------------
        //! @brief      前のノードを持つかチェックします.
        //---------------------------------------------------------------------
        bool HasPrev() const
        { return m_Prev != nullptr; }

    private:
        //=====================================================================
        // private variables.
        //=====================================================================
        T*  m_Next = nullptr;       //!< 次のノード.
        T*  m_Prev = nullptr;       //!< 前のノード.

        //---------------------------------------------------------------------
        //! @brief      リンクを設定します.
        //---------------------------------------------------------------------
        static void Link(T* lhs, T* rhs)
        {
            if (lhs == nullptr || rhs == nullptr)
            { return; }

            lhs->ListNode::m_Next = rhs;
            rhs->ListNode::m_Prev = lhs;
        }

        //---------------------------------------------------------------------
        //! @brief      リンクを解除します.
        //---------------------------------------------------------------------
        static void Unlink(T* node)
        {
            if (node == nullptr)
            { return; }

            auto prev = node->ListNode::m_Prev;
            auto next = node->ListNode::m_Next;

            if (prev != nullptr)
            { prev->ListNode::m_Next = next; }

            if (next != nullptr)
            { next->ListNode::m_Prev = prev; }

            node->ListNode::m_Prev = nullptr;
            node->ListNode::m_Next = nullptr;
        }
    };

    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    List()
    : m_Head(nullptr)
    , m_Tail(nullptr)
    , m_Count(0)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~List()
    { Clear(); }

    //-------------------------------------------------------------------------
    //! @brief      リストを破棄します.
    //-------------------------------------------------------------------------
    void Clear()
    {
        auto itr = m_Head;
        while(itr != nullptr)
        {
            auto node = itr;
            itr = itr->GetNext();
            ListNode::Unlink(node);
        }

        m_Head = nullptr;
        m_Tail = nullptr;

        m_Count = 0;
    }

    //-------------------------------------------------------------------------
    //! @brief      先頭に追加します.
    //-------------------------------------------------------------------------
    void PushFront(T* node)
    {
        if (node == nullptr)
        { return; }

        // 継承チェック.
        assert(static_cast<ListNode*>(node) != nullptr);

        if (m_Head == nullptr)
        {
            m_Head = node;
            m_Tail = node;
        }
        else
        {
            ListNode::Link(node, m_Head);
            m_Head = node;
        }
        m_Count++;
    }

    //-------------------------------------------------------------------------
    //! @brief      末尾に追加します.
    //-------------------------------------------------------------------------
    void PushBack(T* node)
    {
        if (node == nullptr)
        { return; }

        // 継承チェック.
        assert(static_cast<ListNode*>(node) != nullptr);

        if (m_Head == nullptr)
        {
            m_Head = node;
            m_Tail = node;
        }
        else
        {
            ListNode::Link(m_Tail, node);
            m_Tail = node;
        }
        m_Count++;
    }

    //-------------------------------------------------------------------------
    //! @brief      先頭要素を取り出します.
    //-------------------------------------------------------------------------
    T* PopFront()
    {
        if (m_Head == nullptr)
        { return nullptr; }

        auto head = m_Head;
        auto next = m_Head->GetNext();
        ListNode::Unlink(head);
        m_Head = next;
        m_Count--;

        return head;
    }

    //-------------------------------------------------------------------------
    //! @brief      末尾要素を取り出します.
    //-------------------------------------------------------------------------
    T* PopBack()
    {
        if (m_Tail == nullptr)
        { return nullptr; }

        auto tail = m_Tail;
        auto prev = m_Tail->GetPrev();
        ListNode::Unlink(tail);
        m_Tail = prev;
        m_Count--;

        return tail;
    }

    //-------------------------------------------------------------------------
    //! @brief      ノードを挿入します.
    //-------------------------------------------------------------------------
    void InsertBefore(T* target, T* node)
    {
        if (target == nullptr || node == nullptr)
        { return; }

        ListNode::Link(node, target);
        if (target == m_Head)
        { m_Head = node; }

        m_Count++;
    }

    //-------------------------------------------------------------------------
    //! @brief      ノードを挿入します.
    //-------------------------------------------------------------------------
    void InsertAfter(T* target, T* node)
    {
        if (target == nullptr || node == nullptr)
        { return; }

        ListNode::Link(target, node);
        if (target == m_Tail)
        { m_Tail = node; }

        m_Count++;
    }

    //-------------------------------------------------------------------------
    //! @brief      指定ノードをリストから削除します.
    //-------------------------------------------------------------------------
    void Remove(T* node)
    {
        if (node == nullptr)
        { return; }

        if (node == m_Tail)
        { m_Tail = node->GetPrev(); }
        else if (node == m_Head)
        { m_Head = node->GetNext(); }

        ListNode::Unlink(node);
        m_Count--;
    }

    //-------------------------------------------------------------------------
    //! @brief      指定ノードが含まれるかチェックします.
    //-------------------------------------------------------------------------
    bool Contains(const T* node) const
    {
        if (node == nullptr)
        { return false; }

        auto itr = m_Head;
        while(itr != nullptr)
        {
            if (itr == node)
            { return true; }

            itr = itr->GetNext();
        }

        return false;
    }

    //-------------------------------------------------------------------------
    //! @brief      空かどうかチェックします.
    //-------------------------------------------------------------------------
    bool IsEmpty() const
    { return m_Count == 0; }

    //-------------------------------------------------------------------------
    //! @brief      先頭ノードを取得します.
    //-------------------------------------------------------------------------
    T* GetHead() const
    { return m_Head; }

    //-------------------------------------------------------------------------
    //! @brief      末尾ノードを取得します.
    //-------------------------------------------------------------------------
    T* GetTail() const
    { return m_Tail; }

    //-------------------------------------------------------------------------
    //! @brief      ノード数を取得します.
    //-------------------------------------------------------------------------
    size_t GetCount() const
    { return m_Count; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    T*      m_Head  = nullptr;
    T*      m_Tail  = nullptr;
    size_t  m_Count = 0;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx
