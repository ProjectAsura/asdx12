﻿//-----------------------------------------------------------------------------
// File : asdxStack.h
// Desc : Stack
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Stack class
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class Stack
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    class Node;
    typedef Node StackNode;

    ///////////////////////////////////////////////////////////////////////////
    // Node class
    ///////////////////////////////////////////////////////////////////////////
    class Node
    {
        //=====================================================================
        // list of friend classes and methods.
        //=====================================================================
        friend class Stack<T>;

    public:
        //=====================================================================
        // public variables.
        //=====================================================================
        /* NOTHING */

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
            { prev->StackNode::m_Next = next; }

            if (next != nullptr)
            { next->StackNode::m_Prev = prev; }

            m_Prev = nullptr;
            m_Next = nullptr;
        }

    private:
        //=====================================================================
        // private variables.
        //=====================================================================
        T*  m_Next = nullptr;   //!< 次のノードです.
        T*  m_Prev = nullptr;   //!< 前のノードです.

        //---------------------------------------------------------------------
        //! @brief      リンクを設定します.
        //---------------------------------------------------------------------
        static void Link(T* lhs, T* rhs)
        {
            if (lhs == nullptr || rhs == nullptr)
            { return; }

            lhs->StackNode::m_Next = rhs;
            rhs->StackNode::m_Prev = lhs;
        }

        //---------------------------------------------------------------------
        //! @brief      リンクを解除します.
        //---------------------------------------------------------------------
        static void Unlink(T* node)
        {
            if (node == nullptr)
            { return; }

            auto prev = node->StackNode::m_Prev;
            auto next = node->StackNode::m_Next;

            if (prev != nullptr)
            { prev->StackNode::m_Next = next; }

            if (next != nullptr)
            { next->StackNode::m_Prev = prev; }

            node->StackNode::m_Prev = nullptr;
            node->StackNode::m_Next = nullptr;
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
    Stack()
    : m_Head(nullptr)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~Stack()
    { Clear(); }

    //-------------------------------------------------------------------------
    //! @brief      ノードを追加します.
    //-------------------------------------------------------------------------
    void Push(T* node)
    {
        if (node == nullptr)
        { return; }

        // 継承チェック.
        assert(static_cast<StackNode*>(node) != nullptr);

        if (m_Head == nullptr)
        {
            m_Head = node;
        }
        else
        {
            StackNode::Link(m_Head, node);
            m_Head = node;
        }
        m_Count++;
    }

    //-------------------------------------------------------------------------
    //! @brief      ノードを取り出します.
    //-------------------------------------------------------------------------
    T* Pop()
    {
        if (m_Head == nullptr)
        { return nullptr; }

        auto head = m_Head;
        auto prev = m_Head->StackNode::m_Prev;
        StackNode::Unlink(head);
        m_Head = prev;
        m_Count--;

        return head;
    }

    //-------------------------------------------------------------------------
    //! @brief      クリアします.
    //-------------------------------------------------------------------------
    void Clear()
    {
        auto itr = m_Head;
        while(itr != nullptr)
        {
            auto node = itr;
            itr = itr->StackNode::m_Prev;
            StackNode::Unlink(node);
        }

        m_Count = 0;
        m_Head  = nullptr;
    }

    //-------------------------------------------------------------------------
    //! @brief      空かどうかチェックします.
    //-------------------------------------------------------------------------
    bool IsEmpty() const
    { return m_Count == 0; }

    //-------------------------------------------------------------------------
    //! @brief      格納されている要素数を取得します.
    //-------------------------------------------------------------------------
    size_t GetCount() const
    { return m_Count; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    T*     m_Head  = nullptr;   //!< スタック先頭ポインタ.
    size_t m_Count = 0;         //!< 格納数.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx