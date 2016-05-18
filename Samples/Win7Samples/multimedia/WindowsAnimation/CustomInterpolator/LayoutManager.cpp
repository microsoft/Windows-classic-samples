// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "LayoutManager.h"
#include "UIAnimationSample.h"
#include "CustomInterpolator.h"

#include <math.h>

CLayoutManager::CLayoutManager() :
    m_pAnimationManager(NULL),
    m_pAnimationTimer(NULL),
    m_pTransitionLibrary(NULL),
    m_pTransitionFactory(NULL),
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
    SafeRelease(&m_pTransitionFactory);
    
    // Do not delete m_thumbs, as client owns that
}

// Initialization

HRESULT CLayoutManager::Initialize(
    IUIAnimationManager *pAnimationManager,
    IUIAnimationTimer *pAnimationTimer,
    IUIAnimationTransitionLibrary *pTransitionLibrary,
    IUIAnimationTransitionFactory *pTransitionFactory,
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

    m_pTransitionFactory = pTransitionFactory;
    m_pTransitionFactory->AddRef();

    // Store a pointer to thumbnail array
    
    m_uThumbCount = uThumbCount;
    m_thumbs = thumbs;

    return hr;
}
                                                        
// Arranges the thumbnails randomly

HRESULT CLayoutManager::Arrange(
    D2D1_SIZE_F sizeClient
    )
{
    // Create storyboard for all the thumbnail transitions    

    IUIAnimationStoryboard *pStoryboard;
    HRESULT hr = m_pAnimationManager->CreateStoryboard(
        &pStoryboard
        );
    if (SUCCEEDED(hr))
    {
        // Arrange the thumbnails, adding transitions to move each thumbnail to a random new location
    
        for (UINT i = 0; i < m_uThumbCount; i++)
        {
            D2D1_SIZE_F size = m_thumbs[i].GetSize();
            DOUBLE xDest = RandomFromRange(
                size.width * 0.5,
                sizeClient.width - size.width * 0.5
                );
            DOUBLE yDest = RandomFromRange(
                sizeClient.height * 0.25 + size.height * 0.5,
                sizeClient.height * 0.75 - size.height * 0.5
                );

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
        }

        if (SUCCEEDED(hr))
        {
            hr = ScheduleStoryboard(pStoryboard);
        }

        pStoryboard->Release();
    }

    return hr;
}

// Animates all y values of the thumbnails to a final value using the same acceleration

HRESULT CLayoutManager::Attract(
    DOUBLE finalYValue
    )
{
    const DOUBLE ACCELERATION = 2500.0;

    // Create storyboard for all the thumbnail transitions    

    IUIAnimationStoryboard *pStoryboard;
    HRESULT hr = m_pAnimationManager->CreateStoryboard(
        &pStoryboard
        );
    if (SUCCEEDED(hr))
    {
        // Add transitions to move each thumbnail to the final y value

        for (UINT i = 0; i < m_uThumbCount; i++)
        {
            // Compute an offset to align the thumbnail's edge with the final value,
            // rather than its center
        
            D2D1_SIZE_F size = m_thumbs[i].GetSize();
            DOUBLE offset = (finalYValue > 0.0 ? -1.0 : 1.0) * size.height * 0.5;
            
            IUIAnimationTransition *pTransition;
            hr = CAttractInterpolator::CreateTransition(
                m_pTransitionFactory,
                finalYValue + offset,
                ACCELERATION,
                &pTransition
                );
            if (SUCCEEDED(hr))
            {
                hr = pStoryboard->AddTransition(
                    m_thumbs[i].m_pAnimationVariableY,
                    pTransition
                    );
                pTransition->Release();
            }

            if (FAILED(hr))
            {
                break;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = ScheduleStoryboard(pStoryboard);
        }

        pStoryboard->Release();
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

// Gets the current time and schedules a storyboard for play

HRESULT CLayoutManager::ScheduleStoryboard(
    IUIAnimationStoryboard *pStoryboard
    )
{
    UI_ANIMATION_SECONDS secondsNow;
    HRESULT hr = m_pAnimationTimer->GetTime(
        &secondsNow
        );
    if (SUCCEEDED(hr))
    {
        hr = pStoryboard->Schedule(
            secondsNow
            );
    }
    
    return hr;
}

// Returns a random value between the given minimum and maximum

DOUBLE CLayoutManager::RandomFromRange(
    DOUBLE minimum,
    DOUBLE maximum
    )
{
     return minimum + (maximum - minimum) * rand() / RAND_MAX;
}
