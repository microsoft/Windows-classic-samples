// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shellapi.h>
#include <Psapi.h>

#include "resource.h"

#define GUID_SIZE               128
#define SZ_REG_PATH_HISTORY     L"Software\\Microsoft\\KnownFolderSample"

// command line args
#define SZ_CLA_REGISTER         L"/register"
#define SZ_CLA_ENUM             L"/enum"
#define SZ_CLA_UNREGISTER       L"/unregister"
#define SZ_CLA_CATEGORY         L"/category:"
#define SZ_CLA_DEFFLAG          L"/defFlag:"
#define SZ_CLA_ID               L"/id:"
#define SZ_CLA_PSZNAME          L"/pszName:"
#define SZ_CLA_PSZCREATOR       L"/pszCreator:"
#define SZ_CLA_PSZDESCRIPTION   L"/pszDescription:"
#define SZ_CLA_PSZRELPATH       L"/pszRelativePath:"
#define SZ_CLA_PSZPARSENAME     L"/pszParsingName:"
#define SZ_CLA_PSZLOCALIZEDNAME L"/pszLocalizedName:"
#define SZ_CLA_PSZICON          L"/pszIcon:"
#define SZ_CLA_PSZTOOLTIP       L"/pszTooltip:"
#define SZ_CLA_PSZSECURITY      L"/pszSecurity:"
#define SZ_CLA_FINDFORPATH      L"/pszFindForPath:"
#define SZ_CLA_CLEAN            L"/clean"
#define SZ_CLA_SHOW_USAGE       L"/?"

typedef enum
{
    ACT_UNDEFINED,
    ACT_REGISTER,
    ACT_ENUM,
    ACT_UNREGISTER,
    ACT_CLEAN,
    ACT_SHOW_USAGE,
    ACT_FIND_FOR_PATH
} ACTION_TYPE;


BOOL g_fVerbose = FALSE;


// function declarations necessary for improved sample readability
void DumpKnownFolderDef(REFKNOWNFOLDERID kfid, KNOWNFOLDER_DEFINITION const &kfd);
void DumpKnownFolderInfo(IKnownFolder *pkf);
BOOL ParseAndValidateCommandLine(PWSTR *ppszArgs, int iArgs, ACTION_TYPE *at, KNOWNFOLDERID *pkfid,
                                 KNOWNFOLDER_DEFINITION *pkfd, PWSTR *ppszFindForPath);
void CompleteKnownFolderDef(KNOWNFOLDER_DEFINITION *pkfd);
HRESULT CreatePhysicalFolderIfNecessary(KNOWNFOLDERID kfidParent, PWSTR pszRelativePath);
void DumpUsage();
void UnregisterAllKFsAddedByThisTool(DWORD *pdwKFs);
void AddRegisteredFolderToHistory(KNOWNFOLDERID kfid);

HRESULT RegisterKnownFolder(REFKNOWNFOLDERID kfid, KNOWNFOLDER_DEFINITION *pkfd)
{
    IKnownFolderManager *pkfm = NULL;
    HRESULT hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pkfm));
    if (SUCCEEDED(hr))
    {
        hr = pkfm->RegisterFolder(kfid, pkfd);
        if (SUCCEEDED(hr))
        {
            // to make it easy to clean up everything this sample does, we'll track each added kfid
            AddRegisteredFolderToHistory(kfid);
        }
        else
        {
            wprintf(L"IKnownFolder::RegisterFolder() failed with hr = 0x%x\nMake sure this tool is run as an administrator as that is necessary to regiser a known folder", hr);
        }
        pkfm->Release();
    }

    return hr;
}

void EnumAndDumpKnownFolders(DWORD *pdwKFCount, PCWSTR pszNameSrchStr, REFKNOWNFOLDERID kfidSearch)
{
    *pdwKFCount = 0;
    IKnownFolderManager *pkfm = NULL;
    HRESULT hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pkfm));
    if (SUCCEEDED(hr))
    {
        KNOWNFOLDERID *rgKFIDs = NULL;
        UINT cKFIDs = 0;
        hr = pkfm->GetFolderIds(&rgKFIDs, &cKFIDs);
        if (SUCCEEDED(hr))
        {
            WCHAR szKFIDGuid[GUID_SIZE];
            IKnownFolder *pkfCurrent = NULL;
            for (UINT i = 0; i < cKFIDs; ++i)
            {
                // if we are searching for a specific GUID, make sure we match before going
                // any further.  GUID_NULL means "show all."
                if (kfidSearch == GUID_NULL || kfidSearch == rgKFIDs[i])
                {
                    StringFromGUID2(rgKFIDs[i], szKFIDGuid, ARRAYSIZE(szKFIDGuid));
                    hr = pkfm->GetFolder(rgKFIDs[i], &pkfCurrent);
                    if (SUCCEEDED(hr))
                    {
                        KNOWNFOLDERID kfid;
                        hr = pkfCurrent->GetId(&kfid);
                        if (FAILED(hr))
                        {
                            wprintf(L"IKnownFolder::GetId() failed for %s!  hr=0x%x\n", szKFIDGuid, hr);
                        }

                        KNOWNFOLDER_DEFINITION kfd;
                        hr = pkfCurrent->GetFolderDefinition(&kfd);
                        if (FAILED(hr))
                        {
                            wprintf(L"IKnownFolderManager::GetFolderDefinition() failed hr=0x%x KNOWNFOLDERID=%s", hr, szKFIDGuid);
                        }
                        else
                        {
                            BOOL fDumpThisFolder = TRUE;
                            if (pszNameSrchStr)
                            {
                                if (NULL == wcsstr(kfd.pszName, pszNameSrchStr))
                                {
                                    fDumpThisFolder = FALSE;
                                }
                            }

                            if (fDumpThisFolder)
                            {
                                ++*pdwKFCount;
                                DumpKnownFolderDef(kfid, kfd);
                                DumpKnownFolderInfo(pkfCurrent);
                            }

                            FreeKnownFolderDefinitionFields(&kfd);
                        }

                        pkfCurrent->Release();
                    }
                    else
                    {
                        wprintf(L"IKnownFolderManager::GetFolder() failed for %s  hr=0x%x\n", szKFIDGuid, hr);
                    }
                }
            }
            CoTaskMemFree(rgKFIDs);
        }
        pkfm->Release();
    }
}

HRESULT UnregisterFolder(REFKNOWNFOLDERID kfid)
{
    IKnownFolderManager *pkfm = NULL;
    HRESULT hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pkfm));
    if (SUCCEEDED(hr))
    {
        hr = pkfm->UnregisterFolder(kfid);
        if (FAILED(hr))
        {
            wprintf(L"IKnownFolder::UnregisterFolder() failed with hr = 0x%x\n", hr);
        }
        pkfm->Release();
    }
    return hr;
}

HRESULT GetKnownFolderForPath(PCWSTR pszPath, KNOWNFOLDERID *pkfid, KNOWNFOLDER_DEFINITION *pkfd)
{
    IKnownFolderManager *pkfm;
    HRESULT hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pkfm));
    if (SUCCEEDED(hr))
    {
        IKnownFolder *pkf;
        hr = pkfm->FindFolderFromPath(pszPath, FFFP_EXACTMATCH, &pkf);
        if (S_OK == hr)
        {
            hr = pkf->GetId(pkfid);
            if (S_OK == hr)
            {
                hr = pkf->GetFolderDefinition(pkfd);
                if (S_OK != hr)
                {
                    wprintf(L"IKnownFolderManager::GetFolderDefinition return hr=0x%x\n", hr);
                }
            }
            else
            {
                wprintf(L"IKnownFolder::GetId return hr=0x%x\n", hr);
            }
            pkf->Release();
        }
        else
        {
            wprintf(L"IKnownFolderManager::FindFolderFromPath return hr=0x%x\n", hr);
        }
        pkfm->Release();
    }
    return hr;
}

int main(int, char *[])
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        int iArgs = 0;
        PWSTR *ppszArgs = CommandLineToArgvW(GetCommandLineW(), &iArgs);

        KNOWNFOLDERID kfid = GUID_NULL;
        KNOWNFOLDER_DEFINITION kfd = {};

        ACTION_TYPE at;
        PWSTR pszFindForPath = NULL;
        if (ParseAndValidateCommandLine(ppszArgs, iArgs, &at, &kfid, &kfd, &pszFindForPath))
        {
            switch (at)
            {
            case ACT_REGISTER:
                {
                    if (GUID_NULL == kfid)
                    {
                        CoCreateGuid(&kfid);
                    }

                    CompleteKnownFolderDef(&kfd);
                    hr = RegisterKnownFolder(kfid, &kfd);
                    if (S_OK == hr)
                    {
                        // we create our knownfolder with SHGetKnownFolderPath() so that the shell will write
                        // the desktop.ini file in the folder.  This is how our customizations
                        // (i.e.: pszIcon, pszTooltip and pszLocalizedName) get picked up by explorer.
                        PWSTR pszPath = NULL;
                        hr = SHGetKnownFolderPath(kfid, KF_FLAG_CREATE | KF_FLAG_INIT, NULL, &pszPath);
                        if (S_OK != hr)
                        {
                            wprintf(L"SHGetKnownFolderPath(KF_FLAG_CREATE | KF_FLAG_INIT) returned hr=0x%x\nThe KnownFolder was not registered.\n", hr);
                            hr = UnregisterFolder(kfid);
                            if (S_OK != hr)
                            {
                                wprintf(L"IKnownFolderManager::UnregisterFolder returned hr=0x%x.\nThe folder was NOT unregistered.\n", hr);
                            }
                            else
                            {
                                wprintf(L"The KnownFolder was not registered.\n");
                            }
                        }
                        else
                        {
                            CoTaskMemFree(pszPath);
                            DumpKnownFolderDef(kfid, kfd);
                        }
                    }
                }
                break;

            case ACT_ENUM:
                wprintf(L"Enumerating all registered KnownFolders \n");
                if (kfd.pszName)
                {
                    wprintf(L" matching pszName '%s'...\n", kfd.pszName);
                }
                else if (GUID_NULL != kfid)
                {
                    WCHAR szGuid[GUID_SIZE];
                    StringFromGUID2(kfid, szGuid, ARRAYSIZE(szGuid));
                    wprintf(L" matching KNOWNFOLDERID %s...\n", szGuid);
                }

                DWORD dwCKF;
                EnumAndDumpKnownFolders(&dwCKF, kfd.pszName, kfid);
                wprintf(L"Finished enumerating %d registered KnownFolders enumerated.\n", dwCKF);
                break;

            case ACT_UNREGISTER:
                if (GUID_NULL == kfid)
                {
                    DumpUsage();
                }
                else
                {
                    hr = UnregisterFolder(kfid);
                    if (S_OK != hr)
                    {
                        wprintf(L"IKnownFolderManager::UnregisterFolder returned hr=0x%x\n", hr);
                    }
                }
                break;

            case ACT_CLEAN:
                wprintf(L"Unregistering all KnownFolders registered by this tool\n");
                DWORD dw;
                UnregisterAllKFsAddedByThisTool(&dw);
                wprintf(L"Unregistered %d KnownFolders\n", dw);
                break;

            case ACT_FIND_FOR_PATH:
                hr = GetKnownFolderForPath(pszFindForPath, &kfid, &kfd);
                if (SUCCEEDED(hr))
                {
                    DumpKnownFolderDef(kfid, kfd);
                }
                else
                {
                    wprintf(L"Failed to find KnownFolder for path: %s\n", pszFindForPath);
                }
                break;

            case ACT_SHOW_USAGE:
            default:
                DumpUsage();
                break;
            }

            FreeKnownFolderDefinitionFields(&kfd);

            CoTaskMemFree(pszFindForPath);
        }
        else
        {
            DumpUsage();
        }

        CoUninitialize();
    }

    return 0;
}

BOOL ExtractParam(PCWSTR pszPrefix, PCWSTR pszArg, PWSTR *ppszParam)
{
    *ppszParam = NULL;

    BOOL fSuccess = FALSE;
    size_t cchPrefix = wcslen(pszPrefix);
    size_t cchParam = wcslen(pszArg) - cchPrefix;
    if (cchParam > 0)
    {
        *ppszParam = (PWSTR)CoTaskMemAlloc(sizeof(WCHAR) * (cchParam + 1));
        if (*ppszParam)
        {
            StringCchCopyW(*ppszParam, cchParam + 1, pszArg + cchPrefix);
            fSuccess = TRUE;
        }
    }
    return fSuccess;
}

struct
{
    KF_DEFINITION_FLAGS flags;
    PCWSTR pszFlagName;
}
const c_rgKFFlagMap[] =
{
    { KFDF_LOCAL_REDIRECT_ONLY,     L"redirectonly" },
    { KFDF_ROAMABLE,                L"roamable" },
    { KFDF_PRECREATE,               L"precreate" },
    { KFDF_STREAM,                  L"streamable" },
    { KFDF_PUBLISHEXPANDEDPATH,     L"expandedpath" },
};

BOOL ArgToFlag(PCWSTR pszCat, KF_DEFINITION_FLAGS *pFlags)
{
    *pFlags = (KF_DEFINITION_FLAGS)0;
    BOOL fSuccess = FALSE;
    for (int i = 0; i < ARRAYSIZE(c_rgKFFlagMap); ++i)
    {
        if (0 == StrCmpIW(pszCat, c_rgKFFlagMap[i].pszFlagName))
        {
            *pFlags = c_rgKFFlagMap[i].flags;
            fSuccess = TRUE;
            break;
        }
    }
    return fSuccess;
}

struct
{
    KF_CATEGORY category;
    PCWSTR pszCategoryName;
}
const c_rgKFCategoryMap[] =
{
    { KF_CATEGORY_VIRTUAL,      L"virtual" },
    { KF_CATEGORY_FIXED,        L"fixed" },
    { KF_CATEGORY_COMMON,       L"common" },
    { KF_CATEGORY_PERUSER,      L"user" },
};

BOOL ArgToCategory(PCWSTR pszCat, KF_CATEGORY *pCategory)
{
    *pCategory = (KF_CATEGORY)0;
    BOOL fSuccess = FALSE;
    for (int i = 0; i < ARRAYSIZE(c_rgKFCategoryMap); ++i)
    {
        if (0 == StrCmpIW(pszCat, c_rgKFCategoryMap[i].pszCategoryName))
        {
            *pCategory = c_rgKFCategoryMap[i].category;
            fSuccess = TRUE;
            break;
        }
    }
    return fSuccess;
}

PCWSTR KFCategoryToString(KF_CATEGORY category)
{
    PCWSTR psz = NULL;
    for (int i = 0; i < ARRAYSIZE(c_rgKFCategoryMap); ++i)
    {
        if (category == c_rgKFCategoryMap[i].category)
        {
            psz = c_rgKFCategoryMap[i].pszCategoryName;
            break;
        }
    }
    return psz;
}

BOOL ParseAndValidateCommandLine(PWSTR *ppszArgs, int iArgs, ACTION_TYPE *at, KNOWNFOLDERID *pkfid,
                                 KNOWNFOLDER_DEFINITION *pkfd, PWSTR *ppszFindForPath)
{
    BOOL fSuccess = TRUE;
    *at = ACT_UNDEFINED;
    *ppszFindForPath = NULL;

    for (int i = 0; fSuccess && (i < iArgs); ++i)
    {
        if (wcsstr(ppszArgs[i], SZ_CLA_REGISTER))
        {
            *at = ACT_REGISTER;
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_ENUM))
        {
            *at = ACT_ENUM;
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_UNREGISTER))
        {
            *at = ACT_UNREGISTER;
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_CLEAN))
        {
            *at = ACT_CLEAN;
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_SHOW_USAGE))
        {
            *at = ACT_SHOW_USAGE;
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_CATEGORY))
        {
            PWSTR pszCat;
            fSuccess = ExtractParam(SZ_CLA_CATEGORY, ppszArgs[i], &pszCat);
            if (fSuccess)
            {
                fSuccess = ArgToCategory(pszCat, &pkfd->category);
                CoTaskMemFree(pszCat);
            }
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_DEFFLAG))
        {
            PWSTR pszCat;
            fSuccess = ExtractParam(SZ_CLA_DEFFLAG, ppszArgs[i], &pszCat);
            if (fSuccess)
            {
                KF_DEFINITION_FLAGS flags;
                fSuccess = ArgToFlag(pszCat, &flags);
                if (fSuccess)
                {
                    pkfd->kfdFlags |= flags;
                }
                CoTaskMemFree(pszCat);
            }
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_ID))
        {
            PWSTR pszKFID;
            fSuccess = ExtractParam(SZ_CLA_ID, ppszArgs[i], &pszKFID);
            if (fSuccess)
            {
                CLSIDFromString(pszKFID, pkfid);
                CoTaskMemFree(pszKFID);
            }
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_PSZNAME))
        {
            fSuccess = ExtractParam(SZ_CLA_PSZNAME, ppszArgs[i], &(pkfd->pszName));
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_PSZDESCRIPTION))
        {
            fSuccess = ExtractParam(SZ_CLA_PSZDESCRIPTION, ppszArgs[i], &(pkfd->pszDescription));
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_PSZRELPATH))
        {
            fSuccess = ExtractParam(SZ_CLA_PSZRELPATH, ppszArgs[i], &(pkfd->pszRelativePath));
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_PSZPARSENAME))
        {
            fSuccess = ExtractParam(SZ_CLA_PSZPARSENAME, ppszArgs[i], &(pkfd->pszParsingName));
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_PSZLOCALIZEDNAME))
        {
            fSuccess = ExtractParam(SZ_CLA_PSZLOCALIZEDNAME, ppszArgs[i], &(pkfd->pszLocalizedName));
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_PSZICON))
        {
            fSuccess = ExtractParam(SZ_CLA_PSZICON, ppszArgs[i], &(pkfd->pszIcon));
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_PSZTOOLTIP))
        {
            fSuccess = ExtractParam(SZ_CLA_PSZICON, ppszArgs[i], &(pkfd->pszTooltip));
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_PSZSECURITY))
        {
            fSuccess = ExtractParam(SZ_CLA_PSZSECURITY, ppszArgs[i], &(pkfd->pszSecurity));
        }
        else if (wcsstr(ppszArgs[i], SZ_CLA_FINDFORPATH))
        {
            *at = ACT_FIND_FOR_PATH;
            fSuccess = ExtractParam(SZ_CLA_FINDFORPATH, ppszArgs[i], ppszFindForPath);
        }
    }

    return fSuccess;
}

void GenerateResourcePath(PWSTR *ppsz, DWORD dwRID, BOOL fUseAtSign)
{
    WCHAR szExePath[MAX_PATH] = {};
    GetModuleFileName(NULL, szExePath, ARRAYSIZE(szExePath));
    size_t cch = ARRAYSIZE(szExePath) + 6;  // + 6 for ",11111"

    if (fUseAtSign)
    {
        cch += 1;  // for "@" prefix
    }

    *ppsz = (PWSTR)CoTaskMemAlloc(sizeof(WCHAR) * cch);
    if (*ppsz)
    {
        ZeroMemory(*ppsz, sizeof(WCHAR) * cch);
        StringCchPrintfW(*ppsz, cch, (fUseAtSign ? L"@%s,-%d" : L"%s,-%d"), szExePath, dwRID);
    }
}

// here we'll fill out all fields not entered by the user
void CompleteKnownFolderDef(KNOWNFOLDER_DEFINITION *pkfd)
{
    if (0 == pkfd->category)
    {
        pkfd->category = KF_CATEGORY_PERUSER;
    }

    if (NULL == pkfd->pszName)
    {
        pkfd->pszName = L"SDK Sample KnownFolder";
    }

    if (GUID_NULL == pkfd->fidParent)
    {
        // by default we root under user profile
        pkfd->fidParent = FOLDERID_Profile;
    }

    if (NULL == pkfd->pszDescription)
    {
        pkfd->pszDescription= L"This folder is a sample known folder";
    }

    if (NULL == pkfd->pszRelativePath)
    {
        pkfd->pszRelativePath = L"SDKSampleFolder";
    }

    if (NULL == pkfd->pszParsingName)
    {
        GUID guid;
        CoCreateGuid(&guid);
        pkfd->pszParsingName = (PWSTR)CoTaskMemAlloc(sizeof(WCHAR) * GUID_SIZE);
        if (pkfd->pszParsingName)
        {
            StringFromGUID2(guid, pkfd->pszParsingName, GUID_SIZE);
        }
    }

    if (NULL == pkfd->pszTooltip)
    {
        GenerateResourcePath(&pkfd->pszTooltip, IDS_KFSAMPLE_TOOLTIP, TRUE);
    }

    if (NULL == pkfd->pszLocalizedName)
    {
        GenerateResourcePath(&pkfd->pszLocalizedName, IDS_KFSAMPLE_LOCALIZEDNAME, TRUE);
    }

    if (NULL == pkfd->pszIcon)
    {
        GenerateResourcePath(&pkfd->pszIcon, IDI_KFSAMPLE_ICON, FALSE);
    }
}

void DumpUsage()
{
    wprintf(L"kfexplorer.exe /[register | enum | unregister | clean] </arg:param> ...\n\n");
    wprintf(L"\t/register may be combined with any of the <args> below.\n");
    wprintf(L"\t\t\tkfexplorer.exe /register\n");
    wprintf(L"\t\t\tkfexplorer.exe /register \"/pszName:Sample KnownFolder\" /category:user /pszRelativePath:SampleFolder \"/pszDescription:This KnownFolder is for samples!\"\n\n");
    wprintf(L"\t/enum may be combined with /pszName.  If /pszName is specified only KnownFolders with names containing /pszName will be enumerated\n");
    wprintf(L"\t\t\tkfexplorer.exe /enum\n");
    wprintf(L"\t\t\tkfexplorer.exe /enum /pszName:Fonts\n\n");
    wprintf(L"\t/unregister requires either /id or /pszPath\n");
    wprintf(L"\t\t\tkfexplorer.exe /unregister /id:{7B396E54-9EC5-4300-BE0A-2482EBAE1A26}\n\n");
    wprintf(L"\t<arg> may be any number of the following:\n");
    wprintf(L"\t\t/pszName\tNon-localized human readable name of KnownFolder\n");
    wprintf(L"\t\t/pszDescription\tDescription and purpose of the KnownFolder\n");
    wprintf(L"\t\t/fidParent\tKNOWNFOLDERID (GUID) of parent knownfolder\n");
    wprintf(L"\t\t/pszRelativePath\tPath of KnownFolder relative to pfidParent. If this folder does not exist it will be created.\n");
    wprintf(L"\t\t/pszTooltip\tResource path for tooltip string (i.e.: c:\\kf.dll,-119)\n");
    wprintf(L"\t\t/pszLocalizedName\tResource path for default localized name (i.e.: c:\\kf.dll,-119)\n");
    wprintf(L"\t\t/pszIcon\tResource path for custom folder icon (i.e.: c:\\kf.dll,-119)\n");
    wprintf(L"\t\t/pszSecurity\tSSDL formatted string describing default security descriptor\n");
    wprintf(L"\t\t/dwAttributes\tFolder attributes\n");
    wprintf(L"\t\t/defFlag\tUse this arg multiple times to build dwDefinitionFlags.\n");
    wprintf(L"\t\t\tpersonalize - Can display a personalized name for this folder\n");
    wprintf(L"\t\t\tlocal - Can redirect to local disk only\n");
    wprintf(L"\t\t\troam - Can be synched to another machine\n");
    wprintf(L"\t\t/category\tSpecify the KnownFolder category\n");
    wprintf(L"\t\t\tvirtual - virtual shell folders appear in the namespace but do not represent a physical folder (i.e.: Control Panel)\n");
    wprintf(L"\t\t\tfixed - folders not managed by shell and not redirectable (i.e.: C:\\Windows)\n");
    wprintf(L"\t\t\tcommon - folders used for sharing data between users (i.e.: C:\\users\\public\\desktop)\n");
    wprintf(L"\t\t\tuser - per user folders rooted in the user profile (i.e.: C:\\users\\<user>\\Pictures)\n");
}

void DumpKnownFolderDef(REFKNOWNFOLDERID kfid, KNOWNFOLDER_DEFINITION const &kfd)
{
    WCHAR szGuid[GUID_SIZE] = {};

    wprintf(L"KNOWNFOLDER_DEFINITION for: %s\n", kfd.pszName);
    wprintf(L"\tCategory: 0x%x (%s)\n", kfd.category, KFCategoryToString(kfd.category));

    StringFromGUID2(kfid, szGuid, ARRAYSIZE(szGuid));
    wprintf(L"\tKNOWNFOLDERID    : %s\n", szGuid);

    wprintf(L"\tpszName          : %s\n", kfd.pszName);
    wprintf(L"\tpszDescription   : %s\n", kfd.pszDescription);

    StringFromGUID2(kfd.fidParent, szGuid, ARRAYSIZE(szGuid));
    wprintf(L"\tfidParent        : %s\n", szGuid);

    wprintf(L"\tpszRelativePath  : %s\n", kfd.pszRelativePath);
    wprintf(L"\tpszParsingName   : %s\n", kfd.pszParsingName);
    wprintf(L"\tpszTooltip       : %s\n", kfd.pszTooltip);
    wprintf(L"\tpszLocalizedName : %s\n", kfd.pszLocalizedName);
    wprintf(L"\tpszIcon          : %s\n", kfd.pszIcon);
    wprintf(L"\tpszSecurity      : %s\n", kfd.pszSecurity);
    wprintf(L"\tdwAttributes     : %d\n", kfd.dwAttributes);
    wprintf(L"\tkfdFlags         : 0x%x\n", kfd.kfdFlags);
}

// You can get some information from IKnownFolder that you cannot get from
// the KNOWNFOLDER_DEFINITION.
void DumpKnownFolderInfo(IKnownFolder *pkf)
{
    KNOWNFOLDERID kfid = GUID_NULL;
    HRESULT hr = pkf->GetId(&kfid);
    if (SUCCEEDED(hr))
    {
        KNOWNFOLDER_DEFINITION kfd;
        ZeroMemory(&kfd, sizeof(kfd));
        hr = pkf->GetFolderDefinition(&kfd);
        if (SUCCEEDED(hr))
        {
            WCHAR szGuid[GUID_SIZE];
            StringFromGUID2(kfid, szGuid, ARRAYSIZE(szGuid));
            wprintf(L"IKnownFolder info for %s (%s)\n", kfd.pszName, szGuid);

            PWSTR pszPath = NULL;
            hr = pkf->GetPath(0, &pszPath);
            if (SUCCEEDED(hr))
            {
                wprintf(L"\tCurrent Path    : %s\n", pszPath);
                CoTaskMemFree(pszPath);
            }
            else
            {
                wprintf(L"\tERROR: IKnownFolder::GetPath() returned hr=0x%x\n", hr);
            }

            IShellItem *psi;
            hr = pkf->GetShellItem(0, IID_PPV_ARGS(&psi));
            if (SUCCEEDED(hr))
            {
                PWSTR psz;
                hr = psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &psz);
                if (SUCCEEDED(hr))
                {
                    wprintf(L"\tCurrent Location: %s\n", psz);
                    CoTaskMemFree(psz);
                }
                psi->Release();
            }
            else
            {
                wprintf(L"\tERROR: IKnownFolder::GetLocation() returned hr=0x%x\n", hr);
            }
        }
        else
        {
            wprintf(L"\tERROR: IKnownFolderManager::GetFolderDefinition() failed.  hr=0x%x\n", hr);
        }
    }
    else
    {
        wprintf(L"\tERROR: IKnownFolder::GetId() failed.  hr=0x%x\n", hr);
    }
}

HRESULT RemovePhysicalFolder(REFKNOWNFOLDERID kfid)
{
    IKnownFolderManager *pkfm;
    HRESULT hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pkfm));
    if (SUCCEEDED(hr))
    {
        IKnownFolder *pkf;
        hr = pkfm->GetFolder(kfid, &pkf);
        if (SUCCEEDED(hr))
        {
            PWSTR pszPath;
            hr = pkf->GetPath(0, &pszPath);
            if (SUCCEEDED(hr))
            {
                SHFILEOPSTRUCT fos = {};
                fos.wFunc = FO_DELETE;
                fos.pFrom = pszPath;
                fos.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
                if (0 != SHFileOperation(&fos))
                {
                    hr = E_FAIL;
                }

                CoTaskMemFree(pszPath);
            }
            pkf->Release();
        }
        pkfm->Release();
    }
    return hr;
}


void AddRegisteredFolderToHistory(KNOWNFOLDERID kfid)
{
    HKEY hKey;
    DWORD dwDisp = 0;
    if (ERROR_SUCCESS == RegCreateKeyExW(HKEY_LOCAL_MACHINE, SZ_REG_PATH_HISTORY, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp))
    {
        WCHAR szGuid[GUID_SIZE];
        StringFromGUID2(kfid, szGuid, ARRAYSIZE(szGuid));
        SHSetValueW(hKey, NULL, szGuid, REG_SZ, NULL, NULL);
        RegCloseKey(hKey);
    }
}

void UnregisterAllKFsAddedByThisTool(DWORD *pdwKFs)
{
    *pdwKFs = 0;
    HKEY hKey;
    if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE, SZ_REG_PATH_HISTORY, 0, KEY_ALL_ACCESS, &hKey))
    {
        DWORD dwValues = 0;
        DWORD cchMaxValueNameLen = 0;
        if (ERROR_SUCCESS == RegQueryInfoKeyW(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &dwValues, &cchMaxValueNameLen, NULL, NULL, NULL))
        {
            DWORD const cchValName = cchMaxValueNameLen + 1;   // add 1 for trailing NULL
            PWSTR pszValName = (PWSTR)CoTaskMemAlloc(sizeof(WCHAR) * cchValName);
            if (pszValName)
            {
                for (DWORD dw = 0; dw < dwValues; ++dw)
                {
                    DWORD cchValNameInOut = cchValName;
                    if (ERROR_SUCCESS == RegEnumValueW(hKey, dw, pszValName, &cchValNameInOut, NULL, NULL, NULL, NULL))
                    {
                        KNOWNFOLDERID kfid = GUID_NULL;
                        CLSIDFromString(pszValName, &kfid);
                        RemovePhysicalFolder(kfid);
                        HRESULT hr = UnregisterFolder(kfid);
                        if (SUCCEEDED(hr))
                        {
                            ++*pdwKFs;
                        }
                        else
                        {
                            wprintf(L"Failed to UnregisterFolder %s hr=0x%x\n", pszValName, hr);
                        }
                    }
                }
                CoTaskMemFree(pszValName);
            }
        }
        RegDeleteTree(hKey, NULL);
        RegCloseKey(hKey);
    }
}