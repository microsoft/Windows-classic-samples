//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1998-2000 Microsoft Corporation.  
//
// @doc 
//
// @module Allocator Header Module | 	This module contains declaration information
//			for the CAllocator class
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//---------------------------------------------------------------------------

// File alocator.hpp
#ifndef __ALLOCATOR__
#define __ALLOCATOR__

class CPropSets;
class CPropInfoSets;
class CSourceInfo;

class CAllocator
{
	public:
		static void			Free(LPVOID *pv){}
		static void			Free(DBPROP *pProp) {
			  					ReleaseDBID(&pProp->colid, FALSE);	
								VariantClear(&pProp->vValue); 
		}
		static void			Free(DBPROPSET *pPropSet) {
							Free(&pPropSet->cProperties, &pPropSet->rgProperties);
		}
		static void			Free(CPropSets *pPropSet){}
		static void			Free(CPropInfoSets *pPropInfoSets){}
		static void			Free(GUID *pGuid) {}
		static void			Free(CSourceInfo *pSourceInfo) {}

		static void			Free(ULONG *pcV, LPVOID **prgV);
		static void			Free(ULONG *pcProps, DBPROP **prgProps);
		static void			Free(ULONG *pcPropSets, DBPROPSET **prgPropSets);
		static void			Free(ULONG *pcPropSets, CPropSets **prgPropSets);
		static void			Free(ULONG *pcPropInfoSets, CPropInfoSets **prgPropInfoSets);
		static void			Free(ULONG *pcGUIDs, GUID **prgGUIDs);
		static void			Free(ULONG *pcArray, ULONG_PTR ***prgArray);
		static void			Free(ULONG *pcSourceInfo, CSourceInfo **prgSourceInfo);


		static HRESULT		Alloc(ULONG cV, LPVOID **prgV);
		static HRESULT		Alloc(ULONG cProps, DBPROP **prgProps);
		static HRESULT		Alloc(ULONG cPropSets, DBPROPSET **prgPropSets);
		static HRESULT		Alloc(ULONG cPropSets, CPropSets **prgPropSets);
		static HRESULT		Alloc(ULONG cPropInfoSets, CPropInfoSets **prgPropInfoSets);
		static HRESULT		Alloc(ULONG cGUIDs, GUID **prgGUIDs);
		static HRESULT		Alloc(ULONG cArray, ULONG_PTR ***prgArray);
		static HRESULT		Alloc(ULONG cSourceInfo, CSourceInfo **prgSourceInfo);

		static HRESULT		Realloc(ULONG cOldV, LPVOID **prgV, ULONG cV);
		static HRESULT		Realloc(ULONG cOldProps, DBPROP **prgProps, ULONG cProps);
		static HRESULT		Realloc(ULONG cOldPropSets, DBPROPSET **prgPropSets, ULONG cPropSets);
		static HRESULT		Realloc(ULONG cOldPropSets, CPropSets **prgPropSets, ULONG cPropSets);
		static HRESULT		Realloc(ULONG cOldPropInfoSets, CPropInfoSets **prgPropInfoSets, ULONG cPropInfoSets);
		static HRESULT		Realloc(ULONG cOldGUIDs, GUID **prgGUIDs, ULONG cGUIDs);
		static HRESULT		Realloc(ULONG cOldArray, ULONG_PTR ***prgArray, ULONG cArray);
		static HRESULT		Realloc(ULONG cOldSourceInfo, CSourceInfo **prgSourceInfo, ULONG cSourceInfo);
		
}; //CAllocator


#endif //__ALLOCATOR__