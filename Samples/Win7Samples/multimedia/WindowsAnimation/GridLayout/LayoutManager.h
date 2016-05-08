// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "Thumbnail.h"

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

    HRESULT Arrange(
        D2D1_SIZE_F sizeClient
        );

protected:

    HRESULT ArrangeRow(
        IUIAnimationStoryboard *pStoryboard,
        int iThumbMin, 
        int iThumbMax,
        DOUBLE xRow,
        DOUBLE yRow,
        DOUBLE heightMax
        );

    HRESULT AddThumbnailTransitions(
        IUIAnimationStoryboard *pStoryboard,
        IUIAnimationVariable *pVariablePrimary,
        DOUBLE valuePrimary,
        IUIAnimationVariable *pVariableSecondary,
        DOUBLE valueSecondary
        );

    // Animation objects

    IUIAnimationManager *m_pAnimationManager;
    IUIAnimationTimer *m_pAnimationTimer;
    IUIAnimationTransitionLibrary *m_pTransitionLibrary;

    // Thumbnail array

    UINT m_uThumbCount;
    CThumbnail *m_thumbs;
};
