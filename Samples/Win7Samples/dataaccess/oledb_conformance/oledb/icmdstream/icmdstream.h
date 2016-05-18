//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module ICmdStream.h | This module contains header information for 
//	    				OLE DB ICommandStream interface Test
//
// @rev 01 | 10-26-99 | Microsoft | Created
//

#ifndef _ICMDSTREAM_H_
#define _ICMDSTREAM_H_
						  
#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "privlib.h"		// Private Library


///////////////////////////////////////////////////////////
// ENUMS
///////////////////////////////////////////////////////////
enum ENCODING
{
	UTF8	= 1,
	UTF16	= 2
};

///////////////////////////////////////////////////////////
// MACROS
///////////////////////////////////////////////////////////

//Read a stream in increments of ...
#define	READ_INCR		300

//Allocate new stogare object for calling SetCommandStream.
#define	ALLOC_SETSTG	m_pStorageSet = new CStorage();		\
						if(!m_pStorageSet) goto CLEANUP;

//Release the m_pStorageSet storage object.
#define	RELEASE_SETSTG	{	if(m_pStorageSet)				\
							{								\
								m_pStorageSet->Release();	\
								m_pStorageSet = NULL;		\
							}								\
						}

//Release the m_pStorageGet storage object.
#define	RELEASE_GETSTG	{	if(m_pStorageGet)				\
							{								\
								m_pStorageGet->Release();	\
								m_pStorageGet = NULL;		\
							}								\
						}


#endif  // _ICMDSTREAM_H_