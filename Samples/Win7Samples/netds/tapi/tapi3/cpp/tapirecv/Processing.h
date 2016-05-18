/*
 
Copyright (c) 1999 - 2000  Microsoft Corporation


Module Name:

    Processing.h 


Abstract: 
    
    Declaration of the CTAPIEventNotification object

*/


#ifndef __TAPIEventNotification_H__
#define __TAPIEventNotification_H__


//
// declaration of implementation of the interface that is registered
// with TAPI to receive notification messages
//

class CTAPIEventNotification : public ITTAPIEventNotification
{

private:

    //
    // reference counting
    //

    LONG       m_dwRefCount;

public:


    //
    // ITTAPIEventNotification
    //

    //
    // the method called by TAPI when there is an event
    //

    HRESULT STDMETHODCALLTYPE Event(IN TAPI_EVENT TapiEvent,
                                    IN IDispatch *pEvent);
    

    //
    // IUnknown
    //

    HRESULT STDMETHODCALLTYPE QueryInterface(IN REFIID iid,
                                             OUT void **ppvObject)
    {
        if (iid == IID_ITTAPIEventNotification)
        {
            AddRef();
            *ppvObject = (void *)this;
            return S_OK;
        }

        if (iid == IID_IUnknown)
        {
            AddRef();
            *ppvObject = (void *)this;
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    
    //
    // reference counting needs to be thread safe
    //

    ULONG STDMETHODCALLTYPE AddRef()
    {
        ULONG l = InterlockedIncrement(&m_dwRefCount);

        return l;
    }
    
	ULONG STDMETHODCALLTYPE Release()
    {
        ULONG l = InterlockedDecrement(&m_dwRefCount);

        if ( 0 == l)
        {
            delete this;
        }
        
        return l;
    }



public:


    CTAPIEventNotification() 
    { 

        //
        // freshly created object has an outstanding reference count
        // 

        m_dwRefCount = 1;
    }



    //
    // the object should only be destroyed from Release().
    // destructor is private to enforce this.
    //

private:

    ~CTAPIEventNotification()
    {

        //
        // reference count here must be 0, otherwise we have a bug in the app
        //

        _ASSERTE(m_dwRefCount == 0);
    }

};

#endif //__TAPIEventNotification_H__


