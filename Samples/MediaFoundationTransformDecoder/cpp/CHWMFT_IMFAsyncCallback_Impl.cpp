#include "CHWMFT.h"

// Helper Macros
#define SAFERELEASE(x) \
    if((x) != NULL) \
    { \
        (x)->Release(); \
        (x) = NULL; \
    } \

HRESULT CHWMFT::GetParameters(
    DWORD*  pdwFlags,
    DWORD*  pdwQueue)
{
    HRESULT hr = S_OK;

    do
    {
        if((pdwFlags == NULL) || (pdwQueue == NULL))
        {
            hr = E_POINTER;
            break;
        }

        (*pdwFlags) = 0;
        (*pdwQueue) = m_dwDecodeWorkQueueID;
    }while(false);

    return hr;
}

HRESULT CHWMFT::Invoke(
    IMFAsyncResult *pAsyncResult)
{
    /*********************************
    ** Todo: This function is called
    ** when you schedule an async event
    ** Determine the event type from
    ** the result and take appropriate
    ** action
    *********************************/
    
    HRESULT hr = S_OK;

    do
    {
        if(pAsyncResult == NULL)
        {
            hr = E_POINTER;
            break;
        }
    }while(false);

    return hr;
}