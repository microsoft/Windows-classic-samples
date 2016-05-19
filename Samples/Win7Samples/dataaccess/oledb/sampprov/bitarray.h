//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module BITARRAY.H | Class Definitions for Bitarray Class
//
//
#ifndef _BITARRAY_H_
#define _BITARRAY_H_

// Forward Declaration
class FAR CBitArray;
typedef CBitArray FAR *LPBITARRAY;


//--------------------------------------------------------------------
// @class Allocates and manages a bit array through various methods 
// defined in the class
// 
// @hungarian bits or pbits
//
class FAR CBitArray
{
	private:					//@access private
		//@cmember Count of Slots
		ULONG		m_cslotCurrent;
		//@cmember Maximum number of pages 
		ULONG		m_cPageMax;
		//@cmember Current number of pages
		ULONG		m_cPageCurrent;
		//@cmember Number of bytes per page
		ULONG		m_cbPage;
		//@cmember Mask buffer
		BYTE		m_rgbBitMask[8];
		//@cmember Bit Array
		BYTE		*m_rgbBit;
		

	public:						//@access public
		//@cmember Class constructor
		CBitArray( void );
		//@cmember Class destructor
		~CBitArray( void );
		//@cmember Initialization method
		STDMETHODIMP FInit(ULONG cslotMax, ULONG cbPage);
		//@cmember Set a range of slots
		STDMETHODIMP SetSlots(ULONG islotFirst, ULONG islotLast);
		//@cmember Reset a range of slots
		STDMETHODIMP ResetSlots(ULONG islotFirst, ULONG islotLast);
		//@cmember Check if any bits are set
		STDMETHODIMP ArrayEmpty(void);
		//@cmember Check the status of a particular bit
		STDMETHODIMP IsSlotSet(ULONG islot);
		//@cmember Find the first set bit in a range of bits
		STDMETHODIMP FindSet(ULONG islotStart, ULONG islotLimit, ULONG* pislot);

};

#endif

