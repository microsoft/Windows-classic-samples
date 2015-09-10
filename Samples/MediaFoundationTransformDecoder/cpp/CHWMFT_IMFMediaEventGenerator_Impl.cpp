#include "CHWMFT.h"
#include <Mfapi.h>

// Helper Macros
#define SAFERELEASE(x) \
    if((x) != NULL) \
    { \
        (x)->Release(); \
        (x) = NULL; \
    } \

HRESULT CHWMFT::BeginGetEvent(
    IMFAsyncCallback*   pCallback,
    IUnknown*           punkState)
{
    /***************************************
    ** Since this in an internal function
    ** we know m_pEventQueue can never be
    ** NULL due to InitializeTransform()
    ***************************************/

    return m_pEventQueue->BeginGetEvent(pCallback, punkState);
}

HRESULT CHWMFT::EndGetEvent(
    IMFAsyncResult* pResult,
    IMFMediaEvent** ppEvent)
{
    /***************************************
    ** Since this in an internal function
    ** we know m_pEventQueue can never be
    ** NULL due to InitializeTransform()
    ***************************************/

    return m_pEventQueue->EndGetEvent(pResult, ppEvent);
}

HRESULT CHWMFT::GetEvent(
    DWORD           dwFlags,
    IMFMediaEvent** ppEvent)
{
    /***************************************
    ** Since this in an internal function
    ** we know m_pEventQueue can never be
    ** NULL due to InitializeTransform()
    ***************************************/

    return m_pEventQueue->GetEvent(dwFlags, ppEvent);
}

HRESULT CHWMFT::QueueEvent(
    MediaEventType      met,
    REFGUID             guidExtendedType,
    HRESULT             hrStatus,
    const PROPVARIANT*  pvValue)
{
    /***************************************
    ** Since this in an internal function
    ** we know m_pEventQueue can never be
    ** NULL due to InitializeTransform()
    ***************************************/

    return m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
}