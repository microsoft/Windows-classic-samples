//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "SampleDesktopWindow.h"
#include <ShellScalingApi.h>
#include <InteractionContext.h>

class CInteractionContextSample : public CSampleDesktopWindow, public IDeviceNotify
{
public:
    CInteractionContextSample();
    ~CInteractionContextSample();

	// IDeviceNotify Interface functions.
	virtual void OnDeviceLost();
	virtual void OnDeviceRestored();

    // Initializes member variables and objects required by CInteractionContextSample class. 
    HRESULT Initialize(_In_ RECT bounds, _In_ std::wstring title);
	void UpdateTransforms(_In_ float newRotation, _In_ float newTranslationX,
		_In_ float newTranslastionY, _In_ float newScale);

protected:
    // Create and release resources required for Dx functionality.
    virtual void CreateDeviceIndependentResources();
    virtual void ReleaseDeviceIndependentResources();
    virtual void CreateDeviceResources();
    virtual void ReleaseDeviceResources();

    // Extends drawing functionality provided by base Desktop Window class. 
    virtual void Draw();

    // Extend message handlers provided by base Desktop Window class.
	virtual void OnPointerUp(float _In_ /* x */, float _In_ /* y */, _In_ UINT pointerId);
	virtual void OnPointerDown(float _In_ /* x */, float _In_ /* y */, _In_ UINT pointerId);
	virtual void OnPointerUpdate(float _In_ /* x */, float _In_ /* y */, _In_ UINT pointerId);
	virtual void OnPointerWheel(_In_ UINT pointerId);
	virtual void OnPointerHWheel(_In_ UINT pointerId);
	virtual void OnPointerCaptureChange(_In_ UINT pointerId);
    virtual void OnDisplayChange();
    virtual void OnDpiChange(int _In_ Dpi, _In_ LPRECT newRect);
	virtual void OnTimerUpdate(_In_ WPARAM wparam);

private:
	// Handler for callbacks after a gesture is recognized by the interaction context.
	void OnInteractionOutputCallback(__in_ecount(1) const INTERACTION_CONTEXT_OUTPUT *output);
	static void CALLBACK InteractionOutputCallback(
		__in_opt void *clientData,
		__in_ecount(1) const INTERACTION_CONTEXT_OUTPUT *output);

	// Unified processing for the vairous sources of pointer message.
	void PointerHandling(UINT pointerId);

	// Helper to reset the rectangle to the starting position.
	void ResetRect();

	FLOAT GetNewDpi(RECT rect);
	std::wstring m_dpiString;
	UINT m_frameId;

	// Brush to draw the box and lines that will be manipulated.
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_blackBrush;
	// D2D transform matrix used to represent the result of previous and ongoing manipulations.
	D2D1::Matrix3x2F m_transformToUpdate;
	// Whether or not diagonal lines are being drawn.
	bool m_diagonalsOn;
	// Whether initialization of variables has completed.
	bool m_initializeDone;
	// Interaction context object that will be used to recognize gestures.
	HINTERACTIONCONTEXT m_interactionContext;
	// Timer for triggering inertia.
	UINT_PTR m_timerId;
};