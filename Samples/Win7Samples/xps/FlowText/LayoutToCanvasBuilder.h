//+--------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Abstract:
//      Definition of LayoutToCanvasBuilder class implementing IDWriteTextRenderer interface
//----------------------------------------------------------------------------

#pragma once

// 
// Implementation of IDWriteTextRenderer interface which builds XPS canvas with multiple Glyphs
// 
class LayoutToCanvasBuilder : public IDWriteTextRenderer
{
private:
    static const UINT FONT_MAP_MAX_SIZE = 64;
    static const UINT GLYPH_MAPPING_MAX_COUNT = 32;

    static
    HRESULT
    ClusterMapToMappingArray(
        const UINT16 *clusterMap, 
        UINT32 mapLen, // number of elements in clusterMap array
        UINT32 glyphsArrayLen, // number of elements in glyphs array
        UINT32 resultMaxCount, // size of output buffer resultGlyphMapping (max number of elements)
        XPS_GLYPH_MAPPING* resultGlyphMapping, // output buffer 
        UINT32* resultGlyphMappingCount // number of elements returned in resultGlyphMapping
        );

    ULONG _refCount;

    // xpsFactory is stored for optimization only. Each callback method can create an instance if so preferred.
    IXpsOMObjectFactory* _xpsFactory;

    // Result canvas and resources, empty before IDWriteTextLayout::Draw is called with this callback object.
    IXpsOMCanvas* _xpsCanvas;
    IXpsOMPartResources* _xpsResources;

    // This maps COM identity of IDWriteFontFace object to IXpsOMFontResource object
    struct FontMapEntry 
    {
        UINT_PTR key;
        IXpsOMFontResource* fontResource;
    };
    UINT _fontMapSize;
    FontMapEntry _fontMap[FONT_MAP_MAX_SIZE];

    // Constructors and destructor are private:
    // Instance of this object may be created only by CreateInstance static method.
    // It may be deleted only by Release.
    LayoutToCanvasBuilder();
    
    explicit 
    LayoutToCanvasBuilder(
        IXpsOMObjectFactory* xpsFactory
        );

    ~LayoutToCanvasBuilder();

    // Internal helper methods
    HRESULT
    CreateRootCanvasAndResources();

    HRESULT 
    FindOrCreateFontResource( 
        IDWriteFontFace *fontFace,
        IXpsOMFontResource** xpsFontResource
        );
    
    HRESULT 
    GenerateNewFontPartUri(
        IOpcPartUri** partUri
        );
    
    HRESULT 
    AddLinePath(
        const XPS_POINT *begin,
        const XPS_POINT *end,
        FLOAT thickness
        );

public:
    static 
    HRESULT 
    CreateInstance(
        IXpsOMObjectFactory* xpsFactory,
        LayoutToCanvasBuilder** ppNewInstance
        );

    // getter methods for accessing generated XPS canvas and resources. 
    // NOTE: Returned pointer is NOT AddRef'ed by method!
    IXpsOMCanvas* GetCanvas();
    IXpsOMPartResources* GetResources();

    // IUnknown methods
    STDMETHOD(QueryInterface)( 
        REFIID riid,
        void** ppvObject
        );

    ULONG STDMETHODCALLTYPE AddRef();

    ULONG STDMETHODCALLTYPE Release() ;

    // IDWritePixelSnapping methods
    STDMETHOD(IsPixelSnappingDisabled)(
        void* clientDrawingContext,
        BOOL* isDisabled
        );

    STDMETHOD(GetCurrentTransform)(
        void* clientDrawingContext,
        DWRITE_MATRIX* transform
        );

    STDMETHOD(GetPixelsPerDip)(
        void* clientDrawingContext,
        FLOAT* pixelsPerDip
        );

    // IDWriteTextRenderer methods
    STDMETHOD(DrawGlyphRun)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect
        );

    STDMETHOD(DrawUnderline)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect
        );

    STDMETHOD(DrawStrikethrough)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect
        );

    STDMETHOD(DrawInlineObject)(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
        );
};
