// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// Application which demonstrates the use of IThumbnailProvider
// to retrieve a thumbnail for an image file.
// This application takes two command line arguments.
// The first argument should be the name of the
// image file to retrieve a thumbnail for, and the second
// argument should be a number indicating the size of the
// thumbnail to retrieve.

#include <windows.h>
#include <shlobj.h>
#include <thumbcache.h>
#include <GdiPlus.h>

HBITMAP g_hThumbnail;           // Thumbnail to create

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // The GDI+ objects provide an operator new implementation (in GdiplusBase) and that does
            // not throw. Thus clients must check the return value from new rather than catch this failure
            // as an exception
            Gdiplus::Graphics *pGraphics = new Gdiplus::Graphics(hdc);
            if (pGraphics)
            {
                Gdiplus::Bitmap * pBitmap = Gdiplus::Bitmap::FromHBITMAP(g_hThumbnail, NULL);
                if (pBitmap)
                {
                    pGraphics->DrawImage(pBitmap, 0, 0);
                    delete pBitmap;
                }
                pBitmap = NULL;
                delete pGraphics;
            }

            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pszCmdLine, int nCmdShow)
{
    Gdiplus::GdiplusStartupInput startupInput;
    ULONG_PTR           gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &startupInput, NULL);

    int numArgs;
    PWSTR *ppszArgs = CommandLineToArgvW(pszCmdLine, &numArgs);
    if (ppszArgs && (numArgs == 2))
    {
        PCWSTR pszSize = ppszArgs[0];
        PCWSTR pszFile = ppszArgs[1];

        int nSize = _wtoi(pszSize);     // Size of thumbnail

        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr))
        {
            IShellItem *psi;
            hr = SHCreateItemFromParsingName(pszFile, NULL, IID_PPV_ARGS(&psi));
            if (SUCCEEDED(hr))
            {
                IThumbnailProvider *pThumbProvider;
                hr = psi->BindToHandler(NULL, BHID_ThumbnailHandler, IID_PPV_ARGS(&pThumbProvider));
                if (SUCCEEDED(hr))
                {
                    WTS_ALPHATYPE wtsAlpha;
                    hr = pThumbProvider->GetThumbnail(nSize, &g_hThumbnail, &wtsAlpha);
                    if (SUCCEEDED(hr))
                    {
                        WNDCLASSEX wcex = { sizeof(wcex) };
                        wcex.lpfnWndProc = WndProc;
                        wcex.hInstance = hInstance;
                        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
                        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
                        wcex.lpszClassName = L"ThumbnailAppClass";

                        RegisterClassEx(&wcex);

                        HWND hWnd = CreateWindowEx(0, wcex.lpszClassName, L"Thumbnail Provider SDK Sample",
                            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                            nSize, nSize, NULL, NULL, hInstance, NULL);
                        if (hWnd)
                        {
                            ShowWindow(hWnd, nCmdShow);

                            MSG msg;
                            while (GetMessage(&msg, NULL, 0, 0))
                            {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                        }
                        DeleteObject(g_hThumbnail);
                    }
                    pThumbProvider->Release();
                }
                psi->Release();
            }
            CoUninitialize();
        }
    }
    else
    {
        MessageBox(NULL, L"Usage: ThumbnailProvider.exe <size> <Absolute Path to file>", L"Wrong number of arguments.", MB_OK);
    }

    if (ppszArgs)
    {
        LocalFree(ppszArgs);
    }

    Gdiplus::GdiplusShutdown(gdiplusToken); // shut down GDIPlus
    return 0;
}
