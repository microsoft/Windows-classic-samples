//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module EXTBUFF.H | Class Definitions for CExtBuffer Class
//
//
#ifndef _EXTBUFF_H_
#define _EXTBUFF_H_

// Forward Declaration
class FAR CExtBuffer;
typedef CExtBuffer FAR *LPEXTBUFFER;


//--------------------------------------------------------------------
// @class Allocates and manages fixed sized block memory routines
// 
// @hungarian ext or pext
//
class FAR CExtBuffer
{			
	private: //@access private		
		//@cmember Item size, in bytes
		ULONG    m_cbItem;		
		//@cmember Current count of items
		ULONG    m_cItem;		
		//@cmember Reserved byte count
		ULONG    m_cbReserved;	
		//@cmember Allocated byte count
		ULONG    m_cbAlloc;
		//@cmember increment value
		ULONG    m_dbAlloc;
		//@cmember Ptr to beginning of buffer
		BYTE     *m_rgItem;		
		

	private:
		// Not implemented; private so dcl prevents generation.
		CExtBuffer( const CExtBuffer & p);
		CExtBuffer& operator=(const CExtBuffer & p);

	public:	//@access public
		//@cmember Construcutor
		CExtBuffer ( void );
		//@cmember Destructor
		~CExtBuffer ( void );
		//@cmember Calculated data pointer from index value
		void * operator[] (DBCOUNTITEM nIndex);	
		//@cmember Initialize the fixed size buffer
		STDMETHODIMP 	FInit (ULONG cItemMax, ULONG cbItem, ULONG cbPage);
		//@cmember Add new items to the buffer
		STDMETHODIMP 	InsertIntoExtBuffer (VOID* pvItem, HACCESSOR &hItem);
		//cmember Retrieve items from buffer
		STDMETHODIMP    GetItemOfExtBuffer (HACCESSOR hItem, VOID* pvItem);
		//@cmember Get usage extent indexes
		STDMETHODIMP    GetFirstLastItemH (HACCESSOR &hItemFirst, HACCESSOR &hItemLast);
};

#endif
