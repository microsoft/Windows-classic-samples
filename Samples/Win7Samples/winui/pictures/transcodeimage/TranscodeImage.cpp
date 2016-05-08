//////////////////////////////////////////////////////////////////////
//
// TranscodeImage.cpp : Demonstrates the use of the TranscodeImage method.
// The application takes two command line arguments.  
// First argument: the path of the image file to transcode.
// Second argument: the path at which to save the transcoded file.
//
// THIS CODE AND INIFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE. 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <tchar.h>
#include <imagetranscode.h>
#include <shlobj.h>
#include <shlwapi.h>  // needed for SHCreateStreamOnFileW

// forward declarations
BOOL WriteImageStreamToFile(LPWSTR filename, IStream* pStream);
void DisplayErrorMessage(DWORD errorCode);

// constants

// Transcoded image will fit in a box of the requested height and width
const int REQUESTED_WIDTH = 100;  
const int REQUESTED_HEIGHT = 100; 
// Requested format may be TI_BITMAP or TI_JPEG. 
const TI_FLAGS REQUESTED_FORMAT = TI_BITMAP; 

int _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr = S_OK;
    // ITranscodeImage object
    ITranscodeImage *pTransImg = NULL;
    // Used for creating an IShellItem object representing the image.
    LPITEMIDLIST pItemIdList = NULL;
    // IShellItem representing the image to pass to TranscodeImage
    IShellItem *pItemToTranscode = NULL;
    //command line arguments
    LPWSTR pwszCommandLineArg1 = NULL;
    LPWSTR pwszCommandLineArg2 = NULL;
    // dimensions of the converted image
    UINT uActualX = 0;
    UINT uActualY = 0;
    // Handle of new file
    HANDLE hFile = NULL;
    // A stream to hold the transcoded image
    IStream* pImgStream = NULL;


    if (argc == 3)
    {
        pwszCommandLineArg1 = argv[1];
        pwszCommandLineArg2 = argv[2];
    } else // TODO: accept size arguments
    {
        wprintf(L"%s %s %s\n", L"Wrong # of arguments.  Usage:", argv[0], L"sourceimagepath destinationimagepath");
        hr = E_INVALIDARG;
        return -1;
    }

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if SUCCEEDED(hr)
    {
    hr = CoCreateInstance(
        CLSID_ImageTranscode, 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_ITranscodeImage, 
        (LPVOID*)&pTransImg);
    }

    // Get a pointer to an ITEMIDLIST by parsing the first command line argument
    if SUCCEEDED(hr)
    {
        hr = SHParseDisplayName(pwszCommandLineArg1, NULL, &pItemIdList, 0, NULL);
    }

    // Create an IShellItem object with the ITEMIDLIST you created above.
    if SUCCEEDED(hr)
    {
        hr = SHCreateShellItem(NULL, NULL, pItemIdList, &pItemToTranscode);
    }

    // Create a file
    if SUCCEEDED(hr)
    {
        hFile = CreateFileW(pwszCommandLineArg2, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            
        }
        CloseHandle(hFile);
    }

    // Create a stream on a file to pass to TranscodeImage later.
    if SUCCEEDED(hr)
    {
        hr = SHCreateStreamOnFileW(pwszCommandLineArg2, STGM_READWRITE, &pImgStream);
    }

    // Transcode the image to the indicated format,
    // which is either TI_BITMAP, or TI_JPEG
    // and resize it to REQUESTED_WIDTH x REQUESTED_WIDTH.
    if SUCCEEDED(hr)
    {
        hr = pTransImg->TranscodeImage(
            pItemToTranscode, 
            REQUESTED_WIDTH, 
            REQUESTED_HEIGHT, 
            REQUESTED_FORMAT,
            pImgStream, 
            &uActualX, 
            &uActualY);
    }

    // Write the stream containing the transcoded image to the destination file.
    if SUCCEEDED(hr)
    {
        hr = pImgStream->Commit(STGC_DEFAULT);
    }

    // Release the stream.
    if (NULL != pImgStream)
    {
        pImgStream->Release();
    }

    if FAILED(hr)
    {
        DisplayErrorMessage(hr);
    }
    CoUninitialize();
    return 0;
}


void DisplayErrorMessage(DWORD errorCode)
{
        WCHAR lpMsgBuf[256];
        ZeroMemory(lpMsgBuf, 256 * sizeof(WCHAR));
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            lpMsgBuf,
            256, // size of output buffer 
            NULL );
        MessageBox(NULL, lpMsgBuf, L"Error", MB_OK);

}



