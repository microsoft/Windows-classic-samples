// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "LayoutManager.h"
#include "UIAnimationSample.h"

#include <math.h>

const FLOAT PADDING = 14.0;

CLayoutManager::CLayoutManager() :
    m_pAnimationManager(NULL),
    m_pAnimationTimer(NULL),
    m_pTransitionLibrary(NULL),
    m_uThumbCount(0),
    m_thumbs(NULL)
{
}

CLayoutManager::~CLayoutManager()
{
    // Animation

    SafeRelease(&m_pAnimationManager);
    SafeRelease(&m_pAnimationTimer);
    SafeRelease(&m_pTransitionLibrary);
    
    // Do not delete m_thumbs, as client owns that
}

// Initialization

HRESULT CLayoutManager::Initialize(
    IUIAnimationManager *pAnimationManager,
    IUIAnimationTimer *pAnimationTimer,
    IUIAnimationTransitionLibrary *pTransitionLibrary,
    UINT uThumbCount,
    CThumbnail *thumbs
    )
{
    HRESULT hr = S_OK;

    m_pAnimationManager = pAnimationManager;
    m_pAnimationManager->AddRef();

    m_pAnimationTimer = pAnimationTimer;
    m_pAnimationTimer->AddRef();

    m_pTransitionLibrary = pTransitionLibrary;
    m_pTransitionLibrary->AddRef();

    // Store a pointer to thumbnail array
    
    m_uThumbCount = uThumbCount;
    m_thumbs = thumbs;

    return hr;
}
                                                        
// Arranges thumbnails into centered rows

HRESULT CLayoutManager::Arrange(
    D2D1_SIZE_F sizeClient
    )
{
    DOUBLE widthRow = 0.0;
    DOUBLE yRow = PADDING;
    DOUBLE heightMax = 0.0;
    int iRowFirst = 0;

    // Create storyboard for all the thumbnail transitions    

    IUIAnimationStoryboard *pStoryboard;
    HRESULT hr = m_pAnimationManager->CreateStoryboard(
        &pStoryboard
        );
    if (SUCCEEDED(hr))
    {
        // Arrange the thumbnails into rows, adding transitions to move
        // each thumbnail to its new location once it is known
    
        for (UINT i = 0; i < m_uThumbCount; i++)
        {    
            D2D1_SIZE_F size = m_thumbs[i].GetSize();
            
            if ((PADDING + widthRow + size.width + PADDING > sizeClient.width) && (i > 0))
            {
                // This thumbnail won't fit on this row - arrange the current row and start a new one

                hr = ArrangeRow(
                    pStoryboard,
                    iRowFirst,
                    i, 
                    0.5 * (sizeClient.width - (widthRow - PADDING)),
                    yRow,
                    heightMax
                    );
                if (FAILED(hr))
                {
                    break;
                }
                
                iRowFirst = i;
                widthRow = 0.0;
                yRow += heightMax + PADDING;
                heightMax = 0.0;
            }
            
            if (heightMax < size.height)
            {
                heightMax = size.height;
            }

            widthRow += PADDING + size.width;
        }
        
        if (SUCCEEDED(hr))
        {
            // Arrange the last row

            hr = ArrangeRow(
                pStoryboard,
                iRowFirst,
                m_uThumbCount,
                0.5 * (sizeClient.width - (widthRow - PADDING)),
                yRow,
                heightMax
                );
            if (SUCCEEDED(hr))
            {
                // Get the current time and schedule the storyboard for play

                UI_ANIMATION_SECONDS timeNow;
                hr = m_pAnimationTimer->GetTime(
                    &timeNow
                    );
                if (SUCCEEDED(hr))
                {
                     hr = pStoryboard->Schedule(
                        timeNow
                        );
                }
            }
        }

        pStoryboard->Release();
    }

    return hr;
}

// Creates a center-justified row of images that will fit in the 
// in application's client area

HRESULT CLayoutManager::ArrangeRow(
    IUIAnimationStoryboard *pStoryboard,
    int iThumbMin,
    int iThumbMax,
    DOUBLE xRow,
    DOUBLE yRow,
    DOUBLE heightMax
    )
{
    HRESULT hr = S_OK;

    for (int i = iThumbMin; i < iThumbMax; i++)
    {
        D2D1_SIZE_F size = m_thumbs[i].GetSize();
        DOUBLE xDest = xRow + 0.5 * size.width;
        DOUBLE yDest = yRow + 0.5 * heightMax;

        // Get the current position
        // Note that this technique is valid only when the storyboard will begin playing immediately

        DOUBLE xCur;
        hr = m_thumbs[i].m_pAnimationVariableX->GetValue(&xCur);
        if (SUCCEEDED(hr))
        {
            DOUBLE yCur;
            hr = m_thumbs[i].m_pAnimationVariableY->GetValue(&yCur);
            if (SUCCEEDED(hr))
            {
                // Add transitions for x and y movement

                if (fabs(xDest - xCur) > fabs(yDest - yCur))
		        {
		            // If the thumbnail has further to travel horizontally than vertically, use a parabolic transition
		            // on X that will determine the duration of the storyboard, and stretch an accelerate-decelerate
		            // transition on Y to have the same duration.
        		    
        		    hr = AddThumbnailTransitions(
        		        pStoryboard,
        		        m_thumbs[i].m_pAnimationVariableX,
        		        xDest,
        		        m_thumbs[i].m_pAnimationVariableY,
        		        yDest
        		        );
        		}
        		else
        		{
		            // If the thumbnail has further to travel vertically than horizontally, use a parabolic transition
		            // on Y that will determine the duration of the storyboard, and stretch an accelerate-decelerate
		            // transition on X to have the same duration.
        		    
        		    hr = AddThumbnailTransitions(
        		        pStoryboard,
        		        m_thumbs[i].m_pAnimationVariableY,
        		        yDest,
        		        m_thumbs[i].m_pAnimationVariableX,
        		        xDest
        		        );
        		}
            }
        }
        		
		if (FAILED(hr))
		{
		    break;
		}

        xRow += size.width + PADDING;
    }

    return hr;
}

// Adds two transitions to a storyboard: one primary parabolic transition, which will determine
// the storyboard duration, and a secondary accelerate-decelerate transition, which will be
// stretched to that duration.

HRESULT CLayoutManager::AddThumbnailTransitions(
    IUIAnimationStoryboard *pStoryboard,
    IUIAnimationVariable *pVariablePrimary,
    DOUBLE valuePrimary,
    IUIAnimationVariable *pVariableSecondary,
    DOUBLE valueSecondary
    )
{
    const DOUBLE ACCELERATION = 2000;
    const DOUBLE ACCELERATION_RATIO = 0.3;
    const DOUBLE DECELERATION_RATIO = 0.3;

    IUIAnimationTransition *pTransitionPrimary;
    HRESULT hr = m_pTransitionLibrary->CreateParabolicTransitionFromAcceleration(
        valuePrimary,
        0.0,
        ACCELERATION,
        &pTransitionPrimary
        );
    if (SUCCEEDED(hr))
    {
        hr = pStoryboard->AddTransition(
	        pVariablePrimary,
	        pTransitionPrimary
	        );
	    if (SUCCEEDED(hr))
	    {
	        UI_ANIMATION_KEYFRAME keyframeEnd;
	        hr = pStoryboard->AddKeyframeAfterTransition(
	            pTransitionPrimary,
	            &keyframeEnd
	            );
	        if (SUCCEEDED(hr))
	        {
                IUIAnimationTransition *pTransitionSecondary;
	            hr = m_pTransitionLibrary->CreateAccelerateDecelerateTransition(
	                1.0,    // Will be overwritten, so unimportant
	                valueSecondary,
			        ACCELERATION_RATIO,
			        DECELERATION_RATIO,    
	                &pTransitionSecondary
	                );
                if (SUCCEEDED(hr))
                {
                    hr = pStoryboard->AddTransitionBetweenKeyframes(
                        pVariableSecondary,
                        pTransitionSecondary,
                        UI_ANIMATION_KEYFRAME_STORYBOARD_START,
                        keyframeEnd
                        );
                    pTransitionSecondary->Release();
                }	            
	        }
	    }
	    
	    pTransitionPrimary->Release();
	}

    return hr;
}
