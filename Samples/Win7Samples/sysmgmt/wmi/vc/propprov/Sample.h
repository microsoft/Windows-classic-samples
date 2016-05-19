//***************************************************************************

//

//  SAMPLE.h

//

//  Module: WMI Sample Property Provider

//

//  Purpose: Genral purpose include file.

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
//***************************************************************************

#ifndef _SAMPLE_H_
#define _SAMPLE_H_

#include <wbemcli.h>
#include <wbemprov.h>

typedef LPVOID * PPVOID;

// Provider interfaces are provided by objects of this class
 
class CPropPro : public IWbemPropertyProvider
    {
    protected:
        ULONG           m_cRef;         //Object reference count
   
    public:
        CPropPro();
        ~CPropPro(void);

    //Non-delegating object IUnknown
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

    
    /* IWbemPropertyProvider methods */

       virtual HRESULT STDMETHODCALLTYPE GetProperty( 
           	/* [in] */ long lFlags,
            /* [in] */ const BSTR Locale,						   
            /* [in] */ const BSTR ClassMapping,
            /* [in] */ const BSTR InstMapping,
            /* [in] */ const BSTR PropMapping,
            /* [out] */ VARIANT *pvValue);
        
        virtual HRESULT STDMETHODCALLTYPE PutProperty( 
            /* [in] */ long lFlags,
            /* [in] */ const BSTR Locale,						   
            /* [in] */ const BSTR ClassMapping,
            /* [in] */ const BSTR InstMapping,
            /* [in] */ const BSTR PropMapping,
            /* [in] */ const VARIANT __RPC_FAR *pvValue);

    };

// This class is the class factory for CPropPro objects.

class CProvFactory : public IClassFactory
    {
    protected:
        ULONG           m_cRef;

    public:
        CProvFactory(void);
        ~CProvFactory(void);

        //IUnknown members
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        //IClassFactory members
        STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID
                                 , PPVOID);
        STDMETHODIMP         LockServer(BOOL);
    };

typedef CProvFactory *PCProvFactory;

// These variables keep track of when the module can be unloaded

extern long       g_cObj;
extern long       g_cLock;

// General purpose utilities.  

#endif //_SAMPLE_H_



