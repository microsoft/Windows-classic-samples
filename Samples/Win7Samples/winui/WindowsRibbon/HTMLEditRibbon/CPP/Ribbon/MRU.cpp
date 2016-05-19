// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "..\stdafx.h"
#include <shellapi.h>
#include <strsafe.h>
#include "MRU.h"

//PURPOSE: Implement the properties that describe a Recent Item to the Windows Ribbon
class CRecentFileProperties
    : public IUISimplePropertySet
{
public:

    // Static method to create an instance of the object.
    __checkReturn static HRESULT CreateInstance(__in PWSTR wszFullPath, __deref_out_opt CRecentFileProperties **ppProperties)
    {
        if (!wszFullPath || !ppProperties)
        {
            return E_POINTER;
        }

        *ppProperties = NULL;

        HRESULT hr;

        CRecentFileProperties* pProperties = new CRecentFileProperties();

        if (pProperties != NULL)
        {
            hr = ::StringCchCopyW(pProperties->m_wszFullPath, MAX_PATH, wszFullPath);
            SHFILEINFOW sfi;

            DWORD_PTR dwPtr = NULL;
            if (SUCCEEDED(hr))
            {
                dwPtr = ::SHGetFileInfoW(wszFullPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
            }

            if (dwPtr != NULL)
            {
                hr = ::StringCchCopyW(pProperties->m_wszDisplayName, MAX_PATH, sfi.szDisplayName);
            }
            else // Provide a reasonable fallback.
            {
                hr = ::StringCchCopyW(pProperties->m_wszDisplayName, MAX_PATH, pProperties->m_wszFullPath);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr))
        {
            *ppProperties = pProperties;
            (*ppProperties)->AddRef();
        }
        
        if (pProperties)
        {
            pProperties->Release();
        }

        return hr;
    }

    // IUnknown methods.
    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG) Release()
    {
        LONG cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete this;
        }

        return cRef;
    }

    STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
    {
        if (!ppv)
        {
            return E_POINTER;
        }

        if (iid == __uuidof(IUnknown))
        {
            *ppv = static_cast<IUnknown*>(this);
        }
        else if (iid == __uuidof(IUISimplePropertySet))
        {
            *ppv = static_cast<IUISimplePropertySet*>(this);
        }
        else 
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    // IUISimplePropertySet methods.
    STDMETHODIMP GetValue(__in REFPROPERTYKEY key, __out PROPVARIANT *value)
    {
        HRESULT hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

        if (key == UI_PKEY_Label)
        {
            hr = UIInitPropertyFromString(UI_PKEY_Label, m_wszDisplayName, value);
        }
        else if (key == UI_PKEY_LabelDescription)
        {
            hr = UIInitPropertyFromString(UI_PKEY_LabelDescription, m_wszFullPath, value);
        }

        return hr;
    }

private:
    CRecentFileProperties()
        : m_cRef(1)
    {
        m_wszFullPath[0] = L'\0';
        m_wszDisplayName[0] = L'\0';
    }

    LONG m_cRef;                        // Reference count.
    WCHAR m_wszDisplayName[MAX_PATH];
    WCHAR m_wszFullPath[MAX_PATH];
};

HRESULT PopulateRibbonRecentItems(__in CRecentFileList& recentFiles, __deref_out PROPVARIANT* pvarValue)
{
    int iFileCount = recentFiles.GetSize();
    LONG iCurrentFile = 0;
    HRESULT hr = E_FAIL;
    SAFEARRAY* psa = SafeArrayCreateVector(VT_UNKNOWN, 0, iFileCount);

    if (psa != NULL)
    {
        while (iCurrentFile < iFileCount)
        {
            CString strCurrentFile = recentFiles[iCurrentFile];
            WCHAR wszCurrentFile[MAX_PATH] = {0};
            if (0 != MultiByteToWideChar(CP_THREAD_ACP, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, strCurrentFile.GetString(), strCurrentFile.GetLength(), wszCurrentFile, MAX_PATH))
            {
                CRecentFileProperties* pPropertiesObj;
                hr = CRecentFileProperties::CreateInstance(wszCurrentFile, &pPropertiesObj);

                if (SUCCEEDED(hr))
                {
                    IUnknown* pUnk = NULL;

#pragma warning( disable : 6011)    // pPropertiesObj cannot be NULL.
                    hr = pPropertiesObj->QueryInterface(__uuidof(IUnknown), reinterpret_cast<void**>(&pUnk));
#pragma warning( default : 6011)    //

                    if (SUCCEEDED(hr))
                    {
                        hr = SafeArrayPutElement(psa, &iCurrentFile, static_cast<void*>(pUnk));
                        pUnk->Release();
                    }
                }

                if (pPropertiesObj)
                {
                    pPropertiesObj->Release();
                }

                if (FAILED(hr))
                {
                    break;
                }

                iCurrentFile ++;
            }
            else
            {
                break;
            }
        }

        // We will only populate items up to before the first failed item, and discard the rest.
        SAFEARRAYBOUND sab = {iCurrentFile,0};
        SafeArrayRedim(psa, &sab);
        hr = UIInitPropertyFromIUnknownArray(UI_PKEY_RecentItems, psa, pvarValue);

        SafeArrayDestroy(psa);
    }

    // Note that this result could be S_OK even when we encounter errors when
    // populating the list of items.
    return hr;
}
