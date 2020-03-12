// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Interactive3dTextSample.h"

static const UINT sc_maxMsaaSampleCount = 4;
static const float sc_flatteningTolerance = .1f;
static const float sc_cursorWidth = 5.0f;

static const D3DXVECTOR3 sc_eyeLocation(0.0f, -100.0f, 400.0f);
static const D3DXVECTOR3 sc_eyeAt(0.0f, -50.0f, 0.0f);
static const D3DXVECTOR3 sc_eyeUp(0.0f, -1.0f, 0.0f);

//
// Note: Gabriola is a Win7 only font family. If the system cannot find this
// font, DWrite will fall back to another font family.
//
static const PCWSTR sc_fontFace = L"Gabriola";
static const float sc_fontSize = 96.0f;


/******************************************************************
*                                                                 *
*  WinMain                                                        *
*                                                                 *
*  Application entrypoint                                         *
*                                                                 *
******************************************************************/

int WINAPI WinMain(
    HINSTANCE /*hInstance*/,
    HINSTANCE /*hPrevInstance*/,
    LPSTR /*lpCmdLine*/,
    int /*nCmdShow*/
    )
{
    // Ignoring the return value because we want to continue running even in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            Interactive3dTextSampleApp app;

            if (SUCCEEDED(app.Initialize()))
            {
                app.RunMessageLoop();
            }
        }
        CoUninitialize();
    }

    return 0;
}

/******************************************************************
*                                                                 *
*  NoRefComObject                                                 *
*                                                                 *
*  Template that creates dummy IUnknown method implementations.   *
*  This is useful in situations where one is absolutely sure      *
*  none of the IUnknown methods will be called (neither D2D nor   *
*  DWrite will call these methods on a ID2D1SimplifiedGeometrySink*
*  or IDWriteTextRenderer). Using this technique also allows one  *
*  to stack allocate such objects.                                *
*                                                                 *
******************************************************************/
template<class T>
class NoRefComObject : public T
{
public:
    template<typename U>
    NoRefComObject(U u)
        : T(u)
    {
    }

    template<typename U, typename V>
    NoRefComObject(U u, V v)
        : T(u, v)
    {
    }

    template<typename U, typename V, typename W>
    NoRefComObject(U u, V v, W w)
        : T(u, v, w)
    {
    }

    STDMETHOD_(ULONG, AddRef)(THIS)
    {
        return 0;
    }

    STDMETHOD_(ULONG, Release)(THIS)
    {
        return 0;
    }

    STDMETHOD(QueryInterface)(THIS_ REFIID /*riid*/, void** /*ppvObj*/)
    {
        return E_UNEXPECTED;
    }
};

/******************************************************************
*                                                                 *
*  IsEmptyBounds                                                  *
*                                                                 *
*  Returns true if the bounds are empty                           *
*                                                                 *
******************************************************************/
bool IsEmptyBounds(D2D1_RECT_F &bounds)
{
    // Direct2D uses the convention that right>left is empty.
    return bounds.left > bounds.right;
}

/******************************************************************
*                                                                 *
*  CreateEmptyGeometry                                            *
*                                                                 *
*  Create a path geometry containing no figures.                  *
*                                                                 *
******************************************************************/
HRESULT CreateEmptyGeometry(ID2D1Factory *pFactory, ID2D1Geometry **ppGeometry)
{
    HRESULT hr;
    ID2D1PathGeometry *pGeometry = NULL;

    hr = pFactory->CreatePathGeometry(&pGeometry);
    if (SUCCEEDED(hr))
    {
        ID2D1GeometrySink *pSink = NULL;
        hr = pGeometry->Open(&pSink);

        if (SUCCEEDED(hr))
        {
            hr = pSink->Close();

            if (SUCCEEDED(hr))
            {
                *ppGeometry = pGeometry;
                (*ppGeometry)->AddRef();
            }
            pSink->Release();
        }
        pGeometry->Release();
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  PointSnapper                                                   *
*                                                                 *
*  Utility class for snapping points and vertices to grid points. *
*                                                                 *
*  This is useful when preparing and consuming tessellation data  *
*  (see notes further on in the code).                            *
*                                                                 *
*  Grid-points are spaced at 1/16 intervals and aligned on        *
*  integer boundaries.                                            *
*                                                                 *
******************************************************************/
class PointSnapper
{
public:
    static HRESULT SnapGeometry(ID2D1Geometry *pGeometry, ID2D1Geometry **ppGeometry)
    {
        HRESULT hr;

        ID2D1Factory *pFactory = NULL;
        pGeometry->GetFactory(&pFactory);

        ID2D1PathGeometry *pPathGeometry = NULL;
        hr = pFactory->CreatePathGeometry(&pPathGeometry);

        if (SUCCEEDED(hr))
        {
            ID2D1GeometrySink *pSink = NULL;
            hr = pPathGeometry->Open(&pSink);

            if (SUCCEEDED(hr))
            {
                NoRefComObject<PointSnappingSink> snapper(pSink);

                hr = pGeometry->Simplify(
                        D2D1_GEOMETRY_SIMPLIFICATION_OPTION_LINES,
                        NULL, // world transform
                        &snapper
                        );

                if (SUCCEEDED(hr))
                {
                    hr = snapper.Close();

                    if (SUCCEEDED(hr))
                    {
                        *ppGeometry = pPathGeometry;
                        (*ppGeometry)->AddRef();
                    }
                }

                pSink->Release();
            }
            pPathGeometry->Release();
        }
        pFactory->Release();

        return hr;
    }

    static float SnapCoordinate(float x)
    {
        return floorf(16.0f*x+.5f)/16.0f;
    }

    static D2D1_POINT_2F SnapPoint(D2D1_POINT_2F pt)
    {
        return D2D1::Point2F(
            SnapCoordinate(pt.x),
            SnapCoordinate(pt.y)
            );
    }

    static D3DXVECTOR2 SnapPoint(D3DXVECTOR2 pt)
    {
        return D3DXVECTOR2(
            SnapCoordinate(pt.x),
            SnapCoordinate(pt.y)
            );
    }

private:

    /******************************************************************
    *                                                                 *
    *  PointSnappingSink                                              *
    *                                                                 *
    *  Internal sink used to implement SnapGeometry.                  *
    *                                                                 *
    *  Note: This class makes certain assumptions about it's usage    *
    *  (e.g. no Beziers), which is why it's a private class.          *
    *                                                                 *
    ******************************************************************/
    class PointSnappingSink : public ID2D1SimplifiedGeometrySink
    {
    public:
        PointSnappingSink(ID2D1SimplifiedGeometrySink *pSink)
            : m_pSinkNoRef(pSink)
        {
        }

        STDMETHOD_(void, AddBeziers)(const D2D1_BEZIER_SEGMENT * /*beziers*/, UINT /*beziersCount*/)
        {
            //
            // Users should be sure to flatten their geometries prior to passing
            // through a PointSnappingSink. It makes little sense snapping
            // the control points of a Bezier, as the vertices from the
            // flattened Bezier will almost certainly not be snapped.
            //
        }

        STDMETHOD_(void, AddLines)(const D2D1_POINT_2F *points, UINT pointsCount)
        {
            for (UINT i = 0; i < pointsCount; ++i)
            {
                D2D1_POINT_2F pt = SnapPoint(points[i]);

                m_pSinkNoRef->AddLines(&pt, 1);
            }
        }

        STDMETHOD_(void, BeginFigure)(D2D1_POINT_2F startPoint, D2D1_FIGURE_BEGIN figureBegin)
        {
            D2D1_POINT_2F pt = SnapPoint(startPoint);

            m_pSinkNoRef->BeginFigure(pt, figureBegin);
        }

        STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END figureEnd)
        {
            m_pSinkNoRef->EndFigure(figureEnd);
        }

        STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE fillMode)
        {
            m_pSinkNoRef->SetFillMode(fillMode);
        }

        STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT vertexFlags)
        {
            m_pSinkNoRef->SetSegmentFlags(vertexFlags);
        }

        STDMETHOD(Close)()
        {
            return m_pSinkNoRef->Close();
        }

    private:
        ID2D1SimplifiedGeometrySink *m_pSinkNoRef;
    };
};


/******************************************************************
*                                                                 *
*  D2DFlatten                                                     *
*                                                                 *
*  Helper function for performing "flattening" -- transforming    *
*  a geometry with Beziers into one containing only line segments.*
*                                                                 *
******************************************************************/
HRESULT D2DFlatten(
    ID2D1Geometry *pGeometry,
    float flatteningTolerance,
    ID2D1Geometry **ppGeometry
    )
{
    HRESULT hr;
    ID2D1Factory *pFactory = NULL;
    pGeometry->GetFactory(&pFactory);

    ID2D1PathGeometry *pPathGeometry = NULL;
    hr = pFactory->CreatePathGeometry(&pPathGeometry);

    if (SUCCEEDED(hr))
    {
        ID2D1GeometrySink *pSink = NULL;
        hr = pPathGeometry->Open(&pSink);

        if (SUCCEEDED(hr))
        {
            hr = pGeometry->Simplify(
                    D2D1_GEOMETRY_SIMPLIFICATION_OPTION_LINES,
                    NULL, // world transform
                    flatteningTolerance,
                    pSink
                    );

            if (SUCCEEDED(hr))
            {
                hr = pSink->Close();

                if (SUCCEEDED(hr))
                {
                    *ppGeometry = pPathGeometry;
                    (*ppGeometry)->AddRef();
                }
            }
            pSink->Release();
        }
        pPathGeometry->Release();
    }

    pFactory->Release();

    return hr;
}


/******************************************************************
*                                                                 *
*  D2DOutline                                                     *
*                                                                 *
*  Helper function for performing "outlining" -- constructing an  *
*  equivalent geometry with no self-intersections.                *
*                                                                 *
*  Note: This uses the default flattening tolerance and hence     *
*  should not be used with very small geometries.                 *
*                                                                 *
******************************************************************/
HRESULT D2DOutline(
    ID2D1Geometry *pGeometry,
    ID2D1Geometry **ppGeometry
    )
{
    HRESULT hr;
    ID2D1Factory *pFactory = NULL;
    pGeometry->GetFactory(&pFactory);

    ID2D1PathGeometry *pPathGeometry = NULL;
    hr = pFactory->CreatePathGeometry(&pPathGeometry);

    if (SUCCEEDED(hr))
    {
        ID2D1GeometrySink *pSink = NULL;
        hr = pPathGeometry->Open(&pSink);

        if (SUCCEEDED(hr))
        {
            hr = pGeometry->Outline(NULL, pSink);

            if (SUCCEEDED(hr))
            {
                hr = pSink->Close();

                if (SUCCEEDED(hr))
                {
                    *ppGeometry = pPathGeometry;
                    (*ppGeometry)->AddRef();
                }
            }
            pSink->Release();
        }
        pPathGeometry->Release();
    }

    pFactory->Release();

    return hr;
}

/******************************************************************
*                                                                 *
*  D2DCombine                                                     *
*                                                                 *
*  Helper function for performing Boolean operations.             *
*                                                                 *
*  Note: This uses the default flattening tolerance and hence     *
*  should not be used with very small geometries.                 *
*                                                                 *
******************************************************************/
HRESULT D2DCombine(
    D2D1_COMBINE_MODE combineMode,
    ID2D1Geometry *pGeometry1,
    ID2D1Geometry *pGeometry2,
    ID2D1Geometry **ppGeometry
    )
{
    HRESULT hr;
    ID2D1Factory *pFactory = NULL;
    pGeometry1->GetFactory(&pFactory);

    ID2D1PathGeometry *pPathGeometry = NULL;
    hr = pFactory->CreatePathGeometry(&pPathGeometry);

    if (SUCCEEDED(hr))
    {
        ID2D1GeometrySink *pSink = NULL;
        hr = pPathGeometry->Open(&pSink);

        if (SUCCEEDED(hr))
        {
            hr = pGeometry1->CombineWithGeometry(
                pGeometry2,
                combineMode,
                NULL, // world transform
                pSink
                );

            if (SUCCEEDED(hr))
            {
                hr = pSink->Close();

                if (SUCCEEDED(hr))
                {
                    *ppGeometry = pPathGeometry;
                    (*ppGeometry)->AddRef();
                }
            }
            pSink->Release();
        }
        pPathGeometry->Release();
    }

    pFactory->Release();

    return hr;
}

/******************************************************************
*                                                                 *
*  Extruder                                                       *
*                                                                 *
*  Class for extruding an ID2D1Geometry into 3-D.                 *
*                                                                 *
******************************************************************/
class Extruder
{
public:
    static HRESULT ExtrudeGeometry(
        ID2D1Geometry *pGeometry,
        float height,
        CArray<SimpleVertex> &vertices
        )
    {
        HRESULT hr;

        //
        // The basic idea here is to generate the side faces by walking the
        // geometry and constructing quads, and use ID2D1Geometry::Tessellate
        // to generate the front and back faces.
        //
        // There are two things to be careful of here:
        //
        // 1) We must not produce overlapping triangles, as this can cause
        //    "depth-buffer fighting".
        // 2) The vertices on the front and back faces must perfectly align with
        //    the vertices on the side faces.
        //
        // Thankfully, D2D correctly handles self-intersections, which makes
        // solving issue 1 easy.
        //
        // Issue 2 is more complicated, since the D2D tessellation algorithm
        // will jitter vertices slightly. To get around this, we snap vertices
        // to grid-points. To ensure that the tessellation jittering does cause
        // a vertex to be snapped to a neighboring grid-point, we actually snap
        // the  vertices twice: once prior to tessellating and once after
        // tessellating. As long as our grid points are spaced further than
        // twice max-jitter distance, we can be sure the vertices will snap
        // to the right spot.
        //

        ID2D1Geometry *pFlattenedGeometry = NULL;

        //
        // Flatten our geometry first so we don't have to worry about stitching
        // together seams of Beziers.
        //

        hr = D2DFlatten(pGeometry, sc_flatteningTolerance, &pFlattenedGeometry);

        if (SUCCEEDED(hr))
        {
            ID2D1Geometry *pOutlinedGeometry = NULL;

            //
            // D2DOutline will remove any self-intersections. This is important to
            // ensure that the tessellator doesn't introduce new vertices (which
            // can cause T-junctions).
            //

            hr = D2DOutline(pFlattenedGeometry, &pOutlinedGeometry);

            if (SUCCEEDED(hr))
            {
                ID2D1Geometry *pSnappedGeometry = NULL;
                hr = PointSnapper::SnapGeometry(pOutlinedGeometry, &pSnappedGeometry);

                if (SUCCEEDED(hr))
                {
                    NoRefComObject<ExtrudingSink> helper(height, &vertices);

                    hr = pSnappedGeometry->Tessellate(NULL, &helper);

                    if (SUCCEEDED(hr))
                    {
                        // Simplify is a convenient API for extracting the data out of a geometry.
                        hr = pOutlinedGeometry->Simplify(
                            D2D1_GEOMETRY_SIMPLIFICATION_OPTION_LINES,
                            NULL, // world transform
                            &helper
                            );

                        if (SUCCEEDED(hr))
                        {
                            //
                            // This Close() call is a little ambiguous, since it refers both to the
                            // ID2D1TessellationSink and to the ID2D1SimplifiedGeometrySink.
                            // Thankfully, it really doesn't matter with our ExtrudingSink.
                            //

                            hr = helper.Close();
                        }
                    }
                    pSnappedGeometry->Release();
                }
                pOutlinedGeometry->Release();
            }

            pFlattenedGeometry->Release();
        }

        return hr;
    }

private:
    /******************************************************************
    *                                                                 *
    *  ExtrudingSink                                                  *
    *                                                                 *
    *  Internal sink used to implement Extruder.                      *
    *                                                                 *
    *  Note: This class makes certain assumptions about it's usage    *
    *  (e.g. no Beziers), which is why it's a private class.          *
    *                                                                 *
    *  Note 2: Both ID2D1SimplifiedGeometrySink and                   *
    *  ID2D1TessellationSink define a Close() method, which is        *
    *  bending the rules a bit. This is another reason why we are     *
    *  significantly limiting its usage.                              *
    *                                                                 *
    *                                                                 *
    ******************************************************************/
    class ExtrudingSink : public ID2D1SimplifiedGeometrySink, public ID2D1TessellationSink
    {
    public:
        ExtrudingSink(float height, CArray<SimpleVertex> *pVertices)
            : m_height(height), m_vertices(*pVertices), m_hr(S_OK)
        {
        }

        STDMETHOD_(void, AddBeziers)(const D2D1_BEZIER_SEGMENT * /*beziers*/, UINT /*beziersCount*/)
        {
            //
            // ExtrudingSink only handles line segments. Users should flatten
            // their geometry prior to passing through an ExtrudingSink.
            //
        }

        STDMETHOD_(void, AddLines)(const D2D1_POINT_2F *points, UINT pointsCount)
        {
            if (SUCCEEDED(m_hr))
            {
                for (UINT i = 0; SUCCEEDED(m_hr) && i < pointsCount; ++i)
                {
                    Vertex2D v;

                    v.pt = D3DXVECTOR2(points[i].x, points[i].y);

                    //
                    // Take care to ignore degenerate segments, as we will be
                    // unable to compute proper normals for them.
                    //
                    // Note: This doesn't handle near-degenerate segments, which
                    // should probably also be removed. The one complication here
                    // is that the segments should be removed from both the outline
                    // and the front/back tessellations.
                    //
                    if ((m_figureVertices.GetCount() == 0) ||
                        (v.pt.x != m_figureVertices.GetBack().pt.x) ||
                        (v.pt.y != m_figureVertices.GetBack().pt.y)
                        )
                    {
                        m_hr = m_figureVertices.Add(v);
                    }
                }
            }
        }

        STDMETHOD_(void, BeginFigure)(D2D1_POINT_2F startPoint, D2D1_FIGURE_BEGIN /*figureBegin*/)
        {
            if (SUCCEEDED(m_hr))
            {
                m_figureVertices.RemoveAll();

                Vertex2D v = {
                    D3DXVECTOR2(startPoint.x, startPoint.y),
                    D3DXVECTOR2(0,0), // dummy
                    D3DXVECTOR2(0,0), // dummy
                    D3DXVECTOR2(0,0) // dummy
                };

                m_hr = m_figureVertices.Add(v);
            }
        }

        STDMETHOD(Close)()
        {
            return m_hr;
        }

        STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END /*figureEnd*/)
        {
            if (SUCCEEDED(m_hr))
            {
                D3DXVECTOR2 front =  m_figureVertices.GetFront().pt;
                D3DXVECTOR2 back =  m_figureVertices.GetBack().pt;

                if (front.x == back.x && front.y == back.y)
                {
                    m_figureVertices.RemoveLast();
                }

                // If we only have one vertex, then there is nothing to draw!
                if (m_figureVertices.GetCount() > 1)
                {
                    //
                    // We construct the triangles corresponding to the sides of
                    // the extruded object in 3 steps:
                    //

                    //
                    // Step 1:
                    //
                    // Snap vertices and calculate normals.
                    //
                    //
                    // Note: it is important that we compute normals *before*
                    // snapping the vertices, otherwise, the normals will become
                    // discretized, which will manifest itself as faceting.
                    //
                    for (UINT i = 0; i < m_figureVertices.GetCount(); ++i)
                    {
                        m_figureVertices[i].norm = GetNormal(i);
                        m_figureVertices[i].pt = PointSnapper::SnapPoint(m_figureVertices[i].pt);
                    }

                    //
                    // Step 2:
                    //
                    // Interpolate normals as appropriate.
                    //
                    for (UINT i = 0; i < m_figureVertices.GetCount(); ++i)
                    {
                        UINT h = (i+m_figureVertices.GetCount()-1)%m_figureVertices.GetCount();

                        D3DXVECTOR2 n1 =  m_figureVertices[h].norm;
                        D3DXVECTOR2 n2 =  m_figureVertices[i].norm;

                        //
                        // Take a dot-product to determine if the angle between
                        // the normals is small. If it is, then average them so we
                        // get a smooth transition from one face to the next.
                        //
                        if ( (n1.x * n2.x + n1.y * n2.y) > .5f )
                        {
                            D3DXVECTOR2 sum = m_figureVertices[i].norm + m_figureVertices[h].norm;

                            m_figureVertices[i].interpNorm1 = m_figureVertices[i].interpNorm2 =
                                Normalize(sum);
                        }
                        else
                        {
                            m_figureVertices[i].interpNorm1 = m_figureVertices[h].norm;
                            m_figureVertices[i].interpNorm2 = m_figureVertices[i].norm;
                        }
                    }

                    //
                    // Step 3:
                    //
                    // Output the triangles.
                    //

                    // interpNorm1 == end normal of previous segment
                    // interpNorm2 == begin normal of next segment
                    for (UINT i = 0; i < m_figureVertices.GetCount(); ++i)
                    {
                        UINT j = (i+1) % m_figureVertices.GetCount();

                        D3DXVECTOR2 pt =  m_figureVertices[i].pt;
                        D3DXVECTOR2 nextPt = m_figureVertices[j].pt;

                        D3DXVECTOR2 ptNorm3 =  m_figureVertices[i].interpNorm2;
                        D3DXVECTOR2 nextPtNorm2 =  m_figureVertices[j].interpNorm1;

                        //
                        // These 6 vertices define two adjacent triangles that
                        // together form a quad.
                        //
                        SimpleVertex newVertices[6] =
                        {
                            { D3DXVECTOR3( pt.x, pt.y, m_height/2 ), D3DXVECTOR3( ptNorm3.x, ptNorm3.y, 0.0f)},
                            { D3DXVECTOR3( pt.x, pt.y, -m_height/2 ), D3DXVECTOR3( ptNorm3.x, ptNorm3.y, 0.0f)},
                            { D3DXVECTOR3( nextPt.x, nextPt.y, -m_height/2 ), D3DXVECTOR3( nextPtNorm2.x, nextPtNorm2.y, 0.0f)},
                            { D3DXVECTOR3( nextPt.x, nextPt.y, -m_height/2 ), D3DXVECTOR3( nextPtNorm2.x, nextPtNorm2.y, 0.0f)},
                            { D3DXVECTOR3( nextPt.x, nextPt.y, m_height/2 ), D3DXVECTOR3( nextPtNorm2.x, nextPtNorm2.y, 0.0f)},
                            { D3DXVECTOR3( pt.x, pt.y, m_height/2 ), D3DXVECTOR3( ptNorm3.x, ptNorm3.y, 0.0f)},
                        };

                        for (UINT n = 0; SUCCEEDED(m_hr) && n < ARRAYSIZE(newVertices); ++n)
                        {
                            m_hr = m_vertices.Add(newVertices[n]);
                        }
                    }
                }
            }
        }

        STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE /*fillMode*/)
        {
            // Do nothing
        }
        STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT /*vertexFlags*/)
        {
            // Do nothing
        }
        STDMETHOD_(void, AddTriangles)(const D2D1_TRIANGLE *triangles, UINT trianglesCount)
        {
            if (SUCCEEDED(m_hr))
            {
                //
                // These triangles reprent the front and back faces of the extrusion.
                //
                for (UINT i = 0; i < trianglesCount; ++i)
                {
                    D2D1_TRIANGLE tri = triangles[i];

                    D2D1_POINT_2F d1 = {tri.point2.x - tri.point1.x, tri.point2.y - tri.point1.y};
                    D2D1_POINT_2F d2 = {tri.point3.x - tri.point2.x, tri.point3.y - tri.point2.y};

                    tri.point1 = PointSnapper::SnapPoint(tri.point1);
                    tri.point2 = PointSnapper::SnapPoint(tri.point2);
                    tri.point3 = PointSnapper::SnapPoint(tri.point3);

                    //
                    // Currently, Tessellate does not guarantee the orientation
                    // of the triangles it produces, so we must check here.
                    //

                    float cross = d1.x * d2.y - d1.y*d2.x;
                    if (cross < 0)
                    {
                        D2D1_POINT_2F tmp = tri.point1;

                        tri.point1 = tri.point2;
                        tri.point2 = tmp;
                    }

                    SimpleVertex newVertices[] =
                    {
                        { D3DXVECTOR3( tri.point1.x, tri.point1.y, m_height/2 ), D3DXVECTOR3( 0.0f, 0.0f, 1.0f )},
                        { D3DXVECTOR3( tri.point2.x, tri.point2.y, m_height/2 ), D3DXVECTOR3( 0.0f, 0.0f, 1.0f )},
                        { D3DXVECTOR3( tri.point3.x, tri.point3.y, m_height/2 ), D3DXVECTOR3( 0.0f, 0.0f, 1.0f )},

                        //
                        // Note: these points are listed in a different order since the orientation of the back
                        // face should be the opposite of the front face.
                        //
                        { D3DXVECTOR3( tri.point2.x, tri.point2.y, -m_height/2 ), D3DXVECTOR3( 0.0f, 0.0f, -1.0f )},
                        { D3DXVECTOR3( tri.point1.x, tri.point1.y, -m_height/2 ), D3DXVECTOR3( 0.0f, 0.0f, -1.0f )},
                        { D3DXVECTOR3( tri.point3.x, tri.point3.y, -m_height/2 ), D3DXVECTOR3( 0.0f, 0.0f, -1.0f )},
                    };

                    for (UINT i = 0; SUCCEEDED(m_hr) && i < ARRAYSIZE(newVertices); ++i)
                    {
                        m_hr = m_vertices.Add(newVertices[i]);
                    }
                }
            }
        }

    private:
        D3DXVECTOR2 GetNormal(UINT i)
        {
            UINT j = (i+1) % m_figureVertices.GetCount();

            D3DXVECTOR2 pti = m_figureVertices[i].pt;
            D3DXVECTOR2 ptj = m_figureVertices[j].pt;
            D3DXVECTOR2 vecij = ptj - pti;

            return Normalize(D3DXVECTOR2(vecij.y, vecij.x));
        }

        D3DXVECTOR2 Normalize(D3DXVECTOR2 pt)
        {
            return pt / sqrtf(pt.x*pt.x+pt.y*pt.y);
        }

        struct Vertex2D
        {
            D3DXVECTOR2 pt;
            D3DXVECTOR2 norm;
            D3DXVECTOR2 interpNorm1;
            D3DXVECTOR2 interpNorm2;
        };

        HRESULT m_hr;

        float m_height;
        D2D1_POINT_2F m_lastPoint;
        D2D1_POINT_2F m_startPoint;

        CArray<SimpleVertex> &m_vertices;

        CArray<Vertex2D> m_figureVertices;
    };
};


/******************************************************************
*                                                                 *
*  OutlineRenderer                                                *
*                                                                 *
*  A "TextRenderer" that extracts the glyph runs of a text        *
*  layout object and turns them into a geometry.                  *
*                                                                 *
******************************************************************/
class OutlineRenderer : public IDWriteTextRenderer
{
public:
    OutlineRenderer(ID2D1Factory *pFactory)
        : m_pFactory(pFactory)
    {
        m_pFactory->AddRef();
        m_pGeometry = NULL;
    }

    ~OutlineRenderer()
    {
        SafeRelease(&m_pFactory);
        SafeRelease(&m_pGeometry);
    }

    STDMETHOD(DrawGlyphRun)(
        void* /*clientDrawingContext*/,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE /*measuringMode*/,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* /*glyphRunDescription*/,
        IUnknown* /*clientDrawingEffect*/
        )
    {
        HRESULT hr;

        ID2D1PathGeometry *pPathGeometry = NULL;

        hr = m_pFactory->CreatePathGeometry(&pPathGeometry);

        if (SUCCEEDED(hr))
        {
            ID2D1GeometrySink *pSink = NULL;

            hr = pPathGeometry->Open(&pSink);

            if (SUCCEEDED(hr))
            {
                hr = glyphRun->fontFace->GetGlyphRunOutline(
                        glyphRun->fontEmSize,
                        glyphRun->glyphIndices,
                        glyphRun->glyphAdvances,
                        glyphRun->glyphOffsets,
                        glyphRun->glyphCount,
                        glyphRun->isSideways,
                        (glyphRun->bidiLevel % 2) == 1,
                        pSink
                        );

                if (SUCCEEDED(hr))
                {
                    hr = pSink->Close();

                    if (SUCCEEDED(hr))
                    {
                        ID2D1TransformedGeometry *pTransformedGeometry = NULL;
                        hr = m_pFactory->CreateTransformedGeometry(
                            pPathGeometry,
                            D2D1::Matrix3x2F::Translation(baselineOriginX, baselineOriginY),
                            &pTransformedGeometry
                            );

                        if (SUCCEEDED(hr))
                        {
                            hr = AddGeometry(pTransformedGeometry);

                            pTransformedGeometry->Release();
                        }
                    }
                }
                pSink->Release();
            }
            pPathGeometry->Release();
        }

        return hr;
    }

    STDMETHOD(DrawUnderline)(
        void* /*clientDrawingContext*/,
        FLOAT /*baselineOriginX*/,
        FLOAT /*baselineOriginY*/,
        DWRITE_UNDERLINE const* /*underline*/,
        IUnknown* /*clientDrawingEffect*/
        )
    {
        // Implement this to add support for underlines. See the
        // CustomTextRender in the DWrite PDC hands-on lab for
        // an example on how to do this.
        return E_NOTIMPL;
    }

    STDMETHOD(DrawStrikethrough)(
        void* /*clientDrawingContext*/,
        FLOAT /*baselineOriginX*/,
        FLOAT /*baselineOriginY*/,
        DWRITE_STRIKETHROUGH const* /*strikethrough*/,
        IUnknown* /*clientDrawingEffect*/
        )
    {
        // Implement this to add support for strikethroughs. See the
        // CustomTextRender in the DWrite PDC hands-on lab for
        // an example on how to do this.
        return E_NOTIMPL;
    }

    STDMETHOD(DrawInlineObject)(
        void* /*clientDrawingContext*/,
        FLOAT /*originX*/,
        FLOAT /*originY*/,
        IDWriteInlineObject* /*inlineObject*/,
        BOOL /*isSideways*/,
        BOOL /*isRightToLeft*/,
        IUnknown* /*clientDrawingEffect*/
        )
    {
        // Implement this to add support for inline objects.
        // See the CustomTextRender in the DWrite PDC hands-on lab for an
        // example on how to do this.
        return E_NOTIMPL;
    }

    STDMETHOD(IsPixelSnappingDisabled)(
        void* /*clientDrawingContext*/,
        BOOL* isDisabled
        )
    {
        *isDisabled = TRUE;
        return S_OK;
    }

    STDMETHOD(GetCurrentTransform)(
        void* /*clientDrawingContext*/,
        DWRITE_MATRIX* transform
        )
    {
        DWRITE_MATRIX matrix = {1, 0, 0, 1, 0, 0};

        *transform = matrix;

        return S_OK;
    }

    STDMETHOD(GetPixelsPerDip)(
        void* /*clientDrawingContext*/,
        FLOAT* pixelsPerDip
        )
    {
        *pixelsPerDip = 1.0f;

        return S_OK;
    }

    HRESULT GetGeometry(
        ID2D1Geometry **ppGeometry
        )
    {
        HRESULT hr = S_OK;

        if (m_pGeometry)
        {
            *ppGeometry = m_pGeometry;
            (*ppGeometry)->AddRef();
        }
        else
        {
            ID2D1PathGeometry *pGeometry = NULL;

            hr = m_pFactory->CreatePathGeometry(&pGeometry);

            if (SUCCEEDED(hr))
            {
                ID2D1GeometrySink *pSink = NULL;
                hr = pGeometry->Open(&pSink);

                if (SUCCEEDED(hr))
                {
                    hr = pSink->Close();

                    if (SUCCEEDED(hr))
                    {
                        *ppGeometry = pGeometry;
                        (*ppGeometry)->AddRef();
                    }

                    pSink->Release();
                }
                pGeometry->Release();
            }
        }

        return hr;
    }

protected:
    HRESULT AddGeometry(
        ID2D1Geometry *pGeometry
        )
    {
        HRESULT hr = S_OK;

        if (m_pGeometry)
        {
            ID2D1Geometry *pCombinedGeometry = NULL;

            hr = D2DCombine(
                D2D1_COMBINE_MODE_UNION,
                m_pGeometry,
                pGeometry,
                &pCombinedGeometry
                );
            if (SUCCEEDED(hr))
            {
                SafeReplace(&m_pGeometry, pCombinedGeometry);
                pCombinedGeometry->Release();
            }
        }
        else
        {
            SafeReplace(&m_pGeometry, pGeometry);
        }

        return hr;
    }

private:
    ID2D1Factory *m_pFactory;
    ID2D1Geometry *m_pGeometry;
};



/******************************************************************
*                                                                 *
*  Static Data                                                    *
*                                                                 *
******************************************************************/
/*static*/ const D3D10_INPUT_ELEMENT_DESC Interactive3dTextSampleApp::s_InputLayout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0},
};


// This must be a multiple of 3
/* static*/ const UINT Interactive3dTextSampleApp::sc_vertexBufferCount = 3*1000;


/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::Interactive3dTextSampleApp         *
*                                                                 *
*  Constructor -- initialize member data                          *
*                                                                 *
******************************************************************/

Interactive3dTextSampleApp::Interactive3dTextSampleApp() :
    m_hwnd(NULL),
    m_pD2DFactory(NULL),
    m_pDWriteFactory(NULL),
    m_pDevice(NULL),
    m_pSwapChain(NULL),
    m_pState(NULL),
    m_pDepthStencil(NULL),
    m_pDepthStencilView(NULL),
    m_pRenderTargetView(NULL),
    m_pShader(NULL),
    m_pVertexBuffer(NULL),
    m_pVertexLayout(NULL),
    m_pTechniqueNoRef(NULL),
    m_pWorldVariableNoRef(NULL),
    m_pViewVariableNoRef(NULL),
    m_pProjectionVariableNoRef(NULL),
    m_pLightPosVariableNoRef(NULL),
    m_pLightColorVariableNoRef(NULL),
    m_pTextGeometry(NULL),
    m_pTextLayout(NULL)
{
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::~Interactive3dTextSampleApp        *
*                                                                 *
*  Destructor -- tear down member data                            *
*                                                                 *
******************************************************************/

Interactive3dTextSampleApp::~Interactive3dTextSampleApp()
{
    SafeRelease(&m_pD2DFactory);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pDevice);
    SafeRelease(&m_pSwapChain);
    SafeRelease(&m_pState);
    SafeRelease(&m_pDepthStencil);
    SafeRelease(&m_pDepthStencilView);
    SafeRelease(&m_pRenderTargetView);
    SafeRelease(&m_pShader);
    SafeRelease(&m_pVertexBuffer);
    SafeRelease(&m_pVertexLayout);
    SafeRelease(&m_pTextGeometry);
    SafeRelease(&m_pTextLayout);
}

/******************************************************************
*                                                                 *
*  DemoApp::Initialize                                            *
*                                                                 *
*  Create application window and device-independent resources     *
*                                                                 *
******************************************************************/
HRESULT Interactive3dTextSampleApp::Initialize()
{
    HRESULT hr;

    hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
    {
        // Register window class
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = Interactive3dTextSampleApp::WndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = HINST_THISCOMPONENT;
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName  = NULL;
        wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wcex.lpszClassName = L"D2DDemoApp";

        RegisterClassEx(&wcex);

        // Create the application window.
        //
        // This sample does not handle resize so we create the window such that
        // it can't be resized.
        // 
        // Because the CreateWindow function takes its size in pixels, we
        // obtain the system DPI and use it to scale the window size.
        FLOAT dpiX;
        FLOAT dpiY;
        m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

        m_hwnd = CreateWindow(
            L"D2DDemoApp",
            L"Direct2D Demo App",
            WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX),
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(1024.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
            );

        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            // Create a timer and receive WM_TIMER messages at a rough
            // granularity of 33msecs. If you need a more precise timer,
            // consider modifying the message loop and calling more precise
            // timing functions.
            SetTimer(
                m_hwnd,
                0, //timerId
                33, //msecs
                NULL //lpTimerProc
                );

            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            UpdateWindow(m_hwnd);
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::CreateDeviceIndependentResources   *
*                                                                 *
*  This method is used to create resources which are not bound    *
*  to any device. Their lifetime effectively extends for the      *
*  duration of the app. These resources include the D2D,          *
*  DWrite, and WIC factories; and a DWrite Text Format object     *
*  (used for identifying particular font characteristics) and     *
*  a D2D geometry.                                                *
*                                                                 *
******************************************************************/

HRESULT Interactive3dTextSampleApp::CreateDeviceIndependentResources()
{
    HRESULT hr;

    // Create D2D factory
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

    if (SUCCEEDED(hr))
    {
        // Create DWrite factory
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }

    //
    // Add command-prompt-ish like text to the start of the string.
    //
    if (SUCCEEDED(hr))
    {
        hr = m_characters.Add('>');
    }
    if (SUCCEEDED(hr))
    {
        hr = m_characters.Add(' ');
    }

    if (SUCCEEDED(hr))
    {
        hr = UpdateTextGeometry();
    }

    return hr;
}



/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::UpdateTextGeometry                 *
*                                                                 *
*  Generates a layout object and a geometric outline              *
*  corresponding to the current text string and stores the        *
*  results in m_pTextLayout and m_pTextGeometry.                  *
*                                                                 *
******************************************************************/
HRESULT Interactive3dTextSampleApp::UpdateTextGeometry()
{
    HRESULT hr;

    IDWriteTextFormat *pFormat = NULL;

    hr = m_pDWriteFactory->CreateTextFormat(
            sc_fontFace,
            NULL,
            DWRITE_FONT_WEIGHT_EXTRA_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            sc_fontSize,
            L"", // locale name
            &pFormat
            );
    if (SUCCEEDED(hr))
    {
        IDWriteTextLayout *pLayout = NULL;
        hr = m_pDWriteFactory->CreateTextLayout(
                &m_characters[0],
                m_characters.GetCount(),
                pFormat,
                0.0f, // lineWidth (ignored because of NO_WRAP)
                sc_fontSize, // lineHeight
                &pLayout
                );
        if (SUCCEEDED(hr))
        {
            hr = pLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

            if (SUCCEEDED(hr))
            {
                IDWriteTypography *pTypography = NULL;
                hr = m_pDWriteFactory->CreateTypography(&pTypography);
                if (SUCCEEDED(hr))
                {
                    DWRITE_FONT_FEATURE fontFeature =
                    {
                        DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_7,
                        1
                    };
                    hr = pTypography->AddFontFeature(fontFeature);
                    if (SUCCEEDED(hr))
                    {
                        DWRITE_TEXT_RANGE textRange = {0, m_characters.GetCount()};
                        hr = pLayout->SetTypography(pTypography, textRange);
                    }

                    pTypography->Release();
                }

                NoRefComObject<OutlineRenderer> renderer(m_pD2DFactory);

                hr = pLayout->Draw(
                    NULL, // clientDrawingContext
                    &renderer,
                    0.0f, // originX
                    0.0f // originY
                    );
                if (SUCCEEDED(hr))
                {
                    ID2D1Geometry *pGeometry = NULL;;
                    hr = renderer.GetGeometry(&pGeometry);

                    if (SUCCEEDED(hr))
                    {
                        SafeReplace(&m_pTextGeometry, pGeometry);
                        SafeReplace(&m_pTextLayout, pLayout);
                        pGeometry->Release();
                    }
                }
            }
            pLayout->Release();
        }
        pFormat->Release();
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::GenerateTextOutline                *
*                                                                 *
*  Returns a version of the text geometry that is horizontally    *
*  centered and vertically positioned so (0,0) is on the          *
*  base line.                                                     *
*                                                                 *
******************************************************************/
HRESULT Interactive3dTextSampleApp::GenerateTextOutline(
    bool includeCursor,
    ID2D1Geometry **ppGeometry
    )
{
    HRESULT hr;

    DWRITE_LINE_METRICS lineMetrics;
    DWRITE_TEXT_METRICS textMetrics;

    UINT actualNumLines;

    //
    // We're assuming here that the text doesn't wrap and doesn't contain
    // newlines.
    //
    hr = m_pTextLayout->GetLineMetrics(
            &lineMetrics,
            1, // maxLineCount
            &actualNumLines // ignored
            );
    if (SUCCEEDED(hr))
    {
        hr = m_pTextLayout->GetMetrics(&textMetrics);
        if (SUCCEEDED(hr))
        {
            float offsetY = -lineMetrics.baseline;
            float offsetX = -textMetrics.widthIncludingTrailingWhitespace / 2;
            ID2D1TransformedGeometry *pTransformedGeometry = NULL;

            hr = m_pD2DFactory->CreateTransformedGeometry(
                m_pTextGeometry,
                D2D1::Matrix3x2F::Translation(offsetX, offsetY),
                &pTransformedGeometry
                );
            if (SUCCEEDED(hr))
            {
                ID2D1Geometry *pGeometry = NULL;
                pGeometry = pTransformedGeometry;
                pGeometry->AddRef();

                if (includeCursor)
                {
                    float x, y;
                    DWRITE_HIT_TEST_METRICS hitTestMetrics;

                    hr = m_pTextLayout->HitTestTextPosition(
                        lineMetrics.length,
                        TRUE, // isTrailingHit
                        &x,
                        &y,
                        &hitTestMetrics
                        );
                    if (SUCCEEDED(hr))
                    {
                        float width = sc_cursorWidth;
                        float left = x+offsetX;
                        ID2D1RectangleGeometry *pCursorGeometry = NULL;

                        hr = m_pD2DFactory->CreateRectangleGeometry(
                            D2D1::RectF(
                                left,
                                hitTestMetrics.top + offsetY,
                                left + width,
                                0.0f),
                            &pCursorGeometry
                            );
                        if (SUCCEEDED(hr))
                        {
                            ID2D1Geometry *pCombinedGeometry = NULL;
                            hr = D2DCombine(
                                    D2D1_COMBINE_MODE_UNION,
                                    pGeometry,
                                    pCursorGeometry,
                                    &pCombinedGeometry
                                    );
                            if (SUCCEEDED(hr))
                            {
                                SafeReplace(&pGeometry, pCombinedGeometry);
                                pCombinedGeometry->Release();
                            }
                            pCursorGeometry->Release();
                        }
                    }
                }

                // transfer reference
                *ppGeometry = pGeometry;
                pGeometry = NULL;
            }
            pTransformedGeometry->Release();
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::CreateDeviceResources              *
*                                                                 *
*  This method is responsible for creating the D3D device and     *
*  all corresponding device-bound resources that are required to  *
*  render.                                                        *
*                                                                 *
*  Of the objects created in this method, 2 are interesting ...   *
*                                                                 *
*     1. D3D Device (m_pDevice)                                   *
*                                                                 *
*     2. DXGI Swap Chain (m_pSwapChain) -- Mapped to current      *
*           window (m_hwnd)                                       *
*                                                                 *
******************************************************************/

HRESULT Interactive3dTextSampleApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;
    RECT rcClient;
    UINT msaaQuality = 0;
    UINT sampleCount = 0;
    ID3D10Resource *pBackBufferResource = NULL;
    ID3D10Device1 *pDevice = NULL;
    IDXGIDevice *pDXGIDevice = NULL;
    IDXGIAdapter *pAdapter = NULL;
    IDXGIFactory *pDXGIFactory = NULL;
    IDXGISurface *pSurface = NULL;
    IDXGISurface *pBackBuffer = NULL;
    Assert(m_hwnd);

    GetClientRect(m_hwnd, &rcClient);

    UINT nWidth = abs(rcClient.right - rcClient.left);
    UINT nHeight = abs(rcClient.bottom - rcClient.top);

    // If we don't have a device, need to create one now and all
    // accompanying D3D resources
    if (!m_pDevice)
    {
        UINT nDeviceFlags = 0;

        // Create device
        hr = CreateD3DDevice(
            NULL, // adapter
            D3D10_DRIVER_TYPE_HARDWARE,
            nDeviceFlags,
            &pDevice
            );

        if (FAILED(hr))
        {
            hr = CreateD3DDevice(
                NULL,
                D3D10_DRIVER_TYPE_WARP,
                nDeviceFlags,
                &pDevice
                );
        }

        if (SUCCEEDED(hr))
        {
            hr = pDevice->QueryInterface(&m_pDevice);
        }
        if (SUCCEEDED(hr))
        {
            hr = pDevice->QueryInterface(&pDXGIDevice);
        }
        if (SUCCEEDED(hr))
        {
            hr = pDXGIDevice->GetAdapter(&pAdapter);
        }
        if (SUCCEEDED(hr))
        {
            hr = pAdapter->GetParent(IID_PPV_ARGS(&pDXGIFactory));
        }

        if (SUCCEEDED(hr))
        {
            msaaQuality = 0;
            for (sampleCount = sc_maxMsaaSampleCount; SUCCEEDED(hr) && sampleCount > 0; --sampleCount)
            {
                hr = m_pDevice->CheckMultisampleQualityLevels(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    sampleCount,
                    &msaaQuality
                    );

                if (SUCCEEDED(hr))
                {
                    if (msaaQuality > 0)
                    {
                        break;
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            DXGI_SWAP_CHAIN_DESC swapDesc;
            ::ZeroMemory(&swapDesc, sizeof(swapDesc));
            swapDesc.BufferDesc.Width = nWidth;
            swapDesc.BufferDesc.Height = nHeight;
            swapDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            swapDesc.SampleDesc.Count = sampleCount;
            swapDesc.SampleDesc.Quality = msaaQuality-1;
            swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapDesc.BufferCount = 1;
            swapDesc.OutputWindow = m_hwnd;
            swapDesc.Windowed = TRUE;

            hr = pDXGIFactory->CreateSwapChain(m_pDevice, &swapDesc, &m_pSwapChain);
        }

        if (SUCCEEDED(hr))
        {
            // Create rasterizer state object
            D3D10_RASTERIZER_DESC rsDesc;
            rsDesc.AntialiasedLineEnable = FALSE;
            rsDesc.CullMode = D3D10_CULL_BACK;
            rsDesc.DepthBias = 0;
            rsDesc.DepthBiasClamp = 0;
            rsDesc.DepthClipEnable = TRUE;
            rsDesc.FillMode = D3D10_FILL_SOLID;
            rsDesc.FrontCounterClockwise = FALSE; // Must be FALSE for 10on9
            rsDesc.MultisampleEnable = TRUE;
            rsDesc.ScissorEnable = FALSE;
            rsDesc.SlopeScaledDepthBias = 0;

            hr = m_pDevice->CreateRasterizerState(&rsDesc, &m_pState);
        }

        if (SUCCEEDED(hr))
        {
            m_pDevice->RSSetState(m_pState);

            msaaQuality = 0;
            for (sampleCount = sc_maxMsaaSampleCount; SUCCEEDED(hr) && sampleCount > 0; --sampleCount)
            {
                hr = m_pDevice->CheckMultisampleQualityLevels(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    sampleCount,
                    &msaaQuality
                    );

                if (SUCCEEDED(hr))
                {
                    if (msaaQuality > 0)
                    {
                        break;
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            // Create views on the RT buffers and set them on the device
            hr = m_pSwapChain->GetBuffer(
                0,
                IID_PPV_ARGS(&pBackBufferResource)
                );
        }

        if (SUCCEEDED(hr))
        {
            D3D10_RENDER_TARGET_VIEW_DESC renderDesc;
            renderDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            renderDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DMS;
            renderDesc.Texture2D.MipSlice = 0;

            hr = m_pDevice->CreateRenderTargetView(pBackBufferResource, &renderDesc, &m_pRenderTargetView);
        }

        if (SUCCEEDED(hr))
        {
            D3D10_TEXTURE2D_DESC texDesc;
            texDesc.ArraySize = 1;
            texDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
            texDesc.CPUAccessFlags = 0;
            texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            texDesc.Height = nHeight;
            texDesc.Width = nWidth;
            texDesc.MipLevels = 1;
            texDesc.MiscFlags = 0;
            texDesc.SampleDesc.Count = sampleCount;
            texDesc.SampleDesc.Quality = msaaQuality-1;
            texDesc.Usage = D3D10_USAGE_DEFAULT;

            hr = m_pDevice->CreateTexture2D(&texDesc, NULL, &m_pDepthStencil);

            if (SUCCEEDED(hr))
            {
                D3D10_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
                depthViewDesc.Format = texDesc.Format;
                depthViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DMS;
                depthViewDesc.Texture2D.MipSlice = 0;
                hr = m_pDevice->CreateDepthStencilView(m_pDepthStencil, &depthViewDesc, &m_pDepthStencilView);
            }
        }

        if (SUCCEEDED(hr))
        {
            ID3D10RenderTargetView *viewList[1] = {m_pRenderTargetView};
            m_pDevice->OMSetRenderTargets(1, viewList, m_pDepthStencilView);

            // Set a new viewport based on the new dimensions
            D3D10_VIEWPORT viewport;
            viewport.Width = nWidth;
            viewport.Height = nHeight;
            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            viewport.MinDepth = 0;
            viewport.MaxDepth = 1;
            m_pDevice->RSSetViewports(1, &viewport);

            // Get a surface in the swap chain
            hr = m_pSwapChain->GetBuffer(
                0,
                IID_PPV_ARGS(&pBackBuffer)
                );
        }

        if (SUCCEEDED(hr))
        {
            // Load pixel shader
            hr = LoadResourceShader(
                m_pDevice,
                MAKEINTRESOURCE(IDR_PIXEL_SHADER),
                &m_pShader
                );
        }

        if (SUCCEEDED(hr))
        {
            // Obtain the technique
            m_pTechniqueNoRef = m_pShader->GetTechniqueByName("Render");
            hr = m_pTechniqueNoRef ? S_OK : E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            // Obtain the variables
            m_pWorldVariableNoRef = m_pShader->GetVariableByName("World")->AsMatrix();
            hr = m_pWorldVariableNoRef ? S_OK : E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            m_pViewVariableNoRef = m_pShader->GetVariableByName("View")->AsMatrix();
            hr = m_pViewVariableNoRef ? S_OK : E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            m_pProjectionVariableNoRef = m_pShader->GetVariableByName("Projection")->AsMatrix();
            hr = m_pProjectionVariableNoRef ? S_OK : E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            m_pLightPosVariableNoRef = m_pShader->GetVariableByName("vLightPos")->AsVector();
            hr = m_pLightPosVariableNoRef ? S_OK : E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            m_pLightColorVariableNoRef = m_pShader->GetVariableByName("vLightColor")->AsVector();
            hr = m_pLightColorVariableNoRef ? S_OK : E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            // Define the input layout
            UINT numElements = ARRAYSIZE(s_InputLayout);

            // Create the input layout
            D3D10_PASS_DESC PassDesc;
            m_pTechniqueNoRef->GetPassByIndex(0)->GetDesc(&PassDesc);

            hr = m_pDevice->CreateInputLayout(
                s_InputLayout,
                numElements,
                PassDesc.pIAInputSignature,
                PassDesc.IAInputSignatureSize,
                &m_pVertexLayout
                );
        }

        if (SUCCEEDED(hr))
        {

            // Set the input layout.
            m_pDevice->IASetInputLayout(m_pVertexLayout);

            D3D10_BUFFER_DESC bd;
            bd.Usage = D3D10_USAGE_DYNAMIC;
            bd.ByteWidth = sc_vertexBufferCount * sizeof(SimpleVertex);
            bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
            bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
            bd.MiscFlags = 0;
            hr = m_pDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer);
        }

        if (SUCCEEDED(hr))
        {
            // Set the vertex buffer.
            UINT stride = sizeof(SimpleVertex);
            UINT offset = 0;
            ID3D10Buffer *pVertexBuffer = m_pVertexBuffer;

            m_pDevice->IASetVertexBuffers(
                0, // StartSlot
                1, // NumBuffers
                &pVertexBuffer,
                &stride,
                &offset
                );

            // Set primitive topology.
            m_pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Initialize the world matrices.
            D3DMatrixIdentity(&m_WorldMatrix);

            // Initialize the view matrix.
            D3DMatrixLookAtLH(&m_ViewMatrix, &sc_eyeLocation, &sc_eyeAt, &sc_eyeUp);

            // Initialize the projection matrix.
            D3DMatrixPerspectiveFovLH(
                &m_ProjectionMatrix,
                (float)D3DX_PI * 0.24f, // fovy
                nWidth / (FLOAT)nHeight, // aspect
                0.1f, // zn
                800.0f // zf
                );

            // Update variables that never change.
            m_pViewVariableNoRef->SetMatrix((float*)&m_ViewMatrix);

            m_pProjectionVariableNoRef->SetMatrix((float*)&m_ProjectionMatrix);
        }
    }

    SafeRelease(&pBackBufferResource);
    SafeRelease(&pDevice);
    SafeRelease(&pDXGIDevice);
    SafeRelease(&pAdapter);
    SafeRelease(&pDXGIFactory);
    SafeRelease(&pSurface);
    SafeRelease(&pBackBuffer);

    return hr;
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::DiscardDeviceResources             *
*                                                                 *
*  Certain resources (eg. device, swap chain, RT) are bound to a  *
*  particular D3D device. Under certain conditions (eg. change    *
*  display mode, remoting, removing a video adapter), it is       *
*  necessary to discard device-specific resources. This method    *
*  just releases all of the device-bound resources that we're     *
*  holding onto.                                                  *
*                                                                 *
******************************************************************/

void Interactive3dTextSampleApp::DiscardDeviceResources()
{
    SafeRelease(&m_pDevice);
    SafeRelease(&m_pSwapChain);
    SafeRelease(&m_pState);

    SafeRelease(&m_pDepthStencil);
    SafeRelease(&m_pDepthStencilView);
    SafeRelease(&m_pRenderTargetView);
    SafeRelease(&m_pShader);
    SafeRelease(&m_pVertexBuffer);
    SafeRelease(&m_pVertexLayout);
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::RunMessageLoop                     *
*                                                                 *
*  This is the main message pump for the application              *
*                                                                 *
******************************************************************/

void Interactive3dTextSampleApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::OnChar                             *
*                                                                 *
******************************************************************/
void Interactive3dTextSampleApp::OnChar(SHORT key)
{
    if (key == '\r')
    {
        // swallow
    }
    else if (key == '\b')
    {
        if (m_characters.GetCount() > 2)
        {
            m_characters.RemoveLast();
        }
    }
    else
    {
        // In the case of failure we will keep the previous characters, so we
        // can safely ignore the return value.
        m_characters.Add(key);
    }

    // In the case of failure we will keep the previous text geometry, so we can
    // safely ignore the return value.
    UpdateTextGeometry();
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::OnRender                           *
*                                                                 *
*  This method is called when the app needs to paint the window.  *
*  It uses a D2D RT to draw a gradient background into the swap   *
*  chain buffer. Then, it uses a separate D2D RT to draw a        *
*  2D scene into a D3D texture. This texture is mapped onto a     *
*  simple planar 3D model and displayed using D3D.                *
*                                                                 *
******************************************************************/

HRESULT Interactive3dTextSampleApp::OnRender()
{
    HRESULT hr;
    static float t = 0.0f;
    static DWORD dwTimeStart = 0;

    hr = CreateDeviceResources();

    if (SUCCEEDED(hr) && m_pRenderTargetView)
    {
        DWORD dwTimeCur = GetTickCount();
        if (dwTimeStart == 0)
        {
            dwTimeStart = dwTimeCur;
        }
        t = (dwTimeCur - dwTimeStart) / 3000.0f;

        float a = ((float)D3DX_PI)/4 * sinf(2*t);

        // A silly way to get a blinking cursor, but it works!
        bool showCursor = sinf(20*t) < 0;

        ID2D1Geometry *pGeometry = NULL;

        //
        // Note: We are dynamically extruding the geometry every frame here.
        // This is somewhat wasteful, but allows us to introduce more complex
        // geometry animations in the future.
        //

        hr = GenerateTextOutline(showCursor, &pGeometry);

        if (SUCCEEDED(hr))
        {
            CArray<SimpleVertex> vertices;

            hr = Extruder::ExtrudeGeometry(
                pGeometry,
                24.0f, // height
                vertices
                );

            if (SUCCEEDED(hr))
            {
                D3DMatrixRotationY(&m_WorldMatrix, a);

                // Setup our lighting parameters
                D3DXVECTOR4 vLightPos[3] =
                {
                    D3DXVECTOR4( 1200.0f, -20.0f, 400.0f, 0.0f )
                };
                D3DXVECTOR4 vLightColors[3] =
                {
                    D3DXVECTOR4( 0.9f, 0.0f, 0.0f, 1.0f )
                };

                //Swap chain will tell us how big the back buffer is
                DXGI_SWAP_CHAIN_DESC swapDesc;
                hr = m_pSwapChain->GetDesc(&swapDesc);

                if (SUCCEEDED(hr))
                {
                    m_pDevice->ClearDepthStencilView(m_pDepthStencilView, D3D10_CLEAR_DEPTH, 1, 0);

                    const float black[4] ={0};

                    m_pDevice->ClearRenderTargetView(m_pRenderTargetView, black);

                    m_pTechniqueNoRef->GetPassByIndex(0)->Apply(0);

                    // Update variables that change once per frame
                    m_pWorldVariableNoRef->SetMatrix((float*)&m_WorldMatrix);

                    // Update lighting variables
                    m_pLightPosVariableNoRef->SetFloatVectorArray((float*)vLightPos, 0, 1);
                    m_pLightColorVariableNoRef->SetFloatVectorArray((float*)vLightColors, 0, 1);

                    // Render the scene
                    m_pTechniqueNoRef->GetPassByIndex(0)->Apply(0);

                    UINT verticesLeft = vertices.GetCount();

                    UINT index = 0;

                    while (SUCCEEDED(hr) && verticesLeft > 0)
                    {
                        UINT verticesToCopy = min(verticesLeft,sc_vertexBufferCount);

                        void *pVertices;

                        hr = m_pVertexBuffer->Map(
                            D3D10_MAP_WRITE_DISCARD,
                            0, // MapFlags
                            &pVertices
                            );
                        if (SUCCEEDED(hr))
                        {
                            memcpy(pVertices, &vertices[index], verticesToCopy*sizeof(SimpleVertex));

                            m_pVertexBuffer->Unmap();

                            m_pDevice->Draw(verticesToCopy, 0);

                            verticesLeft -= verticesToCopy;
                            index += verticesToCopy;
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        hr = m_pSwapChain->Present(1, 0);
                    }
                }
            }
            pGeometry->Release();
        }
    }

    // If the device is lost for any reason, we need to recreate it
    if (hr == DXGI_ERROR_DEVICE_RESET ||
        hr == DXGI_ERROR_DEVICE_REMOVED)
    {
        hr = S_OK;

        DiscardDeviceResources();
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::OnTimer                            *
*                                                                 *
*                                                                 *
******************************************************************/

void Interactive3dTextSampleApp::OnTimer()
{
    InvalidateRect(m_hwnd, NULL, FALSE);
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::WndProc                            *
*                                                                 *
*  This static method handles our app's window messages           *
*                                                                 *
******************************************************************/

LRESULT CALLBACK Interactive3dTextSampleApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        Interactive3dTextSampleApp *pInteractive3dTextSampleApp = (Interactive3dTextSampleApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pInteractive3dTextSampleApp)
            );

        result = 1;
    }
    else
    {
        Interactive3dTextSampleApp *pInteractive3dTextSampleApp = reinterpret_cast<Interactive3dTextSampleApp *>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
                ));

        bool wasHandled = false;

        if (pInteractive3dTextSampleApp)
        {
            switch (message)
            {
            case WM_PAINT:
            case WM_DISPLAYCHANGE:
                {
                    PAINTSTRUCT ps;
                    BeginPaint(hwnd, &ps);
                    pInteractive3dTextSampleApp->OnRender();
                    EndPaint(hwnd, &ps);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_CHAR:
                {
                    pInteractive3dTextSampleApp->OnChar(static_cast<SHORT>(wParam));
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_TIMER:
                {
                    pInteractive3dTextSampleApp->OnTimer();
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DESTROY:
                {
                    PostQuitMessage(0);
                }
                result = 1;
                wasHandled = true;
                break;
            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::CreateD3DDevice                    *
*                                                                 *
******************************************************************/

HRESULT Interactive3dTextSampleApp::CreateD3DDevice(
    IDXGIAdapter *pAdapter,
    D3D10_DRIVER_TYPE driverType,
    UINT flags,
    ID3D10Device1 **ppDevice
    )
{
    HRESULT hr = S_OK;

    static const D3D10_FEATURE_LEVEL1 levelAttempts[] =
    {
        D3D10_FEATURE_LEVEL_10_0,
        D3D10_FEATURE_LEVEL_9_3,
        D3D10_FEATURE_LEVEL_9_2,
        D3D10_FEATURE_LEVEL_9_1,
    };

    for (UINT level = 0; level < ARRAYSIZE(levelAttempts); level++)
    {
        ID3D10Device1 *pDevice = NULL;
        hr = D3D10CreateDevice1(
            pAdapter,
            driverType,
            NULL,
            flags,
            levelAttempts[level],
            D3D10_1_SDK_VERSION,
            &pDevice
            );

        if (SUCCEEDED(hr))
        {
            // transfer reference
            *ppDevice = pDevice;
            pDevice = NULL;
            break;
        }
    }

    if (FAILED(hr))
    {
        hr = D2DERR_NO_HARDWARE_DEVICE;
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  Interactive3dTextSampleApp::LoadResourceShader                 *
*                                                                 *
*  This method loads and creates a pixel shader from a DLL        *
*  resource                                                       *
*                                                                 *
******************************************************************/

HRESULT Interactive3dTextSampleApp::LoadResourceShader(
    ID3D10Device *pDevice,
    PCWSTR pszResource,
    ID3D10Effect **ppShader
    )
{
    HRESULT hr;

    HRSRC hResource = ::FindResource(HINST_THISCOMPONENT, pszResource, RT_RCDATA);
    hr = hResource ? S_OK : E_FAIL;

    if (SUCCEEDED(hr))
    {
        HGLOBAL hResourceData = ::LoadResource(HINST_THISCOMPONENT, hResource);
        hr = hResourceData ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
            LPVOID pData = ::LockResource(hResourceData);
            hr = pData ? S_OK : E_FAIL;

            if (SUCCEEDED(hr))
            {
                hr = ::D3D10CreateEffectFromMemory(
                    pData,
                    ::SizeofResource(HINST_THISCOMPONENT, hResource),
                    0,
                    pDevice,
                    NULL,
                    ppShader
                    );
            }
        }
    }

    return hr;
}
