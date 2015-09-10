#include "stdafx.h"

#include "ViewportEventHandler.h"

#define MAX_WAIT_TIME (1000)

namespace DManipSample
{
    CViewportEventHandler::CViewportEventHandler(HWND hWnd)
    {
        _currentStatus[0] = '\0';
        _hWnd = hWnd;
    }

    ///
    /// Implement IDirectManipulationViewportEventHandler.
    ///

    // OnStatusChanged events occur when status changes
    IFACEMETHODIMP CViewportEventHandler::OnViewportStatusChanged(IDirectManipulationViewport* /*viewport*/,
                                    DIRECTMANIPULATION_STATUS current,
                                    DIRECTMANIPULATION_STATUS /*previous*/)
    {
        HRESULT hr = S_OK;

        WCHAR* statusString = nullptr;

        switch(current)
        {
        case DIRECTMANIPULATION_BUILDING:
            statusString = L"DIRECTMANIPULATION_BUILDING";
            break;
        case DIRECTMANIPULATION_ENABLED:
            statusString = L"DIRECTMANIPULATION_ENABLED";
            break;
        case DIRECTMANIPULATION_DISABLED:
            statusString = L"DIRECTMANIPULATION_DISABLED";
            break;
        case DIRECTMANIPULATION_RUNNING:
            statusString = L"DIRECTMANIPULATION_RUNNING";
            break;
        case DIRECTMANIPULATION_INERTIA:
            statusString = L"DIRECTMANIPULATION_INERTIA";
            break;
        case DIRECTMANIPULATION_READY:
            statusString = L"DIRECTMANIPULATION_READY";
            break;
        case DIRECTMANIPULATION_SUSPENDED:
            statusString = L"DIRECTMANIPULATION_SUSPENDED";
            break;
        default:
            statusString = L"Unknown";
        }

        hr = StringCchCopy(_currentStatus, ARRAYSIZE(_currentStatus), statusString);

        PostMessage(_hWnd, UWM_REDRAWSTATUS, 0, 0);

        return hr;
    }

    // OnViewportUpdated events occur after all the content instances in the viewport have been updated for the current input.
    IFACEMETHODIMP CViewportEventHandler::OnViewportUpdated(IDirectManipulationViewport* /*viewport*/)
    {
        return S_OK;
    }

    // OnContentUpdated events occur when a content inside viewport is updated. 
    IFACEMETHODIMP CViewportEventHandler::OnContentUpdated(IDirectManipulationViewport* /*viewport*/,
                                IDirectManipulationContent* /*content*/)
    {
        return S_OK;
    }

    HRESULT CViewportEventHandler::GetViewportStatus(WCHAR* data, UINT length)
    {      
        HRESULT hr = S_OK;

        hr = StringCchCopy(data, length, _currentStatus);

        return hr;
    }
}