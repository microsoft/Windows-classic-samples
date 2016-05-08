//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module CVTTYPE.CPP | CImpIConvertType implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"

//----------------------------------------------------------------------------
// CImpIConvertType::CanConvert
//
// @mfunc Used by consumer to determine provider support for a conversion
//
// @rdesc HRESULT indicating the status of the method
//		@flag S_OK | Conversion supported
//		@flag S_FALSE | Conversion unsupported
//		@flag DB_E_BADCONVERTFLAG | dwConvertFlags was invalid
//		@flag DB_E_BADCONVERTFLAG | called on rowset for DBCONVERTFLAG_PARAMETER
//		@flag OTHER | HRESULTS returned from support functions
//
STDMETHODIMP CImpIConvertType::CanConvert
(
	DBTYPE			wFromType,		//@parm IN | src type
	DBTYPE			wToType,		//@parm IN | dst type
	DBCONVERTFLAGS	dwConvertFlags	//@parm IN | conversion flags
)
{
	//
	// We only support conversions on Rowsets, we only allow DBCONVERTFLAGS_COLUMN
	//
	if( (dwConvertFlags & ~(DBCONVERTFLAGS_ISLONG | 
							DBCONVERTFLAGS_ISFIXEDLENGTH | 
							DBCONVERTFLAGS_FROMVARIANT)) != DBCONVERTFLAGS_COLUMN )
		return (DB_E_BADCONVERTFLAG);

	//
	// Make sure that we check that the type is a variant if they say so
	//
	if( dwConvertFlags & DBCONVERTFLAGS_FROMVARIANT )
	{
		DBTYPE	wVtType = wFromType & VT_TYPEMASK;

		//
		// Take out all of the Valid VT_TYPES (36 is VT_RECORD in VC 6)
		//
		if( (wVtType > VT_DECIMAL && wVtType < VT_I1) ||
			((wVtType > VT_LPWSTR && wVtType < VT_FILETIME) && wVtType != 36) ||
			(wVtType > VT_CLSID) )
			return (DB_E_BADTYPE);
	}

	//
	// Don't allow _ISLONG on fixed-length types
	//
	if( dwConvertFlags & DBCONVERTFLAGS_ISLONG )
		switch( wFromType & ~(DBTYPE_RESERVED|DBTYPE_VECTOR|DBTYPE_ARRAY|DBTYPE_BYREF) )
		{
		case DBTYPE_BYTES:
		case DBTYPE_STR:
		case DBTYPE_WSTR:
		case DBTYPE_VARNUMERIC:
			break;

		default:
			return (DB_E_BADCONVERTFLAG);
		}
	
	//
	// If the DC is not there, try to create it again
	if( !g_pIDataConvert )
	{
		CoCreateInstance(CLSID_OLEDB_CONVERSIONLIBRARY,
								NULL,
								CLSCTX_INPROC_SERVER,
								IID_IDataConvert,
								(void**)&g_pIDataConvert);

		//
		// Tell the DC that we are 2.5
		//
		if( g_pIDataConvert )
		{
			DCINFO rgInfo[] = {{DCINFOTYPE_VERSION,{VT_UI4, 0, 0, 0, 0x0}}};
			IDCInfo	*pIDCInfo = NULL;

			if( g_pIDataConvert->QueryInterface(IID_IDCInfo, (void **)&pIDCInfo) == S_OK && 
				pIDCInfo )
			{
				// OLE DB Version 02.50
				V_UI4(&rgInfo->vData) = 0x250;
				pIDCInfo->SetInfo(NUMELEM(rgInfo),rgInfo);
				pIDCInfo->Release();
			}
		}
		else
			return (S_FALSE);
	}

	//
	// Ask the conversion library for the answer
	//
	return (g_pIDataConvert->CanConvert(wFromType, wToType));
}
