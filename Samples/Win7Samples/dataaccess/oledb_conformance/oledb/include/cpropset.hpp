//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1998-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CPropSets Header Module | 	This module contains declaration information
//			for the CPropInfoSets and CPropSets classes
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//---------------------------------------------------------------------------

//	File CPropSet.hpp
#ifndef	__PROPSET_HPP__
#define __PROPSET_HPP__

#include "CVectorEx.hpp"

class CPropSets;

void Ident();

const ULONG		cHTSize = 256;

// defines the structure of an entry in the hash table
struct INDEX_ENTRY{
	GUID		dbPropSet;
	DBPROPID	dbPropID;
	ULONG		ulSetIndex;
	ULONG		ulIndex;
};


class CPropInfoSets{
 protected:
	ULONG				m_cPropInfoSets;
	DBPROPINFOSET		*m_rgPropInfoSets;
	WCHAR				*m_pwszBuffer;
 
 public:
						 CPropInfoSets() : m_cPropInfoSets(0), m_rgPropInfoSets(NULL), m_pwszBuffer(NULL) {}
						 ~CPropInfoSets() {Free();}
	
	void				Free();

	DBPROPINFOSET		*pPropInfoSets(){
							return (DBPROPINFOSET*)m_rgPropInfoSets;}

	ULONG				cPropInfoSets() {
							return m_cPropInfoSets;}

	DBPROPINFOSET		&operator [] (ULONG nIndex) {
							assert(nIndex < cPropInfoSets());
							return m_rgPropInfoSets[nIndex];}

	DBPROPINFOSET		&operator [] (GUID guidPropSet); 
	CPropInfoSets		&operator = (CPropInfoSets&);

	LONG				GetSetIndex(GUID guidPropSet);
	
	HRESULT				CreatePropInfoSet(IUnknown *pIUnknown);
	

	DBPROPINFO			*FindProperty(DBPROPID PropertyID, GUID guidPropertySet);
	DBPROPINFO			*FindProperty(WCHAR *pwszDesc, GUID guidPropertySet);
	BOOL				FindProperty(WCHAR *pwszDesc, DBPROPINFO **ppPropInfo, GUID *pguidPropertySet);

	BOOL				SettableProperty(DBPROPID PropertyID, GUID guidPropertySet);
	BOOL				SupportedProperty(DBPROPID PropertyID, GUID guidPropertySet);

	// @ cmember Finds a property of a given type
	BOOL				FindProperty(
		VARTYPE			vtType,					// [in]  the type of the sought property
		DBPROPINFO		**ppPropInfo,			// [out] the property pointer
		GUID			*pguidPropSet,			// [out] the propset guid
		ULONG			cExclProp = 0,			// [in]  the number of props to be excluded 
		DBPROPID		*rgExclPropID = NULL,	// [in]  the props to be excluded
		GUID			*rgExclPropSet = NULL	// [in]  the propset of the corresponding props
	);
}; //CPropInfoSets



// Wrapper for property sets
class CPropSets{
 protected:
	CVectorExSet<DBPROPSET, DBPROP>					
							m_PropSets;

	// Hash Table

 
 public:
	CPropInfoSets			*m_pPropInfoSets; // used to search a prop based on name and to create a string

							CPropSets(ULONG ulASetSizeInc = 1, ULONG ulSizeInc = 5);
							CPropSets(ULONG cPropSets, DBPROPSET *rgPropSets) {
								Attach(cPropSets, rgPropSets);
							}
							CPropSets(CPropSets &X){
								*this = X;
							}


							~CPropSets() {Free();}
	
	void					Free();
	void					Attach(ULONG cPropSets, DBPROPSET *rgPropSets);
	void					Detach();

	// duplicate the property sets
	BOOL					Clone(DBORDINAL *pcPropSets, DBPROPSET **prgPropSets);

	DBPROPSET				*pPropertySets(){
								return (DBPROPSET*)m_PropSets.GetElements();
	}
	ULONG					cPropertySets() {
								return m_PropSets.GetCount();	
	}

	CVectorEx<DBPROP>	
							*GetSetWrapper(ULONG nIndex) {
								assert(nIndex < cPropertySets());
								return (CVectorEx<DBPROP>*)m_PropSets + nIndex;
	}

	ULONG					GetIndex(GUID guidPropSet);
	
	// SetProperty methods check whether property exists and if so they overwrite it
	HRESULT					SetProperty(DBPROPID PropertyID, GUID guidPropertySet, VARIANT *pv, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
	HRESULT					SetProperty(DBPROPID PropertyID, GUID guidPropertySet, DBTYPE wType, ULONG_PTR ulValue, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
	HRESULT					SetProperty(DBPROPID PropertyID, GUID guidPropertySet, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);

	// do not check whether the property is already in the set; if already exist it will be doubled
	HRESULT					AddProperty(DBPROPID PropertyID, GUID guidPropertySet, VARIANT *pv, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
	HRESULT					AddProperty(DBPROPID PropertyID, GUID guidPropertySet, DBTYPE wType, ULONG_PTR ulValue, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
	HRESULT					AddProperty(DBPROPID PropertyID, GUID guidPropertySet, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);

	DBPROP					*FindProperty(DBPROPID PropertyID, GUID guidPropertySet);
	DBPROPSET				*FindPropertySet(GUID guidPropertySet);

	HRESULT					RemoveProperty(DBPROPID PropertyID, GUID guidPropertySet);

	// check the property status after the operation
	// pIDSO helps identifying the supported, settable attributes of the props
	// pIUnknown if applies allows retrieving the properties from the object, for 
	//	comparison; sometimes this is not possible (i.e. column properties and table props)
	HRESULT					CheckPropertyStatus(
								HRESULT		hrObtained,		// [in] the value returned by the operation 
								IUnknown	*pIDSO,			// [in] ptr to a DSO to check supported props
								IUnknown	*pIUnknown		// [in] used to read the set properties
							);

	HRESULT					PrintPropSets();
	// converts the properties to a string (e.g. init string)
	CWString				ConvertToCWString();

	BOOL					ArePropsIncluded(DBORDINAL cPropSets, DBPROPSET *rgPropSets);

	BOOL					operator == (CPropSets &PropSets) {
								return	ArePropsIncluded(PropSets, PropSets) &&
										ArePropsIncluded(*this, *this);
	}

							operator DBPROPSET* () {
								return (DBPROPSET*)m_PropSets.GetElements();
							}
							operator ULONG () {
								return m_PropSets.GetCount();
							}

	DBPROPSET				&operator [] (ULONG nIndex) {
								assert(nIndex < cPropertySets());
								return m_PropSets[nIndex];}

	CPropSets				&operator = (CPropSets&);
}; // CPropSets - wrapper for property sets


#endif	//__PROPSET_HPP__



