//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994-2000 By Microsoft Corporation.
//
// @doc
//
// @module LIST.H
//
//-----------------------------------------------------------------------------------

#ifndef _LIST_H_
#define _LIST_H_
			
/////////////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////////////
typedef void* POSITION;

typedef ULONG_PTR	INDEX;
const ULONG_PTR		INVALID_INDEX	= (ULONG_PTR)MAX_PTR;
const ULONG_PTR		MAX_BLOCK_SIZE	= 20;


/////////////////////////////////////////////////////////////////////////////
// CNode
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> class CNode
{
public:
	// constructors
	CNode(CNode* pPrevNode, CNode* pNextNode);
	virtual ~CNode();

	// members
	TYPE     m_data;        // element data
	CNode*   m_pNext;		// next CNode
	CNode*   m_pPrev;		// prev CNode
};


/////////////////////////////////////////////////////////////////////////////
// CNode::CNode
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> CNode<TYPE>::CNode(CNode* pPrevNode, CNode* pNextNode)
{
	//Constructor
	m_pPrev = pPrevNode;
	m_pNext = pNextNode;

	//NOTE:  The constructor doesn't have an element passed in for
	//data.  This is so we don't have a copy of the parameter and then
	//need to copy it for assignment...
}

/////////////////////////////////////////////////////////////////////////////
// CNode::~CNode
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> CNode<TYPE>::~CNode()
{
	//Destructor
	m_pPrev = NULL;
	m_pNext = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CList
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> class CList
{
public:

	//constructors
	CList();
	virtual ~CList();

	//members
private:
	//data
	CNode<TYPE>*		m_pNodeHead;					// Head of CList
	CNode<TYPE>*		m_pNodeTail;					// Tail of CList

	ULONG_PTR			m_ulElements;					// Elements in the list
	
public:
	//list modifying operations
	virtual POSITION	AddHead(ARG_TYPE element);		// Add to Head
	virtual POSITION	AddTail(ARG_TYPE element);		// Add to Tail

	virtual POSITION	InsertBefore(POSITION position, ARG_TYPE element);	// Add before position
	virtual POSITION	InsertAfter(POSITION position, ARG_TYPE element);	// Add after position

	virtual TYPE		RemoveHead();					// Remove from Head
	virtual TYPE		RemoveTail();					// Remove from Tail
	virtual void		RemoveAt(POSITION position);	// RemoveAt position
	virtual void		RemoveAll();					// Remove all elements

	//Seeking methods
//	virtual POSITION	Find(ARG_TYPE element);	        // Find element

	//Peek methods
	virtual POSITION	GetHeadPosition()	const;		// Head Position
	virtual POSITION	GetTailPosition()	const;		// Tail Position

	virtual TYPE		GetHead()	const;				// Head element
	virtual TYPE&		GetHead();						// Head element
	virtual TYPE		GetTail()	const;				// Tail element
	virtual TYPE&		GetTail();						// Tail element

	virtual TYPE		GetNext(POSITION& position)	const;	// Next element
	virtual TYPE&		GetNext(POSITION& position);		// Next element
	virtual TYPE		GetPrev(POSITION& position)	const;	// Prev element
	virtual TYPE&		GetPrev(POSITION& position);		// Prev element

	//Data methods
	virtual TYPE		GetAt(POSITION position) const;				//Get element value
	virtual TYPE&		GetAt(POSITION position);					//Get element value
	virtual void		SetAt(POSITION position, ARG_TYPE element);	//Set element value

	//Array-like methods
	virtual POSITION	FindIndex(ULONG_PTR iIndex) const;		//Index element

	//informational methods
	virtual BOOL		IsEmpty()	const;				// IsEmpty
	virtual ULONG_PTR	GetCount()	const;				// Elements in the list
};


/////////////////////////////////////////////////////////////////////////////
// CList::CList
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> CList<TYPE, ARG_TYPE>::CList()
{
	//constructor
	m_pNodeHead		= NULL;
	m_pNodeTail		= NULL;
	m_ulElements	= 0;
}

/////////////////////////////////////////////////////////////////////////////
// CList::~CList
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> CList<TYPE, ARG_TYPE>::~CList() 
{
	//Remove all elements
	RemoveAll();
}


/////////////////////////////////////////////////////////////////////////////
// CList::AddHead
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> POSITION CList<TYPE, ARG_TYPE>::AddHead(ARG_TYPE element) 
{
	//Add to the Head of the CList, (stack)
	CNode<TYPE>* pNewNode = new CNode<TYPE>(NULL, m_pNodeHead);
	pNewNode->m_data = element;

	//If there was a list hook the head->prev to the new head
	if(m_pNodeHead) 
		m_pNodeHead->m_pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;

	m_pNodeHead = pNewNode;
	m_ulElements++;
	return pNewNode;
}


/////////////////////////////////////////////////////////////////////////////
// CList::AddTail
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> POSITION CList<TYPE, ARG_TYPE>::AddTail(ARG_TYPE element) 
{
	//Add to the m_pNodeTail of the CList
	CNode<TYPE>* pNewNode = new CNode<TYPE>(m_pNodeTail, NULL);
	pNewNode->m_data = element;

	if(m_pNodeTail != NULL)
		m_pNodeTail->m_pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	
	m_pNodeTail = pNewNode;
	m_ulElements++;
	return pNewNode;
}



/////////////////////////////////////////////////////////////////////////////
// CList::GetHeadPosition
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline POSITION CList<TYPE, ARG_TYPE>::GetHeadPosition() const
{
	//return Head element Position
	return m_pNodeHead;
}

/////////////////////////////////////////////////////////////////////////////
// CList::GetTailPosition
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline POSITION CList<TYPE, ARG_TYPE>::GetTailPosition() const
{
	//return Tail element Position
	return m_pNodeTail;
}

/////////////////////////////////////////////////////////////////////////////
// CList::GetHead
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline TYPE CList<TYPE, ARG_TYPE>::GetHead() const
{
	//return Head element value
	ASSERT(m_pNodeHead);
	return m_pNodeHead->m_data;
}

/////////////////////////////////////////////////////////////////////////////
// CList::GetHead
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline TYPE& CList<TYPE, ARG_TYPE>::GetHead()
{
	//return Head element value
	ASSERT(m_pNodeHead);
	return m_pNodeHead->m_data;
}

/////////////////////////////////////////////////////////////////////////////
// CList::GetTail
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline TYPE CList<TYPE, ARG_TYPE>::GetTail() const
{
	// return Tail element value
	ASSERT(m_pNodeTail);
	return m_pNodeTail->m_data;
}


/////////////////////////////////////////////////////////////////////////////
// CList::GetTail
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline TYPE& CList<TYPE, ARG_TYPE>::GetTail()
{
	// return Tail element value
	ASSERT(m_pNodeTail);
	return m_pNodeTail->m_data;
}

/////////////////////////////////////////////////////////////////////////////
// CList::GetNext
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline TYPE CList<TYPE, ARG_TYPE>::GetNext(POSITION& position) const
{
	ASSERT(position);

	//Set position to the next element
	CNode<TYPE>* pNode = (CNode<TYPE>*)position;
	position = pNode->m_pNext;

	//return the current element
	return pNode->m_data;
}


/////////////////////////////////////////////////////////////////////////////
// CList::GetNext
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline TYPE& CList<TYPE, ARG_TYPE>::GetNext(POSITION& position)
{
	ASSERT(position);

	//Set position to the next element
	CNode<TYPE>* pNode = (CNode<TYPE>*)position;
	position = pNode->m_pNext;

	//return the current element
	return pNode->m_data;
}


/////////////////////////////////////////////////////////////////////////////
// CList::GetPrev
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline TYPE CList<TYPE, ARG_TYPE>::GetPrev(POSITION& position) const
{
	ASSERT(position);
	
	//Set position to the next element
	CNode<TYPE>* pNode = (CNode<TYPE>*)position;
	position = pNode->m_pPrev;
	
	//return the current element
	return pNode->m_data;
}


/////////////////////////////////////////////////////////////////////////////
// CList::GetPrev
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline TYPE& CList<TYPE, ARG_TYPE>::GetPrev(POSITION& position) 
{
	ASSERT(position);
	
	//Set position to the next element
	CNode<TYPE>* pNode = (CNode<TYPE>*)position;
	position = pNode->m_pPrev;
	
	//return the current element
	return pNode->m_data;
}


/////////////////////////////////////////////////////////////////////////////
// CList::GetAt
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline TYPE CList<TYPE, ARG_TYPE>::GetAt(POSITION position) const
{
	ASSERT(position);
	CNode<TYPE>* pNode = (CNode<TYPE>*)position;
	return pNode->m_data;
}

/////////////////////////////////////////////////////////////////////////////
// CList::GetAt
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline TYPE& CList<TYPE, ARG_TYPE>::GetAt(POSITION position)
{
	ASSERT(position);
	CNode<TYPE>* pNode = (CNode<TYPE>*)position;
	return pNode->m_data;
}

/////////////////////////////////////////////////////////////////////////////
// CList::SetAt
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline void CList<TYPE, ARG_TYPE>::SetAt(POSITION position, ARG_TYPE element)
{
	ASSERT(position);

	//Save the old data
	CNode<TYPE>* pNode = (CNode<TYPE>*)position;

	//Store new data
	pNode->m_data = element;
}


/////////////////////////////////////////////////////////////////////////////
// CList::Find
//
/////////////////////////////////////////////////////////////////////////////
/*
template <class TYPE, class ARG_TYPE> POSITION CList<TYPE, ARG_TYPE>::Find(ARG_TYPE element) 
{
	//return pointer to found element
	for(CNode<TYPE>* p = m_pNodeHead; p != NULL; p = p->m_pNext)
	  if(p->m_data == element)
		return p;   // return position to found CNode

	return NULL;  // return NULL if not found
}
*/

/////////////////////////////////////////////////////////////////////////////
// CList::IsEmpty
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline BOOL CList<TYPE, ARG_TYPE>::IsEmpty() const
{
	// returns TRUE if Empty
	return m_ulElements==0;
}



/////////////////////////////////////////////////////////////////////////////
// CList::RemoveHead
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> TYPE CList<TYPE, ARG_TYPE>::RemoveHead() 
{
	//Remove and return from the Head of the List
	ASSERT(m_pNodeHead);

	CNode<TYPE>* pOldHead = m_pNodeHead;	// pointer to the Removed node
	TYPE element = GetHead();				//make a copy, before deleteing

	m_pNodeHead = pOldHead->m_pNext;	// reroute Head to exclude the first element
	if(m_pNodeHead)
	{
		ASSERT(m_pNodeTail);
		m_pNodeHead->m_pPrev = NULL;
	}
	else
	{
		m_pNodeTail = NULL;
	}

	m_ulElements--;
	delete pOldHead;						// delete head
	return element;
}


/////////////////////////////////////////////////////////////////////////////
// CList::RemoveTail
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> TYPE CList<TYPE, ARG_TYPE>::RemoveTail() 
{
	//Remove and return from the m_pNodeTail of the CList
	ASSERT(m_pNodeTail);

	CNode<TYPE>* pOldTail = m_pNodeTail;
	TYPE element = GetTail();  //make a copy before deleteing

	m_pNodeTail = pOldTail->m_pPrev;
	if(m_pNodeTail)
	{
		ASSERT(m_pNodeHead);
		m_pNodeTail->m_pNext = NULL;
	}
	else
	{
		m_pNodeHead = NULL;
	}

	m_ulElements--;
	delete pOldTail;
	return element;
}


/////////////////////////////////////////////////////////////////////////////
// CList::RemoveAt
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> void CList<TYPE, ARG_TYPE>::RemoveAt(POSITION position)
{
	//Remove CList[position]
	ASSERT(position);
	CNode<TYPE>* pOldNode = (CNode<TYPE>*)position;

	// If removing the head
	if(pOldNode == m_pNodeHead)
	{
		m_pNodeHead = pOldNode->m_pNext;
	}
	else
	{
		pOldNode->m_pPrev->m_pNext = pOldNode->m_pNext;
	}

	if (pOldNode == m_pNodeTail)
	{
		m_pNodeTail = pOldNode->m_pPrev;
	}
	else
	{
		pOldNode->m_pNext->m_pPrev = pOldNode->m_pPrev;
	}

	m_ulElements--;
	delete pOldNode;
}


/////////////////////////////////////////////////////////////////////////////
// CList::RemoveAll
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> void CList<TYPE, ARG_TYPE>::RemoveAll() 
{
	// Remove all items from the CList
	CNode<TYPE>* pNode = m_pNodeHead;
	while(pNode)
	{
		CNode<TYPE>* pTemp = pNode;
		pNode = pNode->m_pNext;
		delete pTemp;
	}

	m_pNodeHead   = NULL;
	m_pNodeTail   = NULL;
	m_ulElements    = 0;
}


/////////////////////////////////////////////////////////////////////////////
// CList::GetCount
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> inline ULONG_PTR CList<TYPE, ARG_TYPE>::GetCount() const
{
	// return the Length
	return m_ulElements;
}

				   
/////////////////////////////////////////////////////////////////////////////
// CList::InsertBefore
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> POSITION CList<TYPE, ARG_TYPE>::InsertBefore(POSITION position, ARG_TYPE element)
{
	//insert before the position
	if(position == m_pNodeHead)    // Add before Head
	  return AddHead(element);

	CNode<TYPE>* pOldNode = (CNode<TYPE>*)position;

	//otherwise a little more difficult
	CNode<TYPE>* pNewNode = new CNode<TYPE>(pOldNode->m_pPrev, pOldNode);
	pNewNode->m_data = element;
	
	//Hook up before after nodes to it
	if(pOldNode->m_pPrev)
	{
		pOldNode->m_pPrev->m_pNext = pNewNode;
	}
	else
	{
		m_pNodeHead = pNewNode;
	}

	m_ulElements++;
	return pNewNode;
}



/////////////////////////////////////////////////////////////////////////////
// CList::InsertAfter
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> POSITION CList<TYPE, ARG_TYPE>::InsertAfter(POSITION position, ARG_TYPE element)
{
	//insert after the position
	if(position == m_pNodeTail)     // Add after the m_pNodeTail
	  return AddTail(element);
	
	CNode<TYPE>* pOldNode = (CNode<TYPE>*)position;

	//other wise a little more difficult
	CNode<TYPE>* pNewNode = new CNode<TYPE>(pOldNode, pOldNode->m_pNext);
	pNewNode->m_data = element;

	//Hook up before after nodes to it
	if(pOldNode->m_pNext)
	{
		pOldNode->m_pNext->m_pPrev = pNewNode;
	}
	else
	{
		m_pNodeTail = pNewNode;
	}

	m_ulElements++;
	return pNewNode;
}


/////////////////////////////////////////////////////////////////////////////
// CList::FindIndex
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE, class ARG_TYPE> POSITION CList<TYPE, ARG_TYPE>::FindIndex(ULONG_PTR iIndex) const
{
	ASSERT(iIndex>=0 && iIndex<m_ulElements);

	CNode<TYPE>* pNode = m_pNodeHead;

	//Find the specified index
	while(iIndex--)
		pNode = pNode->m_pNext;

	return (POSITION)pNode;
}




/////////////////////////////////////////////////////////////////////////////
// CVector
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> class CVector
{
 public:

	//constructors
	CVector(ULONG_PTR ulLowerBound = 0, ULONG_PTR ulInitSize = MAX_BLOCK_SIZE);
	virtual ~CVector();

	//array modifying operations
	virtual void		AddElement(TYPE element);
	virtual TYPE		RemoveElement(TYPE element);
	virtual void		RemoveAll();

	//insertion deletion routines
	virtual void		InsertAt(INDEX iIndex, TYPE element);	// Insert element at index
	virtual TYPE		RemoveAt(INDEX iIndex);					// Remove element at index
	
	//Seeking methods
	virtual INDEX		FindElement(TYPE element);				// Find index of element

	//Array-like methods
	virtual TYPE&		operator[](INDEX iIndex);				//Index element

	//informational methods
	virtual ULONG_PTR		GetCount();								// Elements in the list

private:
	//data
	ULONG_PTR			m_ulMaxSize;							// MAX sizeof array
	ULONG_PTR			m_ulLowerBound;							// LowerBound of array
	ULONG_PTR			m_cElements;							// Number of elements
	TYPE*				m_rgElements;							// Array of elements
};


/////////////////////////////////////////////////////////////////////////////
// CVector::CVector
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> CVector<TYPE>::CVector(ULONG_PTR ulLowerBound, ULONG_PTR ulInitSize)
{
	//construct the array
	m_ulLowerBound = ulLowerBound;
	m_ulMaxSize = ulInitSize;
	
	m_cElements = 0;
	m_rgElements = (TYPE*)CoTaskMemAlloc(m_ulMaxSize * sizeof(TYPE));
}


/////////////////////////////////////////////////////////////////////////////
// CVector::~CVector
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> CVector<TYPE>::~CVector() 
{
	RemoveAll();

	//Delete the Array
	CoTaskMemFree(m_rgElements);
}


/////////////////////////////////////////////////////////////////////////////
// CVector::RemoveAll
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> void CVector<TYPE>::RemoveAll() 
{
	m_cElements = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CVector::FindElement
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> INDEX CVector<TYPE>::FindElement(TYPE element)
{
	//return pointer to found element
	for(ULONG_PTR i=0; i<m_cElements; i++)
	  if(m_rgElements[i] == element)
		return i + m_ulLowerBound;   // return position to found element

	return INVALID_INDEX;
}


/////////////////////////////////////////////////////////////////////////////
// CVector::RemoveElement
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> TYPE CVector<TYPE>::RemoveElement(TYPE element)
{
	//Try to find the index of the element
	INDEX iIndex = FindElement(element);
	
	if(iIndex == INVALID_INDEX)
		return NULL;

	//Deletgate to our RemoveAt method
	return RemoveAt(iIndex);
}

/////////////////////////////////////////////////////////////////////////////
// CVector::RemoveAt
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> TYPE CVector<TYPE>::RemoveAt(INDEX iIndex)
{
	//Remove CVector[position]
	iIndex -= m_ulLowerBound;
	ASSERT(iIndex < m_cElements);

	//Save the old element
	TYPE oldElement = m_rgElements[iIndex];

	//May Need to shift the entire array after this element up
	//If it is not the last row in the vector
	if(iIndex < m_cElements)
		memmove(m_rgElements + iIndex, m_rgElements + iIndex + 1, (size_t)((m_cElements-iIndex) * sizeof(TYPE)));

	m_cElements--;
	return oldElement;
}


/////////////////////////////////////////////////////////////////////////////
// CVector::GetCount
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> inline ULONG_PTR CVector<TYPE>::GetCount() 
{
	// return the number of elements
	return m_cElements;
}

/////////////////////////////////////////////////////////////////////////////
// CVector::AddElement
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> void CVector<TYPE>::AddElement(TYPE element)
{
	//Add an element to the end of the list

	//Just delegate out to our InsertAt method, since we may need
	//to enlarge the buffer size...
	InsertAt(m_cElements + m_ulLowerBound, element);
}

/////////////////////////////////////////////////////////////////////////////
// CVector::InsertAt
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> void CVector<TYPE>::InsertAt(INDEX iIndex, TYPE element)
{
	iIndex -= m_ulLowerBound;
	ASSERT(iIndex <= m_cElements); //Could be adding at end

	//May need to alloc more room (if needed)
	if(m_cElements == m_ulMaxSize)
	{
		m_ulMaxSize += MAX_BLOCK_SIZE;
		m_rgElements = (TYPE*)CoTaskMemRealloc(m_rgElements, m_ulMaxSize * sizeof(TYPE));
	}

	//May Need to shift the entire array after this element down
	//If not inserting at the end of the list
	if(iIndex < m_cElements)
		memmove(m_rgElements + iIndex, m_rgElements + iIndex + 1, (size_t)((m_cElements-iIndex) * sizeof(TYPE)));

	//Set new value
	m_rgElements[iIndex] = element;
	m_cElements++;
}



/////////////////////////////////////////////////////////////////////////////
// CVector::operator[]
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> TYPE& CVector<TYPE>::operator[](INDEX iIndex)
{
	iIndex -= m_ulLowerBound;

	ASSERT(iIndex < m_cElements);
	return m_rgElements[iIndex];
}





/////////////////////////////////////////////////////////////////////////////
// CAssoc
//
/////////////////////////////////////////////////////////////////////////////
template <class KEY, class VALUE> class CAssoc
{
public:
	// constructors
	CAssoc();
	virtual ~CAssoc();

	// members
	KEY		  m_key;		// key
	VALUE     m_value;      // element data
};


/////////////////////////////////////////////////////////////////////////////
// CAssoc::CAssoc
//
/////////////////////////////////////////////////////////////////////////////
template <class KEY, class VALUE> CAssoc<KEY, VALUE>::CAssoc()
{
	//Constructor
//	m_key		= key;
//	m_value		= value;      // element data

	//NOTE:  The constructor doesn't have an element passed in for
	//data.  This is so we don't have a copy of the parameter and then
	//need to copy it for assignment...
}

/////////////////////////////////////////////////////////////////////////////
// CAssoc::~CAssoc
//
/////////////////////////////////////////////////////////////////////////////
template <class KEY, class VALUE> CAssoc<KEY, VALUE>::~CAssoc()
{
}


/////////////////////////////////////////////////////////////////////////////
// CMap
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CMap
{
public:
	// Construction
	CMap();
	virtual ~CMap();

	// number of elements
	ULONG_PTR	GetCount();
	BOOL		IsEmpty();

	// Lookup
	BOOL		Lookup(ARG_KEY key, VALUE& rValue);

	// add a new (key, value) pair
	void		SetAt(ARG_KEY key, ARG_VALUE newValue);

	// removing existing (key, ?) pair
	BOOL		RemoveKey(ARG_KEY key);
	void		RemoveAll();

	// iterating all (key, value) pairs
	POSITION	GetStartPosition();
	void		GetNextAssoc(POSITION& rNextPosition, KEY& rKey, VALUE& rValue);
	void		GetNextAssoc(POSITION& rNextPosition, VALUE& rValue);

// Implementation
protected:
	POSITION			GetPosition(ARG_KEY key);
	CAssoc<KEY, VALUE>*	AssocFromPos(POSITION);
	
	CList<CAssoc<KEY, VALUE>*, CAssoc<KEY, VALUE>*>  m_listValues;
};


/////////////////////////////////////////////////////////////////////////////
// CMap
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CMap()
{
}

/////////////////////////////////////////////////////////////////////////////
// ~CMap
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::~CMap()
{
	m_listValues.RemoveAll();
}


/////////////////////////////////////////////////////////////////////////////
// CMap::GetPosition
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
POSITION CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetPosition(ARG_KEY key)
{
	//Find this "key" in the list...
	//NOTE:  This is an extremly slow process for a CMap class.
	//This is a linear search to just make the code simple and quick,
	//This really should be a hashtable lookup
	POSITION pos = m_listValues.GetHeadPosition();
	while(pos)
	{
		if(AssocFromPos(pos)->m_key == key)
			return pos;
		m_listValues.GetNext(pos);
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CMap::AssocFromPos
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE> 
CAssoc<KEY, VALUE>* CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::AssocFromPos(POSITION pos)
{
	if(!pos)
		return NULL;

	CNode<CAssoc<KEY, VALUE>*>* pNode = (CNode<CAssoc<KEY, VALUE>*>*)pos;
	return pNode->m_data;
}

/////////////////////////////////////////////////////////////////////////////
// CMap::GetStartPosition
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
POSITION CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetStartPosition()
{
	return m_listValues.GetHeadPosition();
}

/////////////////////////////////////////////////////////////////////////////
// CMap::GetCount
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
ULONG_PTR CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetCount() 
{
	return m_listValues.GetCount();
}

/////////////////////////////////////////////////////////////////////////////
// CMap::IsEmpty
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::IsEmpty() 
{
	return m_listValues.IsEmpty();
}

/////////////////////////////////////////////////////////////////////////////
// CMap::Lookup
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::Lookup(ARG_KEY key, VALUE& rValue)
{
	POSITION pos = GetPosition(key);
	if(!pos)
		return FALSE;
	
	rValue = AssocFromPos(pos)->m_value;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMap::RemoveAll
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveAll()
{
	m_listValues.RemoveAll();
}


/////////////////////////////////////////////////////////////////////////////
// CMap::RemoveKey
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveKey(ARG_KEY key)
// remove key - return TRUE if removed
{
	//Find the key in the List...
	POSITION pos = GetPosition(key);
	if(pos == NULL)
		return FALSE;
	
	//Remove this Node...
	m_listValues.RemoveAt(pos);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMap::SetAt
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::SetAt(ARG_KEY key, ARG_VALUE value)
{
	//Try to find this key...
	POSITION pos = GetPosition(key);
	if(pos)
	{
		//Found already, just change the value...
		AssocFromPos(pos)->m_value = value;
	}	
	else
	{
		//Not found, just need to add to the list...
		CAssoc<KEY, VALUE>* pCAssoc = new CAssoc<KEY, VALUE>;
		pCAssoc->m_key = key;
		pCAssoc->m_value = value;

		//Add it to the list...
		m_listValues.AddTail(pCAssoc);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMap::GetNextAssoc
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetNextAssoc(POSITION& rNextPosition,
	KEY& rKey, VALUE& rValue) 
{
	if(rNextPosition)
	{
		CAssoc<KEY, VALUE>* pCAssoc = AssocFromPos(rNextPosition);
		rKey = pCAssoc->m_key;
		rValue = pCAssoc->m_value;

		//Update the position to the next item, for iteration...
		m_listValues.GetNext(rNextPosition);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMap::GetNextAssoc
//
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetNextAssoc(POSITION& rNextPosition, VALUE& rValue) 
{
	if(rNextPosition)
	{
		CAssoc<KEY, VALUE>* pCAssoc = AssocFromPos(rNextPosition);
		rValue = pCAssoc->m_value;

		//Update the position to the next item, for iteration...
		m_listValues.GetNext(rNextPosition);
	}
}


#endif //_LIST_H_
