//-----------------------------------------------------------------------------
// File: TinyMap.h
// Desc: Simple associative container class.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

// NOTES:
// The TinyMap class is designed to hold a small-ish number of elements where
// look-up performance is not important. (It uses a sorted linked list.)
// 
// TinyMap uses "copy semantics" (keys and values are copied into the map).
// Keys must support the comparison operators.

#pragma once
#include "linklist.h"

namespace MediaFoundationSamples
{

    template <class Key, class Value>
    struct Pair
    {
        Key key;
        Value value;

        Pair() {}
        Pair(Key k, Value v)
        {
            key = k;
            value = v;
        }
    };

    template <class Key, class Value>
    class TinyMap : List< Pair<Key, Value> >
    {
    protected:

        typedef Pair<Key, Value> pair_type;

    public:

        TinyMap()
        {
            Clear();
        }
        virtual ~TinyMap()
        {
        }

        HRESULT Insert(Key k, Value v)
        {
            HRESULT hr = S_OK;

            Node *pNode = Front();
            while (TRUE)
            {
                if (pNode == &m_anchor)
                {
                    // Reached the end of the list. Insert at end.
                    hr = InsertBack(pair_type(k, v));
                    break;
                }
                else if (pNode->item.key == k)
                {
                    // Found a duplicate item. Fail.
                    hr = MF_E_INVALID_KEY; 
                    break;
                }
                else if (pNode->item.key > k)
                {
                    // Found a larger key. Insert before this item.
                    hr = InsertAfter(pair_type(k, v), pNode->prev);
                    break;
                }
                pNode = pNode->next;
            }
            return hr;
        }


        HRESULT Remove(Key k)
        {
            HRESULT hr = E_FAIL;

            Node *pNode = Front();
            Node *pToRemove = NULL;

            // Delete the nodes
            while (TRUE)
            {
                if (pNode == &m_anchor)
                {
                    // Reached the end of the list. 
                    break;
                }
                else if (pNode->item.key == k)
                {
                    // Found the node to remove.
                    pToRemove = pNode; 
                    break;
                }
                else if (pNode->item.key > k)
                {
                    // Found a larger key. The item is not on the list.
                    hr = MF_E_INVALID_KEY;
                    break;
                }
                pNode = pNode->next;
            }

            if (pToRemove)
            {
                hr = RemoveItem(pToRemove, NULL);
            }

            return hr;
        }

        // Find: Search the map for "k" and return the value in pv.
        // pv can be NULL if you don't want to get the value back.
        HRESULT Find(Key k, Value *pv)
        {
            HRESULT hr = S_OK;
            BOOL bFound = FALSE;

            pair_type pair;

            POSITION pos = List<pair_type>::FrontPosition();

            while (pos != List<pair_type>::EndPosition())
            {
                hr = GetItemPos(pos, &pair);

                if (FAILED(hr))
                {
                    break;
                }

                if (pair.key == k)
                {
                    // Found a match
                    if (pv)
                    {
                        *pv = pair.value; 
                    }
                    bFound = TRUE;
                    break;
                }

                if (pair.key > k)
                {
                    // Reached a larger key. Item not found.
                    break;
                }

                pos = List<pair_type>::Next(pos);
            }
            return (bFound ? S_OK : MF_E_INVALID_KEY);
        }

        void Clear()
        {
            List<pair_type>::Clear();
        }

        // ClearValues
        // Clear the list, using a defined function to free the values.
        //
        // clear_fn: Functor object whose operator() frees the *values* on the list.
        //
        // NOTE: This function assumes that the keys do not require special handling.

        template <class FN>
        void ClearValues(FN& clear_fn)
        {
            Node *n = m_anchor.next;

            // Delete the nodes
            while (n != &m_anchor)
            {
                clear_fn(n->item.value);

                Node *tmp = n->next;
                delete n;
                n = tmp;
            }

            // Reset the anchor to point at itself
            m_anchor.next = &m_anchor;
            m_anchor.prev = &m_anchor;

            m_count = 0;
        }

        DWORD GetCount() const 
        {
            return List<pair_type>::GetCount();
        }

    
        ////////// Enumeration methods //////////

        // Object for enumerating the list.
        class MAPPOS
        {
            friend class TinyMap;

            typedef List<pair_type>::POSITION LISTPOS;

        public:
            MAPPOS() 
            {
            }

            bool operator==(const MAPPOS &p) const
            {
                return pos == p.pos;
            }

            bool operator!=(const MAPPOS &p) const
            {
                return pos != p.pos;
            }

        private:
            LISTPOS pos;

            MAPPOS(LISTPOS p) : pos(p) 
            {
            }
        };


        MAPPOS FrontPosition()
        {
            return MAPPOS( List<pair_type>::FrontPosition() );
        }

        MAPPOS EndPosition() const
        {
            return MAPPOS( List<pair_type>::EndPosition() );
        }

        HRESULT GetValue(MAPPOS vals, Value *ppItem)
        {  
            HRESULT hr = S_OK;

            pair_type pair;

            hr = List<pair_type>::GetItemPos(vals.pos, &pair);

            if (SUCCEEDED(hr))
            {
                *ppItem = pair.value;
            }

            return hr;
        }


        HRESULT GetKey(MAPPOS vals, Key *ppItem)
        {  
            HRESULT hr = S_OK;

            pair_type pair;

            hr = List<pair_type>::GetItemPos(vals.pos, &pair);

            if (SUCCEEDED(hr))
            {
                *ppItem = pair.key;
            }

            return hr;
        }

        MAPPOS Next(const MAPPOS vals)
        {
            return MAPPOS( List<pair_type>::Next( vals.pos ) );
        }


    };

} // namespace MediaFoundationSamples
