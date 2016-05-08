// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

//
// The following is a relatively simple implementation of realizations, built
// on top of Direct2D's mesh and opacity mask primitives (meshes for aliased and
// multi-sampled anti-aliased rendering and opacity masks for per-primitive
// anti-aliased rendering).
//
// One IGeometryRealization object can hold up to 4 "sub-realizations",
// corresponding to the matrix {Aliased, PPAA} x {Filled, Stroked}. The user
// can specify which sub-realizations to generate by passing in the appropriate
// REALIZATION_CREATION_OPTIONS.
//
// The implementation of the PPAA realizations is somewhat primitive. It will
// attempt to reuse existing bitmaps when possible, but it will not, for
// instance, attempt to use a single bitmap to store multiple realizations
// ("atlasing"). An atlased implementation would be somewhat more performant,
// as the number of state switches that Direct2D would have to make when
// interleaving realizations of different geometries would be greatly reduced.
//
// Another limitation in the PPAA implementation below is that it can use very
// large amounts of video-memory even for very simple primitives. Consider, for
// instance, a thin diagonal line stretching from the top-left corner of the
// render-target to the bottom-right. To store this as a single opacity mask, a
// bitmap the size of the entire render target must be created, even though the
// area of the stroke is very small. This can also have a severe impact on
// performance, as the video card has to waste numerous cycles rendering fully-
// transparent pixels.
//
// A more sophisticated implementation of PPAA realizations would divide the
// geometry up into a grid of bitmaps. Grid cells that contained no
// content or were fully covered by the geometry could be optimized away. The
// partially covered grid cells could be atlased for a further boost in
// performance.
//


#ifndef _NO_PRECOMPILED_HEADER_
#include "stdafx.h"
#endif


//+-----------------------------------------------------------------------------
//
//  Class:
//      GeometryRealization
//
//------------------------------------------------------------------------------
class GeometryRealization : public IGeometryRealization
{
public:
    STDMETHOD(Fill)(
        ID2D1RenderTarget *pRT,
        ID2D1Brush *pBrush,
        REALIZATION_RENDER_MODE mode
        );

    STDMETHOD(Draw)(
        ID2D1RenderTarget *pRT,
        ID2D1Brush *pBrush,
        REALIZATION_RENDER_MODE mode
        );

    STDMETHOD(Update)(
        ID2D1Geometry *pGeometry,
        REALIZATION_CREATION_OPTIONS options,
        CONST D2D1_MATRIX_3X2_F *pWorldTransform,
        float strokeWidth,
        ID2D1StrokeStyle *pIStrokeStyle
        );

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(QueryInterface)(
        REFIID iid,
        void ** ppvObject
        );

    static HRESULT Create(
        ID2D1RenderTarget *pRT,
        UINT maxRealizationDimension,
        ID2D1Geometry *pGeometry,
        REALIZATION_CREATION_OPTIONS options,
        CONST D2D1_MATRIX_3X2_F *pWorldTransform,
        float strokeWidth,
        ID2D1StrokeStyle *pIStrokeStyle,
        IGeometryRealization **ppRealization
        );

// These methods are declared protected for white-box unit-testing.
protected:
    // Non-interface methods
    GeometryRealization();
    ~GeometryRealization();

    HRESULT Initialize(
        ID2D1RenderTarget *pRT,
        UINT maxRealizationDimension,
        ID2D1Geometry *pGeometry,
        REALIZATION_CREATION_OPTIONS options,
        CONST D2D1_MATRIX_3X2_F *pWorldTransform,
        float strokeWidth,
        ID2D1StrokeStyle *pIStrokeStyle
        );

    static HRESULT GenerateOpacityMask(
        bool fill,
        ID2D1RenderTarget *pBaseRT,
        UINT maxRealizationDimension,
        ID2D1BitmapRenderTarget **ppBitmapRT,
        ID2D1Geometry *pIGeometry,
        const D2D1_MATRIX_3X2_F *pWorldTransform,
        float strokeWidth,
        ID2D1StrokeStyle *pStrokeStyle,
        D2D1_RECT_F *pMaskDestBounds,
        D2D1_RECT_F *pMaskSourceBounds
        );

    HRESULT RenderToTarget(
        bool fill,
        ID2D1RenderTarget *pRT,
        ID2D1Brush *pBrush,
        REALIZATION_RENDER_MODE mode
        );

    ID2D1Mesh *m_pFillMesh;
    ID2D1Mesh *m_pStrokeMesh;

    ID2D1BitmapRenderTarget *m_pFillRT;
    ID2D1BitmapRenderTarget *m_pStrokeRT;

    ID2D1Geometry *m_pGeometry;
    ID2D1StrokeStyle *m_pStrokeStyle;
    float m_strokeWidth;

    D2D1_RECT_F m_fillMaskDestBounds;
    D2D1_RECT_F m_fillMaskSourceBounds;

    D2D1_RECT_F m_strokeMaskDestBounds;
    D2D1_RECT_F m_strokeMaskSourceBounds;

    ID2D1RenderTarget *m_pRT;

    bool m_realizationTransformIsIdentity;
    D2D1::Matrix3x2F m_realizationTransform;
    D2D1::Matrix3x2F m_realizationTransformInv;

    BOOL m_swRT;

    UINT m_maxRealizationDimension;

    ULONG volatile m_cRef;
};

//+-----------------------------------------------------------------------------
//
//  Class:
//      GeometryRealizationFactory
//
//------------------------------------------------------------------------------
class GeometryRealizationFactory : public IGeometryRealizationFactory
{
public:
    STDMETHOD(CreateGeometryRealization)(
        IGeometryRealization **ppRealization
        );

    STDMETHOD(CreateGeometryRealization)(
        ID2D1Geometry *pGeometry,
        REALIZATION_CREATION_OPTIONS options,
        CONST D2D1_MATRIX_3X2_F *pWorldTransform,
        float strokeWidth,
        ID2D1StrokeStyle *pIStrokeStyle,
        IGeometryRealization **ppRealization
        );

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(QueryInterface)(
        REFIID iid,
        void ** ppvObject
        );

    // Non-interface methods
    static HRESULT Create(
        ID2D1RenderTarget *pRT,
        UINT maxRealizationDimension,
        IGeometryRealizationFactory **ppFactory
        );

// These methods are declared protected for white-box unit-testing.
protected:

    GeometryRealizationFactory();
    ~GeometryRealizationFactory();

    HRESULT Initialize(
        ID2D1RenderTarget *pRT,
        UINT maxRealizationDimension
        );

    ID2D1RenderTarget *m_pRT;

    ULONG volatile m_cRef;

    UINT m_maxRealizationDimension;
};

// The maximum granularity of bitmap sizes we allow for AA realizations.
static const UINT sc_bitmapChunkSize = 64;

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
    )
{
    return GeometryRealizationFactory::Create(
        pRT,
        maxRealizationDimension,
        ppFactory
        );
}

//+-----------------------------------------------------------------------------
//
//  Function:
//      CreateGeometryRealizationFactory
//
//------------------------------------------------------------------------------
HRESULT CreateGeometryRealizationFactory(
    ID2D1RenderTarget *pRT,
    IGeometryRealizationFactory **ppFactory
    )
{
    return CreateGeometryRealizationFactory(
        pRT,
        0xffffffff, // maxRealizationDimension
        ppFactory
        );
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealizationFactory::Create
//
//------------------------------------------------------------------------------
/* static */
HRESULT GeometryRealizationFactory::Create(
    ID2D1RenderTarget *pRT,
    UINT maxRealizationDimension,
    IGeometryRealizationFactory **ppFactory
)
{
    HRESULT hr = S_OK;

    GeometryRealizationFactory *pFactory = NULL;
    pFactory = new (std::nothrow) GeometryRealizationFactory();
    hr = pFactory ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pFactory->Initialize(pRT, maxRealizationDimension);

        if (SUCCEEDED(hr))
        {
            *ppFactory = pFactory;
            (*ppFactory)->AddRef();
        }

        pFactory->Release();
    }

    return hr;
}


//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealizationFactory::GeometryRealizationFactory
//
//------------------------------------------------------------------------------
GeometryRealizationFactory::GeometryRealizationFactory() :
    m_cRef(1),
    m_pRT(NULL)
{
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      ~GeometryRealizationFactory::GeometryRealizationFactory
//
//------------------------------------------------------------------------------
GeometryRealizationFactory::~GeometryRealizationFactory()
{
    SafeRelease(&m_pRT);
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealizationFactory::Initialize
//
//------------------------------------------------------------------------------
HRESULT GeometryRealizationFactory::Initialize(
    ID2D1RenderTarget *pRT,
    UINT maxRealizationDimension
    )
{
    HRESULT hr = S_OK;

    if (maxRealizationDimension == 0)
    {
        //
        // 0-sized bitmaps aren't very useful for realizations, and
        // DXGI surface render targets don't support them, anyway.
        //
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        m_pRT = pRT;
        m_pRT->AddRef();

        m_maxRealizationDimension = min(
            pRT->GetMaximumBitmapSize(),
            maxRealizationDimension
            );
    }

    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealizationFactory::CreateGeometryRealization
//
//------------------------------------------------------------------------------
STDMETHODIMP GeometryRealizationFactory::CreateGeometryRealization(
    ID2D1Geometry *pGeometry,
    REALIZATION_CREATION_OPTIONS options,
    CONST D2D1_MATRIX_3X2_F *pWorldTransform,
    float strokeWidth,
    ID2D1StrokeStyle *pIStrokeStyle,
    IGeometryRealization **ppRealization
    )
{
    return GeometryRealization::Create(
        m_pRT,
        m_maxRealizationDimension,
        pGeometry,
        options,
        pWorldTransform,
        strokeWidth,
        pIStrokeStyle,
        ppRealization
        );
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealizationFactory::CreateGeometryRealization
//
//------------------------------------------------------------------------------
STDMETHODIMP GeometryRealizationFactory::CreateGeometryRealization(
    IGeometryRealization **ppRealization
    )
{
    return GeometryRealization::Create(
        m_pRT,
        m_maxRealizationDimension,
        NULL, // pGeometry
        REALIZATION_CREATION_OPTIONS_ALIASED, // ignored.
        NULL,
        0.0f, // strokeWidth
        NULL, // pIStrokeStyle
        ppRealization
        );
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::Create
//
//------------------------------------------------------------------------------
/* static */
HRESULT GeometryRealization::Create(
    ID2D1RenderTarget *pRT,
    UINT maxRealizationDimension,
    ID2D1Geometry *pGeometry,
    REALIZATION_CREATION_OPTIONS options,
    CONST D2D1_MATRIX_3X2_F *pWorldTransform,
    float strokeWidth,
    ID2D1StrokeStyle *pIStrokeStyle,
    IGeometryRealization **ppRealization
    )
{
    HRESULT hr = S_OK;

    GeometryRealization *pRealization = NULL;

    pRealization = new (std::nothrow) GeometryRealization();
    hr = pRealization ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pRealization->Initialize(
            pRT,
            maxRealizationDimension,
            pGeometry,
            options,
            pWorldTransform,
            strokeWidth,
            pIStrokeStyle
            );
        if (SUCCEEDED(hr))
        {
            *ppRealization = pRealization;
            (*ppRealization)->AddRef();
        }

        pRealization->Release();
    }

    return hr;
}

//+----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::GeometryRealization
//
//-----------------------------------------------------------------------------

GeometryRealization::GeometryRealization() :
    m_cRef(1),
    m_pFillMesh(NULL),
    m_pStrokeMesh(NULL),
    m_pFillRT(NULL),
    m_pStrokeRT(NULL),
    m_pGeometry(NULL),
    m_pStrokeStyle(NULL),
    m_pRT(NULL)
{
}

//+----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::GeometryRealization
//
//-----------------------------------------------------------------------------
GeometryRealization::~GeometryRealization()
{
    SafeRelease(&m_pFillMesh);
    SafeRelease(&m_pStrokeMesh);
    SafeRelease(&m_pFillRT);
    SafeRelease(&m_pStrokeRT);
    SafeRelease(&m_pGeometry);
    SafeRelease(&m_pStrokeStyle);
    SafeRelease(&m_pRT);
}


//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::Fill
//
//------------------------------------------------------------------------------
STDMETHODIMP GeometryRealization::Fill(
    ID2D1RenderTarget *pRT,
    ID2D1Brush *pBrush,
    REALIZATION_RENDER_MODE mode
    )
{
    return RenderToTarget(
        true, // => fill
        pRT,
        pBrush,
        mode
        );
}


//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::Draw
//
//------------------------------------------------------------------------------
STDMETHODIMP GeometryRealization::Draw(
    ID2D1RenderTarget *pRT,
    ID2D1Brush *pBrush,
    REALIZATION_RENDER_MODE mode
    )
{
    return RenderToTarget(
        false, // => stroke
        pRT,
        pBrush,
        mode
        );
}


//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::Update
//
//  Description:
//      Discard the current realization's contents and replace with new
//      contents.
//
//      Note: This method attempts to reuse the existing bitmaps (but will
//      replace the bitmaps if they aren't large enough). Since the cost of
//      destroying a texture can be surprisingly astronomical, using this
//      method can be substantially more performant than recreating a new
//      realization every time.
//
//      Note: Here, pWorldTransform is the transform that the realization will
//      be optimized for. If, at the time of rendering, the render target's
//      transform is the same as the pWorldTransform passed in here then the
//      realization will look identical to the unrealized version. Otherwise,
//      quality will be degraded.
//
//------------------------------------------------------------------------------
STDMETHODIMP GeometryRealization::Update(
    ID2D1Geometry *pGeometry,
    REALIZATION_CREATION_OPTIONS options,
    CONST D2D1_MATRIX_3X2_F *pWorldTransform,
    float strokeWidth,
    ID2D1StrokeStyle *pIStrokeStyle
    )
{
    HRESULT hr = S_OK;

    if (pWorldTransform)
    {
        m_realizationTransform = *D2D1::Matrix3x2F::ReinterpretBaseType(pWorldTransform);
        m_realizationTransformIsIdentity = (m_realizationTransform.IsIdentity());
    }
    else
    {
        m_realizationTransform = D2D1::Matrix3x2F::Identity();
        m_realizationTransformIsIdentity = true;
    }

    //
    // We're about to create our realizations with the world transform applied
    // to them.  When we go to actually render the realization, though, we'll
    // want to "undo" this realization and instead apply the render target's
    // current transform.
    //
    // Note: we keep track to see if the passed in realization transform is the
    // identity.  This is a small optimization that saves us from having to
    // multiply matrices when we go to draw the realization.
    //

    m_realizationTransformInv = m_realizationTransform;
    m_realizationTransformInv.Invert();

    if ((options & REALIZATION_CREATION_OPTIONS_UNREALIZED) || m_swRT)
    {
        SafeReplace(&m_pGeometry, pGeometry);
        SafeReplace(&m_pStrokeStyle, pIStrokeStyle);
        m_strokeWidth = strokeWidth;
     }

    if (options & REALIZATION_CREATION_OPTIONS_ANTI_ALIASED)
    {
        //
        // Antialiased realizations are implemented using opacity masks.
        //

        if (options & REALIZATION_CREATION_OPTIONS_FILLED)
        {
            hr = GenerateOpacityMask(
                    true, // => filled
                    m_pRT,
                    m_maxRealizationDimension,
                    IN OUT &m_pFillRT,
                    pGeometry,
                    pWorldTransform,
                    strokeWidth,
                    pIStrokeStyle,
                    &m_fillMaskDestBounds,
                    &m_fillMaskSourceBounds
                    );
        }

        if (SUCCEEDED(hr) && options & REALIZATION_CREATION_OPTIONS_STROKED)
        {
            hr = GenerateOpacityMask(
                    false, // => stroked
                    m_pRT,
                    m_maxRealizationDimension,
                    IN OUT &m_pStrokeRT,
                    pGeometry,
                    pWorldTransform,
                    strokeWidth,
                    pIStrokeStyle,
                    &m_strokeMaskDestBounds,
                    &m_strokeMaskSourceBounds
                    );
        }
    }

    if (SUCCEEDED(hr) && options & REALIZATION_CREATION_OPTIONS_ALIASED)
    {
        //
        // Aliased realizations are implemented using meshes.
        //

        if (options & REALIZATION_CREATION_OPTIONS_FILLED)
        {
            ID2D1Mesh *pMesh = NULL;
            hr = m_pRT->CreateMesh(&pMesh);
            if (SUCCEEDED(hr))
            {
                ID2D1TessellationSink *pSink = NULL;
                hr = pMesh->Open(&pSink);
                if (SUCCEEDED(hr))
                {
                    hr = pGeometry->Tessellate(
                            pWorldTransform,
                            pSink
                            );
                    if (SUCCEEDED(hr))
                    {
                        hr = pSink->Close();
                        if (SUCCEEDED(hr))
                        {
                            SafeReplace(&m_pFillMesh, pMesh);
                        }
                    }
                    pSink->Release();
                }
                pMesh->Release();
            }
        }

        if (SUCCEEDED(hr) && options & REALIZATION_CREATION_OPTIONS_STROKED)
        {
            //
            // In order generate the mesh corresponding to the stroke of a
            // geometry, we first "widen" the geometry and then tessellate the
            // result.
            //
            ID2D1Factory *pFactory = NULL;
            m_pRT->GetFactory(&pFactory);

            ID2D1PathGeometry *pPathGeometry = NULL;
            hr = pFactory->CreatePathGeometry(&pPathGeometry);
            if (SUCCEEDED(hr))
            {
                ID2D1GeometrySink *pGeometrySink = NULL;
                hr = pPathGeometry->Open(&pGeometrySink);
                if (SUCCEEDED(hr))
                {
                    hr = pGeometry->Widen(
                            strokeWidth,
                            pIStrokeStyle,
                            pWorldTransform,
                            pGeometrySink
                            );
                    if (SUCCEEDED(hr))
                    {
                        hr = pGeometrySink->Close();
                        if (SUCCEEDED(hr))
                        {
                            ID2D1Mesh *pMesh = NULL;
                            hr = m_pRT->CreateMesh(&pMesh);
                            if (SUCCEEDED(hr))
                            {
                                ID2D1TessellationSink *pSink = NULL;
                                hr = pMesh->Open(&pSink);
                                if (SUCCEEDED(hr))
                                {
                                    hr = pPathGeometry->Tessellate(
                                            NULL, // world transform (already handled in Widen)
                                            pSink
                                            );
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = pSink->Close();
                                        if (SUCCEEDED(hr))
                                        {
                                            SafeReplace(&m_pStrokeMesh, pMesh);
                                        }
                                    }
                                    pSink->Release();
                                }
                                pMesh->Release();
                            }
                        }
                    }
                    pGeometrySink->Release();
                }
                pPathGeometry->Release();
            }
            pFactory->Release();
        }
    }

    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::RenderToTarget
//
//------------------------------------------------------------------------------
HRESULT GeometryRealization::RenderToTarget(
    bool fill,
    ID2D1RenderTarget *pRT,
    ID2D1Brush *pBrush,
    REALIZATION_RENDER_MODE mode
    )
{
    HRESULT hr = S_OK;

    D2D1_ANTIALIAS_MODE originalAAMode = pRT->GetAntialiasMode();
    D2D1_MATRIX_3X2_F originalTransform;

    if (((mode == REALIZATION_RENDER_MODE_DEFAULT) && m_swRT) ||
        (mode == REALIZATION_RENDER_MODE_FORCE_UNREALIZED)
       )
    {
        if (!m_pGeometry)
        {
            // We're being asked to render the geometry unrealized, but we
            // weren't created with REALIZATION_CREATION_OPTIONS_UNREALIZED.
            hr = E_FAIL;
        }
        if (SUCCEEDED(hr))
        {
            if (fill)
            {
                pRT->FillGeometry(
                    m_pGeometry,
                    pBrush
                    );
            }
            else
            {
                pRT->DrawGeometry(
                    m_pGeometry,
                    pBrush,
                    m_strokeWidth,
                    m_pStrokeStyle
                    );
            }
        }
    }
    else
    {
        if (originalAAMode != D2D1_ANTIALIAS_MODE_ALIASED)
        {
            pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
        }

        if (!m_realizationTransformIsIdentity)
        {
            pRT->GetTransform(&originalTransform);
            pRT->SetTransform(m_realizationTransformInv * originalTransform);
        }

        if (originalAAMode == D2D1_ANTIALIAS_MODE_PER_PRIMITIVE)
        {
            if (fill)
            {
                if (!m_pFillRT)
                {
                    hr = E_FAIL;
                }
                if (SUCCEEDED(hr))
                {
                    ID2D1Bitmap *pBitmap = NULL;
                    m_pFillRT->GetBitmap(&pBitmap);

                    //
                    // Note: The antialias mode must be set to aliased prior to calling
                    // FillOpacityMask.
                    //
                    pRT->FillOpacityMask(
                        pBitmap,
                        pBrush,
                        D2D1_OPACITY_MASK_CONTENT_GRAPHICS,
                        &m_fillMaskDestBounds,
                        &m_fillMaskSourceBounds
                        );

                    pBitmap->Release();
                }
            }
            else
            {
                if (!m_pStrokeRT)
                {
                    hr = E_FAIL;
                }
                if (SUCCEEDED(hr))
                {
                    ID2D1Bitmap *pBitmap = NULL;
                    m_pStrokeRT->GetBitmap(&pBitmap);

                    //
                    // Note: The antialias mode must be set to aliased prior to calling
                    // FillOpacityMask.
                    //
                    pRT->FillOpacityMask(
                        pBitmap,
                        pBrush,
                        D2D1_OPACITY_MASK_CONTENT_GRAPHICS,
                        &m_strokeMaskDestBounds,
                        &m_strokeMaskSourceBounds
                        );

                    pBitmap->Release();
                }
            }
        }
        else
        {
            if (fill)
            {
                if (!m_pFillMesh)
                {
                    hr = E_FAIL;
                }
                if (SUCCEEDED(hr))
                {
                    pRT->FillMesh(
                        m_pFillMesh,
                        pBrush
                        );
                }
            }
            else
            {
                if (!m_pStrokeMesh)
                {
                    hr = E_FAIL;
                }
                if (SUCCEEDED(hr))
                {
                    pRT->FillMesh(
                        m_pStrokeMesh,
                        pBrush
                        );
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            pRT->SetAntialiasMode(originalAAMode);

            if (!m_realizationTransformIsIdentity)
            {
                pRT->SetTransform(originalTransform);
            }
        }
    }

    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::Initialize
//
//------------------------------------------------------------------------------
HRESULT GeometryRealization::Initialize(
    ID2D1RenderTarget *pRT,
    UINT maxRealizationDimension,
    ID2D1Geometry *pGeometry,
    REALIZATION_CREATION_OPTIONS options,
    CONST D2D1_MATRIX_3X2_F *pWorldTransform,
    float strokeWidth,
    ID2D1StrokeStyle *pIStrokeStyle
)
{
    HRESULT hr = S_OK;

    m_pRT = pRT;
    m_pRT->AddRef();

    m_swRT = pRT->IsSupported(
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_SOFTWARE
            ));

    m_maxRealizationDimension = maxRealizationDimension;

    if (pGeometry)
    {
        hr = Update(
            pGeometry,
            options,
            pWorldTransform,
            strokeWidth,
            pIStrokeStyle
            );
    }

    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::GenerateOpacityMask
//
//  Notes:
//      This method is the trickiest part of doing realizations. Conceptually,
//      we're creating a grayscale bitmap that represents the geometry. We'll
//      reuse an existing bitmap if we can, but if not, we'll create the
//      smallest possible bitmap that contains the geometry. In either, case,
//      though, we'll keep track of the portion of the bitmap we actually used
//      (the source bounds), so when we go to draw the realization, we don't
//      end up drawing a bunch of superfluous transparent pixels.
//
//      We also have to keep track of the "dest" bounds, as more than likely
//      the bitmap has to be translated by some amount before being drawn.
//
//------------------------------------------------------------------------------
/* static */
HRESULT GeometryRealization::GenerateOpacityMask(
    bool fill,
    ID2D1RenderTarget *pBaseRT,
    UINT maxRealizationDimension,
    ID2D1BitmapRenderTarget **ppBitmapRT,
    ID2D1Geometry *pIGeometry,
    const D2D1_MATRIX_3X2_F *pWorldTransform,
    float strokeWidth,
    ID2D1StrokeStyle *pStrokeStyle,
    D2D1_RECT_F *pMaskDestBounds,
    D2D1_RECT_F *pMaskSourceBounds
    )
{
    HRESULT hr = S_OK;

    D2D1_RECT_F bounds;

    D2D1_RECT_F inflatedPixelBounds;
    D2D1_SIZE_U inflatedIntegerPixelSize;
    D2D1_SIZE_U currentRTSize;
    D2D1_MATRIX_3X2_F translateMatrix;
    float dpiX, dpiY;
    float scaleX = 1.0f;
    float scaleY = 1.0f;

    ID2D1BitmapRenderTarget *pCompatRT = NULL;
    SafeReplace(&pCompatRT, *ppBitmapRT);

    ID2D1SolidColorBrush *pBrush = NULL;

    hr = pBaseRT->CreateSolidColorBrush(
        D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f),
        &pBrush
        );
    if (SUCCEEDED(hr))
    {
        pBaseRT->GetDpi(&dpiX, &dpiY);

        if (fill)
        {
            hr = pIGeometry->GetBounds(
                pWorldTransform,
                &bounds
                );
        }
        else
        {
            hr = pIGeometry->GetWidenedBounds(
                strokeWidth,
                pStrokeStyle,
                pWorldTransform,
                &bounds
                );
        }

        if (SUCCEEDED(hr))
        {
            //
            // A rect where left > right is defined to be empty.
            //
            // The slightly baroque expression used below is an idiom that also
            // correctly handles NaNs (i.e., if any of the coordinates of the bounds is
            // a NaN, we want to treat the bounds as empty)
            //
            if (
                !(bounds.left <= bounds.right) ||
                !(bounds.top <= bounds.bottom)
               )
            {
                // Bounds are empty or ill-defined.

                // Make up a fake bounds
                inflatedPixelBounds.top = 0.0f;
                inflatedPixelBounds.left = 0.0f;
                inflatedPixelBounds.bottom = 1.0f;
                inflatedPixelBounds.right = 1.0f;
            }
            else
            {
                //
                // We inflate the pixel bounds by 1 in each direction to ensure we have
                // a border of completely transparent pixels around the geometry.  This
                // ensures that when the realization is stretched the alpha ramp still
                // smoothly falls off to 0 rather than being clipped by the rect.
                //
                inflatedPixelBounds.top = floorf(bounds.top*dpiY/96)-1.0f;
                inflatedPixelBounds.left = floorf(bounds.left*dpiX/96)-1.0f;
                inflatedPixelBounds.bottom = ceilf(bounds.bottom*dpiY/96)+1.0f;
                inflatedPixelBounds.right = ceilf(bounds.right*dpiX/96)+1.0f;
            }


            //
            // Compute the width and height of the underlying bitmap we will need.
            // Note: We round up the width and height to be a multiple of
            // sc_bitmapChunkSize. We do this primarily to ensure that we aren't
            // constantly reallocating bitmaps in the case where a realization is being
            // zoomed in on slowly and updated frequently.
            //

            inflatedIntegerPixelSize = D2D1::SizeU(
                static_cast<UINT>(inflatedPixelBounds.right - inflatedPixelBounds.left),
                static_cast<UINT>(inflatedPixelBounds.bottom - inflatedPixelBounds.top)
                );

            // Round up
            inflatedIntegerPixelSize.width =
                (inflatedIntegerPixelSize.width + sc_bitmapChunkSize - 1)/sc_bitmapChunkSize * sc_bitmapChunkSize;

            // Round up
            inflatedIntegerPixelSize.height =
                (inflatedIntegerPixelSize.height + sc_bitmapChunkSize - 1)/sc_bitmapChunkSize * sc_bitmapChunkSize;

            //
            // Compute the bounds we will pass to FillOpacityMask (which are in Device
            // Independent Pixels).
            //
            // Note: The DIP bounds do *not* use the rounded coordinates, since this
            // would cause us to render superfluous, fully-transparent pixels, which
            // would hurt fill rate.
            //
            D2D1_RECT_F inflatedDipBounds = D2D1::RectF(
                inflatedPixelBounds.left * 96/dpiX,
                inflatedPixelBounds.top * 96/dpiY,
                inflatedPixelBounds.right * 96/dpiX,
                inflatedPixelBounds.bottom * 96/dpiY
                );

            if (pCompatRT)
            {
                currentRTSize = pCompatRT->GetPixelSize();
            }
            else
            {
                // This will force the creation of a new target
                currentRTSize = D2D1::SizeU(0,0);
            }

            //
            // We need to ensure that our desired render target size isn't larger than
            // the max allowable bitmap size. If it is, we need to scale the bitmap
            // down by the appropriate amount.
            //

            if (inflatedIntegerPixelSize.width > maxRealizationDimension)
            {
                scaleX = maxRealizationDimension/static_cast<float>(inflatedIntegerPixelSize.width);
                inflatedIntegerPixelSize.width = maxRealizationDimension;
            }

            if (inflatedIntegerPixelSize.height > maxRealizationDimension)
            {
                scaleY = maxRealizationDimension/static_cast<float>(inflatedIntegerPixelSize.height);
                inflatedIntegerPixelSize.height = maxRealizationDimension;
            }


            //
            // If the necessary pixel dimensions are less than half the existing
            // bitmap's dimensions (in either direction), force the bitmap to be
            // reallocated to save memory.
            //
            // Note: The fact that we use > rather than >= is important for a subtle
            // reason: We'd like to have the property that repeated small changes in
            // geometry size do not cause repeated reallocations of memory. >= does not
            // ensure this property in the case where the geometry size is close to
            // sc_bitmapChunkSize, but > does.
            //
            // Example:
            //
            // Assume sc_bitmapChunkSize is 64 and the initial geometry width is 63
            // pixels. This will get rounded up to 64, and we will allocate a bitmap
            // with width 64. Now, say, we zoom in slightly, so the new geometry width
            // becomes 65 pixels. This will get rounded up to 128 pixels, and a new
            // bitmap will be allocated. Now, say the geometry drops back down to 63
            // pixels. This will get rounded up to 64. If we used >=, this would cause
            // another reallocation.  Since we use >, on the other hand, the 128 pixel
            // bitmap will be reused.
            //

            if (currentRTSize.width > 2*inflatedIntegerPixelSize.width ||
                currentRTSize.height > 2*inflatedIntegerPixelSize.height
               )
            {
                SafeRelease(&pCompatRT);
                currentRTSize.width = currentRTSize.height = 0;
            }

            if (inflatedIntegerPixelSize.width > currentRTSize.width ||
                inflatedIntegerPixelSize.height > currentRTSize.height
               )
            {
                SafeRelease(&pCompatRT);
            }

            if (!pCompatRT)
            {
                //
                // Make sure our new rendertarget is strictly larger than before.
                //
                currentRTSize.width =
                    max(inflatedIntegerPixelSize.width, currentRTSize.width);

                currentRTSize.height =
                    max(inflatedIntegerPixelSize.height, currentRTSize.height);

                D2D1_PIXEL_FORMAT alphaOnlyFormat =
                    D2D1::PixelFormat(
                        DXGI_FORMAT_A8_UNORM,
                        D2D1_ALPHA_MODE_PREMULTIPLIED
                        );

                hr = pBaseRT->CreateCompatibleRenderTarget(
                    NULL, // desiredSize
                    &currentRTSize,
                    &alphaOnlyFormat,
                    D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE,
                    &pCompatRT
                    );
            }

            if (SUCCEEDED(hr))
            {
                //
                // Translate the geometry so it is flush against the left and top
                // sides of the render target.
                //

                translateMatrix =
                    D2D1::Matrix3x2F::Translation(
                        -inflatedDipBounds.left,
                        -inflatedDipBounds.top
                        ) *
                    D2D1::Matrix3x2F::Scale(
                        scaleX,
                        scaleY
                        );

                if (pWorldTransform)
                {
                    pCompatRT->SetTransform(
                        *pWorldTransform * translateMatrix
                        );
                }
                else
                {
                    pCompatRT->SetTransform(
                        translateMatrix
                        );
                }

                //
                // Render the geometry.
                //

                pCompatRT->BeginDraw();

                pCompatRT->Clear(
                    D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f)
                    );

                if (fill)
                {
                    pCompatRT->FillGeometry(
                        pIGeometry,
                        pBrush
                        );
                }
                else
                {
                    pCompatRT->DrawGeometry(
                        pIGeometry,
                        pBrush,
                        strokeWidth,
                        pStrokeStyle
                        );
                }

                hr = pCompatRT->EndDraw();
                if (SUCCEEDED(hr))
                {
                    //
                    // Report back the source and dest bounds (to be used as input parameters
                    // to FillOpacityMask.
                    //
                    *pMaskDestBounds = inflatedDipBounds;

                    *pMaskSourceBounds = D2D1::Rect<float>(
                        0.0f,
                        0.0f,
                        static_cast<float>(inflatedDipBounds.right - inflatedDipBounds.left)*scaleX,
                        static_cast<float>(inflatedDipBounds.bottom - inflatedDipBounds.top)*scaleY
                        );

                    if (*ppBitmapRT != pCompatRT)
                    {
                        SafeReplace(ppBitmapRT, pCompatRT);
                    }
                }
            }
        }
        pBrush->Release();
    }

    SafeRelease(&pCompatRT);

    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealizationFactory::AddRef
//
//------------------------------------------------------------------------------
STDMETHODIMP_(ULONG) GeometryRealizationFactory::AddRef()
{
    return InterlockedIncrement(reinterpret_cast<LONG volatile *>(&m_cRef));
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealizationFactory::Release
//
//------------------------------------------------------------------------------
STDMETHODIMP_(ULONG) GeometryRealizationFactory::Release()
{
    ULONG cRef = static_cast<ULONG>(
        InterlockedDecrement(reinterpret_cast<LONG volatile *>(&m_cRef)));

    if(0 == cRef)
    {
        delete this;
    }

    return cRef;
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealizationFactory::QueryInterface
//
//------------------------------------------------------------------------------
STDMETHODIMP GeometryRealizationFactory::QueryInterface(
    REFIID iid,
    void ** ppvObject
    )
{
    HRESULT hr = S_OK;

    if (__uuidof(IUnknown) == iid)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
    }
    else if (__uuidof(IGeometryRealizationFactory) == iid)
    {
        *ppvObject = static_cast<IGeometryRealizationFactory*>(this);
        AddRef();
    }
    else
    {
        *ppvObject = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::AddRef
//
//------------------------------------------------------------------------------
STDMETHODIMP_(ULONG)
GeometryRealization::AddRef()
{
    return InterlockedIncrement(reinterpret_cast<LONG volatile *>(&m_cRef));
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::Release
//
//------------------------------------------------------------------------------
STDMETHODIMP_(ULONG)
GeometryRealization::Release()
{
    ULONG cRef = static_cast<ULONG>(
        InterlockedDecrement(reinterpret_cast<LONG volatile *>(&m_cRef)));

    if(0 == cRef)
    {
        delete this;
    }

    return cRef;
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      GeometryRealization::QueryInterface
//
//------------------------------------------------------------------------------
STDMETHODIMP
GeometryRealization::QueryInterface(
    REFIID iid,
    void ** ppvObject
    )
{
    HRESULT hr = S_OK;

    if (__uuidof(IUnknown) == iid)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
    }
    else if (__uuidof(IGeometryRealization) == iid)
    {
        *ppvObject = static_cast<IGeometryRealization*>(this);
        AddRef();
    }
    else
    {
        *ppvObject = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}

