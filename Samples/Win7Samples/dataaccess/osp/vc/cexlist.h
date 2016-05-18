//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CEXLIST.H
//
//-----------------------------------------------------------------------------
#ifndef _CEXLIST_H_
#define _CEXLIST_H_
			
/////////////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////////////
typedef void* POS;


/////////////////////////////////////////////////////////////////////////////
// CNode
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> class CNode
{
public:
	// constructors
	CNode(TYPE val, CNode* pPrevNode, CNode* pNextNode);

	// members
	TYPE     m_data;       // element data
	CNode*   m_pNextNode;  // next CNode
	CNode*   m_pPrevNode;  // prev CNode
};


/////////////////////////////////////////////////////////////////////////////
// CNode::CNode
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> CNode<TYPE>::CNode(TYPE data, CNode* pPrevNode, CNode* pNextNode)
{
	//Constructor
	m_data = data;
	m_pPrevNode = pPrevNode;
	m_pNextNode = pNextNode;
}



/////////////////////////////////////////////////////////////////////////////
// CExList
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> class CExList
{
public:

	//constructors
	CExList();
	virtual ~CExList();

	//members
	
	//list modifying operations
	virtual POS		AddHead(TYPE element);		// Add to Head
	virtual POS		AddTail(TYPE element);		// Add to Tail
					
	virtual POS		InsertBefore(POS position, TYPE element);	// Add before position
	virtual POS		InsertAfter(POS position, TYPE element);	// Add after position

	virtual TYPE	RemoveHead();				// Remove from Head
	virtual TYPE	RemoveTail();				// Remove from Tail
	virtual TYPE	RemoveAt(POS position);		// RemoveAt position
	virtual void	RemoveAll();				// Remove all elements

	//Seeking methods
	virtual POS		Find(TYPE element);	        // Find element

	//Peek methods
	virtual POS		GetHeadPosition();			// Head Position
	virtual POS		GetTailPosition();			// Tail Position

	virtual TYPE	GetHead();					// Head element
	virtual TYPE	GetTail();					// Tail element
	virtual TYPE	GetNext(POS& position);		// Next element
	virtual TYPE	GetPrev(POS& position);		// Prev element

	//Data methods
	virtual TYPE	GetAt(POS position) const;			//Get element value
	virtual TYPE	SetAt(POS position, TYPE element);	//Set element value

	//Array-like methods
	virtual POS		FindIndex(ULONG iIndex);	//Index element

	//informational methods
	virtual BOOL	IsEmpty();					// IsEmpty
	virtual ULONG	GetCount();					// Elements in the list

private:
	//data
	CNode<TYPE>*	m_pHeadNode;				// Head of CExList
	CNode<TYPE>*	m_pTailNode;				// Tail of CExList
	ULONG			m_ulElements;				// Elements in the list
};


/////////////////////////////////////////////////////////////////////////////
// CExList::CExList
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> CExList<TYPE>::CExList()
{
	//constructor
	m_pHeadNode = NULL;
	m_pTailNode = NULL;
	m_ulElements = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CExList::~CExList
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> CExList<TYPE>::~CExList() 
{
	//Remove all elements
	RemoveAll();
}


/////////////////////////////////////////////////////////////////////////////
// CExList::AddHead
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> POS CExList<TYPE>::AddHead(TYPE element) 
{
	//Add to the Head of the CExList, (stack)
	CNode<TYPE>* pHeadNode = new CNode<TYPE>(element, NULL, m_pHeadNode);

	//If there was a list hook the head->prev to the new head
	if(m_pHeadNode) 
	  m_pHeadNode->m_pPrevNode = pHeadNode;

	//If there isn't a tail element, hook it to the head
	if(!m_pTailNode)
	  m_pTailNode = pHeadNode;

	m_pHeadNode = pHeadNode;
	m_ulElements++;
	return m_pHeadNode;
}


/////////////////////////////////////////////////////////////////////////////
// CExList::AddTail
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> POS CExList<TYPE>::AddTail(TYPE element) 
{
	//Add to the m_pTailNode of the CExList
	CNode<TYPE>* pTailNode = new CNode<TYPE>(element, m_pTailNode, 0);

	//if previously empty
	if(!m_pHeadNode)
		m_pHeadNode = pTailNode;
	else
		m_pTailNode->m_pNextNode = pTailNode;

	m_pTailNode = pTailNode;
	m_ulElements++;
	return m_pTailNode;
}



/////////////////////////////////////////////////////////////////////////////
// CExList::GetHeadPosition
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> inline POS CExList<TYPE>::GetHeadPosition() 
{
	//return Head element Position
	return m_pHeadNode;
}

/////////////////////////////////////////////////////////////////////////////
// CExList::GetTailPosition
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> inline POS CExList<TYPE>::GetTailPosition() 
{
	//return Tail element Position
	return m_pTailNode;
}

/////////////////////////////////////////////////////////////////////////////
// CExList::GetHead
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> inline TYPE CExList<TYPE>::GetHead() 
{
	//return Head element value
	return m_pHeadNode->m_data;
}

/////////////////////////////////////////////////////////////////////////////
// CExList::AddTail
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> inline TYPE CExList<TYPE>::GetTail() 
{
	// return Tail element value
	return m_pTailNode->m_data;
}


/////////////////////////////////////////////////////////////////////////////
// CExList::GetNext
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> inline TYPE CExList<TYPE>::GetNext(POS& position) 
{
	//Set position to the next element
	CNode<TYPE>* pNode = (CNode<TYPE>*)position;
	position = pNode->m_pNextNode;

	//return the current element
	return pNode->m_data;
}


/////////////////////////////////////////////////////////////////////////////
// CExList::GetPrev
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> inline TYPE CExList<TYPE>::GetPrev(POS& position) 
{
	//Set position to the next element
	CNode<TYPE>* pNode = (CNode<TYPE>*)position;
	position = pNode->m_pPrevNode;
	
	//return the current element
	return pNode->m_data;
}


/////////////////////////////////////////////////////////////////////////////
// CExList::GetAt
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> inline TYPE CExList<TYPE>::GetAt(POS position) const
{
	return ((CNode<TYPE>*)position)->m_data;
}

/////////////////////////////////////////////////////////////////////////////
// CExList::SetAt
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> inline TYPE CExList<TYPE>::SetAt(POS position, TYPE element)
{
	//Save the old data
	CNode<TYPE>* pNode = (CNode<TYPE>*)position;
	TYPE oldData = pNode->m_data;

	//Store new data
	pNode->m_data = element;

	//return olddata
	return oldData;
}


/////////////////////////////////////////////////////////////////////////////
// CExList::Find
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> POS CExList<TYPE>::Find(TYPE element) 
{
	//return pointer to found element
	for(CNode<TYPE>* p = m_pHeadNode; p; p = p->m_pNextNode)
	  if(p->m_data == element)
		return p;   // return position to found CNode

	return NULL;  // return NULL if not found
}


/////////////////////////////////////////////////////////////////////////////
// CExList::IsEmpty
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> inline BOOL CExList<TYPE>::IsEmpty() 
{
	// returns TRUE if Empty
	return m_ulElements == 0;
}



/////////////////////////////////////////////////////////////////////////////
// CExList::RemoveHead
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> TYPE CExList<TYPE>::RemoveHead() 
{
	//Remove and return from the Head of the List
	CNode<TYPE>* pHeadNode = m_pHeadNode;	// pointer to the Removed node
	TYPE element = GetHead();				//make a copy, before deleteing

	m_pHeadNode = pHeadNode->m_pNextNode;		// reroute Head to exclude the first element
	if(m_pHeadNode)
		m_pHeadNode->m_pPrevNode = NULL;
	else
		m_pTailNode = NULL;

	m_ulElements--;
	delete pHeadNode;						// delete head
	return element;
}


/////////////////////////////////////////////////////////////////////////////
// CExList::RemoveTail
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> TYPE CExList<TYPE>::RemoveTail() 
{
	//Remove and return from the m_pTailNode of the CExList
	CNode<TYPE>* pTailNode = m_pTailNode->m_pPrevNode;
	TYPE element = GetTail();  //make a copy before deleteing

	m_pTailNode = pTailNode;
	if(m_pTailNode)
		m_pTailNode->m_pNextNode = NULL;
	else
		m_pHeadNode = NULL;

	m_ulElements--;
	delete m_pTailNode;
	return element;
}


/////////////////////////////////////////////////////////////////////////////
// CExList::RemoveAt
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> TYPE CExList<TYPE>::RemoveAt(POS position)
{
	//Remove CExList[position]
	CNode<TYPE>* pNode = (CNode<TYPE>*)position;
	TYPE oldData = pNode->m_data;

	// If removing the head
	if (pNode == m_pHeadNode)
		m_pHeadNode = pNode->m_pNextNode;
	else
		pNode->m_pPrevNode->m_pNextNode = pNode->m_pNextNode;
	
	//If removing the tail
	if (pNode == m_pTailNode)
		m_pTailNode = pNode->m_pPrevNode;
	else
		pNode->m_pNextNode->m_pPrevNode = pNode->m_pPrevNode;

	m_ulElements--;
	delete pNode;
	return oldData;
}


/////////////////////////////////////////////////////////////////////////////
// CExList::RemoveAll
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> void CExList<TYPE>::RemoveAll() 
{
	// Remove all items from the CExList
	CNode<TYPE>* pNode = m_pHeadNode;
	while(pNode)
	{
		CNode<TYPE>* pTemp = pNode;
		pNode = pNode->m_pNextNode; 	
		delete pTemp;
	}

	m_pHeadNode   = NULL;
	m_pTailNode   = NULL;
	m_ulElements  = 0;
}


/////////////////////////////////////////////////////////////////////////////
// CExList::GetCount
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> inline ULONG CExList<TYPE>::GetCount() 
{
	// return the Length
	return m_ulElements;
}

				   
/////////////////////////////////////////////////////////////////////////////
// CExList::InsertBefore
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> POS CExList<TYPE>::InsertBefore(POS position, TYPE element)
{
	//insert before the position
	if(position == m_pHeadNode)    // Add before Head
	  return AddHead(element);

	CNode<TYPE>* pOldNode = (CNode<TYPE>*)position;

	//otherwise a little more difficult
	CNode<TYPE>* pNewNode = new CNode<TYPE>(element, pOldNode->m_pPrevNode, pOldNode);
	
	//Create the new node
	pNewNode->m_pNextNode = new CNode<TYPE>(element, pOldNode->m_pPrevNode, pOldNode->m_pNextNode);

	//Hook up before after nodes to it
	pOldNode->m_pPrevNode->m_pNextNode = pNewNode;
	pOldNode->m_pPrevNode = pNewNode;

	m_ulElements++;
	return pNewNode;
}



/////////////////////////////////////////////////////////////////////////////
// CExList::InsertAfter
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> POS CExList<TYPE>::InsertAfter(POS position, TYPE element)
{
	//insert after the position
	if(position == m_pTailNode)     // Add after the m_pTailNode
	  return AddTail(element);
	
	CNode<TYPE>* pOldNode = (CNode<TYPE>*)position;

	//other wise a little more difficult
	CNode<TYPE>* pNewNode = new CNode<TYPE>(element, pOldNode, pOldNode->m_pNextNode);
	
	//Hook up before after nodes to it
	pOldNode->m_pNextNode->m_pPrevNode = pNewNode;
	pOldNode->m_pNextNode = pNewNode;

	m_ulElements++;
	return pNewNode;
}


/////////////////////////////////////////////////////////////////////////////
// CExList::FindIndex
//
/////////////////////////////////////////////////////////////////////////////
template <class TYPE> POS CExList<TYPE>::FindIndex(ULONG iIndex)
{
	CNode<TYPE>* pNode = m_pHeadNode;

	//Find the specified index
	while(iIndex--)
		pNode = pNode->m_pNextNode;

	return (POS)pNode;
}

#endif //_LIST_H_

