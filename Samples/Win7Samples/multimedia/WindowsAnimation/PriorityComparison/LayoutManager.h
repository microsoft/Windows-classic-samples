// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "Thumbnail.h"
#include "PriorityComparison.h"

#include <UIAnimation.h>
#include <D2D1.h>
#include <D2D1Helper.h>

class CLayoutManager
{
public:

    CLayoutManager();
    ~CLayoutManager();

    HRESULT Initialize(
        IUIAnimationManager *pAnimationManager,
        IUIAnimationTimer *pAnimationTimer,
        IUIAnimationTransitionLibrary *pTransitionLibrary,
        UINT uThumbCount,
        CThumbnail *thumbs
        );

    HRESULT Resize(
        D2D1_SIZE_F sizeClient
        );

    HRESULT Arrange();

    HRESULT Next();

    HRESULT Previous();

    HRESULT Wave(
        UI_ANIMATION_SLOPE slope
        );

protected:

    static const UINT SLIDE_INDEX_NONE = static_cast<UINT>(-1);

    HRESULT CenterThumbnails();

    HRESULT SlideThumbnails(
        STORYBOARD_ID storyboardId,
        DOUBLE acceleration,
        UI_ANIMATION_SECONDS secondsLongestAcceptableDelay,
        UINT slideUpIndex,
        UINT slideDownIndex
        );

    HRESULT ScheduleStoryboard(
        IUIAnimationStoryboard *pStoryboard
        );

    // Animation objects

    IUIAnimationManager *m_pAnimationManager;
    IUIAnimationTimer *m_pAnimationTimer;
    IUIAnimationTransitionLibrary *m_pTransitionLibrary;

    // Thumbnail array

    UINT m_uThumbCount;
    CThumbnail *m_thumbs;

    // Current selection

    UINT m_iThumbSelected;
    
    // Layout values
    
    DOUBLE m_centerX;
    DOUBLE m_rowY;
};
