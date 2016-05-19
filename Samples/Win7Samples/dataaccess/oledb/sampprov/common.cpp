//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module COMMON.CPP | common global functions
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"


// Code ----------------------------------------------------------------------

// IsEqualDBID ---------------------------------------------------------------
//
// @mfunc Checks equality of two DBIDs
//
// @rdesc 
//      @flag  TRUE  | DBID's are the same
//      @flag  FALSE | DBID's differ
//      
BOOL IsEqualDBID
	(
	const DBID * px,
	const DBID * py
	)
{
	if(!px || !py)
		return FALSE;
	
	if(px->eKind == py->eKind)
	{
		switch(px->eKind)
		{
			case DBKIND_GUID_NAME:
				if ((px->uGuid.guid==py->uGuid.guid) &&
				   px->uName.pwszName==NULL && 
				   py->uName.pwszName==NULL)
				   return TRUE;

				if ((px->uGuid.guid==py->uGuid.guid) &&
					wcscmp((px->uName.pwszName),(py->uName.pwszName))==0 )
					return TRUE;
				break;

			case DBKIND_GUID_PROPID:
				if ((px->uGuid.guid == py->uGuid.guid)&&
					(px->uName.ulPropid == py->uName.ulPropid))
					return TRUE;
				break;

			case DBKIND_NAME:
				if (NULL == px->uName.pwszName || NULL == py->uName.pwszName)
					return px->uName.pwszName == py->uName.pwszName;
				if (0==wcscmp((px->uName.pwszName),(py->uName.pwszName)))
					return TRUE;
				break;

			case DBKIND_PGUID_NAME:
				if ((px->uGuid.pguid) == (py->uGuid.pguid))
					return TRUE;
				break;

			case DBKIND_PGUID_PROPID:
				if ((px->uGuid.pguid)==(py->uGuid.pguid)&&
					(px->uName.ulPropid == py->uName.ulPropid))
					return TRUE;
				break;

			case DBKIND_PROPID:
				if (px->uName.ulPropid == py->uName.ulPropid)
					return TRUE;
				break;

			case DBKIND_GUID:
				if (px->uGuid.guid==py->uGuid.guid) 
					return TRUE;
				break;

			default:
				assert(!"Unhandled Case!");
				return FALSE;
		}
	}

	return FALSE;
}
