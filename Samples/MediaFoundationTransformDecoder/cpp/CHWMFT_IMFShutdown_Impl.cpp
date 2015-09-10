#include "CHWMFT.h"
#include "CAutoLock.h"
#include <mferror.h>
#include <mfapi.h>

HRESULT CHWMFT::GetShutdownStatus(
    MFSHUTDOWN_STATUS*  pStatus)
{
    HRESULT hr = S_OK;

    do
    {
        if(pStatus == NULL)
        {
            hr = E_POINTER;
            break;
        }

        {
            CAutoLock lock(&m_csLock);

            if(m_bShutdown == FALSE)
            {
                hr = MF_E_INVALIDREQUEST;
                break;
            }

            *pStatus = MFSHUTDOWN_COMPLETED;
        }
    }while(false);

    return hr;
}

HRESULT CHWMFT::Shutdown(void)
{
    HRESULT hr = S_OK;

    do
    {
        CAutoLock lock(&m_csLock);

        hr = ShutdownEventQueue();
        if(FAILED(hr))
        {
            break;
        }

        hr = MFUnlockWorkQueue(m_dwDecodeWorkQueueID);
        if(FAILED(hr))
        {
            break;
        }

        m_bShutdown = TRUE;
    }while(false);

    return hr;
}
