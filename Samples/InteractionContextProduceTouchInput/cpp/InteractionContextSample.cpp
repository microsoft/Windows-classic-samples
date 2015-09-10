//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "InteractionContextSample.h"
#include <InteractionContext.h>

// C RunTime header files
#define _USE_MATH_DEFINES // has to be defined to activate definition of M_PI
#include <math.h>
#include <limits.h>

#define GESTURE_ENGINE_TIMER_ID     42

using namespace std;
using namespace Microsoft::WRL;

// <summary> 
// Initializes the elements that will be used to draw the application UI. 
// Unless otherwise specified, each of these elements is intended to 
// resize, relayout or show different UI in response to DPI changes. 
// </summary>
CInteractionContextSample::CInteractionContextSample()
{
	m_interactionContext = NULL;
	m_timerId = 0;
	m_frameId = 0xffffffff;
}

CInteractionContextSample::~CInteractionContextSample()
{
	if (m_interactionContext)
	{
		DestroyInteractionContext(m_interactionContext);
		m_interactionContext = NULL;
	}
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// <summary>
// This method initializes member variables and objects required by this class.  
// This method will be called once base class initialize functionality completes. 
// </summary>
HRESULT 
CInteractionContextSample::Initialize(
    _In_    RECT bounds,
    _In_    std::wstring title
    )
{
	HRESULT hr = S_OK;

	// Initialize object state
	m_diagonalsOn = false;
	m_initializeDone = false;

    // First run Initialize method for base class. 
    hr = __super::Initialize(bounds, title);

	// Register this class as a DeviceLostHandler.  This enables the app to release and recreate the
	// device specific resources associated with the app.
	m_deviceResources->RegisterDeviceNotify(this);

	if (SUCCEEDED(hr))
	{
		hr = CreateInteractionContext(&m_interactionContext);
	}

	if (SUCCEEDED(hr))
	{
		hr = RegisterOutputCallbackInteractionContext(m_interactionContext, InteractionOutputCallback, this);
	}

	if (SUCCEEDED(hr))
	{
		hr = SetPropertyInteractionContext(m_interactionContext, INTERACTION_CONTEXT_PROPERTY_FILTER_POINTERS, FALSE);
	}

	if (SUCCEEDED(hr))
	{
		// Configure the recognizer to support the set of gestures we will be using for the sample.  Each gesture
		// you want to support must be listed.
		INTERACTION_CONTEXT_CONFIGURATION cfg[] = 
		{
			{INTERACTION_ID_MANIPULATION,
			INTERACTION_CONFIGURATION_FLAG_MANIPULATION |
			INTERACTION_CONFIGURATION_FLAG_MANIPULATION_TRANSLATION_X |
			INTERACTION_CONFIGURATION_FLAG_MANIPULATION_TRANSLATION_Y |
			INTERACTION_CONFIGURATION_FLAG_MANIPULATION_ROTATION |
			INTERACTION_CONFIGURATION_FLAG_MANIPULATION_SCALING |
			INTERACTION_CONFIGURATION_FLAG_MANIPULATION_TRANSLATION_INERTIA |
			INTERACTION_CONFIGURATION_FLAG_MANIPULATION_ROTATION_INERTIA |
			INTERACTION_CONFIGURATION_FLAG_MANIPULATION_SCALING_INERTIA},
			{INTERACTION_ID_TAP,
			INTERACTION_CONFIGURATION_FLAG_TAP},
			{INTERACTION_ID_SECONDARY_TAP,
			INTERACTION_CONFIGURATION_FLAG_SECONDARY_TAP},
		};

		hr = SetInteractionConfigurationInteractionContext(
			m_interactionContext, sizeof(cfg) / sizeof(cfg[0]), cfg);
	}
	
    return hr;
}

void 
CInteractionContextSample::CreateDeviceIndependentResources()
{
}

void 
CInteractionContextSample::ReleaseDeviceIndependentResources()
{
}

void 
CInteractionContextSample::CreateDeviceResources()
{
	auto d2dContext = m_deviceResources->GetD2DDeviceContext();
	d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_blackBrush);
}

void 
CInteractionContextSample::ReleaseDeviceResources()
{
	m_blackBrush.Reset();
}

// <summary>
// Draw client area. 
//      Function overrides virtual member function to 
//      draw member components defined in DPI window class. 
// </summary>
void 
CInteractionContextSample::Draw()
{
    HRESULT hr = S_OK;
    BOOL fResult = FALSE;
    RECT rect = { };
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();
	auto sizeF = d2dContext->GetSize();
	if (!m_initializeDone)
	{
		ResetRect();
		m_initializeDone = true;
	}

	d2dContext->SetTransform(m_transformToUpdate);

	d2dContext->DrawRectangle(
		D2D1::RectF(
		-(sizeF.width / 4),
		-(sizeF.height / 4),
		sizeF.width/4,
		sizeF.height/4),
		m_blackBrush.Get()
		);

	if (m_diagonalsOn)
	{
		d2dContext->DrawLine(
			D2D1::Point2F(-(sizeF.width / 4), -(sizeF.height / 4)),
			D2D1::Point2F(sizeF.width / 4, sizeF.height / 4),
			m_blackBrush.Get()
			);

		d2dContext->DrawLine(
			D2D1::Point2F(sizeF.width / 4, -(sizeF.height / 4)),
			D2D1::Point2F(-(sizeF.width / 4), sizeF.height / 4),
			m_blackBrush.Get()
			);
	}
}

void
CInteractionContextSample::ResetRect()
{
	auto d2dContext = m_deviceResources->GetD2DDeviceContext();
	auto sizeF = d2dContext->GetSize();
	m_transformToUpdate = (
		D2D1::Matrix3x2F::Translation(sizeF.width / 2, sizeF.height / 2)
		);
}

void 
CInteractionContextSample::OnPointerUp(
    _In_    float /* x */, 
	_In_    float /* y */,
	_In_    UINT pointerId
	)
{
	PointerHandling(pointerId);
}

void 
CInteractionContextSample::OnPointerDown(
    _In_    float /* x */, 
	_In_    float /* y */,
	_In_    UINT pointerId
	)
{
	PointerHandling(pointerId);
}

void 
CInteractionContextSample::OnPointerUpdate(
    _In_    float /* x */, 
	_In_    float /* y */,
	_In_    UINT pointerId
    )
{
	PointerHandling(pointerId);
}

void 
CInteractionContextSample::OnPointerWheel(
	_In_    UINT pointerId
	)
{
	PointerHandling(pointerId);
}

void 
CInteractionContextSample::OnPointerHWheel(
	_In_    UINT pointerId
	)
{
	PointerHandling(pointerId);
}

void 
CInteractionContextSample::PointerHandling(UINT pointerId)
{
	// Get frame id from current message.
	POINTER_INFO pointerInfo = {};
	if (GetPointerInfo(pointerId, &pointerInfo))
	{
		if (pointerInfo.frameId != m_frameId)
		{
			// This is the new frame to process.
			m_frameId = pointerInfo.frameId;

			// Find out pointer count and frame history length.
			UINT entriesCount = 0;
			UINT pointerCount = 0;
			if (GetPointerFrameInfoHistory(pointerId, &entriesCount, &pointerCount, NULL))
			{
				// Allocate space for pointer frame history.
				POINTER_INFO *pointerInfoFrameHistory = NULL;
				try
				{
					pointerInfoFrameHistory = new POINTER_INFO[entriesCount * pointerCount];
				}
				catch (...)
				{
					pointerInfoFrameHistory = NULL;
				}

				if (pointerInfoFrameHistory != NULL)
				{
					// Retrieve pointer frame history.
					if (GetPointerFrameInfoHistory(pointerId, &entriesCount, &pointerCount, pointerInfoFrameHistory))
					{
						// As multiple frames may have occurred, we need to process them.
						ProcessPointerFramesInteractionContext(m_interactionContext, entriesCount, pointerCount, pointerInfoFrameHistory);
					}

					delete [] pointerInfoFrameHistory;
					pointerInfoFrameHistory = NULL;
				}
			}
		}
	}
}

void CALLBACK 
CInteractionContextSample::InteractionOutputCallback(
	__in_opt void *clientData,
	__in_ecount(1) const INTERACTION_CONTEXT_OUTPUT *output)
{
	CInteractionContextSample* icSample = reinterpret_cast<CInteractionContextSample*>(clientData);
	
	if (icSample == NULL)
	{
		return;
	}

	icSample->OnInteractionOutputCallback(output);
}

void 
CInteractionContextSample::OnInteractionOutputCallback(__in_ecount(1) const INTERACTION_CONTEXT_OUTPUT *output)
{
	switch (output->interactionId)
	{
	default:
	case INTERACTION_ID_NONE:
		break;

	case INTERACTION_ID_MANIPULATION:
		{
			// Apply delta transform to the object.
			if (!(output->interactionFlags & INTERACTION_FLAG_END))
			{
				POINT pt =
				{
					static_cast<long>(output->x),
					static_cast<long>(output->y)
				};
				ScreenToClient(&pt);
				auto d2dContext = m_deviceResources->GetD2DDeviceContext();
				d2dContext->GetTransform(&m_transformToUpdate);

				// A manipulation is composed of scaling, translation, and rotation.  This applies
				// those aspects to the existing state of the object to make it stick to the finger(s).
				m_transformToUpdate = m_transformToUpdate * 
					D2D1::Matrix3x2F::Rotation(
						output->arguments.manipulation.delta.rotation * 180 / static_cast<float>(M_PI),
						D2D1::Point2F(static_cast<float>(pt.x), static_cast<float>(pt.y))
					) * D2D1::Matrix3x2F::Scale(
						output->arguments.manipulation.delta.scale,
						output->arguments.manipulation.delta.scale,
						D2D1::Point2F(static_cast<float>(pt.x), static_cast<float>(pt.y))
					) * D2D1::Matrix3x2F::Translation(
						output->arguments.manipulation.delta.translationX,
						output->arguments.manipulation.delta.translationY
					);
			}

			// Kick-off inertia timer.
			if (output->interactionFlags & INTERACTION_FLAG_INERTIA)
			{
				if ((output->interactionFlags & INTERACTION_FLAG_END) != 0)
				{
					if (m_timerId != 0)
					{
						KillTimer(m_timerId);
						m_timerId = 0;
					}
				}
				else
				{
					if (m_timerId == 0)
					{
						m_timerId = SetTimer(GESTURE_ENGINE_TIMER_ID, 15, NULL);
					}
				}
			}
		}
		break;

		case INTERACTION_ID_SECONDARY_TAP:
			ResetRect();
			break;

		case INTERACTION_ID_TAP:
			m_diagonalsOn = !m_diagonalsOn;
			break;
	}
}

void 
CInteractionContextSample::OnPointerCaptureChange(_In_ WPARAM /* wparam */)
{
	StopInteractionContext(m_interactionContext);
}

void 
CInteractionContextSample::OnTimerUpdate(_In_ WPARAM wparam)
{
	if ((wparam == GESTURE_ENGINE_TIMER_ID) && (m_timerId != 0))
	{
		ProcessInertiaInteractionContext(m_interactionContext);
	}
}

// <summary>
// This method is called when the m_deviceResources class determines that there has been an error in the
// underlying graphics hardware device.  All device dependent resources should be released on this event.
// </summary>
void
CInteractionContextSample::OnDeviceLost()
{
	ReleaseDeviceResources();
}

// <summary>
// This method is called when the m_deviceResources class is recovering from an error in the
// underlying graphics hardware device.  A new device has been created and all device dependent
// resources should be recreated on this event.
// </summary>
void
CInteractionContextSample::OnDeviceRestored()
{
	CreateDeviceResources();
}

void
CInteractionContextSample::OnDisplayChange()
{
	RECT current;
	ZeroMemory(&current, sizeof(current) );
	GetWindowRect(&current);

	float oldDpix, oldDpiy, newDpi;
	m_deviceResources->GetD2DDeviceContext()->GetDpi(&oldDpix, &oldDpiy);
	newDpi = GetDpiForWindow();

	if (oldDpix != newDpi)
	{
		auto newRect = CalcWindowRectNewDpi(current, oldDpix, newDpi);
		SetNewDpi(newDpi);
		SetWindowPos(0, &newRect, 0);
		m_dpiString.clear();
	}
}

void
CInteractionContextSample::OnDpiChange(
    _In_ int dpi, 
    _In_ LPRECT newRect
    )
{
	float changedDpi = static_cast<float>(dpi);
	SetNewDpi(changedDpi);
	SetWindowPos(0, newRect, 0);
}
