//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @module hashtbl.h | Class Definitions for CHashTbl Class and 
// miscellaneous bookmark functions
//
//
#ifndef _HASHTBL_H_
#define _HASHTBL_H_
#include "bitarray.h"

#define SIZE_64K			   65535			// Actually 64*1024-1


// This defines the data as stored within the row buffer.
// Each row has columns laid out sequentially.
// Use 'offsetof' when doing pointer addition.
// Note that it is important to align these.
// Suggest ensuring quadword alignment for double and __int64.

typedef struct _COLUMNDATA {
	DBLENGTH	uLength;	// length of data (not space allocated)
	DWORD		dwStatus;	// status of column
	BYTE		bData[1];	// data here and beyond
} COLUMNDATA, *PCOLUMNDATA;

// This is the layout of a row.
// Note the unique arrangement of the hash chain pointers 
// inside the row itself.
// Note also that the structure for columns is defined, and
// each row contains an array of columns.
// Bookmarks are named separately from ColumnData, for clarity
// of usage.  The layout directly matches COLUMNDATA, however.
// There are asserts which enforce this.
typedef struct tagRowBuff
{
	DBREFCOUNT  ulRefCount;		// reference count of outstanding handles
	ULONG       irowbuffNext;	// next row in bookmark hash chain
	USHORT      wBmkHash;		// hash value (redundant)
	DBCOUNTITEM	pbBmk;			// ptr  to bookmark
	DBBKMARK    cbBmk;			// (dwLength) bookmark size, in bytes
	ULONG       dwBmkStatus;	// (dwStatus) bookmark status
	DBBKMARK    dwBmk;			// (bData)    bookmark value, , maybe row count
	COLUMNDATA  cdData[1];		// Column data here and beyond (Bookmark should be here)
} ROWBUFF, *PROWBUFF;

typedef struct tagSLOT
{
	ULONG islotNext;
	ULONG islotPrev;
	ULONG cslot;
} SLOT, *PSLOT;

typedef struct tagLSTSLOT
{
	ULONG       islotFirst;
	ULONG       islotRov;
	ULONG       islotMin;
	ULONG       islotMax;
	BYTE        *rgslot;
	LPBITARRAY	pbitsSlot;	// bit array to mark active rows
	ULONG       cbExtra;
	ULONG       cbslotLeftOver;
	ULONG       cbSlot;
	ULONG       cbPage;
	ULONG       cbCommitCurrent;
	ULONG       cbCommitMax;
} LSTSLOT, *PLSTSLOT;


HRESULT GetNextSlots(PLSTSLOT plstslot,	ULONG cslot, ULONG* pislot);
VOID DecoupleSlot(PLSTSLOT plstslot, ULONG islot, PSLOT pslot);
VOID AddSlotToList(PLSTSLOT plstslot, ULONG islot, PSLOT pslot);
HRESULT ReleaseSlots(PLSTSLOT plstslot,	ULONG islot, ULONG cslot);
HRESULT InitializeSlotList(ULONG cslotMax, ULONG cbSlot, ULONG cbPage, LPBITARRAY pbits, PLSTSLOT* pplstslot, BYTE** prgslot);
HRESULT ResetSlotList(PLSTSLOT plstslot);
HRESULT ReleaseSlotList(PLSTSLOT plstslot);


#endif

