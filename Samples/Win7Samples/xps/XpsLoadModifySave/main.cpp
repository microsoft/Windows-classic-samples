
//+----------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Abstract:
//      A sample demonstrating use of the XPS Object Model to load, modify and save
//      XPS documents.
//-----------------------------------------------------------------------------

#ifndef _WIN32_WINNT            
#define _WIN32_WINNT 0x0601     // Specifies that the minimum required platform is Windows 7.
#endif


#include <stdio.h>
#define NOMINMAX
#include <windows.h>
#include <comdef.h>
#include <XpsObjectModel.h>

#include <StrSafe.h>
#include <shlobj.h>

#include "resource.h"


#define CCHOF(x) (sizeof(x)/sizeof(*(x)))

void __stdcall _com_issue_error(HRESULT hr)
{
    fwprintf(stderr, L"ERROR: _com_issue_error called with HRESULT 0x%X\n", hr);
    // _exit terminates the process immediately, without calling any C++ destructors.
    _exit(1);
}

HRESULT
LoadAttachedResourceToMemory(
    LPCTSTR               streamResourceId,
    LPCTSTR               lpType,
    PVOID                 *ppData,
    PDWORD                pdwSize 
    )
{
    DWORD           dwSize  = 0;
    HGLOBAL         hData   = NULL; // Not really HGlobal. See MSDN's LockResource for details.
    PVOID           pData   = NULL; // Doesn't need to be released.
    HRESULT         hr      = S_OK;
    HMODULE     hTestModule = NULL;

    HRSRC hRes = FindResource(hTestModule, streamResourceId, lpType);

    if (!hRes)
    {
        hr = E_FAIL;
    }

    if ( SUCCEEDED (hr) )
    {
        hData = LoadResource(hTestModule, hRes);
        if (!hData)
        {
            hr = E_FAIL;
        }
    }

    if ( SUCCEEDED(hr) )
    {
        dwSize = SizeofResource(hTestModule, hRes);
        if (dwSize <= 0)
        {
            hr = E_FAIL;
        }
    }

    if ( SUCCEEDED(hr) )
    {
        pData = LockResource(hData);
        if (!pData)
        {
            hr = E_FAIL;
        }
    }

    *ppData = pData;
    *pdwSize = dwSize;
    return hr;
}

HRESULT GetReadStreamFromAttachedResource(
    LPCTSTR           lpName,
    LPCTSTR           lpType,
    IStream         **ppReadStream,
    PDWORD            pdwSize
    )
{
    PVOID           pBuf = NULL; 
    PVOID           pData = NULL;
    HRESULT         hr    = S_OK;
    IStream        *pReadStream = NULL;

    hr = LoadAttachedResourceToMemory(lpName, lpType, &pData, pdwSize);
    
    if ( SUCCEEDED(hr) && pData)
    {
        if ( pData )
        {
            pBuf = GlobalLock(GlobalAlloc(GMEM_MOVEABLE, *pdwSize));
        }

        if ( pBuf )
        {
            CopyMemory(pBuf, pData, *pdwSize);
        }

        if ( pBuf && pData )
        {
            hr = CreateStreamOnHGlobal(pBuf, TRUE, &pReadStream);
        }
    }

    if (FAILED(hr) )
    {
        if ( pBuf )
        {
            GlobalFree(pBuf);
        }
    }
    else
    {
        *ppReadStream = pReadStream;
    }
    return hr;
    
    
}

HRESULT
GetXpsFactory(
    IXpsOMObjectFactory **factory
    )
{
    return CoCreateInstance(
                __uuidof(XpsOMObjectFactory),
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(IXpsOMObjectFactory),
                reinterpret_cast<LPVOID*>(factory)
                );
}

HRESULT 
LoadXpsPackage(
    WORD                   xpsStreamResourceId,
    IXpsOMObjectFactory   *pXpsFactory,
    IXpsOMPackage        **ppXpsPackage
    )
{
    HRESULT      hr = S_OK;
    IStream     *pStream = NULL;
    DWORD        size;


    hr = GetReadStreamFromAttachedResource(
            reinterpret_cast<LPCWSTR>(xpsStreamResourceId),
            reinterpret_cast<LPCWSTR>(RC_PACKAGE_DATA),
            &pStream,
            &size
            );

	if ( SUCCEEDED(hr) )
	{
        hr =  pXpsFactory->CreatePackageFromStream(
                    pStream,
                    FALSE,
                    ppXpsPackage
                    );
    }

    if ( pStream )
    {
        pStream->Release();
        pStream = NULL;
    }

    return hr;
}


HRESULT
FindFirstPageReference(
    IXpsOMObjectFactory*    /* pXpsFactory */,
    IXpsOMPackage          *pXpsPackage,
    IXpsOMPageReference   **ppXpsPageReference
    )
{
    HRESULT                          hr                     = S_OK;
    IXpsOMDocumentSequence          *pXpsDocSeq             = NULL;
    IXpsOMDocumentCollection        *pXpsDocCollection      = NULL;
    IXpsOMDocument                  *pXpsDocument           = NULL;
    IXpsOMPageReferenceCollection   *pXpsPageRefCollection  = NULL;


    hr = pXpsPackage->GetDocumentSequence(&pXpsDocSeq);
    
    if (SUCCEEDED(hr) )
    {
        hr = pXpsDocSeq->GetDocuments(&pXpsDocCollection);
    }

    if (SUCCEEDED(hr) )
    {
        hr = pXpsDocCollection->GetAt(0, &pXpsDocument);
    }

    if (SUCCEEDED(hr) )
    {
        hr = pXpsDocument->GetPageReferences(&pXpsPageRefCollection);
    }

    if (SUCCEEDED(hr) )
    {
        hr = pXpsPageRefCollection->GetAt(0, ppXpsPageReference);
    }


    if ( pXpsPageRefCollection )
    {
        pXpsPageRefCollection->Release();
        pXpsPageRefCollection = NULL;
    }

    if ( pXpsDocument )
    {
        pXpsDocument->Release();
        pXpsDocument = NULL;
    }

    if ( pXpsDocCollection )
    {
        pXpsDocCollection->Release();
        pXpsDocCollection = NULL;
    }

    if ( pXpsDocSeq )
    {
        pXpsDocSeq->Release();
        pXpsDocSeq = NULL;
    }

    return hr;
}

HRESULT
CreateWaterMark(
    IXpsOMObjectFactory   *pXpsFactory,
    IXpsOMFontResource    *pXpsFontResource,
    IXpsOMCanvas         **ppXpsCanvas
    )
{

    HRESULT                  hr = S_OK;
    static const XPS_POINT   glyphOrigin            = {50.0, 500.0};
    static const FLOAT       fontRenderingSize      = 48.0;
    IXpsOMGlyphs             *pXpsGlyphs            = NULL;
    IXpsOMGlyphsEditor       *pXpsGlyphsEditor      = NULL;
    IXpsOMSolidColorBrush    *pXpsSolidColor        = NULL;
    IXpsOMVisualCollection   *pXpsVisualCollection  = NULL;

    hr = pXpsFactory->CreateGlyphs(pXpsFontResource, &pXpsGlyphs);

    if (SUCCEEDED(hr) )
    {
        hr = pXpsGlyphs->GetGlyphsEditor(&pXpsGlyphsEditor);
    }

    if (SUCCEEDED(hr) )
    {
        hr = pXpsGlyphsEditor->SetUnicodeString(L"Draft");
    }

    if (SUCCEEDED(hr) )
    {
        hr = pXpsGlyphsEditor->ApplyEdits();
    }

    if (SUCCEEDED(hr) )
    {
        hr = pXpsGlyphs->SetOrigin(&glyphOrigin);
    }

    if (SUCCEEDED(hr) )
    {
        hr = pXpsGlyphs->SetFontRenderingEmSize(fontRenderingSize);
    }

    if (SUCCEEDED(hr) )
    {
        XPS_COLOR blackColor;
        blackColor.colorType        = XPS_COLOR_TYPE_SRGB;
        blackColor.value.sRGB.alpha = 0xff; //Opaque
        blackColor.value.sRGB.red   = 0x00;
        blackColor.value.sRGB.green = 0x00;
        blackColor.value.sRGB.blue  = 0x00;
        hr = pXpsFactory->CreateSolidColorBrush(&blackColor, NULL, &pXpsSolidColor);
    }

    if (SUCCEEDED(hr) )
    {
        hr = pXpsGlyphs->SetFillBrushLocal(pXpsSolidColor);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = pXpsFactory->CreateCanvas(ppXpsCanvas);
    }

    //Next 2 steps add Glyph to Canvas
    if (SUCCEEDED(hr) )
    {
        hr = (*ppXpsCanvas)->GetVisuals(&pXpsVisualCollection);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = pXpsVisualCollection->Append(pXpsGlyphs);
    }
   
    if ( pXpsSolidColor )
    {
        pXpsSolidColor->Release();
        pXpsSolidColor = NULL;
    }

    if ( pXpsGlyphsEditor )
    {
        pXpsGlyphsEditor->Release();
        pXpsGlyphsEditor = NULL;
    }

    if ( pXpsGlyphs )
    {
        pXpsGlyphs->Release();
        pXpsGlyphs = NULL;
    }

    if ( pXpsVisualCollection )
    {
        pXpsVisualCollection->Release();
        pXpsVisualCollection = NULL;
    }

    return hr;
}

HRESULT
AddWatermarkToPage(
    IXpsOMPage      *pXpsPage,
    IXpsOMCanvas    *pXpsCanvas
    )
{
    IXpsOMVisualCollection *pVisualCollection = NULL;

    HRESULT hr =  pXpsPage->GetVisuals(&pVisualCollection);

    if ( SUCCEEDED(hr) )
    {
        hr = pVisualCollection->Append(pXpsCanvas);
    }

    if ( pVisualCollection )
    {
        pVisualCollection->Release();
        pVisualCollection = NULL;
    }

    return hr;
}

HRESULT
GetFontResource(
    IXpsOMPageReference      *pXpsPageReference,
    IXpsOMFontResource     **ppXpsFontResource
    )
{
    HRESULT                          hr                     = S_OK;
    IXpsOMPartResources             *pXpsPartResources      = NULL;
    IXpsOMFontResourceCollection    *pXpsFontResCollection  = NULL;

    if (SUCCEEDED(hr) )
    {
        hr = pXpsPageReference->CollectPartResources(&pXpsPartResources);
    }

    if (SUCCEEDED(hr) )
    {
        hr = pXpsPartResources->GetFontResources(&pXpsFontResCollection);
    }

    if (SUCCEEDED(hr) )
    {
        hr = pXpsFontResCollection->GetAt(0, ppXpsFontResource);
    }

    if ( pXpsFontResCollection )
    {
        pXpsFontResCollection->Release();
        pXpsFontResCollection = NULL;
    }

    if ( pXpsPartResources )
    {
        pXpsPartResources->Release();
        pXpsPartResources = NULL;
    }

    return hr;
}

HRESULT
SerializePackage(
    IXpsOMPackage *pXpsPackage
    )
{
    WCHAR szDesktopPath[MAX_PATH];
    HRESULT hr = SHGetFolderPath(0, CSIDL_DESKTOPDIRECTORY, 0, SHGFP_TYPE_CURRENT, szDesktopPath);

    if ( SUCCEEDED(hr) )
    {
        hr = StringCchCat(szDesktopPath, CCHOF(szDesktopPath), L"\\SDKSample_XpsLoadModifySave.xps");
    }

    if ( SUCCEEDED(hr) )
    {
        hr = pXpsPackage->WriteToFile(szDesktopPath, NULL, FILE_ATTRIBUTE_NORMAL, FALSE);
    }
    return hr;
}

int
wmain(
    int    /* argc   */, 
    wchar_t* /* argv[] */
)
{
    HRESULT                 hr;

    IXpsOMCanvas            *pXpsCanvas         = NULL;
    IXpsOMPackage           *pXpsPackage        = NULL;
    IXpsOMObjectFactory     *pXpsFactory        = NULL;
    IXpsOMPage              *pXpsPage           = NULL;
    IXpsOMPageReference     *pXpsPageReference  = NULL;
    IXpsOMFontResource      *pXpsFontResource   = NULL;


    if (FAILED(hr = CoInitializeEx(0, COINIT_MULTITHREADED)))
    {
        fwprintf(stderr, L"ERROR: CoInitializeEx failed with HRESULT 0x%X\n", hr);
        return 1;
    }

    hr = GetXpsFactory(&pXpsFactory);
    if (!SUCCEEDED(hr))
    {
        fwprintf(stderr, L"ERROR: Could not create XPS OM Object Factory.\n");
    }
    else
    {
        hr = LoadXpsPackage(IDR_RC_PACKAGE_DATA1,
                pXpsFactory,
                &pXpsPackage);
    }

    if ( !SUCCEEDED(hr) )
    {
        fwprintf(stderr, L"ERROR: Could not load xps package.\n");
    }
    else
    {
        hr = FindFirstPageReference(pXpsFactory, pXpsPackage, &pXpsPageReference);
    }

    if (SUCCEEDED(hr) )
    {
        hr = GetFontResource(pXpsPageReference, &pXpsFontResource);
    }

    if ( SUCCEEDED(hr) )
    {
         hr = CreateWaterMark(pXpsFactory, pXpsFontResource, &pXpsCanvas);
    }

    if (SUCCEEDED(hr) )
    {
        hr = pXpsPageReference->GetPage(&pXpsPage);
    }

    if (SUCCEEDED(hr) )
    {
        hr = AddWatermarkToPage(pXpsPage, pXpsCanvas);
    }

    if (SUCCEEDED(hr) )
    {
        hr = SerializePackage(pXpsPackage);
    }

    if ( pXpsPage )
    {
        pXpsPage->Release();
        pXpsPage = NULL;
    }

    if ( pXpsPageReference )
    {
        pXpsPageReference->Release();
        pXpsPageReference = NULL;
    }

    if ( pXpsFontResource )
    {
        pXpsFontResource->Release();
        pXpsFontResource = NULL;
    }


    if ( pXpsCanvas )
    {
        pXpsCanvas->Release();
        pXpsCanvas = NULL;
    }

    
    if ( pXpsPackage )
    {
        pXpsPackage->Release();
        pXpsPackage = NULL;
    }

    if ( pXpsFactory )
    {
        pXpsFactory->Release();
        pXpsFactory = NULL;
    }

    CoUninitialize();
    return SUCCEEDED(hr) ? 0 : 1;
}

