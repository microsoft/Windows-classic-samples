// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "shvui.h"
#include "napcommon.h"
#include <unknwn.h>
#include "SdkCommon.h"

#ifndef _SHVUICF_
#define _SHVUICF_

extern long g_cObjRefCount;

//Data to be used by the class factory
class SHVUIClassFactoryData
{
public:
	const GUID m_pCLSID;
	const char* m_registryName ;
	const char* m_progID ;
	const char* m_verIndProgID ;
 	
	//
	// Out of process server support
	//

	// Pointer to running class factory for this component
	IClassFactory* m_pIClassFactory ;

	// Cookie to identify running object
	DWORD m_register ;

	SHVUIClassFactoryData() :  
        m_pCLSID(CLSID_SDK_SHV_UI),
        m_registryName("Reg Name") ,
        m_progID("Prog ID"),
        m_verIndProgID("Prog ID ver ind"),
        m_pIClassFactory(NULL),
        m_register(0)
        {}

private:
	const SHVUIClassFactoryData& operator=(const SHVUIClassFactoryData&);
} ;


// The class factory
class ShvUIClassFactory: public IClassFactory
{
public:    

    ShvUIClassFactory();    
    ~ShvUIClassFactory();
		
    // IUnknown
    STDMETHODIMP_(ULONG) AddRef(void);   
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(
        _In_ const IID& riid, 
        _Out_ void** ppvObject);
        
    // IClassFactory
    STDMETHODIMP 
    CreateInstance(
        _In_opt_ IUnknown *pUnkOuter, 
        _In_ REFIID riid, 
        _Outptr_ void **ppvObject);

    STDMETHODIMP 
    LockServer(
        __RPC__in BOOL fLock);
    // end IClassFactory

    static BOOL StartFactory();

    static void StopFactory();

    static LONG s_cServerLocks ;   
    static HMODULE s_hModule ;

private:
    LONG m_cRef;
};

#endif


