//
// register.cpp
//
// Server registration code.
//

#include <windows.h>
#include <ole2.h>
#include "msctf.h"
#include "globals.h"
#include "case.h"

#define CLSID_STRLEN 38  // strlen("{xxxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx}")

const struct
{
    const GUID *pguidCategory;
    const GUID *pguid;
}
c_rgCategories[] =
{
    { &GUID_TFCAT_TIP_KEYBOARD, &c_clsidCaseTextService },
};

static const TCHAR c_szInfoKeyPrefix[] = TEXT("CLSID\\");
static const TCHAR c_szInProcSvr32[] = TEXT("InProcServer32");
static const TCHAR c_szModelName[] = TEXT("ThreadingModel");
static const TCHAR c_szCaseDesc[] = TEXT("Case Text Service");
static const WCHAR c_szCaseDescW[] = L"Case Text Service";
static const TCHAR c_szCaseModel[] = TEXT("Apartment");


//+---------------------------------------------------------------------------
//
//  RegisterProfiles
//
//----------------------------------------------------------------------------

BOOL CCaseTextService::RegisterProfiles()
{
    ITfInputProcessorProfiles *pInputProcessProfiles;
    WCHAR achIconFile[MAX_PATH];
    char achFileNameA[MAX_PATH];
    DWORD cchA;
    int cchIconFile;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfiles, (void**)&pInputProcessProfiles);

    if (hr != S_OK)
        return E_FAIL;

    hr = pInputProcessProfiles->Register(c_clsidCaseTextService);

    if (hr != S_OK)
        goto Exit;

    cchA = GetModuleFileNameA(g_hInst, achFileNameA, ARRAYSIZE(achFileNameA));

    cchIconFile = MultiByteToWideChar(CP_ACP, 0, achFileNameA, cchA, achIconFile, ARRAYSIZE(achIconFile)-1);
    achIconFile[cchIconFile] = '\0';

    hr = pInputProcessProfiles->AddLanguageProfile(c_clsidCaseTextService,
                                  CASE_LANGID, 
                                  c_guidCaseProfile, 
                                  c_szCaseDescW, 
                                  ARRAYSIZE(c_szCaseDescW) - 1,
                                  achIconFile,
                                  cchIconFile,
                                  CASE_ICON_INDEX);

Exit:
    pInputProcessProfiles->Release();
    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
//  UnregisterProfiles
//
//----------------------------------------------------------------------------

void CCaseTextService::UnregisterProfiles()
{
    ITfInputProcessorProfiles *pInputProcessProfiles;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfiles, (void**)&pInputProcessProfiles);

    if (hr != S_OK)
        return;

    pInputProcessProfiles->Unregister(c_clsidCaseTextService);
    pInputProcessProfiles->Release();
}

//+---------------------------------------------------------------------------
//
//  RegisterCategories
//
//----------------------------------------------------------------------------

BOOL CCaseTextService::RegisterCategories(BOOL fRegister)
{
    ITfCategoryMgr *pCategoryMgr;
    int i;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, 
                          IID_ITfCategoryMgr, (void**)&pCategoryMgr);

    if (hr != S_OK)
        return FALSE;

    for (i=0; i<ARRAYSIZE(c_rgCategories); i++)
    {
        if (fRegister)
        {
            hr = pCategoryMgr->RegisterCategory(c_clsidCaseTextService,
                   *c_rgCategories[i].pguidCategory, *c_rgCategories[i].pguid);
        }
        else
        {
            hr = pCategoryMgr->UnregisterCategory(c_clsidCaseTextService,
                   *c_rgCategories[i].pguidCategory, *c_rgCategories[i].pguid);
        }

        if (hr != S_OK)
            break;
    }

    pCategoryMgr->Release();
    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
//  CLSIDToString
//
//----------------------------------------------------------------------------

static const BYTE GuidMap[] = {3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
    8, 9, '-', 10, 11, 12, 13, 14, 15};

static const char szDigits[] = "0123456789ABCDEF";
static const WCHAR wszDigits[] = L"0123456789ABCDEF";

BOOL CLSIDToStringA(REFGUID refGUID, __out_ecount(CLSID_STRLEN + 1) LPSTR pchA)
{
    int i, j;
    char *p = pchA;

    const BYTE * pBytes = (const BYTE *) &refGUID;

    j = 0;
    p[j++] = '{';
    for (i = 0; i < sizeof(GuidMap) && j < CLSID_STRLEN - 2; i++)    // CLSID_STRLEN-2 is rid { and }
    {
        if (GuidMap[i] == '-')
        {
            p[j++] = '-';
        }
        else
        {
            p[j++] = szDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            p[j++] = szDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    p[j++] = '}';
    p[j]   = '\0';

    return TRUE;
}

BOOL CLSIDToStringW(REFGUID refGUID, __out_ecount(CLSID_STRLEN + 1) LPWSTR pchW)
{
    int i, j;
    WCHAR *p = pchW;

    const BYTE * pBytes = (const BYTE *) &refGUID;

    j = 0;
    p[j++] = L'{';
    for (i = 0; i < sizeof(GuidMap) && j < CLSID_STRLEN - 2; i++)    // CLSID_STRLEN-2 is rid { and }
    {
        if (GuidMap[i] == '-')
        {
            p[j++] = L'-';
        }
        else
        {
            p[j++] = wszDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            p[j++] = wszDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    p[j++] = L'}';
    p[j]   = L'\0';

    return TRUE;
}

#ifdef UNICODE
#define CLSIDToString        CLSIDToStringW
#else
#define CLSIDToString        CLSIDToStringA
#endif

//+---------------------------------------------------------------------------
//
// RecurseDeleteKey
//
// RecurseDeleteKey is necessary because on NT RegDeleteKey doesn't work if the
// specified key has subkeys
//----------------------------------------------------------------------------
LONG RecurseDeleteKey(HKEY hParentKey, LPCTSTR lpszKey)
{
    HKEY hKey;
    LONG lRes;
    FILETIME time;
    TCHAR szBuffer[256];
    DWORD dwSize = ARRAYSIZE(szBuffer);

    if (RegOpenKey(hParentKey, lpszKey, &hKey) != ERROR_SUCCESS)
        return ERROR_SUCCESS; // let's assume we couldn't open it because it's not there

    lRes = ERROR_SUCCESS;
    while (RegEnumKeyEx(hKey, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time)==ERROR_SUCCESS)
    {
        szBuffer[ARRAYSIZE(szBuffer)-1] = '\0';
        lRes = RecurseDeleteKey(hKey, szBuffer);
        if (lRes != ERROR_SUCCESS)
            break;
        dwSize = ARRAYSIZE(szBuffer);
    }
    RegCloseKey(hKey);

    return lRes == ERROR_SUCCESS ? RegDeleteKey(hParentKey, lpszKey) : lRes;
}

//+---------------------------------------------------------------------------
//
//  RegisterServer
//
//----------------------------------------------------------------------------

BOOL CCaseTextService::RegisterServer()
{
    DWORD dw;
    HKEY hKey;
    HKEY hSubKey;
    BOOL fRet;
    TCHAR achIMEKey[ARRAYSIZE(c_szInfoKeyPrefix) + CLSID_STRLEN];
    TCHAR achFileName[MAX_PATH];

    if (!CLSIDToString(c_clsidCaseTextService, achIMEKey + ARRAYSIZE(c_szInfoKeyPrefix) - 1))
        return FALSE;
    memcpy(achIMEKey, c_szInfoKeyPrefix, sizeof(c_szInfoKeyPrefix)-sizeof(TCHAR));

    if (fRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, achIMEKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dw)
            == ERROR_SUCCESS)
    {
        fRet &= RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE *)c_szCaseDesc, sizeof(c_szCaseDesc))
            == ERROR_SUCCESS;

        if (fRet &= RegCreateKeyEx(hKey, c_szInProcSvr32, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, &dw)
            == ERROR_SUCCESS)
        {
            dw = GetModuleFileName(g_hInst, achFileName, ARRAYSIZE(achFileName));
            dw = min(dw, (ARRAYSIZE(achFileName) - 1));
            achFileName[dw] = 0;

            fRet &= RegSetValueEx(hSubKey, NULL, 0, REG_SZ, (BYTE *)achFileName, (dw + 1)*sizeof(TCHAR)) == ERROR_SUCCESS;
            fRet &= RegSetValueEx(hSubKey, c_szModelName, 0, REG_SZ, (BYTE *)c_szCaseModel, sizeof(c_szCaseModel)) == ERROR_SUCCESS;
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    return fRet;
}

//+---------------------------------------------------------------------------
//
//  UnregisterServer
//
//----------------------------------------------------------------------------

void CCaseTextService::UnregisterServer()
{
    TCHAR achIMEKey[ARRAYSIZE(c_szInfoKeyPrefix) + CLSID_STRLEN];

    if (!CLSIDToString(c_clsidCaseTextService, achIMEKey + ARRAYSIZE(c_szInfoKeyPrefix) - 1))
        return;
    memcpy(achIMEKey, c_szInfoKeyPrefix, sizeof(c_szInfoKeyPrefix)-sizeof(TCHAR));

    RecurseDeleteKey(HKEY_CLASSES_ROOT, achIMEKey);
}
