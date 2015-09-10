//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "pch.h"
#include "DeviceResources.h"

// <summary>
// This class can be used on its own or extended to provide custom functionality.  
// This class implements a map to respond to messages relevant to DPI events. 
// </summary>
class CSampleDesktopWindow : public CWindowImpl<CSampleDesktopWindow, CWindow, CWinTraits < WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU | WS_SIZEBOX > >
{
public:
    BEGIN_MSG_MAP(c)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
        MESSAGE_HANDLER(WM_DISPLAYCHANGE, OnDisplayChange)
        MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
        MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_POWERBROADCAST, OnPowerBroadcast)
		MESSAGE_HANDLER(WM_USER, OnOcclusion)
		MESSAGE_HANDLER(WM_POINTERDOWN, OnPointerDown)
		MESSAGE_HANDLER(WM_POINTERUP, OnPointerUp)
		MESSAGE_HANDLER(WM_POINTERUPDATE, OnPointerUpdate)
		MESSAGE_HANDLER(WM_POINTERWHEEL, OnPointerWheel)
		MESSAGE_HANDLER(WM_POINTERHWHEEL, OnPointerHWheel)
		MESSAGE_HANDLER(WM_POINTERCAPTURECHANGED, OnPointerCaptureChange)
		MESSAGE_HANDLER(WM_ENTERSIZEMOVE, OnEnterSizeMove)
        MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnExitSizeMove)
		MESSAGE_HANDLER(WM_DPICHANGED, OnDpiChange)
		MESSAGE_HANDLER(WM_TIMER, OnTimerUpdate)
	END_MSG_MAP()

    DECLARE_WND_CLASS_EX(nullptr, 0, -1);

    CSampleDesktopWindow();
    ~CSampleDesktopWindow();

// <summary>
// This method provides the main message pump for the application. 
// </summary>
    HRESULT Run();

protected:
// <summary>
// This method is used to create and initialize this class 
// for use by the application. 
// </summary>
    HRESULT Initialize(_In_ RECT bounds, _In_ std::wstring title);

	// Store DPI information for use by this class.
	void SetNewDpi(_In_ float newPerMonitorDpi);
	static RECT CalcWindowRectNewDpi(_In_ RECT oldRect, _In_ float oldDpi, _In_ float newDPI);
	float GetDpiForWindow();

    // Cached pointer to device resources.
    std::shared_ptr<DeviceResources> m_deviceResources;

    // Store application and display state to optimize rendering and present instructions.
    bool m_visible;
    DWORD m_occlusion;

private:
    // Draw client area of base Desktop Window.
    HRESULT Render();

// <summary>
// These methods will be called as messages are processed by message map
// defined in this class.
// </summary>
    LRESULT OnCreate(_In_ UINT, _In_ WPARAM, _In_ LPARAM lParam, _Out_ BOOL &bHandled);
    LRESULT OnDestroy(_In_ UINT, _In_ WPARAM, _In_ LPARAM, _Out_ BOOL &bHandled);

    LRESULT OnWindowPosChanged(_In_ UINT, _In_ WPARAM, _In_ LPARAM lparam, _Out_ BOOL &bHandled);

    LRESULT OnDisplayChange(_In_ UINT, _In_ WPARAM, _In_ LPARAM, _Out_ BOOL &bHandled);
    LRESULT OnGetMinMaxInfo(_In_ UINT, _In_ WPARAM, _In_ LPARAM lparam, _Out_ BOOL &bHandled);

    LRESULT OnActivate(_In_ UINT, _In_ WPARAM wparam, _In_ LPARAM, _Out_ BOOL &bHandled);
    LRESULT OnPowerBroadcast(_In_ UINT, _In_ WPARAM, _In_ LPARAM lparam, _Out_ BOOL &bHandled);
    LRESULT OnOcclusion(_In_ UINT, _In_ WPARAM, _In_ LPARAM, _Out_ BOOL &bHandled);

    LRESULT OnPointerDown(_In_ UINT, _In_ WPARAM wparam, _In_ LPARAM lparam, _Out_ BOOL &bHandled);
	LRESULT OnPointerUp(_In_ UINT, _In_ WPARAM wparam, _In_ LPARAM lparam, _Out_ BOOL &bHandled);
	LRESULT OnPointerUpdate(_In_ UINT, _In_ WPARAM wparam, _In_ LPARAM lparam, _Out_ BOOL &bHandled);
	LRESULT OnPointerWheel(_In_ UINT, _In_ WPARAM wparam, _In_ LPARAM lparam, _Out_ BOOL &bHandled);
	LRESULT OnPointerHWheel(_In_ UINT, _In_ WPARAM wparam, _In_ LPARAM lparam, _Out_ BOOL &bHandled);
	LRESULT OnPointerCaptureChange(_In_ UINT, _In_ WPARAM wparam, _In_ LPARAM lparam, _Out_ BOOL &bHandled);
	
    LRESULT OnEnterSizeMove(_In_ UINT, _In_ WPARAM, _In_ LPARAM, _Out_ BOOL &bHandled);
    LRESULT OnExitSizeMove(_In_ UINT, _In_ WPARAM, _In_ LPARAM, _Out_ BOOL &bHandled);

    LRESULT OnDpiChange(_In_ UINT, _In_ WPARAM, _In_ LPARAM lparam, _Out_ BOOL &bHandled);
	
	LRESULT OnTimerUpdate(_In_ UINT, _In_ WPARAM wparam, _In_ LPARAM lparam, _Out_ BOOL &bHandled);

// <summary>
// Allocate and release device resources required for Dx operations.
// These are implemented by any classes deriving from SampleDesktopWindow.
// </summary>
	virtual void CreateDeviceIndependentResources() {};
	virtual void ReleaseDeviceIndependentResources() {};

	virtual void CreateDeviceResources() {};
	virtual void ReleaseDeviceResources() {};

// <summary>
// Method to be implemented by any classes inheriting from SampleDesktopWindow.
//    SampleDesktopWindow will call this once default rendering functionality
//    is complete.
// </summary>
	virtual void Draw() {};

// <summary>
// Extend default message handlers provided by DesktopWindow class.
// Note: These do not overwrite handlers provided by this class, 
// but are called by default message handlers provided. 
// </summary>
	virtual void OnPointerUp(_In_ float /* x */, _In_ float /* y */, _In_ UINT /* pointerId */) {};
	virtual void OnPointerDown(_In_ float /* x */, _In_ float /* y */, _In_ UINT /* pointerId */) {};
	virtual void OnPointerUpdate(_In_ float /* x */, _In_ float /* y */, _In_ UINT /* pointerId */) {};
	virtual void OnPointerWheel(_In_ UINT /* pointerId */) {};
	virtual void OnPointerHWheel(_In_ UINT /* pointerId */) {};
	virtual void OnPointerCaptureChange(_In_ UINT /* pointerId */) {};
	virtual void OnEnterSizeMove() {};
	virtual void OnExitSizeMove() {};
	virtual void OnDpiChange(_In_ int /* dpi */, _In_ LPRECT /* rect */) {};
	virtual void OnDisplayChange() {};
	virtual void OnTimerUpdate(_In_ WPARAM /* wparam */) {};
};