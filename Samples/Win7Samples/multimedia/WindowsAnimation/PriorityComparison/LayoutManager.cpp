// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "LayoutManager.h"
#include "UIAnimationSample.h"

const DOUBLE ROW_POSITION = 0.7;
const DOUBLE SELECTED_POSITION = 0.5;

const DOUBLE ACCELERATION_SELECT = 2000.0;
const DOUBLE ACCELERATION_SELECT_AFTER_WAVE = ACCELERATION_SELECT * 0.2;
const DOUBLE ACCELERATION_CENTER = ACCELERATION_SELECT * 0.3;
const UI_ANIMATION_SECONDS PERIOD = 1.0;

const UI_ANIMATION_SECONDS ACCEPTABLE_DELAY_SLIDE = 0.5;
const UI_ANIMATION_SECONDS ACCEPTABLE_DELAY_WAVE = 0.8;

CLayoutManager::CLayoutManager() :
    m_pAnimationManager(NULL),
    m_pAnimationTimer(NULL),
    m_pTransitionLibrary(NULL),
    m_uThumbCount(0),
    m_thumbs(NULL),
    m_iThumbSelected(0),
    m_centerX(0.0),
    m_rowY(0.0)
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
                                                        
// Re-center the thumbnails when the client area is resized

HRESULT CLayoutManager::Resize(
    D2D1_SIZE_F sizeClient
    )
{
    m_centerX = sizeClient.width * 0.5;
    m_rowY = sizeClient.height * ROW_POSITION;

    return Arrange();
}

// Arrange the thumbnails and center the selected one

HRESULT CLayoutManager::Arrange()
{
    // Center the thumbnails horizontally
    
    HRESULT hr = CenterThumbnails();
    
    if (SUCCEEDED(hr))
    {
        // Slide the current thumbnail up
    
        hr = SlideThumbnails(
            STORYBOARD_ID_NONE,
            ACCELERATION_SELECT,
            0.0,
            m_iThumbSelected,
            SLIDE_INDEX_NONE
            );

        if (SUCCEEDED(hr))
        {
            // Center all the other thumbnails vertically
            
            IUIAnimationStoryboard *pStoryboard;
            hr = m_pAnimationManager->CreateStoryboard(
                &pStoryboard
                );
            if (SUCCEEDED(hr))
            {
                for (UINT i = 0; i < m_uThumbCount; i++)
                {
                    if (i != m_iThumbSelected)
                    {
                        IUIAnimationTransition *pTransitionYCenter;
                        hr = m_pTransitionLibrary->CreateParabolicTransitionFromAcceleration(
                            m_rowY,
                            0.0,
                            ACCELERATION_SELECT,
                            &pTransitionYCenter
                            );
                        if (SUCCEEDED(hr))
                        {
                            hr = pStoryboard->AddTransition(
                                m_thumbs[i].m_pAnimationVariableY,
                                pTransitionYCenter
                                );

                            pTransitionYCenter->Release();
                        }

                        if (FAILED(hr))
                        {
                            break;
                        }
                    }
                }

                // Don't tag the Storyboard for priority comparison

                if (SUCCEEDED(hr))
                {
                    hr = ScheduleStoryboard(pStoryboard);
                }

            pStoryboard->Release();
            }
        }
    }

    return hr;
}

// Selects the next thumbnail in the array and slides it up

HRESULT CLayoutManager::Next()
{
    HRESULT hr = S_OK;

    if (m_iThumbSelected < m_uThumbCount - 1)
    {
        hr = SlideThumbnails(
            STORYBOARD_ID_SLIDE,
            ACCELERATION_SELECT,
            ACCEPTABLE_DELAY_SLIDE,
            m_iThumbSelected + 1,
            m_iThumbSelected
            );
        if (SUCCEEDED(hr))
        {
            m_iThumbSelected++;
            
            hr = CenterThumbnails();
        }
    }

    return hr;
}

// Selects the previous thumbnail in the array and slides it up

HRESULT CLayoutManager::Previous()
{
    HRESULT hr = S_OK;
    
    if (m_iThumbSelected > 0)
    {
        hr = SlideThumbnails(
            STORYBOARD_ID_SLIDE,
            ACCELERATION_SELECT,
            ACCEPTABLE_DELAY_SLIDE,
            m_iThumbSelected - 1,
            m_iThumbSelected
            );
        if (SUCCEEDED(hr))
        {
            m_iThumbSelected--;

            hr = CenterThumbnails();
        }
    }

    return hr;
}

// Creates a sinusoidal wave through the array of thumbnails

HRESULT CLayoutManager::Wave(
    UI_ANIMATION_SLOPE slope
    )
{
    const DOUBLE AMPLITUDE = 200.0;
    const UI_ANIMATION_SECONDS SECONDS_BETWEEN_THUMBS = 0.03;
    
    // First make sure the the thumbnails are down

    HRESULT hr = SlideThumbnails(
        STORYBOARD_ID_WAVE,
        ACCELERATION_SELECT,
        ACCEPTABLE_DELAY_WAVE,
        SLIDE_INDEX_NONE,
        m_iThumbSelected
        );
    if (SUCCEEDED(hr))
    {
        // Build a storyboard to animate all thumbnails vertically in a wave pattern
        
        IUIAnimationStoryboard *pStoryboard;
        hr = m_pAnimationManager->CreateStoryboard(
            &pStoryboard
            );
        if (SUCCEEDED(hr))
        {
            for (UINT i = 0; i < m_uThumbCount; i++)
            {
                // Use keyframes to offset each thumbnail's animation a little more from the start than its predecessor
                
                UI_ANIMATION_KEYFRAME keyframe;
                hr = pStoryboard->AddKeyframeAtOffset(
                    UI_ANIMATION_KEYFRAME_STORYBOARD_START,
                    SECONDS_BETWEEN_THUMBS * i,
                    &keyframe
                    );
                if (SUCCEEDED(hr))
                {
                    IUIAnimationTransition *pTransitionYSinusoidal;
                    hr = m_pTransitionLibrary->CreateSinusoidalTransitionFromRange(
                        PERIOD,
                        m_rowY - AMPLITUDE,
                        m_rowY + AMPLITUDE,
                        PERIOD,
                        slope,
                        &pTransitionYSinusoidal
                        );
                    if (SUCCEEDED(hr))
                    {
                        hr = pStoryboard->AddTransitionAtKeyframe(
                            m_thumbs[i].m_pAnimationVariableY,
                            pTransitionYSinusoidal,
                            keyframe
                            );
                        pTransitionYSinusoidal->Release();
                        if (SUCCEEDED(hr))
                        {
                            // Add a zero-duration constant transition to bring the velocity to zero
                        
                            IUIAnimationTransition* pTransitionYConstant;
                            hr = m_pTransitionLibrary->CreateConstantTransition(
                                0.0,
                                &pTransitionYConstant
                                );
                            if (SUCCEEDED(hr))
                            {
                                hr = pStoryboard->AddTransition(
                                    m_thumbs[i].m_pAnimationVariableY,
                                    pTransitionYConstant
                                    );
                                pTransitionYConstant->Release();
                            }
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
                // Tag the storyboard for priority comparison

                hr = pStoryboard->SetTag(
                    NULL,
                    STORYBOARD_ID_WAVE
                    );
                if (SUCCEEDED(hr))
                {
                    hr = pStoryboard->SetLongestAcceptableDelay(
                        UI_ANIMATION_SECONDS_EVENTUALLY
                        );
                    if (SUCCEEDED(hr))
                    {
                        hr = ScheduleStoryboard(pStoryboard);
                        if (SUCCEEDED(hr))
                        {
                            // Slide the selected thumbnail back up

                            hr = SlideThumbnails(
                                STORYBOARD_ID_SLIDE_AFTER_WAVE,
                                ACCELERATION_SELECT_AFTER_WAVE,
                                UI_ANIMATION_SECONDS_EVENTUALLY,
                                m_iThumbSelected,
                                SLIDE_INDEX_NONE
                                );
                        }
                    }
                }
            }

            pStoryboard->Release();
        }
    }

    return hr;
}

// Moves the selected image horizontally to the center of the client area

HRESULT CLayoutManager::CenterThumbnails()
{
    const DOUBLE THUMB_SPACING = 60.0;

    IUIAnimationStoryboard *pStoryboard;
    HRESULT hr = m_pAnimationManager->CreateStoryboard(
        &pStoryboard
        );
    if (SUCCEEDED(hr))
    {
        for (UINT i = 0; i < m_uThumbCount; i++)
        {
            D2D1_SIZE_F size = m_thumbs[i].GetSize();
            IUIAnimationTransition *pTransitionX;
            hr = m_pTransitionLibrary->CreateParabolicTransitionFromAcceleration(
                m_centerX + ((static_cast<INT32>(i) - static_cast<INT32>(m_iThumbSelected)) * THUMB_SPACING),
                0.0,
                ACCELERATION_CENTER,
                &pTransitionX
                );
            if (SUCCEEDED(hr))
            {
                hr = pStoryboard->AddTransition(
                    m_thumbs[i].m_pAnimationVariableX,
                    pTransitionX
                    );

                pTransitionX->Release();
            }

            if (FAILED(hr))
            {
                break;
            }
        }

        // Don't tag the storyboard for priority comparison

        if (SUCCEEDED(hr))
        {
            hr = ScheduleStoryboard(pStoryboard);
        }

        pStoryboard->Release();
    }

    return hr;
}

// Slides one thumbnail up and/or another thumbnail down
// SLIDE_INDEX_NONE can be passed for slideUpIndex or slideDownIndex

HRESULT CLayoutManager::SlideThumbnails(
    STORYBOARD_ID storyboardId,
    DOUBLE acceleration,
    UI_ANIMATION_SECONDS secondsLongestAcceptableDelay,
    UINT slideUpIndex, 
    UINT slideDownIndex
    )
{
    DOUBLE selectedY = m_rowY * SELECTED_POSITION;

    IUIAnimationStoryboard *pStoryboard;
    HRESULT hr = m_pAnimationManager->CreateStoryboard(
        &pStoryboard
        );
    if (SUCCEEDED(hr))
    {
        if (slideUpIndex != SLIDE_INDEX_NONE)
        {
            D2D1_SIZE_F size = m_thumbs[slideUpIndex].GetSize();

            IUIAnimationTransition *pTransitionYUp;
            hr = m_pTransitionLibrary->CreateParabolicTransitionFromAcceleration(
                selectedY,
                0.0,
                acceleration,
                &pTransitionYUp
                );
            if (SUCCEEDED(hr))
            {
                hr = pTransitionYUp->SetInitialVelocity(
                    0.0
                    );
                if (SUCCEEDED(hr))
                {
                    hr = pStoryboard->AddTransition(
                        m_thumbs[slideUpIndex].m_pAnimationVariableY,
                        pTransitionYUp
                        );
                }
                pTransitionYUp->Release();
            }
        }
        
        if (SUCCEEDED(hr))
        {
            if (slideDownIndex != SLIDE_INDEX_NONE)
            {
                IUIAnimationTransition *pTransitionYDown;
                hr = m_pTransitionLibrary->CreateParabolicTransitionFromAcceleration(
                    m_rowY,
                    0.0,
                    acceleration,
                    &pTransitionYDown
                    );
                if (SUCCEEDED(hr))
                {
                    hr = pStoryboard->AddTransition(
                        m_thumbs[slideDownIndex].m_pAnimationVariableY,
                        pTransitionYDown
                        );
                    pTransitionYDown->Release();
                }
            }

            if (SUCCEEDED(hr))
            {
                // Tag the storyboard for priority comparison

                hr = pStoryboard->SetTag(
                    NULL,
                    storyboardId
                    );
                if (SUCCEEDED(hr))
                {
                    hr = pStoryboard->SetLongestAcceptableDelay(
                        secondsLongestAcceptableDelay
                        );
                    if (SUCCEEDED(hr))
                    {
                        hr = ScheduleStoryboard(pStoryboard);
                    }
                }
            }
        }

        pStoryboard->Release();
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
