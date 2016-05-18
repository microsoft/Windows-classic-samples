// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "UIAnimationHelper.h"

enum STORYBOARD_ID
{
    STORYBOARD_ID_NONE,
    STORYBOARD_ID_SLIDE,
    STORYBOARD_ID_SLIDE_AFTER_WAVE,
    STORYBOARD_ID_WAVE,
};

template <class CPriorityComparison>
class CPriorityComparisonBase :
    public CUIAnimationPriorityComparisonBase<CPriorityComparison>
{
public:

    // IUIAnimationPriorityComparison

    IFACEMETHODIMP
    HasPriority
    (
        IUIAnimationStoryboard *pStoryboardScheduled,              // Currently scheduled storyboard
        IUIAnimationStoryboard *pStoryboardNew,                    // New storyboard that conflicts with scheduled storyboard
        UI_ANIMATION_PRIORITY_EFFECT priorityEffect                // Potential effect on attempt to schedule storyboard if HasPriority returns S_FALSE
    )
    {
        UINT32 storyboardIdScheduled;
        HRESULT hr = UIAnimation_GetStoryboardTag(
            pStoryboardScheduled,
            STORYBOARD_ID_NONE,
            NULL,
            &storyboardIdScheduled
            );

        if (SUCCEEDED(hr))
        {        
            UINT32 storyboardIdNew;
            hr = UIAnimation_GetStoryboardTag(
                pStoryboardNew,
                STORYBOARD_ID_NONE,
                NULL,
                &storyboardIdNew
                );
            
            if (SUCCEEDED(hr))
            {
                bool fHasPriority = HasPriority(
                    storyboardIdScheduled,
                    storyboardIdNew,
                    priorityEffect
                    );
                    
                hr = (fHasPriority ? S_OK : S_FALSE);
            }
        }

        return hr;
    }
    
protected:

    virtual bool HasPriority(
        UINT32 storyboardIdScheduled,
        UINT32 storyboardIdNew,
        UI_ANIMATION_PRIORITY_EFFECT priorityEffect
        ) = 0;
};

class CCancelPriorityComparison :
    public CPriorityComparisonBase<CCancelPriorityComparison>
{
public:

    virtual bool HasPriority(
        UINT32 storyboardIdScheduled,
        UINT32 /* storyboardIdNew */,
        UI_ANIMATION_PRIORITY_EFFECT /* priorityEffect */
        )
    {
        // In this application, storyboards are canceled to avoid failure or delay

        // By default, storyboards cannot cancel other storyboards
        
        bool fHasPriority = false;
        
        // SLIDE_AFTER_WAVE can be canceled by any other storyboard
        
        if (storyboardIdScheduled == STORYBOARD_ID_SLIDE_AFTER_WAVE)
        {
            fHasPriority = true;
        }

        return fHasPriority;
    }
};

class CTrimPriorityComparison :
    public CPriorityComparisonBase<CTrimPriorityComparison>
{
public:

    virtual bool HasPriority(
        UINT32 storyboardIdScheduled,
        UINT32 storyboardIdNew,
        UI_ANIMATION_PRIORITY_EFFECT priorityEffect
        )
    {
        bool fHasPriority = false;
        
        // In this application, storyboards are trimmed only to avoid failure

        if (priorityEffect == UI_ANIMATION_PRIORITY_EFFECT_FAILURE)
        {
            // By default, storyboards can trim other storyboards
            
            fHasPriority = true;
            
            if (storyboardIdScheduled == STORYBOARD_ID_SLIDE_AFTER_WAVE &&
                storyboardIdNew == STORYBOARD_ID_WAVE)
            {
                // WAVE can trim SLIDE_AFTER_WAVE
                
                fHasPriority = true;
            }
            else
            {
                switch (storyboardIdNew)
                {
                case STORYBOARD_ID_SLIDE:
                case STORYBOARD_ID_SLIDE_AFTER_WAVE:
                case STORYBOARD_ID_WAVE:
                    
                    // Otherwise, SLIDE, SLIDE_AFTER_WAVE, and WAVE cannot trim each other
                    
                    switch (storyboardIdScheduled)
                    {
                    case STORYBOARD_ID_SLIDE:
                    case STORYBOARD_ID_SLIDE_AFTER_WAVE:
                    case STORYBOARD_ID_WAVE:
                        fHasPriority = false;
                        break;
                    }
                    
                    break;
                }
            }
        }

        return fHasPriority;
    }
};
