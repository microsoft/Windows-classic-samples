//+----------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Abstract:
//      A sample demonstrating use of the XPS Object Model with the XPS Print
//      API.
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <comdef.h>
#include <windows.h>
#include <t2embapi.h>
#include <XpsObjectModel.h>
#include <XpsPrint.h>

// The font to use for the page text.  Needs to be a TrueType font.
const wchar_t g_fontName[] = L"Arial";

// This function creates a font resource for the given font name with the 
// embedding type set appropriately for the licensing intent of the font.
HRESULT
CreateFontResourceForFont(
    IXpsOMObjectFactory* xpsFactory,
    LPCWSTR fontName,
    IXpsOMFontResource **fontResource
    )
{
    HRESULT hr = S_OK;
    HDC hDC = NULL;
    LOGFONT logFont = {};
    HGDIOBJ hFont = NULL;
    ULONG privStatus = 0;
    LONG ttStatus;
    XPS_FONT_EMBEDDING embedding = XPS_FONT_EMBEDDING_NORMAL;
    DWORD fontDataLen = GDI_ERROR;
    HGLOBAL hGlobal = NULL;
    LPVOID ptr = NULL;
    IStream *fontStream = NULL;
    GUID fontObfuscationGuid = {};
    IOpcPartUri* partUri = NULL;
    wchar_t fontPartName[60];

    hDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
    if (!hDC)
    {
        fwprintf(stderr, L"ERROR: Could not create device context\n");
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        if (wcscpy_s(logFont.lfFaceName, fontName) != 0)
        {
            fwprintf(stderr, L"ERROR: Could not copy font name into LOGFONT structure\n");
            hr = E_UNEXPECTED;
        }
    }

    if (SUCCEEDED(hr))
    {
        hFont = CreateFontIndirect(&logFont);
        if (!hFont)
        {
            fwprintf(stderr, L"ERROR: Could not create font\n");
            hr = E_UNEXPECTED;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (!SelectObject(hDC, hFont))
        {
            fwprintf(stderr, L"ERROR: Could not select object\n");
            hr = E_UNEXPECTED;
        }
    }

    if (SUCCEEDED(hr))
    {
        if ((ttStatus = TTGetEmbeddingType(hDC, &privStatus)) != E_NONE)
        {
            if (ttStatus == E_NOTATRUETYPEFONT)
            {
                fwprintf(stderr, L"ERROR: %s is not a TrueType font\n", fontName);
            }
            else
            {
                fwprintf(stderr, L"ERROR: Could not get embedding type: %08X\n", ttStatus);
            }
            hr = E_UNEXPECTED;
        }
    }

    if (SUCCEEDED(hr))
    {
        switch (privStatus)
        {
            case EMBED_PREVIEWPRINT:
                // Restricted font
                embedding = XPS_FONT_EMBEDDING_RESTRICTED;
                break;
            case EMBED_EDITABLE:
                // Editable font - MUST obfuscate
                embedding = XPS_FONT_EMBEDDING_OBFUSCATED;
                break;
            case EMBED_INSTALLABLE:
                // Installable font - SHOULD obfuscate
                embedding = XPS_FONT_EMBEDDING_OBFUSCATED;
                break;
            case EMBED_NOEMBEDDING:
                fwprintf(stderr, L"ERROR: %s may not be embedded\n", fontName);
                hr = E_UNEXPECTED;
                break;
            default:
                fwprintf(stderr, L"ERROR: Unrecognized embedding privileges: %08X\n", privStatus);
                hr =  E_UNEXPECTED;
                break;
        }
    }

    if (SUCCEEDED(hr))
    {
        fontDataLen = GetFontData(hDC, 0, 0, NULL, 0);
        if (fontDataLen == GDI_ERROR)
        {
            fwprintf(stderr, L"ERROR: Could not get length of font data\n");
            hr = E_UNEXPECTED;
        }
    }

    if (SUCCEEDED(hr))
    {
        hGlobal = GlobalAlloc(GMEM_MOVEABLE, fontDataLen);
        if (!hGlobal)
        {
            fwprintf(stderr, L"ERROR: Could not allocate memory with GlobalAlloc\n");
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        ptr = GlobalLock(hGlobal);
        if (!ptr)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            fwprintf(stderr, L"ERROR: Could not lock global memory object: %08X\n", hr);
        }
        else
        {
            if (GetFontData(hDC, 0, 0, ptr, fontDataLen) == GDI_ERROR)
            {
                fwprintf(stderr, L"ERROR: Could not get font data\n");
                hr = E_UNEXPECTED;
            }

            GlobalUnlock(hGlobal);
        }
    }

    // Could perform font subsetting with CreateFontPackage here.

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = CreateStreamOnHGlobal(hGlobal, TRUE, &fontStream)))
        {
            fwprintf(stderr, L"ERROR: Could not create font stream: %08X\n", hr);
        }
        else
        {
            // If CreateStreamOnHGlobal succeeded, the stream now controls the lifetime of the
            // HGLOBAL.
            hGlobal = NULL;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = CoCreateGuid(&fontObfuscationGuid)))
        {
            fwprintf(stderr, L"ERROR: Could not create GUID for obfuscation: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (swprintf_s(
                fontPartName,
                sizeof(fontPartName)/sizeof(wchar_t),
                L"/Resources/Fonts/%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X.odttf",
                fontObfuscationGuid.Data1,
                fontObfuscationGuid.Data2,
                fontObfuscationGuid.Data3,
                fontObfuscationGuid.Data4[0],
                fontObfuscationGuid.Data4[1],
                fontObfuscationGuid.Data4[2],
                fontObfuscationGuid.Data4[3],
                fontObfuscationGuid.Data4[4],
                fontObfuscationGuid.Data4[5],
                fontObfuscationGuid.Data4[6],
                fontObfuscationGuid.Data4[7]
                ) == -1)
        {
            fwprintf(stderr, L"ERROR: Could not format GUID into part name\n");
            hr = E_UNEXPECTED;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = xpsFactory->CreatePartUri(
                    fontPartName,
                    &partUri)))
        {
            fwprintf(stderr, L"ERROR: Could not create part URI: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = xpsFactory->CreateFontResource(
                    fontStream,
                    embedding,
                    partUri,
                    FALSE,
                    fontResource
                    )))
        {
            fwprintf(stderr, L"ERROR: Could not create font resource: %08X\n", hr);
        }
    }

    if (fontStream)
    {
        fontStream->Release();
        fontStream = NULL;
    }

    if (partUri)
    {
        partUri->Release();
        partUri = NULL;
    }

    if (hGlobal)
    {
        GlobalFree(hGlobal);
    }

    if (hFont)
    {
        DeleteObject(hFont);
    }

    if (hDC)
    {
        DeleteDC(hDC);
    }

    return hr;
}

HRESULT
AddGlyphs(
    IXpsOMObjectFactory* xpsFactory,
    IXpsOMPage* page,
    IXpsOMFontResource* font,
    LPCWSTR text,
    XPS_POINT *origin,
    FLOAT fontSize,
    XPS_STYLE_SIMULATION sims
    )
{
    HRESULT hr = S_OK;

    IXpsOMGlyphs* glyphs = NULL;
    IXpsOMGlyphsEditor* glyphsEditor = NULL;
    IXpsOMSolidColorBrush* solidColorBrush = NULL;
    IXpsOMVisualCollection* visuals = NULL;

    if (FAILED(hr = xpsFactory->CreateGlyphs(
                font,
                &glyphs
                )))
    {
        fwprintf(stderr, L"ERROR: Could not create glyphs: %08X\n", hr);
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = glyphs->GetGlyphsEditor(&glyphsEditor)))
        {
            fwprintf(stderr, L"ERROR: Could not get glyphs editor: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = glyphsEditor->SetUnicodeString(
                    text
                    )))
        {
            fwprintf(stderr, L"ERROR: Could not set UnicodeString: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = glyphsEditor->ApplyEdits()))
        {
            fwprintf(stderr, L"ERROR: Could not apply edits: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        XPS_COLOR color;
        color.colorType = XPS_COLOR_TYPE_SRGB;
        color.value.sRGB.alpha = 255;
        color.value.sRGB.red = 0;
        color.value.sRGB.green = 0;
        color.value.sRGB.blue = 0;

        if (FAILED(hr = xpsFactory->CreateSolidColorBrush(
                    &color,
                    NULL,
                    &solidColorBrush
                    )))
        {
            fwprintf(stderr, L"ERROR: Could not create solid color brush: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = glyphs->SetFillBrushLocal(solidColorBrush)))
        {
            fwprintf(stderr, L"ERROR: Could not set fill brush: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = glyphs->SetOrigin(origin)))
        {
            fwprintf(stderr, L"ERROR: Could not set origin: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = glyphs->SetFontRenderingEmSize(fontSize)))
        {
            fwprintf(stderr, L"ERROR: Could not set font size: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = glyphs->SetStyleSimulations(sims)))
        {
            fwprintf(stderr, L"ERROR: Could not set style simulations: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = page->GetVisuals(&visuals)))
        {
            fwprintf(stderr, L"ERROR: Could not get visuals collection: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = visuals->Append(glyphs)))
        {
            fwprintf(stderr, L"ERROR: Could not add glyphs to page: %08X\n", hr);
        }
    }

    if (visuals)
    {
        visuals->Release();
        visuals = NULL;
    }

    if (solidColorBrush)
    {
        solidColorBrush->Release();
        solidColorBrush = NULL;
    }

    if (glyphsEditor)
    {
        glyphsEditor->Release();
        glyphsEditor = NULL;
    }

    if (glyphs)
    {
        glyphs->Release();
        glyphs = NULL;
    }

    return hr;
}

void Usage(wchar_t *argv0)
{
    fwprintf(stderr, L"XPS Object Model Printing Sample\n\n");
    fwprintf(stderr, L"\tUsage: %s <printer name> [<output file name>]\n", argv0);
}

int wmain(int argc, wchar_t* argv[])
{
    HRESULT hr = S_OK;
    HANDLE completionEvent = NULL;
    IXpsPrintJob* job = NULL;
    IXpsPrintJobStream* jobStream = NULL;
    IXpsOMObjectFactory* xpsFactory = NULL;
    IOpcPartUri* partUri = NULL;
    IXpsOMPackageWriter* packageWriter = NULL;
    XPS_SIZE pageSize = {816, 1056};
    IXpsOMPage* xpsPage = NULL;
    IXpsOMFontResource* fontResource = NULL;
    XPS_POINT origin = {50.0f, 200.0f};
    XPS_JOB_STATUS jobStatus = {};

    if (argc < 2 || argc > 3)
    {
        Usage(argv[0]);
        return 1;
    }

    if (FAILED(hr = CoInitializeEx(0, COINIT_MULTITHREADED)))
    {
        fwprintf(stderr, L"ERROR: CoInitializeEx failed with HRESULT 0x%X\n", hr);
        return 1;
    }

    if (SUCCEEDED(hr))
    {
        completionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!completionEvent)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            fwprintf(stderr, L"ERROR: Could not create competion event: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = StartXpsPrintJob(
                    argv[1],
                    NULL,
                    argc == 3 ? argv[2] : NULL,
                    NULL,
                    completionEvent,
                    NULL,
                    0,
                    &job,
                    &jobStream,
                    NULL
                    )))
        {
            fwprintf(stderr, L"ERROR: Could not start XPS print job: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = CoCreateInstance(
                    __uuidof(XpsOMObjectFactory),
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    __uuidof(IXpsOMObjectFactory),
                    reinterpret_cast<void**>(&xpsFactory)
                    )
                )
            )
        {
            fwprintf(stderr, L"ERROR: Could not create XPS OM Object Factory: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = xpsFactory->CreatePartUri(L"/FixedDocumentSequence.fdseq", &partUri)))
        {
            fwprintf(stderr, L"ERROR: Could not create part URI: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = xpsFactory->CreatePackageWriterOnStream(
                    jobStream,
                    TRUE,
                    XPS_INTERLEAVING_ON,
                    partUri,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &packageWriter
                    )
                )
           )
        {
            fwprintf(stderr, L"ERROR: Could not create package writer: 0x%X\n", hr);
        }
    }

    if (partUri)
    {
        partUri->Release();
        partUri = NULL;
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = xpsFactory->CreatePartUri(L"/Documents/1/FixedDocument.fdoc", &partUri)))
        {
            fwprintf(stderr, L"ERROR: Could not create part URI: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = packageWriter->StartNewDocument(partUri, NULL, NULL, NULL, NULL)))
        {
            fwprintf(stderr, L"ERROR: Could not start new document: 0x%X\n", hr);
        }
    }

    if (partUri)
    {
        partUri->Release();
        partUri = NULL;
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = xpsFactory->CreatePartUri(L"/Documents/1/Pages/1.fpage", &partUri)))
        {
            fwprintf(stderr, L"ERROR: Could not create part URI: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = xpsFactory->CreatePage(&pageSize, L"en-US", partUri, &xpsPage)))
        {
            fwprintf(stderr, L"ERROR: Could not create page: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = CreateFontResourceForFont(xpsFactory, g_fontName, &fontResource)))
        {
            // Error already reported.
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = AddGlyphs(
                    xpsFactory,
                    xpsPage,
                    fontResource,
                    L"XPS",
                    &origin,
                    72.0f,
                    XPS_STYLE_SIMULATION_NONE)))
        {
            // Error already reported.
        }
    }

    if (SUCCEEDED(hr))
    {
        origin.x = 420.0f;
        origin.y = 300.0f;

        if (FAILED(hr = AddGlyphs(
                    xpsFactory,
                    xpsPage,
                    fontResource,
                    L"Print with Confidence",
                    &origin,
                    36.0f,
                    XPS_STYLE_SIMULATION_ITALIC)))
        {
            // Error already reported.
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = packageWriter->AddPage(
                    xpsPage,
                    &pageSize,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    )))
        {
            fwprintf(stderr, L"ERROR: Could not add page to document: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = packageWriter->Close()))
        {
            fwprintf(stderr, L"ERROR: Could not close package writer: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = jobStream->Close()))
        {
            fwprintf(stderr, L"ERROR: Could not close job stream: %08X\n", hr);
        }
    }
    else
    {
        // Only cancel the job if we succeeded in creating one in the first place.
        if (job)
        {
            // Tell the XPS Print API that we're giving up.  Don't overwrite hr with the return from
            // this function.
            job->Cancel();
        }
    }


    if (SUCCEEDED(hr))
    {
        wprintf(L"Waiting for job completion...\n");

        if (WaitForSingleObject(completionEvent, INFINITE) != WAIT_OBJECT_0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            fwprintf(stderr, L"ERROR: Wait for completion event failed: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = job->GetJobStatus(&jobStatus)))
        {
            fwprintf(stderr, L"ERROR: Could not get job status: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        switch (jobStatus.completion)
        {
            case XPS_JOB_COMPLETED:
                break;
            case XPS_JOB_CANCELLED:
                fwprintf(stderr, L"ERROR: job was cancelled\n");
                hr = E_FAIL;
                break;
            case XPS_JOB_FAILED:
                fwprintf(stderr, L"ERROR: Print job failed: %08X\n", jobStatus.jobStatus);
                hr = E_FAIL;
                break;
            default:
                fwprintf(stderr, L"ERROR: unexpected failure\n");
                hr = E_UNEXPECTED;
                break;
        }
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"Done!\n");
    }

    if (fontResource)
    {
        fontResource->Release();
        fontResource = NULL;
    }

    if (xpsPage)
    {
        xpsPage->Release();
        xpsPage = NULL;
    }

    if (packageWriter)
    {
        packageWriter->Release();
        packageWriter = NULL;
    }

    if (partUri)
    {
        partUri->Release();
        partUri = NULL;
    }

    if (xpsFactory)
    {
        xpsFactory->Release();
        xpsFactory = NULL;
    }

    if (jobStream)
    {
        jobStream->Release();
        jobStream = NULL;
    }

    if (job)
    {
        job->Release();
        job = NULL;
    }

    if (completionEvent)
    {
        CloseHandle(completionEvent);
        completionEvent = NULL;
    }

    CoUninitialize();

    return SUCCEEDED(hr) ? 0 : 1;
}
