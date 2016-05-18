//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module UTILPROP.H | CUtilProp object definitions
//
//
#ifndef _UTILPROP_H_
#define _UTILPROP_H_


//-----------  structs and #defines -------------------------------------------

// @struct PROPSTRUCT  | simple table used to store property information. Used in 
// our read-only implementation of IDBProperties::GetPropertyInfo and IRowsetInfo::GetProperties
typedef struct _tagPROPSTRUCT
    {
    DBPROPID	dwPropertyID;
	DBPROPFLAGS dwFlags;
    VARTYPE     vtType;
    BOOL        boolVal;
#ifdef _WIN64
    LONGLONG    longVal;
#else
    SLONG       longVal;
#endif
    PWSTR       pwstrVal;
    PWSTR		pwstrDescBuffer;
    } PROPSTRUCT;
								   

// Number of supported properties per property set
#define	NUMBER_OF_SUPPORTED_PROPERTY_SETS	4

// Sart of Each PropertySet in the Static Structure
#define	START_OF_SUPPORTED_ROWSET_PROPERTIES				0

#define	START_OF_SUPPORTED_SESSION_PROPERTIES				NUMBER_OF_SUPPORTED_ROWSET_PROPERTIES

#define START_OF_SUPPORTED_DATASOURCEINFO_PROPERTIES		(NUMBER_OF_SUPPORTED_ROWSET_PROPERTIES +		\
															 NUMBER_OF_SUPPORTED_SESSION_PROPERTIES)

#define START_OF_SUPPORTED_DBINIT_PROPERTIES				(NUMBER_OF_SUPPORTED_ROWSET_PROPERTIES +		\
															 NUMBER_OF_SUPPORTED_SESSION_PROPERTIES +		\
															 NUMBER_OF_SUPPORTED_DATASOURCEINFO_PROPERTIES)

// Number of properties in each set
#define NUMBER_OF_SUPPORTED_DBINIT_PROPERTIES				 3
#define	NUMBER_OF_SUPPORTED_DATASOURCEINFO_PROPERTIES		 8
#define	NUMBER_OF_SUPPORTED_SESSION_PROPERTIES				 1
#define	NUMBER_OF_SUPPORTED_ROWSET_PROPERTIES				15
#define	NUMBER_OF_SUPPORTED_PROPERTIES						(NUMBER_OF_SUPPORTED_DBINIT_PROPERTIES +		 \
															 NUMBER_OF_SUPPORTED_DATASOURCEINFO_PROPERTIES + \
															 NUMBER_OF_SUPPORTED_SESSION_PROPERTIES +		 \
															 NUMBER_OF_SUPPORTED_ROWSET_PROPERTIES)

// description size
#define CCH_GETPROPERTYINFO_DESCRIP_BUFFER_SIZE 25

// flags for Get and Set Properties
const DWORD		PROPSET_DSO		= 0x0001;
const DWORD		PROPSET_INIT	= 0x0002;
const DWORD		PROPSET_DSOINIT = PROPSET_DSO | PROPSET_INIT;
const DWORD		PROPSET_SESSION	= 0x0004;
const DWORD		PROPSET_ROWSET	= 0x0008;

// Classes -------------------------------------------------------------------

//----------------------------------------------------------------------------
// @class CUtilProp | Containing class for all interfaces on the UtilProp 
// Object
//
class CUtilProp
{
	private:
		size_t	m_cwchNamePool;
		LPWSTR	m_pwchNamePool;	

	protected: //@access protected
		PROPSTRUCT	m_rgproperties[NUMBER_OF_SUPPORTED_PROPERTIES];
		WCHAR		m_wszFilePath[MAX_PATH];
        
		//@cmember Gets index of entry for a given property in global property table
        BOOL GetPropIndex
        	(
		       	DBPROPID	dwPropertyID,
			 	ULONG*		pulIndex
        	);

        //@cmember Loads fields of DBPROPINFO struct. Helper for GetPropertyInfo            
        void LoadDBPROPINFO
            (
				PROPSTRUCT*		pPropStruct,
				DBPROPINFO*		pPropInfo
            );

        //@cmember Loads fields of DBPROP struct. Helper for GetProperties
        HRESULT LoadDBPROP
            (
				PROPSTRUCT*	pPropStruct,
				DBPROP*		pPropSupport
            );

        //@cmember Checks to see if the value is valid. Helper for SetProperties
		HRESULT IsValidValue
			(
			DBPROP*		pDBProp
			);

	public: //@access public
		//@cmember Constructor		 
		 CUtilProp(void);
		//@cmember Destructor
		~CUtilProp(void);

		static HRESULT CUtilProp::GetPropertiesArgChk
			(
			DWORD					dwBitMask,
			const ULONG				cPropertySets,
			const DBPROPIDSET		rgPropertySets[],
			ULONG*					pcProperties,
			DBPROPSET**				prgProperties
			);

		STDMETHODIMP GetProperties
		    (
				DWORD				dwBitMask,
			    ULONG				cPropertySets,
			    const DBPROPIDSET	rgPropertySets[],
			    ULONG*              pcProperties,
			    DBPROPSET**			prgProperties
		    );

		STDMETHODIMP GetPropertyInfo
		    (
				BOOL				fDSOInitialized,
			    ULONG				cPropertySets,		
			    const DBPROPIDSET	rgPropertySets[],	
				ULONG*				pcPropertyInfoSets,	
				DBPROPINFOSET**		prgPropertyInfoSets,
				WCHAR**				ppDescBuffer		
		    );

		static HRESULT	SetPropertiesArgChk
			(	const ULONG			cProperties, 
				const DBPROPSET		rgProperties[]
			);

		STDMETHODIMP SetProperties
			(
				DWORD				dwBitMask,
				ULONG				cProperties,
				DBPROPSET			rgProperties[]
			);

};

typedef CUtilProp *PCUTILPROP;

#endif

