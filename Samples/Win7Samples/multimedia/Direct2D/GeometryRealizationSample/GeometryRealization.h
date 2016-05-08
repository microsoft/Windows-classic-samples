// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#ifndef REALIZATIONS_DECLARE_INTERFACE
#define REALIZATIONS_DECLARE_INTERFACE(X) DECLSPEC_UUID(X) DECLSPEC_NOVTABLE
#endif

typedef enum REALIZATION_CREATION_OPTIONS
{
    //
    // Generate mesh
    //
    REALIZATION_CREATION_OPTIONS_ALIASED = 1,

    //
    // Generate opacity mask
    //
    REALIZATION_CREATION_OPTIONS_ANTI_ALIASED = 2,

    //
    // Retain pointer to original geometry for unrealized rendering
    //
    REALIZATION_CREATION_OPTIONS_UNREALIZED = 4,

    //
    // Generate fill realization
    //
    REALIZATION_CREATION_OPTIONS_FILLED = 8,

    //
    // Generate stroke realization
    //
    REALIZATION_CREATION_OPTIONS_STROKED = 16
} REALIZATION_CREATION_OPTIONS;

typedef enum REALIZATION_RENDER_MODE
{
    //
    // Force the use of the realization
    //
    REALIZATION_RENDER_MODE_FORCE_REALIZED = 0,

    //
    // Force the use of the original geometry
    //
    REALIZATION_RENDER_MODE_FORCE_UNREALIZED = 1,

    //
    // Key off of the render-target to decide:
    //  SW: Unrealized
    //  HW: Realized
    //
    REALIZATION_RENDER_MODE_DEFAULT = 2

} REALIZATION_RENDER_MODE;


//+-----------------------------------------------------------------------------
//
//  Interface:
//      IGeometryRealization
//
//  Description:
//      Encapsulates various mesh and/or opacity mask instances to provide
//      efficient rendering of complex primitives.
//
//------------------------------------------------------------------------------
interface REALIZATIONS_DECLARE_INTERFACE("a0b504a9-be04-44a7-ae05-71ac89c1b6a7") IGeometryRealization : public IUnknown
{
    //
    // Render the fill realization to the render target
    //
    STDMETHOD(Fill)(
        ID2D1RenderTarget *pRT,
        ID2D1Brush *pBrush,
        REALIZATION_RENDER_MODE mode
        ) PURE;

    //
    // Render the stroke realization to the render target
    //
    STDMETHOD(Draw)(
        ID2D1RenderTarget *pRT,
        ID2D1Brush *pBrush,
        REALIZATION_RENDER_MODE mode
        ) PURE;

    //
    // Discard the current realization's contents and replace with new contents.
    //
    // Note: This method will attempt to reuse the existing bitmaps (but will
    // replace the bitmaps if they aren't large enough). Since the cost of
    // destroying a texture can be surprisingly astronomical, using this method
    // can be substantially more performant than recreating a new realization
    // every time.
    //
    // Note: pWorldTransform is the transform that the realization will
    // be optimized for. If, at the time of rendering, the render target's
    // transform is the same as pWorldTransform, 
    // the realization will appear identical to the unrealized version. Otherwise,
    // quality will be degraded.
    //
    STDMETHOD(Update)(
        ID2D1Geometry *pGeometry,
        REALIZATION_CREATION_OPTIONS options,
        CONST D2D1_MATRIX_3X2_F *pWorldTransform,
        float strokeWidth,
        ID2D1StrokeStyle *pIStrokeStyle
        ) PURE;
};


//+-----------------------------------------------------------------------------
//
//  Interface:
//      IGeometryRealizationFactory
//
//------------------------------------------------------------------------------
interface REALIZATIONS_DECLARE_INTERFACE("27866d9f-8865-461d-8a10-2531156398b2") IGeometryRealizationFactory : public IUnknown
{
    //
    // Create a realization object with no content (should be populated with
    // Update() before use).
    //
    STDMETHOD(CreateGeometryRealization)(
        IGeometryRealization **ppRealization
        ) PURE;

    //
    // Create a geometry realization.
    //
    // Note: Here, pWorldTransform is the transform that the realization will
    // be optimized for. If, at the time of rendering, the render target's
    // transform is the same as the pWorldTransform passed in here then
    // the realization will look identical to the unrealized version. Otherwise,
    // quality will be degraded.
    //
    STDMETHOD(CreateGeometryRealization)(
        ID2D1Geometry *pGeometry,
        REALIZATION_CREATION_OPTIONS options,
        CONST D2D1_MATRIX_3X2_F *pWorldTransform,
        float strokeWidth,
        ID2D1StrokeStyle *pIStrokeStyle,
        IGeometryRealization **ppRealization
        ) PURE;
};

//+-----------------------------------------------------------------------------
//
//  Function:
//      CreateGeometryRealizationFactory
//
//------------------------------------------------------------------------------
HRESULT CreateGeometryRealizationFactory(
    ID2D1RenderTarget *pRT,
    UINT maxRealizationDimension,
    IGeometryRealizationFactory **ppFactory
    );

HRESULT CreateGeometryRealizationFactory(
    ID2D1RenderTarget *pRT,
    IGeometryRealizationFactory **ppFactory
    );
