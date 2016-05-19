/*++

Copyright (c) 1998-1999  Microsoft Corporation

Module Name:

    mspaddr.cpp 

Abstract:

    This module contains implementation of CMSPAddress.

--*/

#include "precomp.h"
#pragma hdrstop



///////////////////////////////////////////////////////////////////////////////
//
// AllocateEventItem and FreeEventItem are MSPEVENTITEM allocation routines.
// they are be used to allocate and deallocate MSPEVENTITEM structures
//
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//  AllocateEventItem
//
//  allocate an MSPEVENTITEM. Since the structure is of variable size, the 
//  number of extra bytes to be allocated (in addition to the size of 
//  MSPEVENTITEM) is optionally passed as the function's argument
//
//  the function returns a pointer to the newly created structure or NULL in 
//  the case of failure. the caller can then call GetLastError to get more 
//  information on the failure
//

MSPEVENTITEM *AllocateEventItem(SIZE_T nExtraBytes)
{

    LOG((MSP_TRACE, "AllocateEventItem - enter, extra bytes = 0x%p", nExtraBytes));


    //
    // if the caller passes us too big a number, fail.
    //

    if ( ( MAXULONG_PTR - sizeof(MSPEVENTITEM) )  < nExtraBytes )
    {
        SetLastError(ERROR_OUTOFMEMORY);

        LOG((MSP_ERROR, 
            "AllocateEventItem - the caller requested an unreasonably large memory block"));

        return NULL;
    }


    //
    // allocate on the process' heap. get the current process' heap handle.
    //

    HANDLE hHeapHandle = GetProcessHeap();

    if (NULL == hHeapHandle)
    {

        //
        // failed to get process's heap. nothing much we can do here.
        // this will cause a leak.
        //

        LOG((MSP_ERROR, 
            "AllocateEventItem - failed to get current process heap. LastError [%ld]", 
            GetLastError()));

        return NULL;
    }


    //
    // calculate the number of bytes to allocate
    //

    SIZE_T nTotalAllocationSize = sizeof(MSPEVENTITEM) + nExtraBytes;


    //
    // attempt to allocate memory and return result of the allocation
    //
    
    MSPEVENTITEM *pMspEventItem = 
         (MSPEVENTITEM *)HeapAlloc(hHeapHandle, 0, nTotalAllocationSize);


    if (NULL == pMspEventItem)
    {

        LOG((MSP_ERROR,
            "AllocateEventItem - failed to allocate [0x%p] bytes. Heap Handle [%p] LastError [%ld]",
            nTotalAllocationSize, hHeapHandle, GetLastError()));
    }
    else
    {

        LOG((MSP_TRACE, "AllocateEventItem - exit. pMspEventItem = [%p]", pMspEventItem));
    }

    return pMspEventItem;
}


//////////////////////////////////////////////////////////////////////////////
//
//  FreeEventItem
//
//  deallocate the MSPEVENTITEM passed as an argument. The memory must have
//  been previously allocated by AllocateEventItem.
//
//  the function eturns FALSE in case of failure. The caller can use 
//  GetLastError to get a more specific error code.
//

BOOL FreeEventItem(MSPEVENTITEM *pEventItemToBeFreed)
{

    LOG((MSP_TRACE, "FreeEventItem - enter. pEventItemToBeFreed = [%p]", 
        pEventItemToBeFreed));


    //
    // always allow freeing NULL
    //

    if (NULL == pEventItemToBeFreed)
    {

        LOG((MSP_TRACE, "FreeEventItem - finish. NULL -- nothing to do"));

        return TRUE;
    }

    //
    // the event item should have been allocated on the process' heap.
    // get the current process' heap hadle.
    //

    HANDLE hHeapHandle = GetProcessHeap();

    if (NULL == hHeapHandle)
    {

        //
        // failed to get process's heap. nothing much we can do here.
        // this will cause a leak.
        //

        LOG((MSP_ERROR, 
            "FreeEventItem - failed to get current process heap. LastError = %ld", 
            GetLastError()));

        return FALSE;
    }


    //
    // attempt to free memory and return result of the operation
    //
    
    BOOL bFreeSuccess = HeapFree( hHeapHandle, 0, pEventItemToBeFreed );

    if (bFreeSuccess)
    {
         LOG((MSP_TRACE, "FreeEventItem - exit."));
    }
    else
    {
         LOG((MSP_ERROR, 
             "FreeEventItem - failed to free. Heap Handle [%p] LastError = %ld",
             hHeapHandle, GetLastError()));
    }


    return bFreeSuccess;
}


//////////////////////////////////////////////////////////////////////////////


HRESULT CPlugTerminalClassInfo::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CPlugTerminalClassInfo::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::FinalConstruct - exit S_OK"));

    return S_OK;

}

//////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CPlugTerminalClassInfo::get_Name(
    /*[out, retval]*/ BSTR*     pName
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Name - enter"));

    //
    // Validates argument
    //

    if( ! pName)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Name exit -"
            "pName invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Validates the name
    //

    if( ! m_bstrName)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Name exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    //
    // Return the name
    //

    *pName = SysAllocString( m_bstrName );

    //
    // Validates SysAllocString
    //

    if( *pName == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Name - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::put_Name(
    /*[in]*/    BSTR            bstrName
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Name - enter"));

    //
    // Validates argument
    //

    if(! bstrName)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Name exit -"
            "bstrName invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Clean-up the old name
    //

    if(!! m_bstrName)
    {
        SysFreeString( m_bstrName );
        m_bstrName = NULL;
    }

    //
    // Set the new name
    //

    m_bstrName = SysAllocString( bstrName );

    //
    // Validates SysAllocString
    //

    if( NULL == m_bstrName )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Name - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::get_Company(
    /*[out, retval]*/ BSTR*     pCompany
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Company - enter"));

    //
    // Validates argument
    //

    if( ! pCompany)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Company exit -"
            "pCompany invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Validates the name
    //

    if( ! m_bstrCompany)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Company exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    //
    // Return the name
    //

    *pCompany = SysAllocString( m_bstrCompany );

    //
    // Validates SysAllocString
    //

    if( *pCompany == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Company exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Company - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::put_Company(
    /*[in]*/    BSTR            bstrCompany
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Company - enter"));

    //
    // Validates argument
    //

    if(! bstrCompany)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Company exit -"
            "bstrCompany invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Clean-up the old name
    //

    if(!! m_bstrCompany)
    {
        SysFreeString( m_bstrCompany );
        m_bstrCompany = NULL;
    }

    //
    // Set the new name
    //

    m_bstrCompany = SysAllocString( bstrCompany );

    //
    // Validates SysAllocString
    //

    if( NULL == m_bstrCompany )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Company exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Company - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::get_Version(
    /*[out, retval]*/ BSTR*     pVersion
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Version - enter"));

    //
    // Validates argument
    //

    if( ! pVersion)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Version exit -"
            "pVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Validates the name
    //

    if( ! m_bstrVersion)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Version exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    //
    // Return the name
    //

    *pVersion = SysAllocString( m_bstrVersion );

    //
    // Validates SysAllocString
    //

    if( *pVersion == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Version exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Version - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::put_Version(
    /*[in]*/    BSTR            bstrVersion
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Version - enter"));

    //
    // Validates argument
    //

    if(! bstrVersion)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Version exit -"
            "bstrVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Clean-up the old name
    //

    if(!! m_bstrVersion)
    {
        SysFreeString( m_bstrVersion );
        m_bstrVersion = NULL;
    }

    //
    // Set the new name
    //

    m_bstrVersion = SysAllocString( bstrVersion );

    //
    // Validates SysAllocString
    //

    if( NULL == m_bstrVersion )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_Version exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_Version - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::get_TerminalClass(
    /*[out, retval]*/ BSTR*     pTerminalClass
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_TerminalClass - enter"));

    //
    // Validates argument
    //

    if( ! pTerminalClass)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_TerminalClass exit -"
            "pVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Validates the name
    //

    if( ! m_bstrTerminalClass)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_TerminalClass exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    //
    // Return the name
    //

    *pTerminalClass = SysAllocString( m_bstrTerminalClass );

    //
    // Validates SysAllocString
    //

    if( *pTerminalClass == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_TerminalClass exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_TerminalClass - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::put_TerminalClass(
    /*[in]*/    BSTR            bstrTerminalClass
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_TerminalClass - enter"));

    //
    // Validates argument
    //

    if(! bstrTerminalClass)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_TerminalClass exit -"
            "bstrTerminalClass invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Is a real CLSID?
    //

    CLSID clsid;
    HRESULT hr = CLSIDFromString(bstrTerminalClass, &clsid);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_TerminalClass exit -"
            "bstrTerminalClass is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }


    //
    // Clean-up the old name
    //

    if(!! m_bstrTerminalClass)
    {
        SysFreeString( m_bstrTerminalClass );
        m_bstrTerminalClass = NULL;
    }

    //
    // Set the new name
    //

    m_bstrTerminalClass = SysAllocString( bstrTerminalClass );

    //
    // Validates SysAllocString
    //

    if( NULL == m_bstrTerminalClass )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_TerminalClass exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_TerminalClass - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::get_CLSID(
    /*[out, retval]*/ BSTR*     pCLSID
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_CLSID - enter"));

    //
    // Validates argument
    //

    if( ! pCLSID)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_CLSID exit -"
            "pVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Validates the name
    //

    if( ! m_bstrCLSID)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_CLSID exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    //
    // Return the name
    //

    *pCLSID = SysAllocString( m_bstrCLSID );

    //
    // Validates SysAllocString
    //

    if( *pCLSID == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_CLSID exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_CLSID - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::put_CLSID(
    /*[in]*/    BSTR            bstrCLSID
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_CLSID - enter"));

    //
    // Validates argument
    //

    if(! bstrCLSID)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_CLSID exit -"
            "bstrCLSID invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Is a real CLSID?
    //

    CLSID clsid;
    HRESULT hr = CLSIDFromString(bstrCLSID, &clsid);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_CLSID exit -"
            "bstrCLSID is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }


    //
    // Clean-up the old name
    //

    if(!! m_bstrCLSID)
    {
        SysFreeString( m_bstrCLSID );
        m_bstrCLSID = NULL;
    }

    //
    // Set the new name
    //

    m_bstrCLSID = SysAllocString( bstrCLSID );

    //
    // Validates SysAllocString
    //

    if( NULL == m_bstrCLSID )
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_CLSID exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::put_CLSID - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::get_Direction(
    /*[out, retval]*/ TERMINAL_DIRECTION*  pDirection
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Direction - enter"));

    //
    // Validates argument
    //

    if( ! pDirection)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_Direction exit -"
            "pDirection invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Return the name
    //

    *pDirection = m_Direction;

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_Direction - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::put_Direction(
    /*[in]*/    TERMINAL_DIRECTION  nDirection
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_Direction - enter"));

    //
    // Set the new name
    //

    m_Direction = nDirection;

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_Direction - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::get_MediaTypes(
    /*[out, retval]*/ long*     pMediaTypes
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_MediaTypes - enter"));

    //
    // Validates argument
    //

    if( ! pMediaTypes)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::get_MediaTypes exit -"
            "pMediaTypes invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Return the name
    //

    *pMediaTypes = m_lMediaType;

    LOG((MSP_TRACE, "CPlugTerminalClassInfo::get_MediaTypes - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalClassInfo::put_MediaTypes(
    /*[in]*/    long            nMediaTypes
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_MediaTypes - enter"));

    //
    // Set the new name
    //

    m_lMediaType = nMediaTypes;

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_MediaTypes - exit"));
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////


HRESULT CPlugTerminalSuperclassInfo::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::FinalConstruct - exit S_OK"));

    return S_OK;

}

//////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CPlugTerminalSuperclassInfo::get_Name(
    /*[out, retval]*/ BSTR*          pName
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::get_Name - enter"));

    //
    // Validates argument
    //

    if( ! pName)
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_Name exit -"
            "pVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Validates the name
    //

    if( ! m_bstrName)
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_Name exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    //
    // Return the name
    //

    *pName = SysAllocString( m_bstrName );

    //
    // Validates SysAllocString
    //

    if( *pName == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::get_Name - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CPlugTerminalSuperclassInfo::put_Name(
    /*[in]*/          BSTR            bstrName
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_Name - enter"));

    //
    // Validates argument
    //

    if(! bstrName)
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::put_Name exit -"
            "bstrName invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Clean-up the old name
    //

    if(!! m_bstrName)
    {
        SysFreeString( m_bstrName );
        m_bstrName = NULL;
    }

    //
    // Set the new name
    //

    m_bstrName = SysAllocString( bstrName );

    //
    // Validates SysAllocString
    //

    if( NULL == m_bstrName )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::put_Name exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_Name - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CPlugTerminalSuperclassInfo::get_CLSID(
    /*[out, retval]*/ BSTR*           pCLSID
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::get_CLSID - enter"));

    //
    // Validates argument
    //

    if( ! pCLSID)
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_CLSID exit -"
            "pVersion invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Validates the name
    //

    if( ! m_bstrCLSID)
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_CLSID exit -"
            "m_bstrName invalid, returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    //
    // Return the name
    //

    *pCLSID = SysAllocString( m_bstrCLSID );

    //
    // Validates SysAllocString
    //

    if( *pCLSID == NULL )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::get_CLSID exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::get_CLSID - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CPlugTerminalSuperclassInfo::put_CLSID(
    /*[in]*/         BSTR            bstrCLSID
    )
{
    //
    // Critical section
    //

    CLock lock(m_CritSect);

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_CLSID - enter"));

    //
    // Validates argument
    //

    if(! bstrCLSID)
    {
        LOG((MSP_ERROR, "CPlugTerminalClassInfo::put_CLSID exit -"
            "bstrCLSID invalid, returns E_POINTER"));
        return E_POINTER;
    }

    //
    // Is a real CLSID?
    //

    CLSID clsid;
    HRESULT hr = CLSIDFromString(bstrCLSID, &clsid);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::put_CLSID exit -"
            "bstrCLSID is not a CLSID, returns E_INVALIDARG"));
        return E_INVALIDARG;
    }


    //
    // Clean-up the old name
    //

    if(!! m_bstrCLSID)
    {
        SysFreeString( m_bstrCLSID );
        m_bstrCLSID = NULL;
    }

    //
    // Set the new name
    //

    m_bstrCLSID = SysAllocString( bstrCLSID );

    //
    // Validates SysAllocString
    //

    if( NULL == m_bstrCLSID )
    {
        LOG((MSP_ERROR, "CPlugTerminalSuperclassInfo::put_CLSID exit -"
            "SysAllocString failed, returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CPlugTerminalSuperclassInfo::put_CLSID - exit"));
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////


//
// Our available static terminal types.
//

const STATIC_TERMINAL_TYPE CMSPAddress::m_saTerminalTypes[] =
{
    {
        (DWORD) TAPIMEDIATYPE_AUDIO,
        &CLSID_CWaveinClassManager,
        CAudioCaptureTerminal::CreateTerminal
    },
    {
        (DWORD) TAPIMEDIATYPE_AUDIO,
        &CLSID_CWaveOutClassManager,
        CAudioRenderTerminal::CreateTerminal
    },
    {
        (DWORD) TAPIMEDIATYPE_VIDEO,
        &CLSID_CVidCapClassManager,
        CVideoCaptureTerminal::CreateTerminal
    }
};

const DWORD CMSPAddress::m_sdwTerminalTypesCount = sizeof(m_saTerminalTypes)
                                              / sizeof (STATIC_TERMINAL_TYPE);

/////////////////////////////////////////////////////////////////////////////
// CMSPAddress
/////////////////////////////////////////////////////////////////////////////

//
// Check to see if the mediatype is non-zero and is in the mask.
// Your MSP can override this if it needs to do atypically complex
// checks on specific combinations of media types (e.g., can never
// have more than one media type on a call, can have video with audio
// but not video alone, etc.). The default implementation accepts any
// nonempty set of media types that is a subset of the set of types
// in the mask.
//

BOOL CMSPAddress::IsValidSetOfMediaTypes(DWORD dwMediaType, DWORD dwMask)
{
    return (dwMediaType != 0) && ((dwMediaType & dwMask) == dwMediaType);
}

CMSPAddress::CMSPAddress()
    : m_htEvent(NULL),
      m_fTerminalsUpToDate(FALSE),
      m_pITTerminalManager(NULL)
{
    LOG((MSP_TRACE, "CMSPAddress::CMSPAddress[%p] - enter", this));

    LOG((MSP_TRACE, "CMSPAddress::CMSPAddress - finished"));
}
      
CMSPAddress::~CMSPAddress() 
{
    LOG((MSP_TRACE, "CMSPAddress::~CMSPAddress[%p] - enter", this));


    //
    // this should have been taken care of in Shutdown,
    // but just in case shutdown was never called, do this again, since 
    // we need to make sure the thread does not have any stale entries in 
    // its pnp notification list
    // 
    // the call is likely to return error (since the object is likely to have been
    // unregisted earlier) -- so ignore return code
    //

    g_Thread.UnregisterPnpNotification(this);


    LOG((MSP_TRACE, "CMSPAddress::~CMSPAddress - finished"));
}

STDMETHODIMP CMSPAddress::Initialize(
    IN      MSP_HANDLE          htEvent
    )
/*++

Routine Description:

This method is called by TAPI3 when this MSP is first created. The method 
initiailzes data members and creates the terminal manager. It also tells
the global thread object to Start().

Arguments:

htEvent
    Event the MSP signals when passing an event structure back to TAPI.

  
Return Value:

    S_OK
    E_INVALIDARG
    E_OUTOFMEMORY
    TAPI_E_REGISTERCALLBACK

--*/
{
    LOG((MSP_TRACE, 
        "MSP address %x initialize entered, htEvent:%x",
        this, htEvent));

    if ( htEvent == NULL )
    {
        LOG((MSP_ERROR, " bad handle: htEvent:%x", htEvent));

        return E_INVALIDARG;
    }

    // lock the event related data
    m_EventDataLock.Lock();

    if (m_htEvent != NULL)
    {
        m_EventDataLock.Unlock();

        LOG((MSP_ERROR, "Initialze called twice."));
        return E_UNEXPECTED;
    }

    // save handles.
    m_htEvent   = htEvent;
    
    InitializeListHead(&m_EventList);

    HRESULT hr;

    // release the lock on the event related data
    m_EventDataLock.Unlock();

    // lock the terminal related data. This is a auto lock that will unlock
    // when the function returns.
    CLock lock(m_TerminalDataLock);

    // Create the terminal manager.
    hr = CoCreateInstance(
        CLSID_TerminalManager,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITTerminalManager,
        (void **) &m_pITTerminalManager
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "Creating terminal manager failed. return: %x", hr));

        return hr;
    }

    hr = g_Thread.Start();    

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "Creating thread failed. return: %x", hr));

        return hr;
    }

    hr = g_Thread.RegisterPnpNotification(this);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "Unable to register for PNP notification. return: %x", hr));
    }

    LOG((MSP_TRACE, 
        "MSP address %x initialize exited S_OK, htEvent:%x",
        this, htEvent));

    return S_OK;   
}

STDMETHODIMP CMSPAddress::Shutdown ()
/*++

Routine Description:

This method is called by TAPI3 when this address in not in use any more. 
This function releases the terminals and releases the terminal manager.
It releases all unprocessed events, and also stops the worker thread.
When this functions is called, no call should be alive. However, bugs in
the app may keep calls or terminals  around. Currently this function
does not attempt to solve this problem. The calls will have their own
refcounts on the terminals, so it shouldn't fail anyway.

Arguments:
    None.

Return Value:

    S_OK
--*/
{
    LOG((MSP_TRACE, "CMSPAddress::Shutdown - "
        "MSP address %x is shutting down", this));

    HRESULT hr;

    //
    // Unregister for PNP notification
    //

    hr = g_Thread.UnregisterPnpNotification(this);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "Unable to unregister for PNP notification. return: %x", hr));
    }

    //
    // Tell the worker thread to stop.
    //

    g_Thread.Stop();    

    LOG((MSP_INFO, "CMSPAddress::Shutdown - thread has stopped"));

    // acquire the lock on the terminal data because we are writing to it.
    m_TerminalDataLock.Lock();

    // Release the terminal manager.
    if (m_pITTerminalManager != NULL)
    {
        m_pITTerminalManager->Release();
        m_pITTerminalManager = NULL;
    }

    // release all the terminals.
    for (int i = 0; i < m_Terminals.GetSize(); i ++)
    {
        //
        // Clear its CMSPAddress pointer
        //
        CBaseTerminal * pCTerminal = static_cast<CBaseTerminal *> (m_Terminals[i]);

        m_Terminals[i]->Release();
    }
    m_Terminals.RemoveAll();

    // We are done with terminal related data, release the lock.
    m_TerminalDataLock.Unlock();


    LOG((MSP_INFO, "CMSPAddress::Shutdown - terminals released"));

    // acquire the lock on the event data because we are writing to it.
    m_EventDataLock.Lock();
    
    m_htEvent = NULL;

    // release all the unprocessed events in the list.
    while (!IsListEmpty(&m_EventList)) 
    {
        // retrieve first entry
        PLIST_ENTRY pLE = RemoveHeadList(&m_EventList);

        // convert list entry to structure pointer
        PMSPEVENTITEM pItem = CONTAINING_RECORD(pLE, MSPEVENTITEM, Link);

        // release the refcount in the event.
        LOG((MSP_INFO, 
            "CMSPAddress::Shutdown:releasing event still in the queue: %x",
            pItem->MSPEventInfo.Event
            ));

        switch (pItem->MSPEventInfo.Event)
        {
        case ME_ADDRESS_EVENT:
            if (pItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.pTerminal)
            {
                pItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.pTerminal->Release();
            }
            break;

        case ME_CALL_EVENT:
            if (pItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pStream)
            {
                pItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pStream->Release();
            }

            if (pItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pTerminal)
            {
                pItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pTerminal->Release();
            }
            break;

        case ME_PRIVATE_EVENT:
            if (pItem->MSPEventInfo.MSP_PRIVATE_EVENT_INFO.pEvent)
            {
                pItem->MSPEventInfo.MSP_PRIVATE_EVENT_INFO.pEvent->Release();
            }
            break;

        case ME_TSP_DATA:
            // nothing inside the structure that we need to free
            break;

        case ME_FILE_TERMINAL_EVENT:

            if( NULL != pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal)
            {
                (pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal)->Release();
                pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal = NULL;
            }

            if( NULL != pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack )
            {
                (pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack)->Release();
                pItem->MSPEventInfo.MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack = NULL;
            }

            break;

        case ME_ASR_TERMINAL_EVENT:

            if( NULL != pItem->MSPEventInfo.MSP_ASR_TERMINAL_EVENT_INFO.pASRTerminal)
            {
                (pItem->MSPEventInfo.MSP_ASR_TERMINAL_EVENT_INFO.pASRTerminal)->Release();
            }

            break;

        case ME_TTS_TERMINAL_EVENT:

            if( NULL != pItem->MSPEventInfo.MSP_TTS_TERMINAL_EVENT_INFO.pTTSTerminal)
            {
                (pItem->MSPEventInfo.MSP_TTS_TERMINAL_EVENT_INFO.pTTSTerminal)->Release();
            }

            break;

        case ME_TONE_TERMINAL_EVENT:

            if( NULL != pItem->MSPEventInfo.MSP_TONE_TERMINAL_EVENT_INFO.pToneTerminal)
            {
                (pItem->MSPEventInfo.MSP_TONE_TERMINAL_EVENT_INFO.pToneTerminal)->Release();
            }

            break;

        default:
            LOG((MSP_WARN, "CMSPAddress::Shutdown: unknown event type: %x",
                pItem->MSPEventInfo.Event));            

            break;
        }

        FreeEventItem(pItem);
    }

    // We are done with event related data, release the lcok.
    m_EventDataLock.Unlock();

    LOG((MSP_TRACE, "CMSPAddress::Shutdown - exit S_OK"));

    return S_OK;
}


STDMETHODIMP CMSPAddress::ReceiveTSPData(
    IN      IUnknown        *   pMSPCall,
    IN      LPBYTE              pBuffer,
    IN      DWORD               dwBufferSize
    )
/*++

Routine Description:

This method is called by TAPI3 when the TSP address sends data to this 
MSP address object. The semantics of the data passed in the buffer are
specific to each  TSP - MSP pair. This method dispatches the received
buffer to the address (call == NULL) or call (call != NULL).

Arguments:

pMSPCall
    The call object that the data is for. If it is NULL, the data is for 
    this address.

pBuffer
    Opaque buffer from TSP.

dwBufferSize
    Size in bytes of pBuffer


    
Return Value:

    S_OK

--*/
{
    LOG((MSP_TRACE, "CMSPAddress::ReceiveTSPData entered. pMSPCall:%x",
        pMSPCall));

    _ASSERTE(dwBufferSize > 0);
    _ASSERTE(!!pBuffer);

    HRESULT hr;
    
    if ( NULL == pMSPCall )
    {
        hr = ReceiveTSPAddressData(pBuffer, dwBufferSize);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::ReceiveTSPData - "
                "ReceiveTSPAddressData failed - exit 0x%08x", hr));

            return hr;
        }

        LOG((MSP_TRACE, "CMSPAddress::ReceiveTSPData - "
            "exit S_OK (dispatched to address)"));

        return S_OK;
    }

    //
    // We have a call to dispatch to.
    //

    _ASSERTE(!!pMSPCall);
    
    ITStreamControl * pIStreamControl;

    hr = pMSPCall->QueryInterface(IID_ITStreamControl,
                                  (void **)&pIStreamControl);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::ReceiveTSPData - "
            "can't get the ITStream Control interface - exit 0x%08x", hr));

        return hr;
    }

    CMSPCallBase * pCall = static_cast<CMSPCallBase *> (pIStreamControl);

    if (pCall == NULL)
    {
        LOG((MSP_ERROR, "CMSPAddress::ReceiveTSPData - "
            "invalid msp call pointer: %x", pMSPCall));

        pIStreamControl->Release();

        return E_UNEXPECTED;
    }
    
    hr = pCall->ReceiveTSPCallData(pBuffer, dwBufferSize);

    pIStreamControl->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::ReceiveTSPData - "
            "method on call failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CMSPAddress::ReceiveTSPData - "
        "exit S_OK (dispatched to call)"));

    return S_OK;
}


HRESULT CMSPAddress::GetStaticTerminals(
    IN OUT  DWORD *             pdwNumTerminals,
    OUT     ITTerminal **       ppTerminals
    )
/*++

Routine Description:

This method is called by TAPI3 to get a list of static terminals that can 
be used on this address. If our list is not empty, just return the list. 
If our list is still empty, create the static terminals and return the list.
Derived class can override this method to have their own terminals. Locks 
the terminal lists.

Arguments:

pdwNumTerminals
    Pointer to a DWORD.  On entry, indicates the size of the buffer pointed 
    to in ppTerminals. On success, it will be filled in with the actual number
    of terminals returned.  If the buffer is not big enough, the method will 
    return TAPI_E_NOTENOUGHMEMORY, and it will be filled in the with number
    of terminals needed. 

ppTerminals
    On success, filled in with an array of terminals object pointers that are 
    supported by the MSP for this address.  This value may be NULL, in which 
    case pdwNumTerminals will return the needed buffer size.

    
Return Value:

S_OK
E_OUTOFMEMORY
TAPI_E_NOTENOUGHMEMORY

--*/
{
    LOG((MSP_TRACE, 
        "GetStaticTerminals entered. NumTerminals:%x, ppTerminals:%x",
        *pdwNumTerminals, ppTerminals
        ));

    // lock the terminal related data. This is a auto lock that will unlock
    // when the function returns.
    CLock lock(m_TerminalDataLock);

    if (!m_fTerminalsUpToDate)
    {
        HRESULT hr = UpdateTerminalList();

        if (FAILED(hr))
        {
            LOG((MSP_ERROR,
                "CMSPAddress::GetStaticTerminals - "
                "UpdateTerminalList failed - returning 0x%08x", hr));

            return hr;
        }
    }

    //
    // Check if initialized.
    //

    if ( m_htEvent == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::GetStaticTerminals - "
            "not initialized - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    //
    // Check parameters.
    //

    if ( !pdwNumTerminals)
    {
        LOG((MSP_ERROR, "CMSPAddress::GetStaticTerminals - "
            "bad pdwNumTerminals pointer - exit E_POINTER"));

        return E_POINTER;
    }
    
    if ( ppTerminals != NULL )
    {
        if (!ppTerminals)
        {
            LOG((MSP_ERROR, "CMSPAddress::GetStaticTerminals - "
                "bad ppTerminals pointer - exit E_POINTER"));

            return E_POINTER;
        }
    }


    //
    // Grab the size of the terminals list.
    //

    int   iSize = m_Terminals.GetSize();
    _ASSERTE( iSize >= 0 );

    //
    // Add our terminals to the output list if the caller wants an output
    // list, and provided there is enough room in the output list.
    //

    if ( ( ppTerminals != NULL ) &&
         ( (DWORD) iSize <= *pdwNumTerminals ) )
    {
        //
        // For each terminal in the list of terminals we created,
        // AddRef and copy the terminal pointer.
        //

        for (int i = 0; i < iSize; i++)
        {
            m_Terminals[i]->AddRef();

            ppTerminals[i] = m_Terminals[i];
        }
    }
    
    //
    // If there was no output list then we just have to report the number
    // of terminals available.
    //
    
    if ( ppTerminals == NULL )
    {
        *pdwNumTerminals = (DWORD) iSize;

        LOG((MSP_TRACE,
            "CMSPAddress::GetStaticTerminals - just returned number of "
            "terminals available - exit S_OK"));

        return S_OK;
    }
    
    //
    // If there was an output list but it was not large enough, then
    // return the appropriate error.
    //
    
    if ( (DWORD) iSize > *pdwNumTerminals )
    {
        *pdwNumTerminals = (DWORD) iSize;

        LOG((MSP_ERROR,
            "CMSPAddress::GetStaticTerminals - passed-in array not "
            "large enough - exit TAPI_E_NOTENOUGHMEMORY"));

        return TAPI_E_NOTENOUGHMEMORY;
    }
    
    //
    // Otherwise, everything was fine. We just need to report the actual
    // number of terminals we copied and return S_OK.
    //
    
    *pdwNumTerminals = (DWORD) iSize;
    
    LOG((MSP_TRACE,
        "CMSPAddress::GetStaticTerminals - "
        "returned terminals - exit S_OK"));

    return S_OK;
}

HRESULT CMSPAddress::IsMonikerInTerminalList(IMoniker* pMoniker)
{
    CSingleFilterStaticTerminal *pCSingleFilterStaticTerminal;

    //
    // Grab the size of the terminals list.
    //

    int   iSize = m_Terminals.GetSize();
    _ASSERTE( iSize >= 0 );

    //
    // Add our terminals to the output list if the caller wants an output
    // list, and provided there is enough room in the output list.
    //

    for (int i = 0; i < iSize; i++)
    {
        pCSingleFilterStaticTerminal = static_cast<CSingleFilterStaticTerminal *>(m_Terminals[i]);

        if ( pCSingleFilterStaticTerminal->CompareMoniker( pMoniker ) == S_OK )
        {
            LOG((MSP_TRACE, "CMSPAddress::IsMonikerInTerminalList - "
                "moniker found in terminal list"));

            pCSingleFilterStaticTerminal->m_bMark = TRUE;  // mark this terminal so we don't remove it

            return S_OK;
        }

    }

    LOG((MSP_TRACE, "CMSPAddress::IsMonikerInTerminalList - "
                "moniker not found in terminal list"));
    return S_FALSE;
}

HRESULT CMSPAddress::UpdateTerminalListForPnp(
        IN  BOOL    bDeviceArrival
        )
{
    CSingleFilterStaticTerminal *pCSingleFilterStaticTerminal;

    //
    // Clear all marks in the terminal list
    //

    int   iSize = m_Terminals.GetSize();
    _ASSERTE( iSize >= 0 );

    for (int i = 0; i < iSize; i++)
    {
        pCSingleFilterStaticTerminal = static_cast<CSingleFilterStaticTerminal *>(m_Terminals[i]);

        if (pCSingleFilterStaticTerminal == NULL)
        {           
            LOG((MSP_ERROR, "CMSPAddress::IsMonikerInTerminalList - "
                    "bad terminal pointer"));
            return E_FAIL;
        }

        pCSingleFilterStaticTerminal->m_bMark = FALSE;
    }

    //
    // Create DevEnum, which is the DirectShow Category Enumerator Creator
    //

    HRESULT hr;
    ICreateDevEnum * pCreateDevEnum;

    hr = CoCreateInstance(CLSID_SystemDeviceEnum,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ICreateDevEnum,
                          (void**)&pCreateDevEnum);

    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp "
               "can't CoCreate DevEnum - returning  0x%08x", hr));
        return hr;
    }    

    IEnumMoniker  * pCatEnum;
    int i=0;
    for ( i = 0; i < m_sdwTerminalTypesCount; i++ )
    {
        //
        // Skip any terminal types that don't use one of the supported media
        // modes.
        //

        if ( ! IsValidSingleMediaType( 
            m_saTerminalTypes[i].dwMediaType, GetCallMediaTypes() ) )
        {
            continue;
        }


        //
        // Create the actual category enumerator.
        //

        hr = pCreateDevEnum->CreateClassEnumerator(
                                *(m_saTerminalTypes[i].clsidClassManager),
                                &pCatEnum,
                                0);

        if ( hr != S_OK ) // S_FALSE means the category does not exist!
        {
            LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp "
                   "can't create class enumerator - returning 0x%08x", hr));

            continue;
        }

        IMoniker      * pMoniker;

        while ((hr = pCatEnum->Next(1, &pMoniker, NULL)) == S_OK)
        {
            if (IsMonikerInTerminalList(pMoniker) == S_FALSE)
            {
                //
                // Create a terminal and give it its moniker.
                //

                ITTerminal * pTerminal;
                hr = (m_saTerminalTypes[i].pfnCreateTerm)(pMoniker,
                                                          (MSP_HANDLE) this,
                                                          &pTerminal);

                //
                // The terminal keeps a reference to the moniker if it needs to.
                //

                pMoniker->Release();

                if (SUCCEEDED(hr))
                {
                    //
                    // Add this terminal pointer to our list. Don't release it; we
                    // keep this one reference to it in the list.
                    //

                    BOOL fSuccess = m_Terminals.Add(pTerminal);

                    if ( ! fSuccess )
                    {
                        pCatEnum->Release();
                        pTerminal->Release();

                        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                            "can't add terminal to list; returning E_OUTOFMEMORY"));

                        return E_OUTOFMEMORY;
                    }

                    //
                    // Set its CMSPAddress pointer
                    //
                    CBaseTerminal * pCTerminal = static_cast<CBaseTerminal *> (pTerminal);

                    //
                    // Mark this terminal so we don't remove it
                    //

                    pCSingleFilterStaticTerminal = static_cast<CSingleFilterStaticTerminal *>(pTerminal);

                    if (pCSingleFilterStaticTerminal == NULL)
                    {           
                        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                                "bad terminal pointer"));
                        return E_FAIL;
                    }

                    pCSingleFilterStaticTerminal->m_bMark = TRUE;

                    //
                    // Post a TAPI message about the new terminal's arrival
                    //

                    pTerminal->AddRef();

                    MSPEVENTITEM *pEventItem;

                    pEventItem = AllocateEventItem();

                    if (pEventItem == NULL)
                    {
                        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                        "can't allocate event item; returning E_OUTOFMEMORY"));

                        pTerminal->Release();
                        return E_OUTOFMEMORY;
                    }

                    pEventItem->MSPEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
                    pEventItem->MSPEventInfo.Event = ME_ADDRESS_EVENT;
                    pEventItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.Type = ADDRESS_TERMINAL_AVAILABLE;
                    pEventItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.pTerminal = pTerminal;

                    hr = PostEvent(pEventItem);

                    if (FAILED(hr))
                    {
                        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                        "post event failed"));

                        pTerminal->Release();
                        FreeEventItem(pEventItem);
                    } 
                    
                }
            }

            //
            // If it failed, that either means we skipped the device because it
            // was unsuitable (a routine occurance) or something failed, like
            // out of memory. I should come up with a way to differentiate and
            // handle this well.
            //
        }

        //
        // We are done with the enumerator.
        //

        pCatEnum->Release();
    }

    //
    // Release DevEnum.
    //

    pCreateDevEnum->Release();

    //
    // Sweep the terminal list and clean up any terminals which are no longer present
    //

    iSize = m_Terminals.GetSize();
    _ASSERTE( iSize >= 0 );

    for (i = 0; i < iSize; i++)
    {
        pCSingleFilterStaticTerminal = static_cast<CSingleFilterStaticTerminal *>(m_Terminals[i]);

        if (pCSingleFilterStaticTerminal == NULL)
        {           
            LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                    "bad terminal pointer"));
            return E_FAIL;
        }

        if (!pCSingleFilterStaticTerminal->m_bMark)
        {        
            //
            // This terminal has is no longer present, lets remove it from the list
            //

            LOG((MSP_TRACE, "CMSPAddress::UpdateTerminalListForPnp "
                   "found a terminal to be removed"));

            ITTerminal * pTerminal = m_Terminals[i];

            if (m_Terminals.RemoveAt(i))
            {
                //
                // Clear its CMSPAddress pointer
                //
                CBaseTerminal * pCTerminal = static_cast<CBaseTerminal *> (pTerminal);
                
                //
                // We don't release the terminal here even though we are removing
                // it from the terminal list because TAPI3.dll will release it
                // when it releases the event.
                //

                //
                // Post a TAPI message about the new terminal's removal
                //

                MSPEVENTITEM *pEventItem;

                pEventItem = AllocateEventItem();

                if (pEventItem == NULL)
                {
                    LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                    "can't allocate event item; returning E_OUTOFMEMORY"));

                    pTerminal->Release();
                    return E_OUTOFMEMORY;
                }

                pEventItem->MSPEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
                pEventItem->MSPEventInfo.Event = ME_ADDRESS_EVENT;
                pEventItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.Type = ADDRESS_TERMINAL_UNAVAILABLE;
                pEventItem->MSPEventInfo.MSP_ADDRESS_EVENT_INFO.pTerminal = pTerminal;

                hr = PostEvent(pEventItem);

                if (FAILED(hr))
                {
                    LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalListForPnp - "
                    "post event failed"));

                    pTerminal->Release();
                    FreeEventItem(pEventItem);
                }
            
                //
                // fix up our search indices to account for a removal
                //
            
                iSize--;
                i--;
            }
        }
    }

    //
    // Our list is now complete.
    //

    m_fTerminalsUpToDate = TRUE;
    
    LOG((MSP_TRACE, "CMSPAddress::UpdateTerminalListForPnp - exit S_OK"));

    return S_OK;
}


HRESULT CMSPAddress::UpdateTerminalList(void)
{
    //
    // Create DevEnum, which is the DirectShow Category Enumerator Creator
    //

    HRESULT hr;
    ICreateDevEnum * pCreateDevEnum;

    hr = CoCreateInstance(CLSID_SystemDeviceEnum,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ICreateDevEnum,
                          (void**)&pCreateDevEnum);

    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalList "
               "can't CoCreate DevEnum - returning  0x%08x", hr));
        return hr;
    }    

    IEnumMoniker  * pCatEnum;

    for ( DWORD i = 0; i < m_sdwTerminalTypesCount; i++ )
    {
        //
        // Skip any terminal types that don't use one of the supported media
        // modes.
        //

        if ( ! IsValidSingleMediaType( 
            m_saTerminalTypes[i].dwMediaType, GetCallMediaTypes() ) )
        {
            continue;
        }


        //
        // Create the actual category enumerator.
        //

        hr = pCreateDevEnum->CreateClassEnumerator(
                                *(m_saTerminalTypes[i].clsidClassManager),
                                &pCatEnum,
                                0);

        if ( hr != S_OK ) // S_FALSE means the category does not exist!
        {
            LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalList "
                   "can't create class enumerator - returning 0x%08x", hr));

            continue;
        }

        IMoniker      * pMoniker;

        while ((hr = pCatEnum->Next(1, &pMoniker, NULL)) == S_OK)
        {
            //
            // Create a terminal and give it its moniker.
            //

            ITTerminal * pTerminal;
            hr = (m_saTerminalTypes[i].pfnCreateTerm)(pMoniker,
                                                      (MSP_HANDLE) this,
                                                      &pTerminal);

            //
            // The terminal keeps a reference to the moniker if it needs to.
            //

            pMoniker->Release();

            if (SUCCEEDED(hr))
            {
                //
                // Add this terminal pointer to our list. Don't release it; we
                // keep this one reference to it in the list.
                //

                BOOL fSuccess = m_Terminals.Add(pTerminal);

                if ( ! fSuccess )
                {
                    pCatEnum->Release();

                    LOG((MSP_ERROR, "CMSPAddress::UpdateTerminalList - "
                        "can't add terminal to list; returning E_OUTOFMEMORY"));

                    return E_OUTOFMEMORY;
                }
                //
                // Set its CMSPAddress pointer
                //
                CBaseTerminal * pCTerminal = static_cast<CBaseTerminal *> (pTerminal);

            }

            //
            // If it failed, that either means we skipped the device because it
            // was unsuitable (a routine occurance) or something failed, like
            // out of memory. I should come up with a way to differentiate and
            // handle this well.
            //
        }

        //
        // We are done with the enumerator.
        //

        pCatEnum->Release();
    }

    //
    // Release DevEnum.
    //

    pCreateDevEnum->Release();

    //
    // Our list is now complete.
    //

    m_fTerminalsUpToDate = TRUE;
    
    LOG((MSP_TRACE, "CMSPAddress::UpdateTerminalList - exit S_OK"));

    return S_OK;
}

HRESULT CMSPAddress::GetDynamicTerminalClasses(
    IN OUT  DWORD *             pdwNumClasses,
    OUT     IID *               pTerminalClasses
    )
/*++

Routine Description:

This method is called by TAPI3 to get a list of dynamic terminal guids 
that can be used on this address. It asks the terminal manager for the 
list of guids and returns them. Derived class can override this method 
to have their own guids.


Arguments:

pdwNumClasses
    Pointer to a DWORD.  On entry, indicates the size of the buffer 
    pointed to in pTerminalClasses. On success, it will be filled in 
    with the actual number of class IIDs returned.  If the buffer is 
    not big enough, the method will return TAPI_E_NOTENOUGHMEMORY, 
    and it will be filled in the with number of IIDs needed. 

pTerminalClasses

    On success, filled in with an array of terminal class IIDs that 
    are supported by the MSP for this address.  This value may be NULL, 
    in which case pdwNumClasses will return the needed buffer size.
    
Return Value:

S_OK
E_OUTOFMEMORY
TAPI_E_NOTENOUGHMEMORY

--*/
{
    LOG((MSP_TRACE,
        "CMSPAddress::GetDynamicTerminalClasses - enter"));

    //
    // Check if initialized.
    //

    // lock the event related data
    m_EventDataLock.Lock();

    if ( m_htEvent == NULL )
    {
        // unlock the event related data
        m_EventDataLock.Unlock();

        LOG((MSP_ERROR,
            "CMSPAddress::GetDynamicTerminalClasses - "
            "not initialized - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    // unlock the event related data
    m_EventDataLock.Unlock();

    //
    // Ask the Terminal Manager for the dynamic terminals that apply to
    // all of our supported media types. Since the mapping is
    // direct, the Terminal Manager takes care of all argument checking.
    //

    HRESULT hr;

    hr = m_pITTerminalManager->GetDynamicTerminalClasses(
                                                  GetCallMediaTypes(),
                                                  pdwNumClasses,
                                                  pTerminalClasses);


    LOG((MSP_TRACE,
        "CMSPAddress::GetDynamicTerminalClasses - exit 0x%08x", hr));

    return hr;
}

STDMETHODIMP CMSPAddress::CreateTerminal(
    IN      BSTR                pTerminalClass,
    IN      long                lMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    OUT     ITTerminal **       ppTerminal
    )
/*++

Routine Description:

This method is called by TAPI3 to create a dynamic terminal. It asks the 
terminal manager to create a dynamic terminal. Derived class can
override this method to have their own way of creating a dynamic terminal.

Arguments:

iidTerminalClass
    IID of the terminal class to be created.

dwMediaType
    TAPI media type of the terminal to be created.

Direction
    Terminal direction of the terminal to be created.

ppTerminal
    Returned created terminal object
    
Return Value:

S_OK

E_OUTOFMEMORY
TAPI_E_INVALIDMEDIATYPE
TAPI_E_INVALIDTERMINALDIRECTION
TAPI_E_INVALIDTERMINALCLASS

--*/
{
    LOG((MSP_TRACE,
        "CMSPAddress::CreateTerminal - enter"));

    //
    // Check if initialized.
    //

    // lock the event related data
    m_EventDataLock.Lock();

    if ( m_htEvent == NULL )
    {
        // unlock the event related data
        m_EventDataLock.Unlock();

        LOG((MSP_ERROR,
            "CMSPAddress::CreateTerminal - "
            "not initialized - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    // unlock the event related data
    m_EventDataLock.Unlock();

    //
    // Get the IID from the BSTR representation.
    //

    HRESULT hr;
    IID     iidTerminalClass;

    hr = CLSIDFromString(pTerminalClass, &iidTerminalClass);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::CreateTerminal - "
            "bad CLSID string - returning E_INVALIDARG"));

        return E_INVALIDARG;
    }

    //
    // Make sure we support the requested media type.
    // The terminal manager checks the terminal class, terminal direction, 
    // and return pointer.
    //


    
    //
    // requested media type may be aggregated, but it must still be valid
    //

    if ( !IsValidAggregatedMediaType(lMediaType) )
    {
        LOG((MSP_ERROR, "CMSPAddress::CreateTerminal - "
            "unrecognized media type requested - returning E_INVALIDARG"));

        return E_INVALIDARG;
    }

    //
    // Use the terminal manager to create the dynamic terminal.
    //

    _ASSERTE( m_pITTerminalManager != NULL );

    hr = m_pITTerminalManager->CreateDynamicTerminal(NULL,
                                                     iidTerminalClass,
                                                     (DWORD) lMediaType,
                                                     Direction,
                                                     (MSP_HANDLE) this,
                                                     ppTerminal);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::CreateTerminal - "
            "create dynamic terminal failed - returning 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CMSPAddress::CreateTerminal - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPAddress::GetDefaultStaticTerminal(
    IN      long                lMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    OUT     ITTerminal **       ppTerminal
    )
/*++

Routine Description:

This method is called by TAPI3 to get the default static terminal 
for a certain type and direction. It updates the list if needed, then
figures out which terminal is the first of the appropriate type in our
list. Derived classes can override this method to have 
their own way of deciding which terminal is the default. Locks the 
terminal lists.


Arguments:

dwMediaType
    The TAPIMEDIATYPE of the terminal to retrieve.  Only one bit will be set.

Direction
    TERMINAL_DIRECTION of the terminal to retrieve.

ppTerminal
    Default terminal returned

    
Return Value:

S_OK

E_POINTER
E_OUTOFMEMORY
TAPI_E_NOTSUPPORTED
TAPI_E_INVALIDMEDIATYPE
TAPI_E_INVALIDTERMINALDIRECTION

--*/
{
    LOG((MSP_TRACE,
        "CMSPAddress::GetDefaultStaticTerminal - enter"));

    //
    // Check if initialized.
    //

    // lock the event related data
    m_EventDataLock.Lock();

    if ( m_htEvent == NULL )
    {
        // unlock the event related data
        m_EventDataLock.Unlock();

        LOG((MSP_ERROR,
            "CMSPAddress::GetDefaultStaticTerminal - "
            "not initialized - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }
    // unlock the event related data
    m_EventDataLock.Unlock();

    //
    // Make sure we support this media type.
    //

    if ( ! IsValidSingleMediaType( (DWORD) lMediaType, GetCallMediaTypes() ) )
    {
        LOG((MSP_ERROR,
            "CMSPAddress::GetDefaultStaticTerminal - "
            "non-audio terminal requested - returning E_INVALIDARG"));

        return E_INVALIDARG;
    }

    //
    // Check the direction.
    //

    if ( ( Direction != TD_CAPTURE ) && ( Direction != TD_RENDER ) )
    {
        LOG((MSP_ERROR,
            "CMSPAddress::GetDefaultStaticTerminal - "
            "invalid direction - returning E_INVALIDARG"));

        return E_INVALIDARG;
    }

    //
    // Check return pointer.
    //

    if ( !ppTerminal)
    {
        LOG((MSP_ERROR,
            "CMSPAddress::GetDefaultStaticTerminal - "
            "bad terminal return pointer - returning E_POINTER"));

        return E_POINTER;
    }

    // lock the terminal related data. This is a auto lock that will unlock
    // when the function returns.
    CLock lock(m_TerminalDataLock);

    if (!m_fTerminalsUpToDate)
    {
       HRESULT hr = UpdateTerminalList();

        if (FAILED(hr))
        {
            LOG((MSP_ERROR,
                "CMSPAddress::GetDefaultStaticTerminal - "
                "UpdateTerminalList failed - returning 0x%08x", hr));

            return hr;
        }
    }

    //
    // For each terminal in the list of terminals we created...
    //

    int iSize = m_Terminals.GetSize();

    for (int i = 0; i < iSize; i++)
    {
        ITTerminal * pTerminal = m_Terminals[i];

        HRESULT      hr;

        //
        // Make sure this is the right direction.
        // 

        TERMINAL_DIRECTION dir;

        hr = pTerminal->get_Direction(&dir);

        if (FAILED(hr))
        {
            LOG((MSP_WARN,
                "CMSPAddress::GetDefaultStaticTerminal - "
                "could not get terminal direction - skipping"));

            continue;
        }

        if ( dir != Direction )
        {
            continue;
        }

        //
        // Make sure this is the right media type.
        //

        long lMediaTypeObserved;

        hr = pTerminal->get_MediaType(&lMediaTypeObserved);

        if (FAILED(hr))
        {
            LOG((MSP_WARN,
                "CMSPAddress::GetDefaultStaticTerminal - "
                "could not get terminal media type - skipping"));

            continue;
        }

        if ( ( lMediaTypeObserved & lMediaType) == 0 )
        {
            continue;
        }

        //
        // Ok, so this is the terminal we want. Addref it and give it to the
        // caller.
        //

        pTerminal->AddRef();

        *ppTerminal = pTerminal;

        LOG((MSP_TRACE,
            "CMSPAddress::GetDefaultStaticTerminal - "
            "returned a terminal - exit S_OK"));

        return S_OK;
    }
    
    //
    // If we get here then we did not find any matching terminals.
    //

    LOG((MSP_TRACE,
        "CMSPAddress::GetDefaultStaticTerminal - "
        "no match - exit TAPI_E_NOITEMS"));

    return TAPI_E_NOITEMS;
}

STDMETHODIMP CMSPAddress::get_PluggableSuperclasses( 
    OUT VARIANT * pVariant
    )
{
    LOG((MSP_TRACE,
        "CMSPAddress::get_PluggableSuperclasses - enter"));

    //
    // Check parameters.
    //

    if ( !pVariant)
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

    //
    // Get ITTemrinalManager2
    //

    ITTerminalManager2* pTermMgr2 = NULL;
    HRESULT hr = E_FAIL;
    hr = m_pITTerminalManager->QueryInterface(
        IID_ITTerminalManager2, (void**)&pTermMgr2);

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR,
            "CMSPAddress::get_PluggableSuperclasses - "
            "QI for ITTerminalManager2 failed - returning 0x%08x", hr));
        return hr;
    }

    //
    // Create the collection object - see mspcoll.h
    //

    typedef CTapiIfCollection< ITPluggableTerminalSuperclassInfo* > SuperclassCollection;
    CComObject<SuperclassCollection> * pCollection;
    hr = CComObject<SuperclassCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
        //Clean-up
        pTermMgr2->Release();

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

    //
    // Get the Collection's IDispatch interface
    //

    IDispatch * pDispatch;
    hr = pCollection->_InternalQueryInterface(
        IID_IDispatch,
        (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        //Clean-up
        pTermMgr2->Release();
        delete pCollection;

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));
        return hr;
    }

    //
    // Find out how many superclasses are available.
    //

    DWORD   dwNumSuperclasses = 0;

    hr = pTermMgr2->GetPluggableSuperclasses(
            &dwNumSuperclasses,
            NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "can't get number of terminals - exit 0x%08x", hr));

        //Clean-up
        pTermMgr2->Release();
        pDispatch->Release();

        return hr;
    }

    //
    // Allocate an array of IID.
    //

    IID* pSuperclassesIID = new IID[dwNumSuperclasses];
    if ( pSuperclassesIID == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "can't allocate IIDs array - exit E_OUTOFMEMORY"));

        //Clean-up
        pTermMgr2->Release();
        pDispatch->Release();

        return E_OUTOFMEMORY;
    }

    //
    // Fill in the array with actual pointers. We must do this before
    // initializing the enumerator, because the enumerator may want to
    // addref the interface pointers during initialize.
    // 

    hr = pTermMgr2->GetPluggableSuperclasses(
            &dwNumSuperclasses,
            pSuperclassesIID
            );

    //
    // Clean-up
    //

    pTermMgr2->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "can't get IIDs - exit 0x%08x", hr));

        //Clean-up
        pDispatch->Release();
        delete[] pSuperclassesIID;

        return hr;
    }

    //
    // Allocate an array of ITPluggableTerminalSuperclassInfo
    //
    typedef ITPluggableTerminalSuperclassInfo* SuperclassPtr; // MS parser
    SuperclassPtr * ppSuperclassesInfo = new SuperclassPtr[dwNumSuperclasses];
    if ( ppSuperclassesInfo == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "can't allocate SuperclassPtr array - exit E_OUTOFMEMORY"));

        //Clean-up
        pDispatch->Release();
        delete[] pSuperclassesIID;

        return E_OUTOFMEMORY;
    }

    //
    // Get ITPluggableTerminalSuperclassRegistration interface
    //

    ITPluggableTerminalSuperclassRegistration* pSuperclassReg = NULL;
    hr = CoCreateInstance(
        CLSID_PluggableSuperclassRegistration,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITPluggableTerminalSuperclassRegistration,
        (void**)&pSuperclassReg
        );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
            "QI for ITPluggableTerminalSuperclassRegistration - exit 0x%08x",hr));

        //Clean-up
        pDispatch->Release();
        delete[] pSuperclassesIID;
        delete[] ppSuperclassesInfo;

        return hr;
    }

    //
    // Create the objects
    //

    for(DWORD dwIndex = 0; dwIndex < dwNumSuperclasses; dwIndex++)
    {
        //
        // Get the string from the IID
        //
        LPOLESTR lpszCLSID = NULL;
        hr = StringFromIID( pSuperclassesIID[dwIndex], &lpszCLSID);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
                "StringFromIID failed - exit 0x%08x",hr));

            //Clean-up
            pDispatch->Release();
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();

            return hr;
        }

        //
        // Get BSTR for IID
        //
        BSTR bstrCLSID = SysAllocString( lpszCLSID );
        CoTaskMemFree( lpszCLSID ); // Clean-up
        if( NULL == bstrCLSID)
        {
            LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
                "SysAllocString failed - exit E_OUTOFMEMORY"));

            // Clean-up
            pDispatch->Release();
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();

            return E_OUTOFMEMORY;
        }

        //
        // Read information from registry
        //

        pSuperclassReg->put_CLSID( bstrCLSID);
        hr = pSuperclassReg->GetTerminalSuperclassInfo();
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
                "GetTerminalSuperclassInfo failed - exit 0x%08x",hr));

            // Clean-up
            pDispatch->Release();
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID);

            return hr;
        }

        //
        // Get the name
        //
        BSTR bstrName = NULL;
        pSuperclassReg->get_Name( &bstrName );

        //
        // Create the information object
        //
        CComObject<CPlugTerminalSuperclassInfo>* pSuperclassInfo = NULL;
        hr = CComObject<CPlugTerminalSuperclassInfo>::CreateInstance(&pSuperclassInfo);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - "
                "CreateInstance failed - exit 0x%08x", hr));

            //Clean-up
            pDispatch->Release();
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID );
            SysFreeString( bstrName );

            return hr;
        }

        //
        // Get ITPluggableTerminalSuperclassInfo from this superclass
        //

        pSuperclassInfo->QueryInterface(
            IID_ITPluggableTerminalSuperclassInfo, 
            (void**)&ppSuperclassesInfo[dwIndex]
            );

        //
        // Set the fields
        //

        pSuperclassInfo->put_Name( bstrName);
        pSuperclassInfo->put_CLSID( bstrCLSID );

        //
        // Clean-up
        //
        SysFreeString( bstrCLSID );
        SysFreeString( bstrName );
    }


    //
    // Clean-up the IIDs array
    //

    pSuperclassReg->Release();
    delete[] pSuperclassesIID;

    //
    // Init the collection using an iterator -- pointers to the beginning and
    // the ending element plus one. If it succeeds, this method addrefs each
    // element of ppterminals by querying for IDispatch.
    //

    hr = pCollection->Initialize( dwNumSuperclasses,
                                  ppSuperclassesInfo,
                                  ppSuperclassesInfo + dwNumSuperclasses );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableSuperclasses - exit "
            "pCollection->Initialize failed. returns 0x%08x", hr));

        delete[] ppSuperclassesInfo;
        pDispatch->Release();

        return hr;
    }

    //
    // put the IDispatch interface pointer into the variant
    //

    LOG((MSP_INFO, "CMSPAddress::get_PluggableSuperclasses - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPAddress::get_PluggableSuperclasses - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPAddress::EnumeratePluggableSuperclasses( 
    OUT IEnumPluggableSuperclassInfo** ppSuperclassEnumerator 
    )
{
    LOG((MSP_TRACE,
        "CMSPAddress::EnumeratePluggableSuperclasses - enter"));

    //
    // Check parameters.
    //

    if ( !ppSuperclassEnumerator)
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

    //
    // Get ITTemrinalManager2
    //

    ITTerminalManager2* pTermMgr2 = NULL;
    HRESULT hr = E_FAIL;
    hr = m_pITTerminalManager->QueryInterface(
        IID_ITTerminalManager2, (void**)&pTermMgr2);

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR,
            "CMSPAddress::EnumeratePluggableSuperclasses - "
            "QI for ITTerminalManager2 failed - returning 0x%08x", hr));
        return hr;
    }

    //
    // Find out how many superclasses are available.
    //

    DWORD   dwNumSuperclasses = 0;

    hr = pTermMgr2->GetPluggableSuperclasses(
            &dwNumSuperclasses,
            NULL);

    if ( FAILED(hr) )
    {
        // Clean-up
        pTermMgr2->Release();

        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't get number of terminals - exit 0x%08x", hr));

        return hr;
    }

    //
    // Allocate an array of IID.
    //

    IID* pSuperclassesIID = new IID[dwNumSuperclasses];
    if ( pSuperclassesIID == NULL )
    {
        // Clean-up
        pTermMgr2->Release();

        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't allocate IIDs array - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    //
    // Fill in the array with actual pointers. We must do this before
    // initializing the enumerator, because the enumerator may want to
    // addref the interface pointers during initialize.
    // 

    hr = pTermMgr2->GetPluggableSuperclasses(
            &dwNumSuperclasses,
            pSuperclassesIID
            );

    //
    // Clean-up
    //

    pTermMgr2->Release();

    if ( FAILED(hr) )
    {
        //Clean-up
        delete[] pSuperclassesIID;

        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't get IIDs - exit 0x%08x", hr));

        return hr;
    }

    //
    // Allocate an array of ITPluggableTerminalSuperclassInfo
    //
    typedef ITPluggableTerminalSuperclassInfo* SuperclassPtr; // MS parser
    SuperclassPtr * ppSuperclassesInfo = new SuperclassPtr[dwNumSuperclasses];
    if ( ppSuperclassesInfo == NULL )
    {
        // Clean-up
        delete[] pSuperclassesIID;

        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't allocate SuperclassPtr array - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    //
    // Get ITPluggableTerminalSuperclassRegistration interface
    //

    ITPluggableTerminalSuperclassRegistration* pSuperclassReg = NULL;
    hr = CoCreateInstance(
        CLSID_PluggableSuperclassRegistration,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITPluggableTerminalSuperclassRegistration,
        (void**)&pSuperclassReg
        );
    if( FAILED(hr) )
    {
        // Clean-up
        delete[] pSuperclassesIID;
        delete[] ppSuperclassesInfo;

        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "QI for ITPluggableTerminalSuperclassRegistration - exit 0x%08x",hr));

        return hr;
    }

    //
    // Create the objects
    //

    for(DWORD dwIndex = 0; dwIndex < dwNumSuperclasses; dwIndex++)
    {
        //
        // Get the string from the IID
        //
        LPOLESTR lpszCLSID = NULL;
        hr = StringFromIID( pSuperclassesIID[dwIndex], &lpszCLSID);
        if( FAILED(hr) )
        {
            //Clean-up
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();

            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
                "StringFromIID failed - exit 0x%08x",hr));

            return hr;
        }

        //
        // Get BSTR for IID
        //
        BSTR bstrCLSID = SysAllocString( lpszCLSID );
        CoTaskMemFree( lpszCLSID ); // Clean-up
        if( NULL == bstrCLSID)
        {
            //Clean-up
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();

            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
                "SysAllocString failed - exit E_OUTOFMEMORY"));

            return E_OUTOFMEMORY;
        }

        //
        // Read information from registry
        //

        pSuperclassReg->put_CLSID( bstrCLSID);
        hr = pSuperclassReg->GetTerminalSuperclassInfo();
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
                "GetTerminalSuperclassInfo failed - exit 0x%08x",hr));

            // Clean-up
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID);

            return hr;
        }

        //
        // Get the name
        //
        BSTR bstrName = NULL;
        pSuperclassReg->get_Name( &bstrName );

        //
        // Create the information object
        //
        CComObject<CPlugTerminalSuperclassInfo>* pSuperclassInfo = NULL;
        hr = CComObject<CPlugTerminalSuperclassInfo>::CreateInstance(&pSuperclassInfo);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
                "CreateInstance failed - exit 0x%08x", hr));

            // Clean-up
            delete[] pSuperclassesIID;
            delete[] ppSuperclassesInfo;
            pSuperclassReg->Release();
            SysFreeString( bstrCLSID );
            SysFreeString( bstrName );

            return hr;
        }

        //
        // Get ITPluggableTerminalSuperclassInfo from this superclass
        //

        pSuperclassInfo->QueryInterface(
            IID_ITPluggableTerminalSuperclassInfo, 
            (void**)&ppSuperclassesInfo[dwIndex]
            );

        //
        // Set the fields
        //

        pSuperclassInfo->put_Name( bstrName);
        pSuperclassInfo->put_CLSID( bstrCLSID );

        //
        // Clean-up
        //
        SysFreeString( bstrCLSID );
        SysFreeString( bstrName );
    }


    //
    // Clean-up the IIDs array
    //

    pSuperclassReg->Release();
    delete[] pSuperclassesIID;

    //
    // Create the enumerator object.
    //

    typedef CSafeComEnum<IEnumPluggableSuperclassInfo,
                     &IID_IEnumPluggableSuperclassInfo,
                     ITPluggableTerminalSuperclassInfo*, 
                     _CopyInterface<ITPluggableTerminalSuperclassInfo> > CEnumerator;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't create enumerator - exit 0x%08x", hr));

        delete[] ppSuperclassesInfo;
        return hr;
    }

    //
    // Query for the desired interface.
    //

    hr = pEnum->_InternalQueryInterface(
        IID_IEnumPluggableSuperclassInfo, 
        (void**) ppSuperclassEnumerator
        );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't get enumerator interface - exit 0x%08x", hr));

        delete pEnum;
        delete[] ppSuperclassesInfo;
        
        return hr;
    }

    //
    // Init the enumerator object.
    //

    hr = pEnum->Init(ppSuperclassesInfo,
                     ppSuperclassesInfo + dwNumSuperclasses,
                     NULL,
                     AtlFlagTakeOwnership); 

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableSuperclasses - "
            "can't init enumerator - exit 0x%08x", hr));

        (*ppSuperclassEnumerator)->Release();
        delete[] ppSuperclassesInfo;
        
        return hr;
    }
    
    LOG((MSP_TRACE, "CMSPAddress::EnumeratePluggableSuperclasses - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPAddress::get_PluggableTerminalClasses( 
    IN  BSTR bstrTerminalSuperclass,
    IN  long lMediaType,
    OUT VARIANT * pVariant
    )
{
    LOG((MSP_TRACE,
        "CMSPAddress::get_PluggableTerminalClasses - enter"));

    //
    // Check parameters.
    //

    if ( !pVariant)
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

    if( ! bstrTerminalSuperclass)
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "bad pointer argument - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    IID iidSuperclass = IID_NULL;
    HRESULT hr = IIDFromString( bstrTerminalSuperclass, &iidSuperclass);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "bad pointer argument - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    //
    // Get ITTemrinalManager2
    //

    ITTerminalManager2* pTermMgr2 = NULL;
    hr = m_pITTerminalManager->QueryInterface(
        IID_ITTerminalManager2, (void**)&pTermMgr2);

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR,
            "CMSPAddress::get_PluggableTerminalClasses - "
            "QI for ITTerminalManager2 failed - returning 0x%08x", hr));
        return hr;
    }

    //
    // Create the collection object - see mspcoll.h
    //

    typedef CTapiIfCollection< ITPluggableTerminalClassInfo* > ClassCollection;
    CComObject<ClassCollection> * pCollection;
    hr = CComObject<ClassCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
        //Clean-up
        pTermMgr2->Release();

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

    //
    // Get the Collection's IDispatch interface
    //

    IDispatch * pDispatch;
    hr = pCollection->_InternalQueryInterface(
        IID_IDispatch,
        (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        // Clean-up
        pTermMgr2->Release();
        delete pCollection;

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        return hr;
    }

    //
    // Find out how many superclasses are available.
    //

    DWORD   dwNumClasses = 0;

    hr = pTermMgr2->GetPluggableTerminalClasses(
            iidSuperclass,
            lMediaType,
            &dwNumClasses,
            NULL);

    if ( FAILED(hr) )
    {
        //Clean-up
        pTermMgr2->Release();
        pDispatch->Release();

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "can't get number of terminals - exit 0x%08x", hr));

        return hr;
    }

    //
    // Allocate an array of IID.
    //

    IID* pClassesIID = new IID[dwNumClasses];
    if ( pClassesIID == NULL )
    {
        //Clean-up
        pTermMgr2->Release();
        pDispatch->Release();

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "can't allocate IIDs array - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    //
    // Fill in the array with actual pointers. We must do this before
    // initializing the enumerator, because the enumerator may want to
    // addref the interface pointers during initialize.
    // 

    hr = pTermMgr2->GetPluggableTerminalClasses(
            iidSuperclass,
            lMediaType,
            &dwNumClasses,
            pClassesIID
            );

    //
    // Clean-up
    //

    pTermMgr2->Release();

    if ( FAILED(hr) )
    {
        //Clean-up
        pDispatch->Release();
        delete[] pClassesIID;

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "can't get IIDs - exit 0x%08x", hr));

        return hr;
    }

    //
    // Allocate an array of ITPluggableTerminalClassInfo
    //
    typedef ITPluggableTerminalClassInfo* ClassPtr;
    ClassPtr * ppClassesInfo = new ClassPtr[dwNumClasses];
    if ( ppClassesInfo == NULL )
    {
        //Clean-up
        pDispatch->Release();
        delete[] pClassesIID;

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "can't allocate ClassPtr array - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    //
    // Get ITPluggableTerminalClassRegistration interface
    //

    ITPluggableTerminalClassRegistration* pClassReg = NULL;
    hr = CoCreateInstance(
        CLSID_PluggableTerminalRegistration,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITPluggableTerminalClassRegistration,
        (void**)&pClassReg
        );
    if( FAILED(hr) )
    {
        //Clean-up
        pDispatch->Release();
        delete[] pClassesIID;
        delete[] ppClassesInfo;

        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
            "QI for ITPluggableTerminalClassRegistration - exit 0x%08x",hr));

        return hr;
    }

    //
    // Create the objects
    //

    for(DWORD dwIndex = 0; dwIndex < dwNumClasses; dwIndex++)
    {
        //
        // Get the string from the IID
        //
        LPOLESTR lpszPublicCLSID = NULL;
        hr = StringFromIID( pClassesIID[dwIndex], &lpszPublicCLSID);
        if( FAILED(hr) )
        {
            //Clean-up
            pDispatch->Release();
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();

            LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
                "StringFromIID failed - exit 0x%08x",hr));

            return hr;
        }

        //
        // Get BSTR for IID
        //
        BSTR bstrPublicCLSID = SysAllocString( lpszPublicCLSID );
        CoTaskMemFree( lpszPublicCLSID ); // Clean-up
        if( NULL == bstrPublicCLSID)
        {
            //Clean-up
            pDispatch->Release();
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();

            LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
                "SysAllocString failed - exit E_OUTOFMEMORY"));

            return E_OUTOFMEMORY;
        }

        //
        // Read information from registry
        //

        pClassReg->put_TerminalClass( bstrPublicCLSID);
        hr = pClassReg->GetTerminalClassInfo(
            bstrTerminalSuperclass);
        if( FAILED(hr) )
        {
            // Clean-up
            pDispatch->Release();
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();
            SysFreeString( bstrPublicCLSID);

            LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
                "GetTerminalInfo failed - exit 0x%08x",hr));

            return hr;
        }

        //
        // Get the name
        //
        BSTR bstrName = NULL;
        pClassReg->get_Name( &bstrName );
        BSTR bstrCompany = NULL;
        pClassReg->get_Company( &bstrCompany );
        BSTR bstrVersion = NULL;
        pClassReg->get_Version( &bstrVersion );
        BSTR bstrCLSID = NULL;
        pClassReg->get_CLSID( &bstrCLSID );
        TMGR_DIRECTION Direction = TMGR_TD_CAPTURE;
        pClassReg->get_Direction( &Direction );
        long lMediaType = 0;
        pClassReg->get_MediaTypes( &lMediaType );

        //
        // Create the information object
        //
        CComObject<CPlugTerminalClassInfo>* pClassInfo = NULL;
        hr = CComObject<CPlugTerminalClassInfo>::CreateInstance(&pClassInfo);
        if( FAILED(hr) )
        {
            //Clean-up
            pDispatch->Release();
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();
            SysFreeString( bstrPublicCLSID );
            SysFreeString( bstrName );
            SysFreeString( bstrCompany );
            SysFreeString( bstrVersion );
            SysFreeString( bstrCLSID );

            LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - "
                "CreateInstance failed - exit 0x%08x", hr));

            return hr;
        }

        //
        // Get ITPluggableTerminalClassInfo from this superclass
        //

        pClassInfo->QueryInterface(
            IID_ITPluggableTerminalClassInfo, 
            (void**)&ppClassesInfo[dwIndex]
            );

        //
        // Set the fields
        //

        if( NULL == bstrName)
        {
            bstrName = SysAllocString(L"");
        }
        pClassInfo->put_Name( bstrName);
        pClassInfo->put_TerminalClass( bstrPublicCLSID );
        if( NULL == bstrCompany)
        {
            bstrCompany = SysAllocString(L"");
        }
        pClassInfo->put_Company( bstrCompany );
        if( NULL == bstrVersion)
        {
            bstrVersion = SysAllocString(L"");
        }
        pClassInfo->put_Version( bstrVersion );
        if( NULL == bstrCLSID)
        {
            LPOLESTR lpszCLSID = NULL;
            StringFromCLSID( CLSID_NULL, &lpszCLSID);
            bstrCLSID = SysAllocString(lpszCLSID);
            CoTaskMemFree( lpszCLSID);
        }
        pClassInfo->put_CLSID( bstrCLSID );
        TERMINAL_DIRECTION TermDirection = TD_CAPTURE;
        switch( Direction )
        {
        case TMGR_TD_RENDER:
            TermDirection = TD_RENDER;
            break;
        case TMGR_TD_BOTH:
            TermDirection = TD_BIDIRECTIONAL;
            break;
        case TMGR_TD_CAPTURE:
        default:
            TermDirection = TD_CAPTURE;
            break;
        }
        pClassInfo->put_Direction( TermDirection );
        pClassInfo->put_MediaTypes( lMediaType );

        //
        // Clean-up
        //
        SysFreeString( bstrPublicCLSID );
        SysFreeString( bstrName );
        SysFreeString( bstrCompany );
        SysFreeString( bstrVersion );
        SysFreeString( bstrCLSID );
    }


    //
    // Clean-up the IIDs array
    //

    pClassReg->Release();
    delete[] pClassesIID;

    //
    // Init the collection using an iterator -- pointers to the beginning and
    // the ending element plus one. If it succeeds, this method addrefs each
    // element of ppterminals by querying for IDispatch.
    //

    hr = pCollection->Initialize( dwNumClasses,
                                  ppClassesInfo,
                                  ppClassesInfo + dwNumClasses );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_PluggableTerminalClasses - exit "
            "pCollection->Initialize failed. returns 0x%08x", hr));

        delete[] ppClassesInfo;
        pDispatch->Release();

        return hr;
    }

    //
    // put the IDispatch interface pointer into the variant
    //

    LOG((MSP_INFO, "CMSPAddress::get_PluggableTerminalClasses - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPAddress::get_PluggableTerminalClasses - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPAddress::EnumeratePluggableTerminalClasses(
    IN  CLSID iidTerminalSuperclass,
    IN  long lMediaType,
    OUT IEnumPluggableTerminalClassInfo ** ppClassEnumerator 
    )
{
    LOG((MSP_TRACE,
        "CMSPAddress::EnumeratePluggableTerminalClasses - enter"));

    //
    // Check parameters.
    //

    if ( !ppClassEnumerator)
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

    LPOLESTR lpszCLSID = NULL;
    HRESULT hr = StringFromCLSID( iidTerminalSuperclass, &lpszCLSID);
    if( FAILED(hr) )
    {
         LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "StringFromCLSID failed - exit 0x%08x", hr));

        return hr;
    }

    BSTR bstrTerminalSuperclass = SysAllocString( lpszCLSID );

    // Clean-up
    CoTaskMemFree(lpszCLSID);
    lpszCLSID = NULL;


    if( NULL == bstrTerminalSuperclass )
    {
         LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "SysAllocString failed - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    //
    // Get ITTemrinalManager2
    //

    ITTerminalManager2* pTermMgr2 = NULL;
    hr = m_pITTerminalManager->QueryInterface(
        IID_ITTerminalManager2, (void**)&pTermMgr2);

    if( FAILED(hr) )
    {
        //Clean-up
        SysFreeString( bstrTerminalSuperclass );

        LOG((MSP_ERROR,
            "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "QI for ITTerminalManager2 failed - returning 0x%08x", hr));
        return hr;
    }

    //
    // Find out how many superclasses are available.
    //

    DWORD   dwNumClasses = 0;

    hr = pTermMgr2->GetPluggableTerminalClasses(
            iidTerminalSuperclass,
            lMediaType,
            &dwNumClasses,
            NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't get number of terminals - exit 0x%08x", hr));

        // Clean-up
        SysFreeString( bstrTerminalSuperclass );
        pTermMgr2->Release();

        return hr;
    }

    //
    // Allocate an array of IID.
    //

    IID* pClassesIID = new IID[dwNumClasses];
    if ( pClassesIID == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't allocate IIDs array - exit E_OUTOFMEMORY"));

        // Clean-up
        SysFreeString( bstrTerminalSuperclass );
        pTermMgr2->Release();

        return E_OUTOFMEMORY;
    }

    //
    // Fill in the array with actual pointers. We must do this before
    // initializing the enumerator, because the enumerator may want to
    // addref the interface pointers during initialize.
    // 

    hr = pTermMgr2->GetPluggableTerminalClasses(
            iidTerminalSuperclass,
            lMediaType,
            &dwNumClasses,
            pClassesIID
            );

    //
    // Clean-up
    //

    pTermMgr2->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't get IIDs - exit 0x%08x", hr));

        // Clean-up
        SysFreeString( bstrTerminalSuperclass );
        delete[] pClassesIID;

        return hr;
    }

    //
    // Allocate an array of ITPluggableTerminalClassInfo
    //
    typedef ITPluggableTerminalClassInfo* ClassPtr;
    ClassPtr * ppClassesInfo = new ClassPtr[dwNumClasses];
    if ( ppClassesInfo == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't allocate ClassPtr array - exit E_OUTOFMEMORY"));

        // Clean-up
        SysFreeString( bstrTerminalSuperclass );
        delete[] pClassesIID;

        return E_OUTOFMEMORY;
    }

    //
    // Get ITPluggableTerminalClassRegistration interface
    //

    ITPluggableTerminalClassRegistration* pClassReg = NULL;
    hr = CoCreateInstance(
        CLSID_PluggableTerminalRegistration,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITPluggableTerminalClassRegistration,
        (void**)&pClassReg
        );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "QI for ITPluggableTerminalClassRegistration - exit 0x%08x",hr));

        // Clean-up
        SysFreeString( bstrTerminalSuperclass );
        delete[] ppClassesInfo;
        delete[] pClassesIID;

        return hr;
    }

    //
    // Create the objects
    //

    for(DWORD dwIndex = 0; dwIndex < dwNumClasses; dwIndex++)
    {
        //
        // Get the string from the IID
        //
        LPOLESTR lpszPublicCLSID = NULL;
        hr = StringFromIID( pClassesIID[dwIndex], &lpszPublicCLSID);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
                "StringFromIID failed - exit 0x%08x",hr));

            // Clean-up
            SysFreeString( bstrTerminalSuperclass );
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();

            return hr;
        }

        //
        // Get BSTR for IID
        //
        BSTR bstrPublicCLSID = SysAllocString( lpszPublicCLSID );
        CoTaskMemFree( lpszPublicCLSID ); // Clean-up
        if( NULL == bstrPublicCLSID)
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
                "SysAllocString failed - exit E_OUTOFMEMORY"));


            // Clean-up
            SysFreeString( bstrTerminalSuperclass );
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();

            return E_OUTOFMEMORY;
        }

        //
        // Read information from registry
        //

        pClassReg->put_TerminalClass( bstrPublicCLSID);
        hr = pClassReg->GetTerminalClassInfo(
            bstrTerminalSuperclass);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
                "GetTerminalInfo failed - exit 0x%08x",hr));

            // Clean-up
            SysFreeString( bstrTerminalSuperclass );
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();
            SysFreeString( bstrPublicCLSID);

            return hr;
        }

        //
        // Get the name
        //
        BSTR bstrName = NULL;
        pClassReg->get_Name( &bstrName );
        BSTR bstrCompany = NULL;
        pClassReg->get_Company( &bstrCompany );
        BSTR bstrVersion = NULL;
        pClassReg->get_Version( &bstrVersion );
        BSTR bstrCLSID = NULL;
        pClassReg->get_CLSID( &bstrCLSID );
        TMGR_DIRECTION Direction = TMGR_TD_CAPTURE;
        pClassReg->get_Direction( &Direction );
        long lMediaType = 0;
        pClassReg->get_MediaTypes( &lMediaType );

        //
        // Create the information object
        //
        CComObject<CPlugTerminalClassInfo>* pClassInfo = NULL;
        hr = CComObject<CPlugTerminalClassInfo>::CreateInstance(&pClassInfo);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
                "CreateInstance failed - exit 0x%08x", hr));

            // Clean-up
            SysFreeString( bstrTerminalSuperclass );
            delete[] pClassesIID;
            delete[] ppClassesInfo;
            pClassReg->Release();
            SysFreeString( bstrPublicCLSID );
            SysFreeString( bstrName );
            SysFreeString( bstrCompany );
            SysFreeString( bstrVersion );
            SysFreeString( bstrCLSID );

            return hr;
        }

        //
        // Get ITPluggableTerminalClassInfo from this superclass
        //

        pClassInfo->QueryInterface(
            IID_ITPluggableTerminalClassInfo, 
            (void**)&ppClassesInfo[dwIndex]
            );

        //
        // Set the fields
        //

        if( NULL == bstrName)
        {
            bstrName = SysAllocString(L"");
        }
        pClassInfo->put_Name( bstrName);
        pClassInfo->put_TerminalClass( bstrPublicCLSID );
        if( NULL == bstrCompany)
        {
            bstrCompany = SysAllocString(L"");
        }
        pClassInfo->put_Company( bstrCompany );
        if( NULL == bstrVersion)
        {
            bstrVersion = SysAllocString(L"");
        }
        pClassInfo->put_Version( bstrVersion );
        if( NULL == bstrCLSID)
        {
            LPOLESTR lpszCLSID = NULL;
            StringFromCLSID( CLSID_NULL, &lpszCLSID);
            bstrCLSID = SysAllocString(lpszCLSID);
            CoTaskMemFree( lpszCLSID);
        }
        pClassInfo->put_CLSID( bstrCLSID );
        TERMINAL_DIRECTION TermDirection = TD_CAPTURE;
        switch( Direction )
        {
        case TMGR_TD_RENDER:
            TermDirection = TD_RENDER;
            break;
        case TMGR_TD_BOTH:
            TermDirection = TD_BIDIRECTIONAL;
            break;
        case TMGR_TD_CAPTURE:
        default:
            TermDirection = TD_CAPTURE;
            break;
        }
        pClassInfo->put_Direction( TermDirection );
        pClassInfo->put_MediaTypes( lMediaType );

        //
        // Clean-up
        //
        SysFreeString( bstrPublicCLSID );
        SysFreeString( bstrName );
        SysFreeString( bstrCompany );
        SysFreeString( bstrVersion );
        SysFreeString( bstrCLSID );
    }


    //
    // Clean-up the IIDs array
    //

    SysFreeString( bstrTerminalSuperclass );
    delete[] pClassesIID;
    pClassReg->Release();

    //
    // Create the enumerator object.
    //

    typedef CSafeComEnum<IEnumPluggableTerminalClassInfo,
                     &IID_IEnumPluggableTerminalClassInfo,
                     ITPluggableTerminalClassInfo*, 
                     _CopyInterface<ITPluggableTerminalClassInfo> > CEnumerator;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't create enumerator - exit 0x%08x", hr));

        delete[] ppClassesInfo;
        return hr;
    }

    //
    // Query for the desired interface.
    //

    hr = pEnum->_InternalQueryInterface(
        IID_IEnumPluggableTerminalClassInfo, 
        (void**) ppClassEnumerator
        );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't get enumerator interface - exit 0x%08x", hr));

        delete pEnum;
        delete[] ppClassesInfo;
        
        return hr;
    }

    //
    // Init the enumerator object.
    //

    hr = pEnum->Init(ppClassesInfo,
                     ppClassesInfo + dwNumClasses,
                     NULL,
                     AtlFlagTakeOwnership); 

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumeratePluggableTerminalClasses - "
            "can't init enumerator - exit 0x%08x", hr));

        (*ppClassEnumerator)->Release();
        delete[] ppClassesInfo;
        
        return hr;
    }
    
    LOG((MSP_TRACE, "CMSPAddress::EnumeratePluggableTerminalClasses - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CMSPAddress::GetEvent(
    IN OUT  DWORD *             pdwSize,
    OUT     BYTE *              pBuffer
    )
/*++

Routine Description:

This method is called by TAPI3 to get detailed information about the 
event that just happened. TAPI3 will normally do this after its event 
is signaled. Locks the event list.

Arguments:

pMSPEvent
    The MSP_EVENT

pdwSize
    Pointer to a DWORD.  On entry, indicates the size in bytes of the 
    buffer pointed to in pbuffer. On success, it will be filled in with 
    the actual number of bytes returned.  If the buffer is not big enough, 
    the method will return TAPI_E_NOTENOUGHMEMORY, and it will be filled 
    in the with number of bytes needed. 

pBuffer
    Event buffer filled in by MSP with the relevant events
    
Return Value:

S_OK
E_OUTOFMEMORY
TAPI_E_NOEVENT
TAPI_E_NOTENOUGHMEMORY

--*/
{
    // We trust TAPI3 not to give us bad pointers.
    _ASSERTE(!!pdwSize);
    _ASSERTE((*pdwSize == 0) ? TRUE : 
        !!pBuffer);

    LOG((MSP_TRACE, "CMSPAddress::GetEvent"));

    CLock lock(m_EventDataLock);

    if (IsListEmpty(&m_EventList))
    {
        return TAPI_E_NOEVENT;
    }

    // retrieve first entry
    PLIST_ENTRY pLE = m_EventList.Flink;

    // convert list entry to structure pointer
    PMSPEVENTITEM pItem = CONTAINING_RECORD(pLE, MSPEVENTITEM, Link);

    if (pItem->MSPEventInfo.dwSize > *pdwSize)
    {
        *pdwSize = pItem->MSPEventInfo.dwSize;
        return TAPI_E_NOTENOUGHMEMORY;
    }
    
    CopyMemory(pBuffer, &pItem->MSPEventInfo, pItem->MSPEventInfo.dwSize);
    *pdwSize = pItem->MSPEventInfo.dwSize;

    // remove the first entry from the event list.
    RemoveHeadList(&m_EventList);

    // free the memory.
    FreeEventItem(pItem);

    return S_OK;
}

HRESULT CMSPAddress::PostEvent(
        IN      MSPEVENTITEM *      pEventItem
        )
/*++

Routine Description:

This method is called by MSPCalls to post an event to TAPI3. This method 
puts the event at the end of the event list and singals TAPI3. 
Locks the event list.


Arguments:

EventItem
    The event to be queued.
    
Return Value:

S_OK
E_OUTOFMEMORY

--*/
{
    CLock lock(m_EventDataLock);

    if (m_htEvent == NULL)
    {
        return E_UNEXPECTED;  // the address was shut down.
    }

    InsertTailList(&m_EventList, &pEventItem->Link);

    SetEvent(m_htEvent);

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// OLE Automation wrappers


STDMETHODIMP CMSPAddress::get_StaticTerminals (
        OUT  VARIANT * pVariant
        )
{
    LOG((MSP_TRACE, "CMSPAddress::get_StaticTerminals - enter"));

    //
    // Check parameters.
    //

    if ( !pVariant)
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

    //
    // create the collection object - see mspcoll.h
    //

    typedef CTapiIfCollection< ITTerminal * > TerminalCollection;
    CComObject<TerminalCollection> * pCollection;
    HRESULT hr = CComObject<TerminalCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

    //
    // get the Collection's IDispatch interface
    //

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(IID_IDispatch,
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

    //
    // Find out how many terminals are available.
    //

    DWORD   dwNumTerminals;

    hr = GetStaticTerminals(&dwNumTerminals,
                            NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "can't get number of terminals - exit 0x%08x", hr));

        pDispatch->Release();

        return hr;
    }

    //
    // Allocate an array of terminal pointers.
    //

    typedef ITTerminal * TermPtr;
    TermPtr * ppTerminals = new TermPtr[dwNumTerminals];

    if ( ppTerminals == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "can't allocate terminals array - exit E_OUTOFMEMORY"));

        pDispatch->Release();

        return E_OUTOFMEMORY;
    }

    //
    // Fill in the array with actual pointers. We must do this before
    // initializing the enumerator, because the enumerator may want to
    // addref the interface pointers during initialize.
    // 

    hr = GetStaticTerminals(&dwNumTerminals,
                            ppTerminals);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "can't get terminals - exit 0x%08x", hr));

        pDispatch->Release();
        delete ppTerminals;

        return hr;
    }
    //
    // Init the collection using an iterator -- pointers to the beginning and
    // the ending element plus one. If it succeeds, this method addrefs each
    // element of ppterminals by querying for IDispatch.
    //

    hr = pCollection->Initialize( dwNumTerminals,
                                  ppTerminals,
                                  ppTerminals + dwNumTerminals );

    //
    // Release the ITTerminal reference to each terminal (leaving the
    // IDispatch reference, if any). Then delete the array; the
    // collection is now storing the pointers.
    //

    for (DWORD i = 0; i < dwNumTerminals; i++)
    {
        ppTerminals[i]->Release();
    }

    delete ppTerminals;
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();

        return hr;
    }

    //
    // put the IDispatch interface pointer into the variant
    //

    LOG((MSP_ERROR, "CMSPAddress::get_StaticTerminals - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPAddress::get_StaticTerminals - exit S_OK"));
 
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CMSPAddress::EnumerateStaticTerminals (
        OUT  IEnumTerminal ** ppTerminalEnumerator
        )
{
    LOG((MSP_TRACE, "CMSPAddress::EnumerateStaticTerminals - "
        "enter"));

    //
    // Check the return pointer.
    //

    if ( !ppTerminalEnumerator)
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "bad return pointer - exit E_POINTER"));

        return E_POINTER;
    }

    //
    // Create the enumerator object.
    //

    typedef _CopyInterface<ITTerminal> CCopy;
    typedef CSafeComEnum<IEnumTerminal, &IID_IEnumTerminal,
        ITTerminal *, CCopy> CEnumerator;

    HRESULT hr;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "can't create enumerator - exit 0x%08x", hr));

        return hr;
    }

    //
    // Query for the desired interface.
    //

    hr = pEnum->_InternalQueryInterface(IID_IEnumTerminal,
                                        (void**) ppTerminalEnumerator);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "can't get enumerator interface - exit 0x%08x", hr));

        delete pEnum;

        return hr;
    }

    //
    // Find out how many terminals are available.
    //

    DWORD   dwNumTerminals;

    hr = GetStaticTerminals(&dwNumTerminals,
                            NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "can't get number of terminals - exit 0x%08x", hr));

        (*ppTerminalEnumerator)->Release();

        return hr;
    }

    //
    // Allocate an array of terminal pointers.
    //

    typedef ITTerminal * TermPtr;
    TermPtr * ppTerminals = new TermPtr[dwNumTerminals];

    if ( ppTerminals == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "can't allocate terminals array - exit E_OUTOFMEMORY"));

        (*ppTerminalEnumerator)->Release();

        return E_OUTOFMEMORY;
    }

    //
    // Fill in the array with actual pointers. We must do this before
    // initializing the enumerator, because the enumerator may want to
    // addref the interface pointers during initialize.
    // 

    hr = GetStaticTerminals(&dwNumTerminals,
                            ppTerminals);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateStaticTerminals - "
            "can't get terminals - exit 0x%08x", hr));

        (*ppTerminalEnumerator)->Release();
        delete ppTerminals;

        return hr;
    }

    //
    // Initialize the object with the array of pointers.
    //

    hr = pEnum->Init(ppTerminals,
                     ppTerminals + dwNumTerminals,
                     NULL,
                     AtlFlagTakeOwnership);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMSPCallBase::EnumerateStaticTerminals - "
            "init enumerator failed - exit 0x%08x", hr));

        for (DWORD i = 0; i < dwNumTerminals; i++)
        {
            ppTerminals[i]->Release();
        }
        
        delete ppTerminals;
        (*ppTerminalEnumerator)->Release();

        return hr;
    }

    LOG((MSP_TRACE, "CMSPAddress::EnumerateStaticTerminals - exit S_OK"));

    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CMSPAddress::get_DynamicTerminalClasses (
        OUT  VARIANT * pVariant
        )
{
    LOG((MSP_TRACE, "CMSPAddress::get_DynamicTerminalClasses - enter"));

    //
    // Check parameters.
    //

    if ( !pVariant)
    {
        LOG((MSP_ERROR, "CMSPAddress::get_DynamicTerminalClasses - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

    //
    // create the collection object - see mspcoll.h
    //

    CComObject<CTapiBstrCollection> * pCollection;
    HRESULT hr = CComObject<CTapiBstrCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_DynamicTerminalClasses - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

    //
    // get the Collection's IDispatch interface
    //

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(IID_IDispatch,
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_DynamicTerminalClasses - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

    //
    // Find out how many terminals classes are available.
    //

    DWORD   dwNumClasses;

    hr = GetDynamicTerminalClasses(&dwNumClasses,
                                   NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't get number of terminal classes - exit 0x%08x", hr));

        pDispatch->Release();

        return hr;
    }

    //
    // Allocate an array of GUIDs.
    //

    IID * pClassGuids = new IID[dwNumClasses];

    if ( pClassGuids == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't allocate class guids array - exit E_OUTOFMEMORY"));

        pDispatch->Release();

        return E_OUTOFMEMORY;
    }

    //
    // Fill in the array with actual pointers.
    // 

    hr = GetDynamicTerminalClasses(&dwNumClasses,
                                   pClassGuids);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't get terminal class guids - exit 0x%08x", hr));

        pDispatch->Release();

        delete pClassGuids;

        return hr;
    }

    //
    // Allocate an array of BSTRs.
    //

    BSTR * pClassBstrs = new BSTR[dwNumClasses];

    if ( pClassBstrs == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't allocate class bstrs array - exit E_OUTOFMEMORY"));

        pDispatch->Release();

        delete pClassGuids;

        return E_OUTOFMEMORY;
    }

    //
    // Allocate a string for each GUID and copy it to the array,
    // then delete the array of GUIDs.
    //

    const int BUFSIZE = 100;
    WCHAR wszBuffer[BUFSIZE];

    for (DWORD i = 0; i < dwNumClasses; i++)
    {
        int ret = StringFromGUID2(pClassGuids[i], wszBuffer, BUFSIZE);

        _ASSERTE(ret != 0);

        pClassBstrs[i] = SysAllocString(wszBuffer);

        if ( pClassBstrs[i] == NULL )
        {
            LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
                "can't allocate a bstr - exit E_OUTOFMEMORY"));

            for (DWORD j = 0; j < i; j++)
            {
                SysFreeString(pClassBstrs[j]);
            }

            delete pClassBstrs;
            delete pClassGuids;

            pDispatch->Release();
    
            return E_OUTOFMEMORY;
        }
    }

    delete pClassGuids;

    //
    // Init the collection using an iterator -- pointers to the beginning and
    // the ending element plus one.
    //

    hr = pCollection->Initialize( dwNumClasses,
                                  pClassBstrs,
                                  pClassBstrs + dwNumClasses );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::get_DynamicTerminalClasses - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();

        for (DWORD k = 0; k < dwNumClasses; k++)
        {
            SysFreeString(pClassBstrs[k]);
        }
        
        delete pClassBstrs;

        return hr;
    }

    delete pClassBstrs;

    //
    // put the IDispatch interface pointer into the variant
    //

    LOG((MSP_ERROR, "CMSPAddress::get_DynamicTerminalClasses - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CMSPAddress::get_DynamicTerminalClasses - exit S_OK"));
 
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CMSPAddress::EnumerateDynamicTerminalClasses (
        OUT  IEnumTerminalClass ** ppTerminalClassEnumerator
        )
{
    LOG((MSP_TRACE, "CMSPAddress::EnumerateDynamicTerminalClasses - enter"));

    //
    // Check the return pointer.
    //

    if (!ppTerminalClassEnumerator)
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "bad return pointer - exit E_POINTER"));

        return E_POINTER;
    }

    //
    // Find out how many terminals classes are available.
    //

    HRESULT hr;
    DWORD   dwNumClasses;

    hr = GetDynamicTerminalClasses(&dwNumClasses,
                                   NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't get number of terminal classes - exit 0x%08x", hr));

        return hr;
    }

    //
    // Allocate an array of GUIDs.
    //

    IID * pClassGuids = new IID[dwNumClasses];

    if ( pClassGuids == NULL )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't allocate class guids array - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    //
    // Fill in the array with actual pointers.
    // 

    hr = GetDynamicTerminalClasses(&dwNumClasses,
                                   pClassGuids);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't get terminal class guids - exit 0x%08x", hr));

        delete pClassGuids;

        return hr;
    }


    //
    // Create an enumerator to hold this array, and have it take ownership
    // so that it will delete the array when it is released. The CSafeComEnum
    // can handle zero-length arrays. This Fn also checks the return arg.
    //

    //
    // Create the enumerator object.
    //

    typedef CSafeComEnum<IEnumTerminalClass,
                     &IID_IEnumTerminalClass,
                     GUID, _Copy<GUID> > CEnumerator;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't create enumerator - exit 0x%08x", hr));

        delete pClassGuids;

        return hr;
    }

    //
    // Query for the desired interface.
    //

    hr = pEnum->_InternalQueryInterface(IID_IEnumTerminalClass, 
                                        (void**) ppTerminalClassEnumerator);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't get enumerator interface - exit 0x%08x", hr));

        delete pEnum;
        delete pClassGuids;
        
        return hr;
    }

    //
    // Init the enumerator object.
    //

    hr = pEnum->Init(pClassGuids,
                     pClassGuids + dwNumClasses,
                     NULL,
                     AtlFlagTakeOwnership); 

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMSPAddress::EnumerateDynamicTerminalClasses - "
            "can't init enumerator - exit 0x%08x", hr));

        (*ppTerminalClassEnumerator)->Release();
        delete pClassGuids;
        
        return hr;
    }

    LOG((MSP_TRACE, "CMSPAddress::EnumerateDynamicTerminalClasses - exit S_OK"));

    return S_OK;
}

HRESULT CMSPAddress::ReceiveTSPAddressData(
        IN      PBYTE               pBuffer,
        IN      DWORD               dwSize
        )
/*++

Routine Description:

  Base class receive TSP address data method... does nothing in base class.
  Implemented so that MSP's that only communicate per-call don't have
  to override it.

Arguments:

  
Return Value:

S_OK

--*/

{
    LOG((MSP_TRACE, "CMSPAddress::ReceiveTSPAddressData - enter"));
    LOG((MSP_TRACE, "CMSPAddress::ReceiveTSPAddressData - exit S_OK"));

    return S_OK;
}

HRESULT CMSPAddress::PnpNotifHandler(
        IN      BOOL                bDeviceArrival
        )
{
    LOG((MSP_TRACE, "CMSPAddress::PnpNotifHandler - enter"));

    if (bDeviceArrival)
        LOG((MSP_TRACE, "CMSPAddress::PnpNotifHandler - device arrival"));
    else
        LOG((MSP_TRACE, "CMSPAddress::PnpNotifHandler - device removal"));

    // lock the terminal related data. This is a auto lock that will unlock
    // when the function returns.
    CLock lock(m_TerminalDataLock);

    // if the terminal list hasn't been built yet, we can skip doing anything
    if (m_fTerminalsUpToDate)
    {
        HRESULT hr = UpdateTerminalListForPnp( bDeviceArrival );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR,
                "CMSPAddress::PnpNotifHandler - "
                "UpdateTerminalList failed - returning 0x%08x", hr));

            return hr;
        }
    }

    LOG((MSP_TRACE, "CMSPAddress::PnpNotifHandler - exit S_OK"));

    return S_OK;
}

// eof
