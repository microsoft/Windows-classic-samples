#include "CHWMFT.h"
#include <strsafe.h>
#include <mfapi.h>
#include <initguid.h>

#pragma region HelperMacros
// Helper Macros
#define SAFERELEASE(x) \
    if((x) != NULL) \
    { \
        (x)->Release(); \
        (x) = NULL; \
    } \

#define SAFEREGCLOSEKEY(x) \
    if((x) != NULL) \
    { \
        RegCloseKey(x); \
        (x) = NULL; \
    } \

#define SAFEARRAYDELETE(x) \
    if((x) != NULL) \
    { \
        delete[] (x); \
        (x) = NULL; \
    } \

#pragma endregion HelperMacros

#pragma region GlobalDefines

// {C637E2D2-01A0-4ACE-9C43-D2AF07E00B8C}
DEFINE_GUID(CLSID_MYMFT, 
0xc637e2d2, 0x1a0, 0x4ace, 0x9c, 0x43, 0xd2, 0xaf, 0x7, 0xe0, 0xb, 0x8c);

#define MY_MFT_NAME     L"Win8 SDK HW MFT Sample"
#define MFT_CODEC_MERIT 8 /*Todo: Replace this with the actual codec Merit*/

#pragma endregion GlobalDefines

#pragma region ClassFactory
// Implements the class factory for the MFT

class ClassFactory : public IClassFactory
{
protected:
            volatile ULONG  m_ulRefCount; // Reference count.
    static  volatile ULONG  m_ulServerLocks; // Number of server locks

public:
    ClassFactory(void)
    {
        m_ulRefCount = 1;
    }

    static bool IsLocked(void)
    {
        return (m_ulServerLocks != 0);
    }

    // IUnknown methods
    ULONG __stdcall AddRef(void)
    {
        return InterlockedIncrement(&m_ulRefCount);
    }

    ULONG __stdcall Release(void)
    {
        ULONG ulRef = 0;

        if(m_ulRefCount > 0)
        {
            ulRef = InterlockedDecrement(&m_ulRefCount);
        }

        if(ulRef == 0)
        {
            delete this;
        }

        return ulRef;
    }

    HRESULT __stdcall QueryInterface(
        REFIID riid,
        void** ppvObject)
    {
        HRESULT hr = S_OK;

        do
        {
            if(ppvObject == NULL)
            {
                hr = E_POINTER;
                break;
            }

            if(riid == IID_IUnknown)
            {
                *ppvObject = (IUnknown*)this;
            }
            else if(riid == IID_IClassFactory)
            {
                *ppvObject = (IClassFactory*)this;
            }
            else 
            {
                *ppvObject = NULL;
                hr = E_NOINTERFACE;
                break;
            }

            AddRef();
        }while(false);

        return hr;
    }

    HRESULT __stdcall CreateInstance(
        IUnknown *pUnkOuter,
        REFIID riid, void**
        ppv)
    {
        HRESULT         hr      = S_OK;
        IMFTransform*   pHWMFT  = NULL;

        do
        {
            if (pUnkOuter != NULL)
            {
                // This MFT not support aggregation.
                hr = CLASS_E_NOAGGREGATION;
                break;
            }

            hr = CHWMFT::CreateInstance(&pHWMFT);
            if(FAILED(hr))
            {
                break;
            }

            hr = pHWMFT->QueryInterface(riid,
                ppv
                );
            if(FAILED(hr))
            {
                break;
            }
        }while(false);

        SAFERELEASE(pHWMFT);

        return hr;
    }

    HRESULT __stdcall LockServer(
        BOOL bLock)
    {   
        HRESULT hr = S_OK;

        do
        {
            if(bLock != FALSE)
            {
                InterlockedIncrement(&m_ulServerLocks);
            }
            else
            {
                InterlockedDecrement(&m_ulServerLocks);
            }
        }while(false);

        return hr;
    }
};

#pragma endregion ClassFactory

#pragma region GlobalVariables

HMODULE         g_hModule                       = NULL;     // DLL module handle
volatile ULONG  CHWMFT::m_ulNumObjects          = 0;        // Number of active COM objects
volatile ULONG ClassFactory::m_ulServerLocks   = 0;        // Number of server locks

#pragma endregion GlobalVariables

#pragma region HelperFunctions

HRESULT CreateObjectKeyName(
    _In_                const   GUID&   guid,
    _Out_writes_(dwMax) WCHAR*          pwszName,
    _In_                const DWORD     dwMax)
{
    HRESULT hr = S_OK;

    do
    {
        if(pwszName == NULL)
        {
            hr = E_POINTER;
            break;
        }

        if(StringCchPrintf(pwszName,
            dwMax,
            L"CLSID\\{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            guid.Data1,
            guid.Data2,
            guid.Data3,
            guid.Data4[0],
            guid.Data4[1],
            guid.Data4[2],
            guid.Data4[3],
            guid.Data4[4],
            guid.Data4[5],
            guid.Data4[6],
            guid.Data4[7]
            ) < 0)
            {
                hr = E_FAIL;
                break;
            }
    }while(false);

    return hr;
}

HRESULT SetKeyValue(
            HKEY    hKey,
    const   WCHAR*  pwszName,
    const   WCHAR*  pwszValue)
{
    HRESULT hr = S_OK;

    do
    {
        hr = HRESULT_FROM_WIN32(
            RegSetValueEx(hKey,
                pwszName,
                0,
                REG_SZ,
                (BYTE*)pwszValue,
                (DWORD)((wcslen(pwszValue) + 1) * sizeof(WCHAR))
                )
            );
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    return hr;
}

HRESULT RegisterObject(
    const GUID&     guid,
    const WCHAR*    pwszDescription,
    const WCHAR*    pwszThreadingModel)
{
    HRESULT hr                      = S_OK;
    HKEY    hKey                    = NULL;
    HKEY    hSubkey                 = NULL;
    DWORD   dwFileNameLen           = 0;
    WCHAR   pwszBuffer[MAX_PATH]    = {0};

    do
    {
        if(g_hModule == NULL)
        {
            hr = E_UNEXPECTED;
            break;
        }

        // Create the name of the key from the object's CLSID
        hr = CreateObjectKeyName(guid,
            pwszBuffer,
            sizeof(pwszBuffer) / sizeof(pwszBuffer[0])
            );
        if(FAILED(hr))
        {
            break;
        }

        // Create the new key.
        hr = HRESULT_FROM_WIN32(
            RegCreateKeyEx(
                HKEY_CLASSES_ROOT,
                pwszBuffer,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,
                &hKey,
                NULL
                )
            );
        if(FAILED(hr))
        {
            break;
        }

        hr = SetKeyValue(hKey,
            NULL,
            pwszDescription);
        if(FAILED(hr))
        {
            break;
        }

        // Create the "InprocServer32" subkey
        hr = HRESULT_FROM_WIN32(
            RegCreateKeyEx(
                hKey,
                L"InprocServer32",
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,
                &hSubkey,
                NULL
                )
            );

        // The default value for this subkey is the path to the DLL.
        // Get the name of the module ...
        dwFileNameLen = GetModuleFileName(g_hModule,
            pwszBuffer,
            MAX_PATH);
        if (dwFileNameLen == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else if (dwFileNameLen == MAX_PATH)
        {
            hr = E_FAIL; // buffer too small
        }

        if(FAILED(hr))
        {
            break;
        }

        // ... and set the default key value.
        hr = SetKeyValue(hSubkey,
            NULL,
            pwszBuffer
            );
        if(FAILED(hr))
        {
            break;
        }

        // Add a new value to the subkey, for "ThreadingModel" = <threading model>
        hr = SetKeyValue(hSubkey,
            L"ThreadingModel",
            pwszThreadingModel
            );
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    SAFEREGCLOSEKEY(hSubkey);
    SAFEREGCLOSEKEY(hKey);

    return hr;
}

HRESULT UnregisterObject(
    const GUID& guid)
{
    HRESULT hr                      = S_OK;
    WCHAR   pwszBuffer[MAX_PATH]    = {0};

    do
    {
        hr = CreateObjectKeyName(guid,
            pwszBuffer,
            sizeof(pwszBuffer) / sizeof(pwszBuffer[0])
            );
        if(FAILED(hr))
        {
            break;
        }

        hr = HRESULT_FROM_WIN32(
            RegDeleteTree(HKEY_CLASSES_ROOT,
                pwszBuffer
                )
            );
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    return hr;
}

#pragma endregion HelperFunctions

#pragma region DLLFunctions

BOOL APIENTRY DllMain(
    HANDLE hModule,
    DWORD  dwReason,
    LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
        g_hModule = (HMODULE)hModule;
        break;
    case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
        // Nothing to do
		break;
	case DLL_PROCESS_DETACH:
		break;
	};

    return TRUE;
}

HRESULT __stdcall DllCanUnloadNow(void)
{
    HRESULT hr = S_FALSE;

    do
    {
        if ((ClassFactory::IsLocked() != FALSE) || (CHWMFT::m_ulNumObjects != 0))
        {
            break;
        }

        hr = S_OK;
    }while(false);

    return hr;
}

HRESULT __stdcall DllRegisterServer(void)
{
    HRESULT                 hr                  = S_OK;
    MFT_REGISTER_TYPE_INFO* pmftrtiInput        = NULL;
    MFT_REGISTER_TYPE_INFO* pmftrtiOutput       = NULL;
    IMFAttributes*          pMFTAttributes      = NULL;

    do
    {
        pmftrtiInput = new MFT_REGISTER_TYPE_INFO[g_dwNumInputTypes];
        if(pmftrtiInput == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        pmftrtiOutput = new MFT_REGISTER_TYPE_INFO[g_dwNumOutputTypes];
        if(pmftrtiOutput == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        for(DWORD i = 0; i < g_dwNumInputTypes; i++)
        {
            pmftrtiInput[i].guidMajorType   = MFMediaType_Video;
            pmftrtiInput[i].guidSubtype     = *(g_ppguidInputTypes[i]);
        }

        for(DWORD i = 0; i < g_dwNumOutputTypes; i++)
        {
            pmftrtiOutput[i].guidMajorType   = MFMediaType_Video;
            pmftrtiOutput[i].guidSubtype     = *(g_ppguidOutputTypes[i]);
        }

        /****************************************
        ** !!MSFT_TODO: Report as HW MFT
        ****************************************
        // Since this is a HW MFT, we have to set MFT_CODEC_MERIT_Attribute
        hr = MFCreateAttributes(&pMFTAttributes, 1);
        if(FAILED(hr))
        {
            break;
        }

        hr = pMFTAttributes->SetUINT32(MFT_CODEC_MERIT_Attribute, MFT_CODEC_MERIT);
        if(FAILED(hr))
        {
            break;
        }
        */

        // Register the object with COM
        hr = RegisterObject(CLSID_MYMFT,
            MY_MFT_NAME,
            L"Both");
        if(FAILED(hr))
        {
            break;
        }

        // Register the MFT with MF
		hr = MFTRegister(
                CLSID_MYMFT,
                MFT_CATEGORY_VIDEO_DECODER, // This MFT acts as a video decoder
                MY_MFT_NAME,
                /****************************************
                ** !!MSFT_TODO: Report as HW MFT
                ****************************************
                MFT_ENUM_FLAG_HARDWARE,*/ MFT_ENUM_FLAG_ASYNCMFT,
                g_dwNumInputTypes,
                pmftrtiInput,
                g_dwNumOutputTypes,
                pmftrtiOutput,
                pMFTAttributes
            );
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    SAFEARRAYDELETE(pmftrtiInput);
    SAFEARRAYDELETE(pmftrtiOutput);
    SAFERELEASE(pMFTAttributes);

    return hr;

}

HRESULT __stdcall DllUnregisterServer(void)
{
    UnregisterObject(CLSID_MYMFT);

    return S_OK;
}

HRESULT __stdcall DllGetClassObject(
    _In_        REFCLSID clsid,
    _In_        REFIID riid,
    _Outptr_    void** ppv)
{
    HRESULT         hr          = S_OK;
    ClassFactory*   pFactory    = NULL;

    do
    {
        if (clsid != CLSID_MYMFT)
        {
            hr = CLASS_E_CLASSNOTAVAILABLE;
            break;
        }

        pFactory = new ClassFactory();
        if (pFactory == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pFactory->QueryInterface(riid, ppv);
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    SAFERELEASE(pFactory);

    return hr;
}

#pragma endregion DLLFunctions

