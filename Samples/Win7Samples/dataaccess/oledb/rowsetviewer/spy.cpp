//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module SPY.CPP
//
//-----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////////////
#include "Headers.h"
#include "Spy.h"
		 

/////////////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////////////

// Format:
// HEADSIGNITURE + BUFFERSIZE + BUFFERID + PFILENAME + LINENUMBER + BUFFER + TAILSIGNITURE

//All the header info must be ULONGs,
//so that the user buffer falls on a word boundary
//The tail must be a byte, since if it was a ULONG it would
//also require a word boundary, but the users buffer could
//be an odd number of bytes, so instead of rounding up, just use BYTE

const ULONG		HEADSIZE		= sizeof(ULONG);  //HEADSIGNITURE
const SIZE_T	LENGTHSIZE		= sizeof(SIZE_T); //BUFFERSIZE
const ULONG		IDSIZE			= sizeof(ULONG);  //BUFFERID
const ULONG		FILENAMESIZE	= sizeof(WCHAR*); //PFILENAME
const ULONG		LINENUMBERSIZE	= sizeof(ULONG);  //LINENUMBER
const ULONG		TAILSIZE		= sizeof(BYTE);	  //TAILSIGNITURE

const ULONG HEADERSIZE = (ULONG)ROUNDUP(HEADSIZE + LENGTHSIZE + IDSIZE + FILENAMESIZE + LINENUMBERSIZE);
const ULONG FOOTERSIZE = TAILSIZE;

const BYTE  HEADSIGN = '{';
const BYTE  TAILSIGN = '}';

const BYTE  ALLOCSIGN = '$';
const BYTE  FREESIGN  = 'Z';

#define HEAD_OFFSET(pActual)		((BYTE*)pActual)
#define TAIL_OFFSET(pActual)		(USERS_OFFSET(pActual)+BUFFER_LENGTH(pActual))

#define USERS_OFFSET(pActual)		(HEAD_OFFSET(pActual) + HEADERSIZE)
#define HEADER_OFFSET(pRequest) 	((BYTE*)(pRequest) - HEADERSIZE)	

#define LENGTH_OFFSET(pActual)		(HEAD_OFFSET(pActual) + HEADSIZE)	
#define BUFFER_LENGTH(pActual)		(*(SIZE_T*)LENGTH_OFFSET(pActual))

#define ID_OFFSET(pActual)			(LENGTH_OFFSET(pActual) + LENGTHSIZE)
#define BUFFER_ID(pActual)			(*(ULONG*)ID_OFFSET(pActual))

#define FILENAME_OFFSET(pActual)	(ID_OFFSET(pActual) + IDSIZE)
#define BUFFER_FILENAME(pActual)	(*(WCHAR**)FILENAME_OFFSET(pActual))

#define LINENUMBER_OFFSET(pActual)	(FILENAME_OFFSET(pActual) + FILENAMESIZE)
#define BUFFER_LINENUMBER(pActual)	(*(ULONG*)LINENUMBER_OFFSET(pActual))

#define HEAD_SIGNITURE(pActual)		(*(ULONG*)HEAD_OFFSET(pActual))
#define TAIL_SIGNITURE(pActual)		(*(BYTE*)TAIL_OFFSET(pActual))


/////////////////////////////////////////////////////////////////////////////
// CMallocSpy::CMallocSpy()
//
/////////////////////////////////////////////////////////////////////////////
CMallocSpy::CMallocSpy()
{
    m_cRef			= 1;
	m_cbRequest		= 0;
	m_cAllocations	= 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMallocSpy::~CMallocSpy()
//
/////////////////////////////////////////////////////////////////////////////
CMallocSpy::~CMallocSpy()
{
	//Remove all the elements of the list
	ASSERT(m_cRef == 0);
	CAllocList.RemoveAll();
}


/////////////////////////////////////////////////////////////////////////////
// CMallocSpy::Reset()
//
/////////////////////////////////////////////////////////////////////////////
void CMallocSpy::Reset()
{
	//Reset the number of allocations
	m_cAllocations = 0;;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT CMallocSpy::AddToList
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CMallocSpy::AddToList(void* pv)
{
	ASSERT(pv);

	//NOTE:  We don't have to worry about access into our list in Multi-Threads as COM states:
	//"The call to the pre-method through the return from the corresponding post-method 
	//is guaranteed to be thread-safe in multi-threaded operations."

	//Add this element to the list
	CAllocList.AddTail(pv);
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT CMallocSpy::RemoveFromList
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CMallocSpy::RemoveFromList(void* pv)
{
	ASSERT(pv);
	
	//NOTE:  We don't have to worry about access into our list in Multi-Threads as COM states:
	//"The call to the pre-method through the return from the corresponding post-method 
	//is guaranteed to be thread-safe in multi-threaded operations."

	//Remove this element from the list
	POSITION pos = CAllocList.Find(pv);
	if(pos)
	{
		CAllocList.RemoveAt(pos);
		return S_OK;
	}

	return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT CMallocSpy::Register
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CMallocSpy::Register()
{
	//NOTE:  This may fail, if unreigster failed to unload the previous one.
	//CO_E_OBJISREG There is already a registered spy. 
	return CoRegisterMallocSpy(this); // Does an AddRef on Object
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT CMallocSpy::Unregister
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CMallocSpy::Unregister()
{
	//NOTE:  Unregister will fail if their are outstanding leaks as the docs indicate:
	
	//If the return code is E_ACCESSDENIED, there are still outstanding allocations that were 
	//made while the spy was active. In this case, the registered spy cannot be revoked at this 
	//time because it may have attached arbitrary headers and/or trailers to these allocations 
	//that only the spy knows about. Only the spy's PreFree (or PreRealloc) method knows how to 
	//account for these headers and trailers. Before returning E_ACCESSDENIED, CoRevokeMallocSpy 
	//notes internally that a revoke is pending. When the outstanding allocations have been freed, 
	//the revoke proceeds automatically, releasing the IMallocSpy object. Thus, it is necessary to
	//call CoRevokeMallocSpy only once for each call to CoRegisterMallocSpy, even if E_ACCESSDENIED
	//is returned.
	return CoRevokeMallocSpy(); //Does a Release on Object
}



/////////////////////////////////////////////////////////////////////////////
// HRESULT CMallocSpy::DumpLeaks
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CMallocSpy::DumpLeaks()
{
	ULONG	cTotalLeaks	= 0;
	SIZE_T	cTotalBytes	= 0;
	DWORD	dwSelection	= IDOK;

	//Display Leaks to the Output Window
	while(!CAllocList.IsEmpty())
	{	
		//Obtain the pointer to the leaked memory
		void* pRequest = CAllocList.RemoveHead();
		ASSERT(pRequest);
		
		void* pActual = HEADER_OFFSET(pRequest);
		ASSERT(pActual);

		//Make sure that the head/tail signitures are intact
		if(HEAD_SIGNITURE(pActual) != HEADSIGN)
			InternalTraceFmt(L"TRACE - IMallocSpy HeadSigniture Corrupted! - 0x%p, ID=%08lu, %Iu byte(s)\n", pRequest, BUFFER_ID(pActual), BUFFER_LENGTH(pActual));

		if(TAIL_SIGNITURE(pActual) != TAILSIGN)
			InternalTraceFmt(L"TRACE - IMallocSpy TailSigniture Corrupted! - 0x%p, ID=%08lu, %Iu byte(s)\n", pRequest, BUFFER_ID(pActual), BUFFER_LENGTH(pActual));

		SIZE_T	ulSize		= BUFFER_LENGTH(pActual);
		ULONG	ulID		= BUFFER_ID(pActual);
		WCHAR*	pwszFileName= BUFFER_FILENAME(pActual);
		ULONG	ulLine		= BUFFER_LINENUMBER(pActual);
		
		//Display a message box for all leaks (until the user is tired of it...)
		if(dwSelection == IDOK)
		{
			dwSelection = wMessageBox(GetFocus(), MB_TASKMODAL | MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON1, 
				L"IMallocSpy Leak", 
					L"IMallocSpy Leak:\n"
					L"0x%p, ID=%08lu, %Iu byte(s), File: %s, Line %d\n\n",
					pRequest, ulID, ulSize, pwszFileName, ulLine);
		}

		//Include FileName and Line Number of the leak...
		InternalTraceFmt(L"-- IMallocSpy Leak! - 0x%p,\tID=%08lu,\t%Iu byte(s),\tFile: %s,\tLine %d" wWndEOL, pRequest, ulID, ulSize, pwszFileName, ulLine);
		
		cTotalLeaks++;
		cTotalBytes += ulSize;

		//Free the Leak
		//You really cant free the leak since the app could be potentially still
		//using it.  Or the DLL may still be in use or have attached threads...
		//SAFE_FREE(pActual);
	}

	if(cTotalLeaks)
		InternalTraceFmt(L"-- IMallocSpy Total Leaks: %lu = %Iu byte(s)" wWndEOL, cTotalLeaks, cTotalBytes);
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT CMallocSpy::QueryInterface
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CMallocSpy::QueryInterface(REFIID riid, void** ppIUnknown)
{
	if(!ppIUnknown)
		return E_INVALIDARG;
	*ppIUnknown = NULL;

	//IID_IUnknown
    if(riid == IID_IUnknown)
		*ppIUnknown = this;
    //IDD_IMallocSpy
	else if(riid == IID_IMallocSpy)
         *ppIUnknown =  this;
    
	if(*ppIUnknown)
    {
        ((IUnknown*)*ppIUnknown)->AddRef();
        return S_OK;
	}

	return E_NOINTERFACE;
}

/////////////////////////////////////////////////////////////////////////////
// ULONG CMallocSpy::AddRef
//
/////////////////////////////////////////////////////////////////////////////
ULONG CMallocSpy::AddRef()
{
    return ++m_cRef;
}

/////////////////////////////////////////////////////////////////////////////
// ULONG CMallocSpy::Release
//
/////////////////////////////////////////////////////////////////////////////
ULONG CMallocSpy::Release()
{
    if(--m_cRef)
    	return m_cRef;

	TRACE(L"TRACE - Releasing IMallocSpy\n");

    delete this;
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CMallocSpy::PreAlloc
//
/////////////////////////////////////////////////////////////////////////////
SIZE_T CMallocSpy::PreAlloc(SIZE_T cbRequest)
{
	//cbRequest is the orginal number of bytes requested by the user
	//Store the users requested size
    m_cbRequest = cbRequest;

	//Return the total size requested, plus extra for header/footer
	return (m_cbRequest + HEADERSIZE + FOOTERSIZE);
}

/////////////////////////////////////////////////////////////////////////////
// void* CMallocSpy::PostAlloc
//
/////////////////////////////////////////////////////////////////////////////
void* CMallocSpy::PostAlloc(void* pActual)
{
	//E_OUTOFMEMORY condition
	if(!pActual)
		return NULL;
	
	//pActual is the pointer to the head of the buffer, including the header
	//Add the users pointer to the list
	AddToList(USERS_OFFSET(pActual));

	//Place the HeadSigniture in the HEADER
	HEAD_SIGNITURE(pActual) = HEADSIGN;
	
	//Place the Size in the HEADER
	BUFFER_LENGTH(pActual) = m_cbRequest;

	//Place the ID in the HEADER
	ULONG ulID = ++m_cAllocations;
	BUFFER_ID(pActual) = ulID;

	//Place the FILENAME in the HEADER
	BUFFER_FILENAME(pActual) = NULL;

	//Place the LINE NUMBER in the HEADER
	BUFFER_LINENUMBER(pActual) = 0;

	//Set the UsersBuffer to a known char
    memset(USERS_OFFSET(pActual), ALLOCSIGN, m_cbRequest);

	//Place the TailSigniture in the HEADER
	TAIL_SIGNITURE(pActual) = TAILSIGN;

	//Show Allocation
	if(GetErrorPosting(EP_IMALLOC_ALLOCS))
		InternalTraceFmt(L"TRACE - IMallocSpy Alloc - 0x%p,\tID=%08lu,\t%Iu byte(s)\n", USERS_OFFSET(pActual), ulID, m_cbRequest);

	//Break at indicated Allocation
	if(g_dwBreakID == ulID)
		BREAKINTO();

	// Return the actual users buffer
    return USERS_OFFSET(pActual);
}

/////////////////////////////////////////////////////////////////////////////
// void* CMallocSpy::PreFree
//
/////////////////////////////////////////////////////////////////////////////
void* CMallocSpy::PreFree(void* pRequest, BOOL fSpyed)
{
	//pRequest is the users pointer to thier buffer, not the header

	//E_OUTOFMEMORY condition
	if(!pRequest)
		return NULL;

    //If this memory was alloced under IMallocSpy, need to remove it
    if(fSpyed)
	{
		//Remove this pointer from the list
		RemoveFromList(pRequest);
	
		void* pActual	= HEADER_OFFSET(pRequest);
		ULONG ulID		= BUFFER_ID(pActual);
		
		//Make sure that the head/tail signitures are intact
		if(HEAD_SIGNITURE(pActual) != HEADSIGN)
			InternalTraceFmt(L"TRACE - IMallocSpy HeadSigniture Corrupted! - 0x%p, ID=%08lu, %Iu byte(s)\n", pRequest, ulID, BUFFER_LENGTH(pActual));

		if(TAIL_SIGNITURE(pActual) != TAILSIGN)
			InternalTraceFmt(L"TRACE - IMallocSpy TailSigniture Corrupted! - 0x%p, ID=%08lu, %Iu byte(s)\n", pRequest, ulID, BUFFER_LENGTH(pActual));

		//Break at indicated Allocation
		if(g_dwBreakID == ulID)
			BREAKINTO();

		//Set the UsersBuffer to a known char
		memset(pRequest, FREESIGN, BUFFER_LENGTH(pActual));

		//Need to return the actual header pointer to
		//free the entire buffer including the heading
		return pActual;
	}

	//else
	return pRequest;
}


/////////////////////////////////////////////////////////////////////////////
// void CMallocSpy::PostFree
//
/////////////////////////////////////////////////////////////////////////////
void CMallocSpy::PostFree(BOOL fSpyed)
{
    // Note the free or whatever
    return;
}


/////////////////////////////////////////////////////////////////////////////
// CMallocSpy::PreRealloc
//
/////////////////////////////////////////////////////////////////////////////
SIZE_T CMallocSpy::PreRealloc(void* pRequest, SIZE_T cbRequest,
                             void** ppNewRequest, BOOL fSpyed)
{
	ASSERT(pRequest && ppNewRequest);
    
	//If this was alloced under IMallocSpy we need to adjust
	//the size stored in the header
    if(fSpyed)
    {
		//Remove the original pRequest pointer from the list
		//Since Realloc could change the original pointer
		RemoveFromList(pRequest);
	
        //Find the start 
		*ppNewRequest = HEADER_OFFSET(pRequest);
		
		//Store the new desired size
		m_cbRequest = cbRequest;
		
		//Return the total size, including extra
		return (m_cbRequest + HEADERSIZE + FOOTERSIZE);
    }

	//else
	*ppNewRequest = pRequest;
    return cbRequest;
}


/////////////////////////////////////////////////////////////////////////////
// void* CMallocSpy::PostRealloc
//
/////////////////////////////////////////////////////////////////////////////
void* CMallocSpy::PostRealloc(void* pActual, BOOL fSpyed)
{
	//E_OUTOFMEMORY condition
	if(!pActual)
		return NULL;

    //If this buffer was alloced under IMallocSpy
    if(fSpyed)
    {
		//pActual is the pointer to header
		//Add the new pointer to the list
		AddToList(USERS_OFFSET(pActual));

		//HeadSigniture should still be intact
		if(HEAD_SIGNITURE(pActual) != HEADSIGN)
			InternalTraceFmt(L"TRACE - IMallocSpy HeadSigniture Corrupted! - 0x%p, ID=%08lu, %Iu byte(s)\n", USERS_OFFSET(pActual), BUFFER_ID(pActual), BUFFER_LENGTH(pActual));
		
		//ID should still be intact

		//Place the new Size in the HEADER
		BUFFER_LENGTH(pActual) = m_cbRequest;

		//Place the new FileName in the HEADER
		BUFFER_FILENAME(pActual) = NULL;

		//Place the new Line Number in the HEADER
		BUFFER_LINENUMBER(pActual) = 0;
		
        //Need to place the tail signiture again, 
		//since it will be over written by the realloc
		TAIL_SIGNITURE(pActual) = TAILSIGN;

		//Show ReAllocations
		if(GetErrorPosting(EP_IMALLOC_ALLOCS))
			InternalTraceFmt(L"TRACE - IMallocSpy Realloc - 0x%p,\tID=%08lu,\t%Iu byte(s)\n", USERS_OFFSET(pActual), BUFFER_ID(pActual), m_cbRequest);

		//Return the actual "user" buffer
		return USERS_OFFSET(pActual);
    }
    
	//else
    return pActual;
}


/////////////////////////////////////////////////////////////////////////////
// void* CMallocSpy::PreGetSize
//
/////////////////////////////////////////////////////////////////////////////
void* CMallocSpy::PreGetSize(void* pRequest, BOOL fSpyed)
{
    if (fSpyed)
        return HEADER_OFFSET(pRequest);

    return pRequest;
}



/////////////////////////////////////////////////////////////////////////////
// CMallocSpy::PostGetSize
//
/////////////////////////////////////////////////////////////////////////////
SIZE_T CMallocSpy::PostGetSize(SIZE_T cbActual, BOOL fSpyed)
{
    if (fSpyed)
        return cbActual - HEADERSIZE - FOOTERSIZE;

    return cbActual;
}




/////////////////////////////////////////////////////////////////////////////
// void* CMallocSpy::PreDidAlloc
//
/////////////////////////////////////////////////////////////////////////////
void* CMallocSpy::PreDidAlloc(void* pRequest, BOOL fSpyed)
{
    if (fSpyed)
        return HEADER_OFFSET(pRequest);

	return pRequest;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL CMallocSpy::PostDidAlloc
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMallocSpy::PostDidAlloc(void* pRequest, BOOL fSpyed, BOOL fActual)
{
    return fActual;
}



/////////////////////////////////////////////////////////////////////////////
// void CMallocSpy::PreHeapMinimize
//
/////////////////////////////////////////////////////////////////////////////
void CMallocSpy::PreHeapMinimize()
{
    // We don't do anything here
    return;
}


/////////////////////////////////////////////////////////////////////////////
// void CMallocSpy::PostHeapMinimize
//
/////////////////////////////////////////////////////////////////////////////
void CMallocSpy::PostHeapMinimize()
{
    // We don't do anything here
    return;
}



/////////////////////////////////////////////////////////////////////////////
// CRTReportHook
//
/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
_CRT_REPORT_HOOK g_fpPrevCRTReportHook = NULL;
int __cdecl CRTReportHook(INT nReportType, CHAR* pszMsg, INT* pReturnVal)
{
	//Output
	if(pReturnVal)
		*pReturnVal = 0;
	
	//If the user wants us to handle the ASSERTs...
	if(TRUE)
	{
		//Determine what type of report this is...
		switch(nReportType)
		{
			case _CRT_ASSERT:	// - only defined for DEBUG
				//Display the Assert
				InternalTraceFmt(L"---------- C-Runtime Assert ----------" wWndEOL);
				InternalTraceFmt(L"%S", pszMsg);
				return TRUE;

			case _CRT_ERROR:	// - only defined for DEBUG
				//Display the Error
				InternalTraceFmt(L"---------- C-Runtime Error ----------" wWndEOL);
				InternalTraceFmt(L"%S", pszMsg);
				return TRUE;

			default:
				//There may be other reports or messages going to the C-runtime
				//such as _CRT_WARN and _CRT_ERRCNT which we don't need to filter...
				//CRT_WARN is used frequently by apps instead of OutputDebugString
				break;
		};
	}

	//If we have made it this far the ASSERT was not handled, so we should either call the previous
	//installed CallBack (that we replaced if there was one) or return FALSE to the CRuntime to 
	//indicate we didn't handle this error, so do the normal processing for the error...
	if(g_fpPrevCRTReportHook)
		return g_fpPrevCRTReportHook(nReportType, pszMsg, pReturnVal);
	return FALSE;
}

#endif //_DEBUG

void EnableCRTReportHook(BOOL fEnable)
{
#ifdef _DEBUG
	if(fEnable)
	{
		if(!g_fpPrevCRTReportHook)
			g_fpPrevCRTReportHook	= _CrtSetReportHook(&CRTReportHook);
	}
	else
	{
		if(g_fpPrevCRTReportHook)
			_CrtSetReportHook(g_fpPrevCRTReportHook);
		g_fpPrevCRTReportHook = NULL;
	}
#endif //_DEBUG
}
   


/////////////////////////////////////////////////////////////////////////////
// CRTAllocHook
//
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
//NOTE:  The reason we have our own function pointer type is that in MSVC5.0
//the definition was "const char* filename", and in 6.0 they changed the debug definition to 
//"const unsigned char* filename".  So to compile with earlier versions we stick with the
//older definition and just cast away the "unsigned" ness...
typedef int (__cdecl * _CRT_ALLOC_HOOK_SIGNED)(int, void *, size_t, int, long, const char *, int);
   
_CRT_ALLOC_HOOK_SIGNED g_fpPrevCRTAllocHook = NULL;
int __cdecl CRTAllocHook(int allocType, void *userData, size_t size, int blockType, 
   long requestNumber, const /*unsigned*/ char *filename, int lineNumber)
{
	//If the user wants us to handle the ASSERTs...
	if(TRUE)
	{
		//Determine what type of Alloc this is...
		switch(allocType)
		{
			case _HOOK_ALLOC:	// - only defined for DEBUG
				//Display the Alloc
				InternalTraceFmt(L"TRACE - CRT Alloc - 0x%p, ID=%08lu, %Iu byte(s), File: %S, Line: %d\n", userData, requestNumber, size, filename, lineNumber);
				return TRUE;

			case _HOOK_REALLOC:	// - only defined for DEBUG
				//Display the ReAlloc
				InternalTraceFmt(L"TRACE - CRT Realloc - 0x%p, ID=%08lu, %Iu byte(s), File: %S, Line: %d\n", userData, requestNumber, size, filename, lineNumber);
				return TRUE;

			case _HOOK_FREE:	// - only defined for DEBUG
				//Display the Free
				InternalTraceFmt(L"TRACE - CRT Free - 0x%p, ID=%08lu, %Iu byte(s), File: %S, Line: %d\n", userData, requestNumber, size, filename, lineNumber);
				return TRUE;

			default:
				//There may be other reports or messages going to the C-runtime
				break;
		};
	}

	//After the hook function has finished processing, it must return a Boolean value, 
	//which tells the main C run-time allocation process how to continue. When the hook function 
	//wants the main allocation process to continue as if the hook function had never been called, 
	//then the hook function should return TRUE. 
	if(g_fpPrevCRTAllocHook)
		return g_fpPrevCRTAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
	return TRUE;
}
#endif //_DEBUG

void EnableCRTAllocHook(BOOL fEnable)
{
#ifdef _DEBUG
	if(fEnable)
	{
		if(!g_fpPrevCRTAllocHook)
			g_fpPrevCRTAllocHook	= (_CRT_ALLOC_HOOK_SIGNED)_CrtSetAllocHook((_CRT_ALLOC_HOOK)&CRTAllocHook);
	}
	else
	{
		if(g_fpPrevCRTAllocHook)
			_CrtSetAllocHook((_CRT_ALLOC_HOOK)g_fpPrevCRTAllocHook);
		g_fpPrevCRTAllocHook = NULL;
	}
#endif //_DEBUG
}


