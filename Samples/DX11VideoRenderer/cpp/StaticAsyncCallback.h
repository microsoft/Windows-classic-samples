#ifndef __STATICASYNCCALLBACK__
#define __STATICASYNCCALLBACK__

////////////////////////////////////////////////////////
//
#define STATICASYNCCALLBACK(Callback, Parent) \
class Callback##AsyncCallback; \
friend class Callback##AsyncCallback; \
class Callback##AsyncCallback : public IMFAsyncCallback \
{ \
public: \
    STDMETHOD_( ULONG, AddRef )() \
    { \
        Parent * pThis = ((Parent*)((BYTE*)this - offsetof(Parent, m_x##Callback))); \
        return pThis->AddRef(); \
    } \
    STDMETHOD_( ULONG, Release )() \
    { \
        Parent * pThis = ((Parent*)((BYTE*)this - offsetof(Parent, m_x##Callback))); \
        return pThis->Release(); \
    } \
    STDMETHOD( QueryInterface )( REFIID riid, __RPC__deref_out _Result_nullonfailure_ void **ppvObject ) \
    { \
        return E_NOINTERFACE; \
    } \
    STDMETHOD( GetParameters )( \
        __RPC__out DWORD *pdwFlags, \
        __RPC__out DWORD *pdwQueue) \
    { \
        return S_OK; \
    } \
    STDMETHOD( Invoke )( __RPC__in_opt IMFAsyncResult * pResult ) \
    { \
        Callback( pResult ); \
        return S_OK; \
    } \
} m_x##Callback; 

   
////////////////////////////////////////////////////////
//
//
// We need to support QI interface so that DCOM is happy when it tries to marshal the interface pointer
//

#define METHODASYNCCALLBACKEX(Callback, Parent, Flag, Queue) \
class Callback##AsyncCallback; \
friend class Callback##AsyncCallback; \
class Callback##AsyncCallback : public IMFAsyncCallback \
{ \
public: \
    STDMETHOD_( ULONG, AddRef )() \
    { \
        return GetParent()->AddRef(); \
    } \
    STDMETHOD_( ULONG, Release )() \
    { \
        return GetParent()->Release(); \
    } \
    STDMETHOD( QueryInterface )( REFIID riid, __RPC__deref_out _Result_nullonfailure_ void **ppvObject ) \
    { \
        if(riid == IID_IMFAsyncCallback || riid == IID_IUnknown) \
        { \
            (*ppvObject) = this; \
            AddRef(); \
            return S_OK; \
        } \
        (*ppvObject) = NULL; \
        return E_NOINTERFACE; \
    } \
    STDMETHOD( GetParameters )( \
        __RPC__out DWORD *pdwFlags, \
        __RPC__out DWORD *pdwQueue) \
    { \
        *pdwFlags = Flag; \
        *pdwQueue = Queue; \
        return S_OK; \
    } \
    STDMETHOD( Invoke )( __RPC__in_opt IMFAsyncResult * pResult ) \
    { \
        GetParent()->Callback( pResult ); \
        return S_OK; \
    } \
protected: \
    Parent* GetParent() \
    { \
        return ((Parent*)((BYTE*)this - offsetof(Parent, m_x##Callback))); \
    } \
} m_x##Callback; 

////////////////////////////////////////////////////////
//
#define METHODFASTCALLBACK(Callback, Parent) \
    METHODASYNCCALLBACKEX(Callback, Parent, MFASYNC_FAST_IO_PROCESSING_CALLBACK, MFASYNC_CALLBACK_QUEUE_STANDARD)
    
#define METHODASYNCCALLBACK(Callback, Parent) \
    METHODASYNCCALLBACKEX(Callback, Parent, 0, MFASYNC_CALLBACK_QUEUE_STANDARD)

#define METHODASYNCRTCALLBACK(Callback, Parent) \
    METHODASYNCCALLBACKEX(Callback, Parent, 0, MFASYNC_CALLBACK_QUEUE_RT)

#define METHODASYNCIOCALLBACK(Callback, Parent) \
    METHODASYNCCALLBACKEX(Callback, Parent, MFASYNC_FAST_IO_PROCESSING_CALLBACK, MFASYNC_CALLBACK_QUEUE_IO)

////////////////////////////////////////////////////////
//
#define METHODASYNCCALLBACKEX2(Callback, Parent, GetQueue ) \
    METHODASYNCCALLBACKEX(Callback, Parent, \
        0, \
        GetParent()->GetQueue() )

#define METHODASYNCCALLBACKEX3(Callback, Parent, GetFlags, GetQueue ) \
    METHODASYNCCALLBACKEX(Callback, Parent, \
        GetParent()->GetFlags(), GetParent()->GetQueue() )

#endif
