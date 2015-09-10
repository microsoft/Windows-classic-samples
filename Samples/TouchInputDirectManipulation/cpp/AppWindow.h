// Copyright (c) Microsoft Corporation. All rights reserved 

#pragma once

#include "ViewportEventHandler.h"
#include <wincodec.h>
#include <list>

namespace DManipSample
{
    class CAppWindow
    {
    public:
        CAppWindow();
        int ShowAndServiceWindow();

    private:
        HRESULT _Initialize();
        HRESULT _InitializeDevices();
        HRESULT _InitializeManagerAndViewport();
        HRESULT _InitializeDrawing();
        HRESULT _SizeDependentChanges();

        HRESULT _DrawOuterPrimaryContent();
        HRESULT _DrawInnerPrimaryContent();
        HRESULT _DrawViewportStatusText();
        HRESULT _DrawButtonState(ComPtr<IDCompositionVisual2> buttonVisual, BOOL isUp, WCHAR* buttonText, size_t buttonTextLength);
        HRESULT _ChangeButtonState(ComPtr<IDCompositionVisual2> buttonVisual);
        BOOL _OuterViewportHasContact(UINT contactId);

        HRESULT _HitTestRect(UINT contactId, HWND hWnd, BOOL applyTransform, float left, float top, float right, float bottom, BOOL* rectHit);

        LRESULT _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

        HWND _hWnd;

        // The IDirectManipulationManager2 instance is one of the two objects that is explicitly CoCreated. It handles
        // creating other DirectManipulation COM objects and activating/deactivating DirectManipulation.
        ComPtr<IDirectManipulationManager2> _manager;

        // The IDirectManipulationCompositor instance is the other object that is explicitly CoCreated. It handles 
        // transforming associated DirectComposition visuals with DirectManipulation updates.
        ComPtr<IDirectManipulationCompositor> _compositor;

        // The pointers to the outer viewport (checked pattern) and the inner (nested) viewport (concentric circles)
        ComPtr<IDirectManipulationViewport2> _viewportOuter;
        ComPtr<IDirectManipulationViewport2> _viewportInner;

        // The IDirectManipulationContent instances for each viewport facilitate setting and retrieving properties of the content in the
        // viewport such as the content boundaries and current transformations.
        ComPtr<IDirectManipulationContent> _contentOuter;
        ComPtr<IDirectManipulationContent> _contentInner;

        // These callbacks are associated with our viewports. The IDirectManipulationViewportEventHandler interface is
        // user defined. In this case, it is implemented in the CViewportEventHandler class.
        ComPtr<CViewportEventHandler> _handlerOuter;
        ComPtr<CViewportEventHandler> _handlerInner;

        // These cookies are returned when an instance of IDirectManipulationViewportEventHandler is added to a
        // viewport via IDirectManipulationViewport2::AddEventHandler. It can be used to later disassociate the
        // handler from the viewport.
        DWORD _viewportOuterHandlerCookie;
        DWORD _viewportInnerHandlerCookie;

        RECT _viewportOuterRect;
        RECT _contentOuterRect;
        RECT _viewportInnerRect;
        RECT _contentInnerRect;

        ComPtr<ID2D1Factory> _d2dFactory;
        ComPtr<IDCompositionDesktopDevice> _dcompDevice;
        ComPtr<IDCompositionTarget> _dcompTarget;
        
        ComPtr<IDCompositionVisual2> _dcompVisualOuterParent;
        ComPtr<IDCompositionVisual2> _dcompVisualOuterChild;
        ComPtr<IDCompositionVisual2> _dcompVisualInnerParent;
        ComPtr<IDCompositionVisual2> _dcompVisualInnerChild;
        ComPtr<IDCompositionVisual2> _dcompVisualText;
        ComPtr<IDCompositionVisual2> _dcompVisualButtonUpDefault;
        ComPtr<IDCompositionVisual2> _dcompVisualButtonUpCapturedByViewport;
        ComPtr<IDCompositionVisual2> _dcompVisualButtonUpClicked;
        ComPtr<IDCompositionVisual2> _dcompVisualButtonDown;
        ComPtr<IDCompositionVisual2> _dcompVisualButtonCurrent;
        ComPtr<IDCompositionSurface> _textSurface;

        ComPtr<IDWriteTextFormat> _statusTextFormat;
        ComPtr<IDWriteTextFormat> _viewportTextFormat;
        ComPtr<IDWriteFactory> _writeFactory;

        ComPtr<ID3D11Device> _d3dDevice;  
        ComPtr<IDXGIDevice> _dxgiDevice;

        ComPtr<IWICFormatConverter> _bitmapConverter;

        // Tracks the contacts being handled by the viewport so we don't end up calling SetContact on the viewports
        // unnecessarily.
        std::list<UINT> _activeContacts;
    };
}