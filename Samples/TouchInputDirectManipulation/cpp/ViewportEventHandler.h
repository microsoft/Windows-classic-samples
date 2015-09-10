#pragma once

#include <directmanipulation.h>
#include "AppWindow.h"

namespace DManipSample
{
    class CViewportEventHandler : public RuntimeClass<RuntimeClassFlags<RuntimeClassType::ClassicCom>,
                                    Implements<RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::ClassicCom>,
                                    Microsoft::WRL::FtmBase, 
                                    IDirectManipulationViewportEventHandler
                                    >
                                  >
    {
    public:
        CViewportEventHandler(HWND hWnd);

        HRESULT GetViewportStatus(WCHAR* data, UINT length);

        /// IDirectManipulationViewportEventHandler
        ///
        IFACEMETHOD(OnViewportStatusChanged)(IDirectManipulationViewport *viewport,
                                             DIRECTMANIPULATION_STATUS current,
                                             DIRECTMANIPULATION_STATUS previous);

        IFACEMETHOD(OnViewportUpdated)(IDirectManipulationViewport *viewport);

        IFACEMETHOD(OnContentUpdated)(IDirectManipulationViewport *viewport,
                                      IDirectManipulationContent *content);

    private:
        HWND _hWnd;
        WCHAR _currentStatus[MAX_PATH];
    };
}