// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "Thumbnail.h"
#include "CustomInterpolator.h"

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
        IUIAnimationTransitionFactory *pTransitionFactory,
        UINT uThumbCount,
        CThumbnail *thumbs
        );

    HRESULT Arrange(
        D2D1_SIZE_F sizeClient
        );

    HRESULT Attract(
        DOUBLE finalYValue
        );

protected:

    HRESULT AddThumbnailTransitions(
        IUIAnimationStoryboard *pStoryboard,
        IUIAnimationVariable *pVariablePrimary,
        DOUBLE valuePrimary,
        IUIAnimationVariable *pVariableSecondary,
        DOUBLE valueSecondary
        );

    HRESULT ScheduleStoryboard(
        IUIAnimationStoryboard *pStoryboard
        );

    DOUBLE RandomFromRange(
        DOUBLE minimum,
        DOUBLE maximum
        );

    // Animation objects

    IUIAnimationManager *m_pAnimationManager;
    IUIAnimationTimer *m_pAnimationTimer;
    IUIAnimationTransitionLibrary *m_pTransitionLibrary;
    IUIAnimationTransitionFactory *m_pTransitionFactory;

    // Thumbnail array

    UINT m_uThumbCount;
    CThumbnail *m_thumbs;
};
