
//+--------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Abstract:
//      Sample implementation of IDWriteTextRenderer converting DWrite text layout to XPS Canvas
//----------------------------------------------------------------------------

#include "common.h"
#include "LayoutToCanvasBuilder.h"

static const WCHAR g_layoutBrushKey[] = L"textLayoutBrush";

//
// Static utility function converting DWRITE_GLYPH_RUN_DESCRIPTION::clusterMap to XPS OM array of XPS_GLYPH_MAPPING structures
//
HRESULT
LayoutToCanvasBuilder::ClusterMapToMappingArray(
    const UINT16 *clusterMap, 
    UINT32 mapLen, // number of elements in clusterMap array
    UINT32 glyphsArrayLen, // number of elements in glyphs array
    UINT32 resultMaxCount, // size of output buffer resultGlyphMapping (max number of elements)
    XPS_GLYPH_MAPPING* resultGlyphMapping, // output buffer
    UINT32* resultGlyphMappingCount // number of elements returned in resultGlyphMapping
    )
{
    // assumption:
    // clusterMap[0] <= clusterMap[1] <= ... <= clusterMap[mapLen-1] < glyphsArrayLen

    HRESULT hr = S_OK;
    UINT32 i = 0; // number of elements added to resultGlyphMapping array
    UINT32 mapPos = 0; // position in clusterMap array

    *resultGlyphMappingCount = 0;

    while (mapPos < mapLen && i < resultMaxCount)
    {
        UINT32 codePointRangeLen = 1, glyphIndexRangeLen = 1;
        while (mapPos + codePointRangeLen < mapLen && 
               clusterMap[mapPos + codePointRangeLen] == clusterMap[mapPos])
        {
            codePointRangeLen++;
        }
        if (mapPos + codePointRangeLen == mapLen) 
        {
            // end of cluster map
            glyphIndexRangeLen = glyphsArrayLen - clusterMap[mapPos];
        }
        else 
        {
            glyphIndexRangeLen = clusterMap[mapPos + codePointRangeLen] - clusterMap[mapPos];
        }

        if (codePointRangeLen > 1 || glyphIndexRangeLen > 1)
        {
            // Add mapping entry for 1 : N, N : 1 and M : N  code point to glyph index clusters
            XPS_GLYPH_MAPPING& mappingEntry = resultGlyphMapping[i];
            mappingEntry.unicodeStringStart = mapPos;
            mappingEntry.unicodeStringLength = UINT16(codePointRangeLen);
            mappingEntry.glyphIndicesStart = clusterMap[mapPos];
            mappingEntry.glyphIndicesLength = UINT16(glyphIndexRangeLen);
            i++;
        }

        mapPos += codePointRangeLen;
    }

    if (mapPos < mapLen && i >= resultMaxCount)
    {
        hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
    }
    *resultGlyphMappingCount = i;
    return hr;
}

LayoutToCanvasBuilder::LayoutToCanvasBuilder(
    IXpsOMObjectFactory* xpsFactory
    )
    : _refCount(1),
      _xpsFactory(NULL), _xpsCanvas(NULL), _xpsResources(NULL),
      _fontMapSize(0)
{
    _xpsFactory = xpsFactory;
    _xpsFactory->AddRef();
}

//
// This internal method creates empty canvas with resource dictionary holding one solid color brush.
// It also creates empty IXpsOMPartResources object.
//
HRESULT
LayoutToCanvasBuilder::CreateRootCanvasAndResources()
{
    HRESULT hr = S_OK;

    IXpsOMDictionary* canvasDictionary = NULL;
    IXpsOMSolidColorBrush* blueSolidBrush = NULL;

    hr = _xpsFactory->CreateCanvas( &_xpsCanvas );
    
    if (SUCCEEDED(hr))
    {
        hr = _xpsFactory->CreatePartResources( &_xpsResources );
    }
    
    // Create dictionary and add to canvas
    if (SUCCEEDED(hr))
    {
        hr = _xpsFactory->CreateDictionary( &canvasDictionary );
    }

    if (SUCCEEDED(hr))
    {
        hr = _xpsCanvas->SetDictionaryLocal( canvasDictionary );
    }

    // Create solid color brush and add to canvas dictionary
    if (SUCCEEDED(hr))
    {
        XPS_COLOR rgbBlue;
        rgbBlue.colorType = XPS_COLOR_TYPE_SRGB;
        rgbBlue.value.sRGB.alpha = 0xFF;
        rgbBlue.value.sRGB.red = 0;
        rgbBlue.value.sRGB.green = 0;
        rgbBlue.value.sRGB.blue = 0xFF;
        hr = _xpsFactory->CreateSolidColorBrush(
                &rgbBlue,
                NULL, // no color profile for this color type
                &blueSolidBrush
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = canvasDictionary->Append( 
                g_layoutBrushKey, 
                blueSolidBrush
                );
    }

    // cleanup
    if (blueSolidBrush) 
    {
        blueSolidBrush->Release();
        blueSolidBrush = NULL;
    }
    if (canvasDictionary) 
    {
        canvasDictionary->Release();
        canvasDictionary = NULL;
    }

    return hr;
}

LayoutToCanvasBuilder::~LayoutToCanvasBuilder()
{
    for (UINT i = 0; i < _fontMapSize; i++)
    {
        _fontMap[i].fontResource->Release();
    }
    _fontMapSize = 0;

    if (_xpsFactory) 
    {
        _xpsFactory->Release();
        _xpsFactory = NULL;
    }

    if (_xpsCanvas) 
    {
        _xpsCanvas->Release();
        _xpsCanvas = NULL;
    }

    if (_xpsResources) 
    {
        _xpsResources->Release();
        _xpsResources = NULL;
    }

}

//static 
HRESULT 
LayoutToCanvasBuilder::CreateInstance(
    IXpsOMObjectFactory* xpsFactory,
    LayoutToCanvasBuilder** ppNewInstance
    )
{
    HRESULT hr = S_OK;

    LayoutToCanvasBuilder* pResult = NULL;

    pResult = new(std::nothrow) LayoutToCanvasBuilder(xpsFactory);
    if (!pResult) 
    {
        hr = E_OUTOFMEMORY;
    }
    if (SUCCEEDED(hr))
    {
        hr = pResult->CreateRootCanvasAndResources();
    }
    if (SUCCEEDED(hr))
    {
        *ppNewInstance = pResult;
        (*ppNewInstance)->AddRef();
    }

    if (pResult)
    {
        pResult->Release();
        pResult = NULL;
    }
    return hr;
}

IXpsOMCanvas* 
LayoutToCanvasBuilder::GetCanvas()
{
    return _xpsCanvas;
}

IXpsOMPartResources* 
LayoutToCanvasBuilder::GetResources()
{
    return _xpsResources;
}

STDMETHODIMP
LayoutToCanvasBuilder::QueryInterface( 
    REFIID riid,
    void** ppvObject
    )
{
    if (ppvObject == NULL)
    {
        return E_POINTER;
    }
    *ppvObject = NULL;

    if (IsEqualGUID(riid, __uuidof(IUnknown)) || 
        IsEqualGUID(riid, __uuidof(IDWritePixelSnapping)) ||
        IsEqualGUID(riid, __uuidof(IDWriteTextRenderer)))
    {
        IUnknown* pUnk = static_cast<IUnknown*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }
    return E_NOINTERFACE;
}

ULONG 
LayoutToCanvasBuilder::AddRef()
{
    return ++_refCount;
}

ULONG 
LayoutToCanvasBuilder::Release()
{
    ULONG newCount = --_refCount;
    if (newCount == 0)
        delete this;
    return newCount;
}

// IDWritePixelSnapping methods
STDMETHODIMP
LayoutToCanvasBuilder::IsPixelSnappingDisabled(
    void* /* clientDrawingContext */,
    BOOL* isDisabled
    )
{
    *isDisabled = FALSE;
    return S_OK; 
}

//
// returns identity matrix - our abstract coordinates are device independent pixels. 
//
STDMETHODIMP
LayoutToCanvasBuilder::GetCurrentTransform(
    void* /* clientDrawingContext */,
    DWRITE_MATRIX* transform
    )
{
    transform->m11 = transform->m22 = 1.0;
    transform->m12 = transform->m21 = transform->dx = transform->dy = 0.0;
    return S_OK;
}

//
// Returns 1.0 because we use DIP pixels
//
STDMETHODIMP
LayoutToCanvasBuilder::GetPixelsPerDip(
    void* /* clientDrawingContext */,
    FLOAT* pixelsPerDip
    )
{
    // DIP used in XPS (96 DPI)
    *pixelsPerDip = FLOAT(1.0);
    return S_OK;
}

// IDWriteTextRenderer
STDMETHODIMP
LayoutToCanvasBuilder::DrawGlyphRun(
    void* /* clientDrawingContext */,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE /* measuringMode */,
    DWRITE_GLYPH_RUN const* glyphRun,
    DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
    IUnknown* /* clientDrawingEffect */
    )
{    
    HRESULT hr = S_OK;

    // supported font types in xps
    DWRITE_FONT_FACE_TYPE fontFaceType = 
        glyphRun->fontFace->GetType();

    if (fontFaceType != DWRITE_FONT_FACE_TYPE_CFF &&
        fontFaceType != DWRITE_FONT_FACE_TYPE_TRUETYPE &&
        fontFaceType != DWRITE_FONT_FACE_TYPE_TRUETYPE_COLLECTION)
    {
        // XPS does not support this type of font - just ignore this glyph run.
        hr = S_OK;
    }
    else
    {
        const FLOAT positionScale = 100.0f / glyphRun->fontEmSize;

        IXpsOMFontResource* fontResource = NULL;
        IXpsOMGlyphs* xpsGlyphs = NULL;
        IXpsOMVisualCollection* canvasVisuals = NULL;
        IXpsOMGlyphsEditor* xpsGlyphsEditor = NULL;
        XPS_GLYPH_INDEX *glyphIndexVector = NULL;
        PWSTR pszUnicodeString = NULL;

        // Find or create XPS font resource for this font face
        if (SUCCEEDED(hr))
        {
            hr = FindOrCreateFontResource( glyphRun->fontFace, &fontResource );
        }

        if (SUCCEEDED(hr))
        {
            hr = _xpsFactory->CreateGlyphs( fontResource, &xpsGlyphs );
        }

        // Add new Glyphs element to canvas
        if (SUCCEEDED(hr)) 
        {
            hr = _xpsCanvas->GetVisuals( &canvasVisuals );
        }

        if (SUCCEEDED(hr))
        {
            hr = canvasVisuals->Append( xpsGlyphs );
        }

        // Now set Glyphs properties
        if (SUCCEEDED(hr))
        {
            XPS_POINT glyphsBaselineOrigin = { baselineOriginX, baselineOriginY };
            hr = xpsGlyphs->SetOrigin( &glyphsBaselineOrigin );
        }

        if (SUCCEEDED(hr))
        {
            hr = xpsGlyphs->SetFontRenderingEmSize( glyphRun->fontEmSize );
        }

        if (SUCCEEDED(hr))
        {
            hr = xpsGlyphs->SetFillBrushLookup( g_layoutBrushKey );
        }

        if (SUCCEEDED(hr) &&
            fontFaceType == DWRITE_FONT_FACE_TYPE_TRUETYPE_COLLECTION)
        {
            // set font face index
            hr = xpsGlyphs->SetFontFaceIndex( (SHORT)glyphRun->fontFace->GetIndex() );
        }

        if (SUCCEEDED(hr))
        {
            DWRITE_FONT_SIMULATIONS dwriteFontSim = 
                glyphRun->fontFace->GetSimulations();

            if (dwriteFontSim & DWRITE_FONT_SIMULATIONS_BOLD)
            {
                if (dwriteFontSim & DWRITE_FONT_SIMULATIONS_OBLIQUE)
                {
                    hr = xpsGlyphs->SetStyleSimulations( XPS_STYLE_SIMULATION_BOLDITALIC );
                }
                else
                {
                    hr = xpsGlyphs->SetStyleSimulations( XPS_STYLE_SIMULATION_BOLD );
                }
            }
            else
            {
                if (dwriteFontSim & DWRITE_FONT_SIMULATIONS_OBLIQUE)
                {
                    hr = xpsGlyphs->SetStyleSimulations( XPS_STYLE_SIMULATION_ITALIC );
                }
            }
        }

        // The rest of the properties must be set through glyphs editor interface because of interdependencies
        if (SUCCEEDED(hr))
        {
            hr = xpsGlyphs->GetGlyphsEditor( &xpsGlyphsEditor );
        }

        if (SUCCEEDED(hr))
        {
            glyphIndexVector = (XPS_GLYPH_INDEX *)CoTaskMemAlloc( glyphRun->glyphCount * sizeof(XPS_GLYPH_INDEX) );
            if (!glyphIndexVector) 
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            for (UINT32 i = 0; i < glyphRun->glyphCount; i++)
            {
                // NOTE: these values may need extra adjustment depending on transformation, IsSideways and bidiLevel
                glyphIndexVector[i].index = glyphRun->glyphIndices[i];
                // advanceWidth, horizontal and vertical offset in Indices attribute in XPS Glyphs element are in 1/100 of font em size.
                glyphIndexVector[i].advanceWidth = FLOAT(glyphRun->glyphAdvances[i] * positionScale);  
                glyphIndexVector[i].horizontalOffset = (glyphRun->glyphOffsets != NULL) ? FLOAT(glyphRun->glyphOffsets[i].advanceOffset * positionScale) : 0; 
                glyphIndexVector[i].verticalOffset = (glyphRun->glyphOffsets != NULL) ? FLOAT(glyphRun->glyphOffsets[i].ascenderOffset * positionScale) : 0; 
            }

            hr = xpsGlyphsEditor->SetGlyphIndices(
                    (UINT32)glyphRun->glyphCount,
                    glyphIndexVector
                    );
        }

        if (SUCCEEDED(hr))
        {
            hr = xpsGlyphsEditor->SetIsSideways( glyphRun->isSideways );
        }

        if (SUCCEEDED(hr))
        {
            hr = xpsGlyphsEditor->SetBidiLevel( glyphRun->bidiLevel );
        }

        // Check for unicode string and cluster map
        if (SUCCEEDED(hr) &&
            glyphRunDescription->string && glyphRunDescription->stringLength > 0)
        {
            // IXpsOMGlyphsEditor::SetUnicodeString expects null terminated string but glyphRunDescription->string may not be so terminated.
            pszUnicodeString = (PWSTR)CoTaskMemAlloc( (glyphRunDescription->stringLength + 1) * sizeof(WCHAR) );
            if ( !pszUnicodeString ) 
            {
                hr = E_OUTOFMEMORY;
            }

            if (SUCCEEDED(hr))
            {
                hr = StringCchCopyN(
                        pszUnicodeString, 
                        glyphRunDescription->stringLength + 1, 
                        glyphRunDescription->string, 
                        glyphRunDescription->stringLength
                        );
            }

            if (SUCCEEDED(hr))
            {
                hr = xpsGlyphsEditor->SetUnicodeString( pszUnicodeString );
            }

            // fill in glyph mapping array and call xpsGlyphsEditor->SetGlyphMappings
            if (SUCCEEDED(hr) &&
                glyphRunDescription->clusterMap)
            {
                //
                // This sample uses GLYPH_MAPPING_MAX_COUNT constant to limit number of non-trivial (1:N, N:1 or M:N) mappings
                // between unicode codepoints and glyph indexes for each glyph run.
                // Complete implementation should handle ERROR_MORE_DATA result from ClusterMapToMappingArray function.
                //
                XPS_GLYPH_MAPPING glyphMapVector[GLYPH_MAPPING_MAX_COUNT];
                UINT32 glyphMappingCount = 0;
                hr = ClusterMapToMappingArray(
                        glyphRunDescription->clusterMap, 
                        glyphRunDescription->stringLength,
                        glyphRun->glyphCount,
                        ARRAYSIZE(glyphMapVector),
                        glyphMapVector,
                        &glyphMappingCount
                        );

                if (SUCCEEDED(hr) &&
                    glyphMappingCount > 0)
                {
                    hr = xpsGlyphsEditor->SetGlyphMappings(
                            glyphMappingCount,
                            glyphMapVector
                            );
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = xpsGlyphsEditor->ApplyEdits();
        }

        // release local resources here
        if (pszUnicodeString) 
        {
            CoTaskMemFree(pszUnicodeString);
            pszUnicodeString = NULL;
        }
        if (glyphIndexVector) 
        {
            CoTaskMemFree(glyphIndexVector);
            glyphIndexVector = NULL;
        }
        if (fontResource) 
        {
            fontResource->Release();
            fontResource = NULL;
        }
        if (xpsGlyphs) 
        {
            xpsGlyphs->Release();
            xpsGlyphs = NULL;
        }
        if (canvasVisuals) 
        {
            canvasVisuals->Release();
            canvasVisuals = NULL;
        }
        if (xpsGlyphsEditor) 
        {
            xpsGlyphsEditor->Release();
            xpsGlyphsEditor = NULL;
        }
    }

    return hr;
}


STDMETHODIMP
LayoutToCanvasBuilder::DrawUnderline(
    void* /* clientDrawingContext */,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_UNDERLINE const* underline,
    IUnknown* /* clientDrawingEffect */
    )
{
    XPS_POINT begin, end;

    begin.x = baselineOriginX;
    begin.y = baselineOriginY + underline->offset;
    end.x = baselineOriginX + underline->width;
    end.y = baselineOriginY + underline->offset;

    return AddLinePath( &begin, &end, underline->thickness );
    // underline->runHeight - not used
    // underline->flowDirection - not used
    // underline->localeName - not used
}

STDMETHODIMP
LayoutToCanvasBuilder::DrawStrikethrough(
    void* /* clientDrawingContext */,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_STRIKETHROUGH const* strikethrough,
    IUnknown* /* clientDrawingEffect */
    )
{
    XPS_POINT begin, end;

    begin.x = baselineOriginX;
    begin.y = baselineOriginY + strikethrough->offset;
    end.x = baselineOriginX + strikethrough->width;
    end.y = baselineOriginY + strikethrough->offset;

    return AddLinePath(&begin, &end, strikethrough->thickness);
    // strikethrough->runHeight - not used
    // strikethrough->flowDirection - not used
    // strikethrough->localeName - not used
}

//
// NOTE: This method is not implemented ! Does nothing.
//
STDMETHODIMP
LayoutToCanvasBuilder::DrawInlineObject(
    void* /* clientDrawingContext */,
    FLOAT /* originX */,
    FLOAT /* originY */,
    IDWriteInlineObject* /* inlineObject */,
    BOOL /* isSideways */,
    BOOL /* isRightToLeft */,
    IUnknown* /* clientDrawingEffect */
    )
{
    return S_OK;
}

//
// This method looks for font file object (by COM identity) in resource map. If found, the corresponding XPS font resource is used.
// If not, a new font resource is created and font file data is copied to new XPS font resource. 
//
HRESULT
LayoutToCanvasBuilder::FindOrCreateFontResource( 
    IDWriteFontFace* fontFace,
    IXpsOMFontResource** ppXpsFontResource
    )
{
    HRESULT hr = S_OK;
    UINT32 numFiles = 1;
    UINT_PTR fontFileKey = 0; // we will use IDWriteFontFile COM object identity to detect equal fonts

    IDWriteFontFile* fontFile = NULL;
    IUnknown* pUnk = NULL;

    *ppXpsFontResource = NULL; // reset output argument

    hr = fontFace->GetFiles(&numFiles, &fontFile);

    if (SUCCEEDED(hr))
    {
        if (numFiles != 1)
        {
            // Font face type is verified by caller. It can not be stored in more that one file.
            hr = E_UNEXPECTED;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = fontFile->QueryInterface( __uuidof(IUnknown), reinterpret_cast<void **>(&pUnk) );
    }

    if (SUCCEEDED(hr))
    {
        fontFileKey = UINT_PTR(pUnk);
        if (pUnk)
        {
            pUnk->Release(); 
            pUnk = NULL;
        }

        for (UINT i = 0; i < _fontMapSize; i++)
        {
            if (_fontMap[i].key == fontFileKey)
            {
                // font is already in map and _xpsResources collection
                *ppXpsFontResource = _fontMap[i].fontResource;
                (*ppXpsFontResource)->AddRef();
                break;
            }
        }
    }

    if (*ppXpsFontResource)
    {
        // Nothing more to do - output argument is set and hr is S_OK.
    }
    else
    {
        // This is a new font file. We have to create XPS resource for it.
    
        // First, create temporary storage with IStream implementation for font bytes. HGLOBAL memory is used in this sample.
        // NOTE: Font data may be large - temp file is recommended for complete implementation.
        IStream* fontResourceStream = NULL;
        IDWriteFontFileLoader* fontFileLoader = NULL;
        IDWriteFontFileStream* fontFileStream = NULL;
        IOpcPartUri* fontPartUri = NULL;
        IXpsOMFontResource* pNewFontResource = NULL;
        IXpsOMFontResourceCollection* fontResources = NULL;
    
        const void *fontFileRef = NULL;
        UINT32 fontFileRefSize = 0;
        UINT64 bytesLeft = 0, readOffset = 0;

        if (SUCCEEDED(hr)) 
        {
            hr = CreateStreamOnHGlobal( // Win32 API
                    NULL, // let implementation take care of memory
                    TRUE, // release memory when done
                    &fontResourceStream
                    );
        }

        // Copy font data to temporary storage
        if (SUCCEEDED(hr))
        {
            hr = fontFile->GetReferenceKey(&fontFileRef, &fontFileRefSize);
        }
    
        if (SUCCEEDED(hr))
        {
            hr = fontFile->GetLoader(&fontFileLoader);
        }
    
        if (SUCCEEDED(hr))
        {
            hr = fontFileLoader->CreateStreamFromKey(
                    fontFileRef,
                    fontFileRefSize,
                    &fontFileStream
                    );
            fontFileLoader->Release(); 
            fontFileLoader = NULL;
        }

        if (SUCCEEDED(hr))
        {
            hr = fontFileStream->GetFileSize(&bytesLeft);
        }

        if (SUCCEEDED(hr))
        {
            while (bytesLeft && SUCCEEDED(hr))
            {
                const void *fragment = NULL;
                PVOID fragmentContext = NULL;
                bool bFragmentContextAcquired = false;
                UINT64 readSize = min(bytesLeft, 16384);

                hr = fontFileStream->ReadFileFragment(
                        &fragment,
                        readOffset,
                        readSize,
                        &fragmentContext
                        );

                if (SUCCEEDED(hr))
                {
                    bFragmentContextAcquired = true;
                    hr = fontResourceStream->Write(
                            fragment, 
                            ULONG(readSize),
                            NULL
                            );
                }

                if (SUCCEEDED(hr))
                {
                    bytesLeft -= readSize;
                    readOffset += readSize;
                }

                if (bFragmentContextAcquired)
                {
                    fontFileStream->ReleaseFileFragment(fragmentContext);
                }
            }
            fontFileStream->Release(); 
            fontFileStream = NULL;
        }

        if (SUCCEEDED(hr))
        {
            LARGE_INTEGER liZero = {0};
            hr = fontResourceStream->Seek(liZero, STREAM_SEEK_SET, NULL);
        }

        // Create new part URI for this resource
        if (SUCCEEDED(hr))
        {
            hr = GenerateNewFontPartUri(&fontPartUri);
        }

        // NOTE: See XPS spec section 2.1.7.2
        // This sample will obfuscate all font resources. It is allowed by XPS specification.
        // This sample does not set RestrictedFont relationship for any font resource! 
        // This may result in incorrect XPS file if one or more fonts have Print&Preview flag!
        if (SUCCEEDED(hr))
        {
            hr = _xpsFactory->CreateFontResource(
                    fontResourceStream,
                    XPS_FONT_EMBEDDING_OBFUSCATED,
                    fontPartUri,
                    FALSE, //isObfSourceStream
                    &pNewFontResource
                    );
        }

        // New font resource must be added to map and fonts collection before returning it to caller
        if (SUCCEEDED(hr))
        {
            hr = _xpsResources->GetFontResources(&fontResources);
        }

        if (SUCCEEDED(hr))
        {
            hr = fontResources->Append(pNewFontResource);
        }

        if (SUCCEEDED(hr))
        {
            if (_fontMapSize < FONT_MAP_MAX_SIZE)
            {
                _fontMap[_fontMapSize].key = fontFileKey;
                _fontMap[_fontMapSize].fontResource = pNewFontResource;
                _fontMap[_fontMapSize].fontResource->AddRef();
                _fontMapSize++;
            }
            else
            {
                //
                // Do nothing here. 
                //
                // NOTE:
                // This sample limits number of entries with FONT_MAP_MAX_SIZE constant. If _fontMap is full and 
                // the same font file is used in another glyph run, it may be added as a new font resource part in XPS.
                // This may result in large XPS file!
                // Complete implementation should dynamically increase _fontMap array size to avoid font data duplication.
                //
            }

            *ppXpsFontResource = pNewFontResource;
            (*ppXpsFontResource)->AddRef();
        }

        // release objects used for converting DWrite font file to XPS font resource
        if (fontResourceStream) 
        {
            fontResourceStream->Release();
            fontResourceStream = NULL;
        }
        if (fontFileLoader) 
        {
            fontFileLoader->Release();
            fontFileLoader = NULL;
        }
        if (fontFileStream) 
        {
            fontFileStream->Release();
            fontFileStream = NULL;
        }
        if (fontPartUri) 
        {
            fontPartUri->Release();
            fontPartUri = NULL;
        }
        if (pNewFontResource) 
        {
            pNewFontResource->Release();
            pNewFontResource = NULL;
        }
        if (fontResources) 
        {
            fontResources->Release();
            fontResources = NULL;
        }
    }

    if (fontFile)
    {
        fontFile->Release();
        fontFile = NULL;
    }
    if (pUnk)
    {
        pUnk->Release(); 
        pUnk = NULL;
    }

    return hr;
}

//
//  Returns IOpcPartUri object built from URI '/Resources/Fonts/__guid__.odttf'
//
HRESULT 
LayoutToCanvasBuilder::GenerateNewFontPartUri(
    IOpcPartUri** ppPartUri
    )
{
    HRESULT hr = S_OK;
    GUID guid;
    WCHAR guidString[128] = {0};
    WCHAR uriString[256] = {0};

    hr = CoCreateGuid(&guid);
    
    if (SUCCEEDED(hr))
    {
        hr = StringFromGUID2( guid, guidString, ARRAYSIZE(guidString) );
    }

    if (SUCCEEDED(hr))
    {
        hr = StringCchCopy( uriString, ARRAYSIZE(uriString), L"/Resources/Fonts/" );
    }

    if (SUCCEEDED(hr))
    {
        // guid string start and ends with curly brackets so they are removed
        hr = StringCchCatN( uriString, ARRAYSIZE(uriString), guidString + 1, wcslen(guidString) - 2 );
    }

    if (SUCCEEDED(hr))
    {
        hr = StringCchCat( uriString,  ARRAYSIZE(uriString), L".odttf" );
    }

    if (SUCCEEDED(hr))
    {
        hr = _xpsFactory->CreatePartUri( uriString, ppPartUri );
    }

    return hr;
}

//
// This methods creates a line path and adds it to current canvas visuals. 
// It is used by callback methods DrawUnderline and DrawStrikethrough.
// 
HRESULT 
LayoutToCanvasBuilder::AddLinePath(
    const XPS_POINT *beginPoint,
    const XPS_POINT *endPoint,
    FLOAT thickness
    )
{
    HRESULT hr = S_OK;

    IXpsOMVisualCollection* canvasVisuals = NULL;
    IXpsOMPath* linePath = NULL;
    IXpsOMGeometry* lineGeom = NULL;
    IXpsOMGeometryFigureCollection* geomFigures = NULL;
    IXpsOMGeometryFigure* lineFigure = NULL;

    // Create Path element and add to canvas
    if (SUCCEEDED(hr)) 
    {
        hr = _xpsFactory->CreatePath(&linePath);
    }
    if (SUCCEEDED(hr)) 
    {
        hr = _xpsCanvas->GetVisuals(&canvasVisuals);
    }
    if (SUCCEEDED(hr)) 
    {
        hr = canvasVisuals->Append(linePath);
    }

    // Set necessary path properties
    if (SUCCEEDED(hr)) 
    {
        hr = linePath->SetStrokeBrushLookup(g_layoutBrushKey);
    }
    if (SUCCEEDED(hr)) 
    {
        hr = linePath->SetSnapsToPixels(TRUE);
    }
    if (SUCCEEDED(hr)) 
    {
        hr = linePath->SetStrokeThickness(thickness);
    }

    // Create geometry and assign it to Path.Data property
    if (SUCCEEDED(hr)) 
    {
        hr = _xpsFactory->CreateGeometry(&lineGeom);
    }
    if (SUCCEEDED(hr)) 
    {
        hr = linePath->SetGeometryLocal( lineGeom );
    }

    // create geometry figure and add it to geometry
    if (SUCCEEDED(hr)) 
    {
        hr = _xpsFactory->CreateGeometryFigure( beginPoint, &lineFigure );
    }
    if (SUCCEEDED(hr)) 
    {
        hr = lineGeom->GetFigures( &geomFigures );
    }
    if (SUCCEEDED(hr)) 
    {
        hr = geomFigures->Append( lineFigure );
    }

    // set line segment in figure
    if (SUCCEEDED(hr)) 
    {
        XPS_SEGMENT_TYPE segmType = XPS_SEGMENT_TYPE_LINE;
        FLOAT segmentData[2] = { endPoint->x , endPoint->y };
        BOOL segmStroke = TRUE;

        hr = lineFigure->SetSegments(
                1, // segment count
                2, // segment data count
                &segmType, // segment types array (1 element)
                segmentData,
                &segmStroke // segment stroke array (1 element)
                );
    }

    // line figure is not closed or filled
    if (SUCCEEDED(hr)) 
    {
        hr = lineFigure->SetIsClosed( FALSE );
    }
    if (SUCCEEDED(hr)) 
    {
        hr = lineFigure->SetIsFilled( FALSE );
    }

    if (canvasVisuals) 
    {
        canvasVisuals->Release();
        canvasVisuals = NULL;
    }
    if (linePath) 
    {
        linePath->Release();
        linePath = NULL;
    }
    if (lineGeom) 
    {
        lineGeom->Release();
        lineGeom = NULL;
    }
    if (geomFigures) 
    {
        geomFigures->Release();
        geomFigures = NULL;
    }
    if (lineFigure) 
    {
        lineFigure->Release();
        lineFigure = NULL;
    }

    return hr;
}
