
//+--------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Abstract:
//      Sample application for converting DWrite text layout to XPS Canvas
//----------------------------------------------------------------------------

#include "common.h"

#include "LayoutToCanvasBuilder.h"

//
// This method creates DWrite text layout with several different glyph runs
// NOTE: Modification in this method will exercise various code paths in sample. Interesting cases: sideways, right-to-left, non-english unicode string, etc.
//
HRESULT
MakeTextLayout(
    IDWriteTextLayout** ppTextLayout
    )
{
    static WCHAR TEXT_STRING[] = 
L"The XML Paper Specification (XPS) provides users and developers with a robust, open and trustworthy format for electronic paper. The XML Paper Specification describes electronic paper in a way that can be read by hardware, read by software, and read by people. XPS documents print better, can be shared easier, are more secure and can be archived with confidence.";

    HRESULT hr = S_OK;

    IDWriteFactory* pDWriteFactory = NULL;
    IDWriteTextFormat* pTextFormat = NULL;
    IDWriteTextLayout* pResultLayout = NULL;

    DWRITE_TEXT_RANGE modifyFormatRange;
    modifyFormatRange.length = 20;

    hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown **>(&pDWriteFactory)
            );

    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory->CreateTextFormat(
                L"Arial",
                NULL, //__maybenull IDWriteFontCollection* fontCollection,
                DWRITE_FONT_WEIGHT_REGULAR,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                12.0,
                L"en-US", //localeName,
                &pTextFormat
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = pTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
    }

    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory->CreateTextLayout(
                TEXT_STRING, // at least 100 characters
                (UINT32)wcslen(TEXT_STRING),
                pTextFormat,
                400.0, // limit text width to cause line breaks if text is long enough
                400.0,
                &pResultLayout
                );
    }

    if (SUCCEEDED(hr))
    {
        modifyFormatRange.startPosition = 10;
        hr = pResultLayout->SetFontSize( 16, modifyFormatRange );
    }

    if (SUCCEEDED(hr))
    {
        modifyFormatRange.startPosition = 35;
        hr = pResultLayout->SetFontFamilyName( L"Courier New", modifyFormatRange );
    }

    if (SUCCEEDED(hr))
    {
        modifyFormatRange.startPosition = 60;
        hr = pResultLayout->SetFontStyle( DWRITE_FONT_STYLE_ITALIC, modifyFormatRange );
    }

    if (SUCCEEDED(hr))
    {
        modifyFormatRange.startPosition = 85;
        hr = pResultLayout->SetUnderline( TRUE, modifyFormatRange );
    }

    if (SUCCEEDED(hr))
    {
        modifyFormatRange.startPosition = 110;
        hr = pResultLayout->SetStrikethrough( TRUE, modifyFormatRange );
    }

    if (SUCCEEDED(hr))
    {
        *ppTextLayout = pResultLayout;
        (*ppTextLayout)->AddRef();
    }

    // cleanup resources
    if (pResultLayout) 
    {
        pResultLayout->Release();
        pResultLayout = NULL;
    }
    if (pTextFormat) 
    {
        pTextFormat->Release();
        pTextFormat = NULL;
    }
    if (pDWriteFactory) 
    {
        pDWriteFactory->Release();
        pDWriteFactory = NULL;
    }

    return hr;
}

//
// This method saves XPS OM canvas to one page XPS file on Desktop.
//
HRESULT
SaveCanvasToXpsViaPackage( 
    IXpsOMObjectFactory *xpsFactory, 
    IXpsOMCanvas *canvas
    )
{
    HRESULT hr = S_OK;

    IXpsOMPackage*                  xpsPackage = NULL;
    IXpsOMDocumentSequence*         xpsFDS = NULL;
    IXpsOMDocumentCollection*       fixedDocuments = NULL;
    IXpsOMDocument*                 xpsFD = NULL;
    IXpsOMPageReferenceCollection*  pageRefs = NULL;
    IXpsOMPageReference*            xpsPageRef = NULL;
    IXpsOMPage*                     xpsPage = NULL;
    IOpcPartUri*                    opcPartUri = NULL;
    IXpsOMVisualCollection*         pageVisuals = NULL;

    XPS_SIZE pageSize = {400.0, 600.0};
    WCHAR szDesktopPath[MAX_PATH] = {0};

    // Create trunk elements of XPS object model
    hr = xpsFactory->CreatePackage( &xpsPackage );

    if (SUCCEEDED(hr))
    {
        hr = xpsFactory->CreatePartUri( L"/FixedDocumentSequence.fdseq", &opcPartUri );
    }

    if (SUCCEEDED(hr))
    {
        hr = xpsFactory->CreateDocumentSequence( opcPartUri, &xpsFDS );
        opcPartUri->Release();
        opcPartUri = NULL;
    }

    if (SUCCEEDED(hr))
    {
        hr = xpsFactory->CreatePartUri( L"/Documents/1/FixedDocument.fdoc", &opcPartUri );
    }

    if (SUCCEEDED(hr))
    {
        hr = xpsFactory->CreateDocument( opcPartUri, &xpsFD );
        opcPartUri->Release();
        opcPartUri = NULL;
    }

    if (SUCCEEDED(hr))
    {
        hr = xpsFactory->CreatePageReference( &pageSize, &xpsPageRef );
    }

    if (SUCCEEDED(hr))
    {
        hr = xpsFactory->CreatePartUri( L"/Documents/1/Pages/1.fpage", &opcPartUri);
    }

    if (SUCCEEDED(hr))
    {
        hr = xpsFactory->CreatePage( 
                &pageSize,
                NULL, // language
                opcPartUri, 
                &xpsPage
                );
        opcPartUri->Release();
        opcPartUri = NULL;
    }

    // Chain document trunk objects from package root to fixed page
    if (SUCCEEDED(hr))
    {
        hr = xpsPackage->SetDocumentSequence( xpsFDS );
    }
    if (SUCCEEDED(hr))
    {
        hr = xpsFDS->GetDocuments( &fixedDocuments );
    }
    if (SUCCEEDED(hr))
    {
        hr = fixedDocuments->Append( xpsFD );
    }
    if (SUCCEEDED(hr))
    {
        hr = xpsFD->GetPageReferences( &pageRefs );
    }
    if (SUCCEEDED(hr))
    {
        hr = pageRefs->Append( xpsPageRef );
    }
    if (SUCCEEDED(hr))
    {
        hr = xpsPageRef->SetPage( xpsPage );
    }
    if (SUCCEEDED(hr))
    {
        hr = xpsPage->SetLanguage(L"en-US");
    }

    // Add canvas to page visuals
    if (SUCCEEDED(hr))
    {
        hr = xpsPage->GetVisuals( &pageVisuals );
    }
    if (SUCCEEDED(hr))
    {
        hr = pageVisuals->Append( canvas );
    }

    // Save XPS OM to file
    if (SUCCEEDED(hr))
    {
        hr = SHGetFolderPath(0, CSIDL_DESKTOPDIRECTORY, 0, SHGFP_TYPE_CURRENT, szDesktopPath);
    }
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(szDesktopPath, ARRAYSIZE(szDesktopPath), L"\\SDKSample_FlowText_Output.xps");
    }

    if (SUCCEEDED(hr))
    {
        hr = xpsPackage->WriteToFile(
                szDesktopPath,
                NULL, // LPSECURITY_ATTRIBUTES
                FILE_ATTRIBUTE_NORMAL,
                FALSE // optimizeMarkupSize
                );
    }

    // release resources
    if (xpsPackage) 
    {
        xpsPackage->Release();
        xpsPackage = NULL;
    }
    if (xpsFDS) 
    {
        xpsFDS->Release();
        xpsFDS = NULL;
    }
    if (fixedDocuments) 
    {
        fixedDocuments->Release();
        fixedDocuments = NULL;
    }
    if (xpsFD) 
    {
        xpsFD->Release();
        xpsFD = NULL;
    }
    if (pageRefs) 
    {
        pageRefs->Release();
        pageRefs = NULL;
    }
    if (xpsPageRef) 
    {
        xpsPageRef->Release();
        xpsPageRef = NULL;
    }
    if (xpsPage) 
    {
        xpsPage->Release();
        xpsPage = NULL;
    }
    if (opcPartUri) 
    {
        opcPartUri->Release();
        opcPartUri = NULL;
    }
    if (pageVisuals) 
    {
        pageVisuals->Release();
        pageVisuals = NULL;
    }

    return hr;
}

int
wmain()
{
    HRESULT hr = S_OK;
    bool bCOMInitialized = false;

    IXpsOMObjectFactory* xpsFactory = NULL;
    IDWriteTextLayout* textLayout = NULL;
    LayoutToCanvasBuilder *pCanvasBuilder = NULL;

    hr = CoInitialize(0);

    if (SUCCEEDED(hr))
    {
        bCOMInitialized = true;
        hr = CoCreateInstance(
                __uuidof(XpsOMObjectFactory), 
                NULL, 
                CLSCTX_INPROC_SERVER,
                __uuidof(IXpsOMObjectFactory), 
                reinterpret_cast<LPVOID*>(&xpsFactory)
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = MakeTextLayout(&textLayout);
    }

    if (SUCCEEDED(hr))
    {
        hr = LayoutToCanvasBuilder::CreateInstance( xpsFactory, &pCanvasBuilder );
    }

    if (SUCCEEDED(hr))
    {        
        hr = textLayout->Draw(
                NULL, //clientDrawingContext
                pCanvasBuilder, // IDWriteTextRenderer implementation
                15.0, // originX
                15.0 // originY
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = SaveCanvasToXpsViaPackage( 
                xpsFactory, 
                pCanvasBuilder->GetCanvas()
                );
        // pCanvasBuilder->GetResources()  -  xpsPartResources object is also available if streaming serialization is preferred
    }

    // release resources
    if (pCanvasBuilder) 
    {
        pCanvasBuilder->Release();
        pCanvasBuilder = NULL;
    }
    if (textLayout) 
    {
        textLayout->Release();
        textLayout = NULL;
    }
    if (xpsFactory) 
    {
        xpsFactory->Release();
        xpsFactory = NULL;
    }
    if (bCOMInitialized) 
    {
        CoUninitialize();
        bCOMInitialized = false;
    }

    return SUCCEEDED(hr) ? 0 : 1;
}
