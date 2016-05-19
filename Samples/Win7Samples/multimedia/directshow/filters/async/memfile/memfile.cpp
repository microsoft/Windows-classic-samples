//------------------------------------------------------------------------------
// File: MemFile.cpp
//
// Desc: DirectShow sample code - application using async filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include <tchar.h>
#include <stdio.h>

#include "asyncio.h"
#include "asyncrdr.h"
#include "memfile.h"

/*  Function prototypes */
HRESULT SelectAndRender(CMemReader *pReader, IFilterGraph **pFG);
HRESULT PlayFileWait(IFilterGraph *pFG);


/*  Read a file into memory, play it (or part of it), then exit */

int __cdecl _tmain( int argc, TCHAR* argv[] )
{
    if(argc < 2 || argc > 3)
    {
        _tprintf(_T("Usage : memfile FileName <Kbytes per sec>\n"));
        return 0;
    }

    DWORD dwKBPerSec = (argc == 2 ? INFINITE : _ttoi(argv[2]));
    LPTSTR lpType;

    CMediaType mt;
    mt.majortype = MEDIATYPE_Stream;

    /*  Find the extension */
    int len = lstrlen(argv[1]);
    if(len >= 4 && argv[1][len - 4] == TEXT('.'))
    {
        lpType = argv[1] + len - 3;
    }
    else
    {
        _tprintf(_T("Invalid file extension\n"));
        return 1;
    }

    /* Set subtype based on file extension */
    if(lstrcmpi(lpType, TEXT("mpg")) == 0) {
        mt.subtype = MEDIASUBTYPE_MPEG1System;
    }
    else if(lstrcmpi(lpType, TEXT("mpa")) == 0) {
        mt.subtype = MEDIASUBTYPE_MPEG1Audio;
    }
    else if(lstrcmpi(lpType, TEXT("mpv")) == 0) {
        mt.subtype = MEDIASUBTYPE_MPEG1Video;
    }
    else if(lstrcmpi(lpType, TEXT("dat")) == 0) {
        mt.subtype = MEDIASUBTYPE_MPEG1VideoCD;
    }
    else if(lstrcmpi(lpType, TEXT("avi")) == 0) {
        mt.subtype = MEDIASUBTYPE_Avi;
    }
    else if(lstrcmpi(lpType, TEXT("mov")) == 0) {
        mt.subtype = MEDIASUBTYPE_QTMovie;
    }
    else if(lstrcmpi(lpType, TEXT("wav")) == 0) {
        mt.subtype = MEDIASUBTYPE_WAVE;
    }
    else
    {
        _tprintf(_T("Unknown file type: %s\n"), lpType);
        return 1;
    }

    /*  Open the file */
    HANDLE hFile = CreateFile(argv[1],
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("Could not open %s\n"), argv[1]);
        return 1;
    }

    /* Determine the file size */
    ULARGE_INTEGER uliSize;
    uliSize.LowPart = GetFileSize(hFile, &uliSize.HighPart);

    /* Allocate a buffer to hold the file's data */
    PBYTE pbMem = new BYTE[uliSize.LowPart];
    if(pbMem == NULL)
    {
        _tprintf(_T("Could not allocate %d bytes\n"), uliSize.LowPart);
        return 1;
    }

    DWORD dwBytesRead;

    if(!ReadFile(hFile,
                (LPVOID)pbMem,
                uliSize.LowPart,
                &dwBytesRead,
                NULL) ||
        (dwBytesRead != uliSize.LowPart))
    {
        _tprintf(_T("Could not read file\n"));
        CloseHandle(hFile);
        return 1;
    }

    CloseHandle(hFile);

    HRESULT hr = S_OK;

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    CMemStream Stream(pbMem, (LONGLONG)uliSize.QuadPart, dwKBPerSec);

    CMemReader *rdr = new CMemReader(&Stream, &mt, &hr);
    if(FAILED(hr) || rdr == NULL)
    {
        delete rdr;
        _tprintf(_T("Could not create filter - HRESULT 0x%8.8X\n"), hr);
        CoUninitialize();
        return 1;
    }

    //  Make sure we don't accidentally go away!
    rdr->AddRef();

    IFilterGraph *pFG = NULL;
    hr = SelectAndRender(rdr, &pFG);

    if(FAILED(hr))
    {
        _tprintf(_T("Failed to create graph and render file - HRESULT 0x%8.8X"), hr);
    }
    else
    {
        //  Play the file
        HRESULT hr = PlayFileWait(pFG);
        if(FAILED(hr))
        {
            _tprintf(_T("Failed to play graph - HRESULT 0x%8.8X"), hr);
        }
    }

    rdr->Release();

    if(pFG)
    {
        ULONG ulRelease = pFG->Release();
        if(ulRelease != 0)
        {
            _tprintf(_T("Filter graph count not 0! (was %d)"), ulRelease);
        }
    }

    CoUninitialize();
    return 0;
}


//  Select a filter into a graph and render its output pin,
//  returning the graph

HRESULT SelectAndRender(CMemReader *pReader, IFilterGraph **ppFG)
{
    CheckPointer(pReader,E_POINTER);
    CheckPointer(ppFG,E_POINTER);

    /*  Create filter graph */
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                                  IID_IFilterGraph, (void**) ppFG);
    if(FAILED(hr))
    {
        return hr;
    }

    /*  Add our filter */
    hr = (*ppFG)->AddFilter(pReader, NULL);

    if(FAILED(hr))
    {
        return hr;
    }

    /*  Get a GraphBuilder interface from the filter graph */
    IGraphBuilder *pBuilder;

    hr = (*ppFG)->QueryInterface(IID_IGraphBuilder, (void **)&pBuilder);
    if(FAILED(hr))
    {
        return hr;
    }

    /*  Render our output pin */
    hr = pBuilder->Render(pReader->GetPin(0));

    /* Release interface and return */
    pBuilder->Release();
    return hr;
}


HRESULT PlayFileWait(IFilterGraph *pFG)
{
    CheckPointer(pFG,E_POINTER);

    HRESULT hr;
    IMediaControl *pMC=0;
    IMediaEvent   *pME=0;

    hr = pFG->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(FAILED(hr))
    {
        return hr;
    }

    hr = pFG->QueryInterface(IID_IMediaEvent, (void **)&pME);
    if(FAILED(hr))
    {
        pMC->Release();
        return hr;
    }

    OAEVENT oEvent;
    hr = pME->GetEventHandle(&oEvent);
    if(SUCCEEDED(hr))
    {
        hr = pMC->Run();

        if(SUCCEEDED(hr))
        {
            LONG levCode;
            hr = pME->WaitForCompletion(INFINITE, &levCode);
        }
    }

    pMC->Release();
    pME->Release();

    return hr;
}

