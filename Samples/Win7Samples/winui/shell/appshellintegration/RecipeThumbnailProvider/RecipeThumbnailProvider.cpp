// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <shlwapi.h>
#include <Wincrypt.h>   // For CryptStringToBinary.
#include <thumbcache.h> // For IThumbnailProvider.
#include <wincodec.h>   // Windows Imaging Codecs
#include <msxml6.h>
#include <new>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "msxml6.lib")

// this thumbnail provider implements IInitializeWithStream to enable being hosted
// in an isolated process for robustness

class CRecipeThumbProvider : public IInitializeWithStream,
                             public IThumbnailProvider
{
public:
    CRecipeThumbProvider() : _cRef(1), _pStream(NULL)
    {
    }

    virtual ~CRecipeThumbProvider()
    {
        if (_pStream)
        {
            _pStream->Release();
        }
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CRecipeThumbProvider, IInitializeWithStream),
            QITABENT(CRecipeThumbProvider, IThumbnailProvider),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        ULONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(IStream *pStream, DWORD grfMode);

    // IThumbnailProvider
    IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha);

private:
    HRESULT _LoadXMLDocument( IXMLDOMDocument **ppXMLDoc);
    HRESULT _GetBase64EncodedImageString(UINT cx, PWSTR *ppszResult);
    HRESULT _GetStreamFromString(PCWSTR pszImageName, IStream **ppStream);

    long _cRef;
    IStream *_pStream;     // provided during initialization.
};

HRESULT CRecipeThumbProvider_CreateInstance(REFIID riid, void **ppv)
{
    CRecipeThumbProvider *pNew = new (std::nothrow) CRecipeThumbProvider();
    HRESULT hr = pNew ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pNew->QueryInterface(riid, ppv);
        pNew->Release();
    }
    return hr;
}

// IInitializeWithStream
IFACEMETHODIMP CRecipeThumbProvider::Initialize(IStream *pStream, DWORD)
{
    HRESULT hr = E_UNEXPECTED;  // can only be inited once
    if (_pStream == NULL)
    {
        // take a reference to the stream if we have not been inited yet
        hr = pStream->QueryInterface(&_pStream);
    }
    return hr;
}

HRESULT CRecipeThumbProvider::_LoadXMLDocument(IXMLDOMDocument **ppXMLDoc)
{
    *ppXMLDoc = NULL;

    IXMLDOMDocument *pXMLDoc;
    HRESULT hr = CoCreateInstance(CLSID_DOMDocument60, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pXMLDoc));
    if (SUCCEEDED(hr))
    {
        IPersistStream *pps;
        hr = pXMLDoc->QueryInterface(&pps);
        if (SUCCEEDED(hr))
        {
            hr = pps->Load(_pStream);
            if (SUCCEEDED(hr))
            {
                hr = pXMLDoc->QueryInterface(ppXMLDoc);
            }
            pps->Release();
        }
        pXMLDoc->Release();
    }
    return hr;
}

// Gets the base64-encoded string which represents the image.
HRESULT CRecipeThumbProvider::_GetBase64EncodedImageString(UINT /* cx */, PWSTR *ppszResult)
{
    *ppszResult = NULL;

    IXMLDOMDocument *pXMLDoc;
    HRESULT hr = _LoadXMLDocument(&pXMLDoc);
    if (SUCCEEDED(hr))
    {
        BSTR bstrQuery = SysAllocString(L"Recipe/Attachments/Picture");
        hr = bstrQuery ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            IXMLDOMNode *pXMLNode;
            hr = pXMLDoc->selectSingleNode(bstrQuery, &pXMLNode);
            if (SUCCEEDED(hr))
            {
                IXMLDOMElement *pXMLElement;
                hr = pXMLNode->QueryInterface(&pXMLElement);
                if (SUCCEEDED(hr))
                {
                    BSTR bstrAttribute = SysAllocString(L"Source");
                    hr = bstrAttribute ? S_OK : E_OUTOFMEMORY;
                    if (SUCCEEDED(hr))
                    {
                        VARIANT varValue;
                        hr = pXMLElement->getAttribute(bstrAttribute, &varValue);
                        if (SUCCEEDED(hr))
                        {
                            if ((varValue.vt == VT_BSTR) && varValue.bstrVal && varValue.bstrVal[0])
                            {
                                hr = SHStrDupW(varValue.bstrVal, ppszResult);
                            }
                            else
                            {
                                hr = E_FAIL;
                            }
                            VariantClear(&varValue);
                        }
                        SysFreeString(bstrAttribute);
                    }
                    pXMLElement->Release();
                }
                pXMLNode->Release();
            }
            SysFreeString(bstrQuery);
        }
        pXMLDoc->Release();
    }
    return hr;
}

// Decodes the base64-encoded string to a stream.
HRESULT CRecipeThumbProvider::_GetStreamFromString(PCWSTR pszImageName, IStream **ppImageStream)
{
    HRESULT hr = E_FAIL;

    DWORD dwDecodedImageSize = 0;
    DWORD dwSkipChars        = 0;
    DWORD dwActualFormat     = 0;

    // Base64-decode the string
    BOOL fSuccess = CryptStringToBinaryW(pszImageName, NULL, CRYPT_STRING_BASE64,
                                         NULL, &dwDecodedImageSize, &dwSkipChars, &dwActualFormat);
    if (fSuccess)
    {
        BYTE *pbDecodedImage = (BYTE*)LocalAlloc(LPTR, dwDecodedImageSize);
        if (pbDecodedImage)
        {
            fSuccess = CryptStringToBinaryW(pszImageName, lstrlenW(pszImageName), CRYPT_STRING_BASE64,
                                            pbDecodedImage, &dwDecodedImageSize, &dwSkipChars, &dwActualFormat);
            if (fSuccess)
            {
                *ppImageStream = SHCreateMemStream(pbDecodedImage, dwDecodedImageSize);
                if (*ppImageStream != NULL)
                {
                    hr = S_OK;
                }
            }
            LocalFree(pbDecodedImage);
        }
    }
    return hr;
}

HRESULT ConvertBitmapSourceTo32BPPHBITMAP(IWICBitmapSource *pBitmapSource,
                                           IWICImagingFactory *pImagingFactory,
                                           HBITMAP *phbmp)
{
    *phbmp = NULL;

    IWICBitmapSource *pBitmapSourceConverted = NULL;
    WICPixelFormatGUID guidPixelFormatSource;
    HRESULT hr = pBitmapSource->GetPixelFormat(&guidPixelFormatSource);
    if (SUCCEEDED(hr) && (guidPixelFormatSource != GUID_WICPixelFormat32bppBGRA))
    {
        IWICFormatConverter *pFormatConverter;
        hr = pImagingFactory->CreateFormatConverter(&pFormatConverter);
        if (SUCCEEDED(hr))
        {
            // Create the appropriate pixel format converter
            hr = pFormatConverter->Initialize(pBitmapSource, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, NULL, 0, WICBitmapPaletteTypeCustom);
            if (SUCCEEDED(hr))
            {
                hr = pFormatConverter->QueryInterface(&pBitmapSourceConverted);
            }
            pFormatConverter->Release();
        }
    }
    else
    {
        hr = pBitmapSource->QueryInterface(&pBitmapSourceConverted);  // No conversion necessary
    }

    if (SUCCEEDED(hr))
    {
        UINT nWidth, nHeight;
        hr = pBitmapSourceConverted->GetSize(&nWidth, &nHeight);
        if (SUCCEEDED(hr))
        {
            BITMAPINFO bmi = {};
            bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
            bmi.bmiHeader.biWidth = nWidth;
            bmi.bmiHeader.biHeight = -static_cast<LONG>(nHeight);
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            BYTE *pBits;
            HBITMAP hbmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, reinterpret_cast<void **>(&pBits), NULL, 0);
            hr = hbmp ? S_OK : E_OUTOFMEMORY;
            if (SUCCEEDED(hr))
            {
                WICRect rect = {0, 0, nWidth, nHeight};

                // Convert the pixels and store them in the HBITMAP.  Note: the name of the function is a little
                // misleading - we're not doing any extraneous copying here.  CopyPixels is actually converting the
                // image into the given buffer.
                hr = pBitmapSourceConverted->CopyPixels(&rect, nWidth * 4, nWidth * nHeight * 4, pBits);
                if (SUCCEEDED(hr))
                {
                    *phbmp = hbmp;
                }
                else
                {
                    DeleteObject(hbmp);
                }
            }
        }
        pBitmapSourceConverted->Release();
    }
    return hr;
}

HRESULT WICCreate32BitsPerPixelHBITMAP(IStream *pstm, UINT /* cx */, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha)
{
    *phbmp = NULL;

    IWICImagingFactory *pImagingFactory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pImagingFactory));
    if (SUCCEEDED(hr))
    {
        IWICBitmapDecoder *pDecoder;
        hr = pImagingFactory->CreateDecoderFromStream(pstm, &GUID_VendorMicrosoft, WICDecodeMetadataCacheOnDemand, &pDecoder);
        if (SUCCEEDED(hr))
        {
            IWICBitmapFrameDecode *pBitmapFrameDecode;
            hr = pDecoder->GetFrame(0, &pBitmapFrameDecode);
            if (SUCCEEDED(hr))
            {
                hr = ConvertBitmapSourceTo32BPPHBITMAP(pBitmapFrameDecode, pImagingFactory, phbmp);
                if (SUCCEEDED(hr))
                {
                    *pdwAlpha = WTSAT_ARGB;
                }
                pBitmapFrameDecode->Release();
            }
            pDecoder->Release();
        }
        pImagingFactory->Release();
    }
    return hr;
}

// IThumbnailProvider
IFACEMETHODIMP CRecipeThumbProvider::GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha)
{
    PWSTR pszBase64EncodedImageString;
    HRESULT hr = _GetBase64EncodedImageString(cx, &pszBase64EncodedImageString);
    if (SUCCEEDED(hr))
    {
        IStream *pImageStream;
        hr = _GetStreamFromString(pszBase64EncodedImageString, &pImageStream);
        if (SUCCEEDED(hr))
        {
            hr = WICCreate32BitsPerPixelHBITMAP(pImageStream, cx, phbmp, pdwAlpha);;
            pImageStream->Release();
        }
        CoTaskMemFree(pszBase64EncodedImageString);
    }
    return hr;
}
