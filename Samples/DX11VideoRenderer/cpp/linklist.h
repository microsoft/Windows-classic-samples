//-----------------------------------------------------------------------------
// File: Linklist.h
// Desc: Linked list class.
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


// Notes:
// 
// This class template implements a simple double-linked list. 
// Objects are held as pointers. (Does not use STL's copy semantics.)
// The Clear() method takes a functor object that releases the objects.

#pragma once

#include <new>
#include <assert.h>
#include <unknwn.h>

template <class T>
class List
{
protected:

    // Nodes in the linked list
    struct Node
    {
        Node *prev;
        Node *next;
        T    *item;

        Node() : prev(NULL), next(NULL), item(NULL)
        {
        }

        Node(T* item) : prev(NULL), next(NULL)
        {
            this->item = item;
        }

        T* Item() const { return item; }
    };



protected:
    Node    m_anchor;  // Anchor node for the linked list.
    DWORD   m_count;   // Number of items in the list	
	
	Node    *m_pEnum;   // Enumeration node

    void InvalidateEnumerator() 
    {
        m_pEnum = NULL;
    }

    Node* Front() const
    {
        return m_anchor.next;
    }

    Node* Back() const
    {
        return m_anchor.prev;
    }	

    virtual HRESULT InsertAfter(T* item, Node *pBefore)
    {
        if (item == NULL || pBefore == NULL) 
        {
            return E_POINTER;
        }

        Node *pNode = new Node(item);
        if (pNode == NULL)
        {
            return E_OUTOFMEMORY;
        }

        Node *pAfter = pBefore->next;
        
        pBefore->next = pNode;
        pAfter->prev = pNode;

        pNode->prev = pBefore;
        pNode->next = pAfter;

        m_count++;

		InvalidateEnumerator(); 

        return S_OK;
    }

	virtual HRESULT GetItem(Node *pNode, T** ppItem)
    {
        if (pNode == NULL || ppItem == NULL)
        {
            return E_POINTER;
        }

        if (pNode->item)
        {
            *ppItem = pNode->item;
            return S_OK;
        }
        else
        {
            return E_INVALIDARG;
        }
    }

	
    // RemoveItem:
    // Removes a node and optionally returns the item.
    // ppItem can be NULL.
    virtual HRESULT RemoveItem(Node *pNode, T **ppItem)
    {
        if (pNode == NULL)
        {
            return E_POINTER;
        }

        if (pNode == &m_anchor)
        {
            return E_INVALIDARG;
        }


        T* item = NULL;

        // anchor points back to the previous thing
        m_anchor.prev = pNode->prev;

        // the previous thing points to the anchor
        pNode->prev->next = &m_anchor;

        item = pNode->item;
        delete pNode;

        m_count--;

        if (ppItem)
        {
            *ppItem = item;
        }

        InvalidateEnumerator();

        return S_OK;
    }

public:

    List()
    {
        m_anchor.next = &m_anchor;
        m_anchor.prev = &m_anchor;

        m_count = 0;
   
		m_pEnum = NULL;
    }

    virtual ~List()
    {

    }

    // Insertion functions
    HRESULT InsertBack(T* item)
    {
        return InsertAfter(item, m_anchor.prev);
    }

    HRESULT InsertFront(T* item)
    {
        return InsertAfter(item, &m_anchor);
    }

    // RemoveBack: Removes the tail of the list and returns the value.
    HRESULT RemoveBack(T **ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return RemoveItem(Back(), ppItem);
        }
    }

    // RemoveFront: Removes the head of the list and returns the value.
    HRESULT RemoveFront(T **ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return RemoveItem(Front(), ppItem);
        }
    }

    HRESULT PopFront()
    {
        return RemoveFront(NULL);
    }

    HRESULT PopBack()
    {
        return RemoveBack(NULL);
    }

    HRESULT GetBack(T **ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return GetItem(Back(), ppItem);
        }
    }

    HRESULT GetFront(T **ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return GetItem(Front(), ppItem);
        }
    }

    // GetCount: Returns the number of items in the list.
    DWORD GetCount() const { return m_count; } 

    bool IsEmpty() const
    {	
		return (GetCount() == 0);
    }

    // Clear: Takes a functor object whose operator()
    // frees the object on the list.
    template <class FN>
    void Clear(FN& clear_fn)
    {
        Node *n = m_anchor.next;

        // Delete the nodes
        while (n != &m_anchor)
        {
            clear_fn(n->item);

            Node *tmp = n->next;
            delete n;
            n = tmp;
        }

        // Reset the anchor to point at itself
        m_anchor.next = &m_anchor;
        m_anchor.prev = &m_anchor;

        m_count = 0;
    }
		
	// Enumerator functions
	void ResetEnumerator()
	{
		m_pEnum = Front();
	}

	HRESULT Next(T **ppItem)
	{
		if (ppItem == NULL)
		{
			return E_POINTER;
		}

		if (m_pEnum == NULL)
		{
			return E_FAIL; // Needs reset
		}

		if (m_pEnum == &m_anchor)
		{
			return __HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
		}

		HRESULT hr = GetItem(m_pEnum, ppItem);

		if (SUCCEEDED(hr))
		{
			m_pEnum = m_pEnum->next;
		}
		return hr;
	}	
		
};



// Typical functors for Clear method.

// ComAutoRelease: Releases COM pointers.
// MemDelete: Deletes pointers to new'd memory.

class ComAutoRelease
{
public: 
    void operator()(IUnknown *p)
    {
        if (p)
        {
            p->Release();
        }
    }
};
    
class MemDelete
{
public: 
    void operator()(void *p)
    {
        if (p)
        {
            delete p;
        }
    }
};


 //ComPtrList class
 //Derived class that makes it safer to store COM pointers in the List<> class.
 //It automatically AddRef's the pointers that are inserted onto the list
 //(unless the insertion method fails). 

 //T must be a COM interface type.

template <class T>
class ComPtrList : public List<T>
{
public:
    void Clear()
    {
        List<T>::Clear(ComAutoRelease());
    }

protected:
    HRESULT InsertAfter(T* item, Node *pBefore)
    {
        item->AddRef();

        HRESULT hr = List<T>::InsertAfter(item, pBefore);
        if (FAILED(hr))
        {
            item->Release();
        }
        return hr;
    }
	
	HRESULT GetItem(Node *pNode, T** ppItem)
    {
        HRESULT hr = List<T>::GetItem(pNode, ppItem);
        if (SUCCEEDED(hr))
        {
            (*ppItem)->AddRef();
        }
        return hr;
    }

    HRESULT RemoveItem(Node *pNode, T **ppItem)
    {
        // ppItem can be NULL, but we need to get the
        // item so that we can release it. 

        // If ppItem is not NULL, we will AddRef it on the way out/

        T* item = NULL;

        HRESULT hr = List<T>::RemoveItem(pNode, &item);

        if (SUCCEEDED(hr))
        {
            if (ppItem)
            {
                *ppItem = item;
                (*ppItem)->AddRef();
            }

            item->Release();
        }

        return hr;
    }
	
};

template <class T, bool NULLABLE = FALSE>
class ComPtrListEx
{
protected:

    typedef T* Ptr;

    // Nodes in the linked list
    struct Node
    {
        Node *prev;
        Node *next;
        Ptr  item;

        Node() : prev(NULL), next(NULL), item(NULL)
        {
        }

        Node(Ptr item) : prev(NULL), next(NULL)
        {
            this->item = item;
            if (item)
            {
                item->AddRef();
            }
        }
        ~Node()
        {
            if (item)
            {
                item->Release();
            }
        }

        Ptr Item() const { return item; }
    };

public:

    // Object for enumerating the list.
    class POSITION
    {
        friend class ComPtrListEx<T>;

    public:
        POSITION() : pNode(NULL)
        {
        }

        bool operator==(const POSITION &p) const
        {
            return pNode == p.pNode;
        }

        bool operator!=(const POSITION &p) const
        {
            return pNode != p.pNode;
        }

    private:
        const Node *pNode;

        POSITION(Node *p) : pNode(p)
        {
        }
    };

protected:
    Node    m_anchor;  // Anchor node for the linked list.
    DWORD   m_count;   // Number of items in the list.

    Node* Front() const
    {
        return m_anchor.next;
    }

    Node* Back() const
    {
        return m_anchor.prev;
    }

    virtual HRESULT InsertAfter(Ptr item, Node *pBefore)
    {
        if (pBefore == NULL)
        {
            return E_POINTER;
        }

        // Do not allow NULL item pointers unless NULLABLE is true.
        if (!item && !NULLABLE)
        {
            return E_POINTER;
        }

        Node *pNode = new Node(item);
        if (pNode == NULL)
        {
            return E_OUTOFMEMORY;
        }

        Node *pAfter = pBefore->next;

        pBefore->next = pNode;
        pAfter->prev = pNode;

        pNode->prev = pBefore;
        pNode->next = pAfter;

        m_count++;

        return S_OK;
    }

    virtual HRESULT GetItem(const Node *pNode, Ptr* ppItem)
    {
        if (pNode == NULL || ppItem == NULL)
        {
            return E_POINTER;
        }

        *ppItem = pNode->item;

        if (*ppItem)
        {
            (*ppItem)->AddRef();
        }

        return S_OK;
    }

    // RemoveItem:
    // Removes a node and optionally returns the item.
    // ppItem can be NULL.
    virtual HRESULT RemoveItem(Node *pNode, Ptr *ppItem)
    {
        if (pNode == NULL)
        {
            return E_POINTER;
        }

        assert(pNode != &m_anchor); // We should never try to remove the anchor node.
        if (pNode == &m_anchor)
        {
            return E_INVALIDARG;
        }


        Ptr item;

        // The next node's previous is this node's previous.
        pNode->next->prev = pNode->prev;

        // The previous node's next is this node's next.
        pNode->prev->next = pNode->next;

        item = pNode->item;

        if (ppItem)
        {
            *ppItem = item;

            if (*ppItem)
            {
                (*ppItem)->AddRef();
            }
        }

        delete pNode;
        m_count--;

        return S_OK;
    }

public:

    ComPtrListEx()
    {
        m_anchor.next = &m_anchor;
        m_anchor.prev = &m_anchor;

        m_count = 0;
    }

    virtual ~ComPtrListEx()
    {
        Clear();
    }

    void Clear()
    {
        Node *n = m_anchor.next;

        // Delete the nodes
        while (n != &m_anchor)
        {
            if (n->item)
            {
                n->item->Release();  
                n->item = NULL;
            }

            Node *tmp = n->next;
            delete n;
            n = tmp;
        }

        // Reset the anchor to point at itself
        m_anchor.next = &m_anchor;
        m_anchor.prev = &m_anchor;

        m_count = 0;
    }

    // Insertion functions
    HRESULT InsertBack(Ptr item)
    {
        return InsertAfter(item, m_anchor.prev);
    }


    HRESULT InsertFront(Ptr item)
    {
        return InsertAfter(item, &m_anchor);
    }

    // RemoveBack: Removes the tail of the list and returns the value.
    // ppItem can be NULL if you don't want the item back.
    HRESULT RemoveBack(Ptr *ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return RemoveItem(Back(), ppItem);
        }
    }

    // RemoveFront: Removes the head of the list and returns the value.
    // ppItem can be NULL if you don't want the item back.
    HRESULT RemoveFront(Ptr *ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return RemoveItem(Front(), ppItem);
        }
    }

    // GetBack: Gets the tail item.
    HRESULT GetBack(Ptr *ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return GetItem(Back(), ppItem);
        }
    }

    // GetFront: Gets the front item.
    HRESULT GetFront(Ptr *ppItem)
    {
        if (IsEmpty())
        {
            return E_FAIL;
        }
        else
        {
            return GetItem(Front(), ppItem);
        }
    }


    // GetCount: Returns the number of items in the list.
    DWORD GetCount() const { return m_count; }

    bool IsEmpty() const
    {
        return (GetCount() == 0);
    }

    // Enumerator functions

    POSITION FrontPosition()
    {
        if (IsEmpty())
        {
            return POSITION(NULL);
        }
        else
        {
            return POSITION(Front());
        }
    }

    POSITION EndPosition() const
    {
        return POSITION();
    }

    HRESULT GetItemByPosition(POSITION pos, Ptr *ppItem)
    {
        if (pos.pNode)
        {
            return GetItem(pos.pNode, ppItem);
        }
        else
        {
            return E_FAIL;
        }
    }

    POSITION Next(const POSITION pos)
    {
        if (pos.pNode && (pos.pNode->next != &m_anchor))
        {
            return POSITION(pos.pNode->next);
        }
        else
        {
            return POSITION(NULL);
        }
    }
};