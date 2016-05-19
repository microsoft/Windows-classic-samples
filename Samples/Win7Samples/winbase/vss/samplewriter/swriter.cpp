/*
    Copyright (c) 2009 Microsoft Corporation

    Module Name:
        swriter.cpp
*/


#include "stdafx.h"
#include "swriter.h"


#define __EVAL(X) X
#define __MERGE(A, B) A##B
#define __MAKE_WIDE(A) __MERGE(L, A)
#define __FUNCTION_WIDE__ __MAKE_WIDE(__EVAL(__FUNCTION__))


#define EXIT_ON_FAILURE(location)                                       \
{                                                                       \
    wprintf(L"(!) %s failed on the %s\n", __FUNCTION_WIDE__, location); \
    goto _exit;                                                         \
}                                                                       \


// GUID uniquely identifying the Writer
static const VSS_ID SampleWriterId =
    { 0x079462f2, 0x1079, 0x48dd, { 0xb3, 0xfb, 0xcc, 0xb2, 0xf2, 0x93, 0x4e, 0xc0 } };

// Name describing the Writer
static const WCHAR  g_wszSampleWriterName[] = L"Sample Writer";

// Subdirectory where files will be put during restore
static const WCHAR  g_wszAlternatePath[]    = L"Restored";

// Registry path where user profiles are stored
const WCHAR         g_wszProfileList[]      = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList";

// Queue representing internal Writer state, its initialized-state flag and critical section
CQueue              *g_queueRoot;
bool                g_bQueueInitialized;
CRITICAL_SECTION    g_cs;

//
// List of the components, their caption and corresponding wildcards
// For simplicity, Component field represents both component name as well as
// the directory in the user profile
// OnIdentify logic assumes that entries are sorted by the component name and
// terminating entry is filled with empty strings
//
SAMPLE_COMPONENT_TYPE   g_sctAll[]    =
{
    {L"Documents",  L"User Documents",  L"*.docx"},
    {L"Documents",  L"User Documents",  L"*.doc"},
    {L"Documents",  L"User Documents",  L"*.xlsx"},
    {L"Documents",  L"User Documents",  L"*.xls"},
    {L"Pictures",   L"User Images",     L"*.jpeg"},
    {L"Pictures",   L"User Images",     L"*.jpg"},
    {L"Pictures",   L"User Images",     L"*.jpe"},
    {L"Pictures",   L"User Images",     L"*.bmp"},
    {L"Pictures",   L"User Images",     L"*.png"},
    {L"Pictures",   L"User Images",     L"*.gif"},
    {L"",           L"",                L""}
};


//
// Concatenate First and Second string placing Binder between them
// Result is allocated by the function on the heap
// If Result was pointing to a memory location, that memory will be released
//
bool ConcatenateWith(
    PWSTR   *pwszResult,
    PCWSTR  wszFirst,
    PCWSTR  wszSecond,
    PCWSTR  wszBinder
)
{
    PWSTR   wszResult   = *pwszResult;
    DWORD   cchLength   = 0;
    bool    bResult     = false;

    *pwszResult = NULL;

    //
    // Calculate length and allocate new string
    //
    cchLength = (DWORD)wcslen(wszFirst) + (DWORD)wcslen(wszSecond) + 2;

    if (wszResult != NULL)
    {
        free(wszResult);
        wszResult = NULL;
    }

    wszResult = (WCHAR *)malloc(cchLength * sizeof(WCHAR));
    if (wszResult == NULL)
        goto _exit;

    // Merge strings
    if (FAILED(StringCchPrintfW(wszResult, cchLength, L"%s%s%s", wszFirst, wszBinder, wszSecond)))
        goto _exit;

    // Delegate ownership of the new string
    *pwszResult = wszResult;
    wszResult = NULL;
    bResult = true;

_exit:
    if (wszResult != NULL)
    {
        free(wszResult);
        wszResult = NULL;
    }
    return bResult;
}


//
// Concatenate First and Second string placing backslash between them
// Result is allocated by the function on the heap
// If Result was pointing to a memory location, that memory will be released
//
bool ConcatenateWithBackslash(
    PWSTR   *pwszResult,
    PCWSTR  wszFirst,
    PCWSTR  wszSecond
)
{
    return ConcatenateWith(pwszResult, wszFirst, wszSecond, L"\\");
}


//
// Search for the files that match given wildcard inside a directory
// Directory to search in is a combination of Path and SubDirectory
// For simplicity function does recursive search that goes up to 3 levels deep
//
bool ValidateDirectory(
    PCWSTR  wszPath,
    PCWSTR  wszSubDirectory,
    PCWSTR  wszMask,
    int     iDepth                  = 0
)
{
    WIN32_FIND_DATA sFindFileData   = {0};
    PWSTR   wszDirectory            = NULL;
    PWSTR   wszFiles                = NULL;
    PWSTR   wszDirectories          = NULL;
    bool    bResult                 = false;
    HANDLE  hFind                   = INVALID_HANDLE_VALUE;

    // Go up to 3 levels in depth
    if (iDepth == 3)
        goto _exit;

    // Pre-create all of the buffers
    if (!ConcatenateWithBackslash(&wszDirectory, wszPath, wszSubDirectory))
        EXIT_ON_FAILURE(L"first ConcatenateWithBackslash call");
    if (!ConcatenateWithBackslash(&wszFiles, wszDirectory, wszMask))
        EXIT_ON_FAILURE(L"second ConcatenateWithBackslash call");
    if (!ConcatenateWithBackslash(&wszDirectories, wszDirectory, L"*"))
        EXIT_ON_FAILURE(L"third ConcatenateWithBackslash call");

    // Search for files matching the wildcard provided
    hFind = FindFirstFileW(wszFiles, &sFindFileData);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        // go through the subdirectories if no files found
        hFind = FindFirstFileW(wszDirectories, &sFindFileData);
        if (INVALID_HANDLE_VALUE == hFind)
            goto _exit;
        do
        {
            // Discard current directory and parent directory
            if ((wcscmp(sFindFileData.cFileName, L".") == 0) || (wcscmp(sFindFileData.cFileName, L"..") == 0))
                continue;

            // Discard files
            if ((sFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                continue;

            //
            // Discard reparse points
            // This is a simple yet not fool proof method to avoid
            // cycles that could be introduced by the reparse points
            //
            if ((sFindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
                continue;

            // Recursively search for the files in sub directories
            if (ValidateDirectory(wszDirectory, sFindFileData.cFileName, wszMask, iDepth + 1))
            {
                bResult = true;
                goto _exit;
            }
        }
        while (FindNextFile(hFind, &sFindFileData) != 0);
        // None of the directories had the file
    }
    else
    {
        // File(s) found, return with success
        bResult = true;
    }

_exit:
    if (wszDirectory != NULL)
    {
        free(wszDirectory);
        wszDirectory = NULL;
    }
    if (wszFiles != NULL)
    {
        free(wszFiles);
        wszFiles = NULL;
    }
    if (wszDirectories != NULL)
    {
        free(wszDirectories);
        wszDirectories = NULL;
    }
    if (INVALID_HANDLE_VALUE == hFind)
    {
        FindClose(hFind);
    }
    return bResult;
}


//
// Constructor
//
SampleWriter::SampleWriter()
{
    wprintf(L"    SampleWriter::SampleWriter called\n");
}


//
// Destructor
//
SampleWriter::~SampleWriter()
{
    wprintf(L"    SampleWriter::~SampleWriter called\n");
    Uninitialize();
}


STDMETHODIMP SampleWriter::Uninitialize()
{
    wprintf(L"    SampleWriter::Uninitialize called\n");

    CQueue *queue = NULL;
    while (g_queueRoot != NULL)
    {
        queue = g_queueRoot->GetNext();
        delete g_queueRoot;
        g_queueRoot = queue;
    }
    queue = NULL;

    DeleteCriticalSection(&g_cs);

    return Unsubscribe();
}


//
// Writer initialization code
// This method controls Writer ID, name, usage type and source type
//
STDMETHODIMP SampleWriter::Initialize()
{
    HRESULT hr  = S_OK;

    wprintf(L"    SampleWriter::Initialize called\n");

    hr = CVssWriter::Initialize(
        SampleWriterId,         // ID
        g_wszSampleWriterName,  // name
        VSS_UT_USERDATA,        // usage type
        VSS_ST_OTHER);          // source type

    if (FAILED(hr))
    {
        wprintf(L"(!) CVssWriter::Initialize failed\n");
        return hr;
    }

    // Subscribe for events
    hr = Subscribe();
    if (FAILED(hr))
        wprintf(L"(!) CVssWriter::Subscribe failed\n");


    // Initialize the critical section preallocating necessary resources
    if (!InitializeCriticalSectionAndSpinCount(&g_cs, 0x80000400))
    {
        wprintf(L"(!) InitializeCriticalSectionAndSpinCount failed\n");
        hr = E_OUTOFMEMORY;
    }

    //
    // Set the initial values for the file group queue
    // This simple Writer is expected to always return the same set
    // of components and file groups
    // In order to update its internal state it has to be restarted
    // To make sure we create queue only once, g_bQueueInitialized flag
    // is used
    // This flag as well as the queue are protected by the critical section
    //
    g_queueRoot = NULL;
    g_bQueueInitialized = false;

    return hr;
}


//
// Helper function creating user@domain string based on the SID provided
// Account name is allocated by the function on the heap
//
STDMETHODIMP_(bool) SampleWriter::CreateAccountName(
    PWSTR   *pwszAccountName,
    PSID    pSid
)
{
    SID_NAME_USE    eSidNameUse         = SidTypeUnknown;
    DWORD           cchUserNameLength   = 0;
    PWSTR           wszUserName         = NULL;
    DWORD           cchDomainLength     = 0;
    PWSTR           wszDomain           = NULL;
    PWSTR           wszAccount          = NULL;
    BOOL            bResult             = FALSE;

    *pwszAccountName = NULL;

    // First call should fail and just return the required buffer sizes
    bResult = LookupAccountSidW(
        NULL,
        pSid,
        NULL,
        &cchUserNameLength,
        NULL,
        &cchDomainLength,
        &eSidNameUse);

    if (bResult != FALSE)
    {
        wprintf(L"(!) SampleWriter::CreateAccountName failed on the first LookupAccountSidW call\n");
        goto _exit;
    }

    //
    // Allocate buffers of a received size
    //
    wszUserName = (WCHAR *)malloc(cchUserNameLength * sizeof(WCHAR));
    if (wszUserName == NULL)
    {
        wprintf(L"(!) SampleWriter::CreateAccountName failed on the first malloc call\n");
        goto _exit;
    }

    wszDomain = (WCHAR *)malloc(cchDomainLength * sizeof(WCHAR));
    if (wszDomain == NULL)
    {
        wprintf(L"(!) SampleWriter::CreateAccountName failed on the second malloc call\n");
        goto _exit;
    }

    ZeroMemory(wszUserName, cchUserNameLength * sizeof(WCHAR));
    ZeroMemory(wszDomain, cchDomainLength * sizeof(WCHAR));

    // Look for the account and domain names
    bResult = LookupAccountSidW(
        NULL,
        pSid,
        wszUserName,
        &cchUserNameLength,
        wszDomain,
        &cchDomainLength,
        &eSidNameUse);

    // Should not fail since buffers were of a right size
    if (bResult == FALSE)
    {
        wprintf(L"(!) SampleWriter::CreateAccountName failed on the second LookupAccountSidW call\n");
        goto _exit;
    }

    //
    // We expect this SID to represent a regular user
    // In other cases just silently skip - those are expected
    //
    if (eSidNameUse != SidTypeUser)
        goto _exit;

    // Create the user@domain string
    if (!ConcatenateWith(&wszAccount, wszUserName, wszDomain, L"@"))
    {
        wprintf(L"(!) SampleWriter::CreateAccountName failed on the ConcatenateWith call\n");
        goto _exit;
    }

    // Delegate ownership of the new string
    *pwszAccountName = wszAccount;
    wszAccount = NULL;
    bResult = TRUE;

_exit:
    if (wszUserName != NULL)
    {
        free(wszUserName);
        wszUserName = NULL;
    }

    if (wszDomain != NULL)
    {
        free(wszDomain);
        wszDomain = NULL;
    }

    if (wszAccount != NULL)
    {
        free(wszAccount);
        wszAccount = NULL;
    }

    return (bResult != FALSE);
}


//
// Helper method that adds components representing user profile
// with all of its subcomponents
//
STDMETHODIMP_(bool) SampleWriter::AddComponent(
    IVssCreateWriterMetadata    *pMetadata,
    PCWSTR                      wszProfile,
    PCWSTR                      wszProfilePath
)
{
    HRESULT hr                      = S_OK;
    PWSTR   wszPath                 = NULL;
    PWSTR   wszAlternatePath        = NULL;
    PWSTR   wszPreviousComponent    = NULL;
    PWSTR   wszPathWithMask         = NULL;
    int     i                       = 0;
    bool    bResult                 = false;
    CQueue  *queue                  = NULL;

    // Add root component for this user profile
    hr = pMetadata->AddComponent(
        VSS_CT_FILEGROUP,   // component type
        NULL,               // logical path
        wszProfile,         // component name
        wszProfile,         // user friendly caption
        NULL,
        0,
        false,
        false,
        true,               // selectable
        true);              // selectable for restore

    // Cycle through all the data this Writer cares to protect
    for (i = 0; SUCCEEDED(hr) && (wcscmp(g_sctAll[i].wszComponent, L"") != 0); ++i)
    {
        //
        // This call will verify if files matching a certain wildcard exist
        // in the directory wszProfilePath
        // This logic exists to ensure that the Writer does not expose files
        // Requester cannot backup (i.e. files that do not exist on the volume)
        //
        // There are several simplifications taking place here
        // First: ValidateDirectory is not perfect and checks only up to
        // three levels deep into the subdirectories; in reality entire tree
        // should be validated
        // Second: not only we skip component creation, we skip adding elements
        // to the global queue
        // This will have impact on the PostRestore as explained further
        // in the appropriate event handler
        //
        // On the first note it may be worth mentioning that in most real life
        // implementation Writer is tightly coupled with some application
        // That application is most likely aware of all the existing as well as
        // potential data stores on the machine
        // In that case internal state may not exist at all and appropriate
        // collections (components, file groups, etc.) will be polled directly
        // from the application via interfaces Writer's author understands
        //
        if (!ValidateDirectory(wszProfilePath, g_sctAll[i].wszComponent, g_sctAll[i].wszFileGroupMask))
            continue;

        hr = S_OK;

        // The wszPreviousComponent variable protects us from creating the same component twice
        if ((wszPreviousComponent == NULL) || (wcscmp(wszPreviousComponent, g_sctAll[i].wszComponent) != 0))
        {
            //
            // This will be a subcomponent to the previous root user profile one
            // Please note it uses non-null logical path corresponding to the
            // name of the parent component
            //
            hr = pMetadata->AddComponent(
                VSS_CT_FILEGROUP,                   // component type
                wszProfile,                         // logical path
                g_sctAll[i].wszComponent,           // component name
                g_sctAll[i].wszComponentCaption,    // user friendly caption
                NULL,
                0,
                false,
                false,
                true,                               // selectable
                true);                              // selectable for restore

            //
            // Preserve most recent component name if component creation succeeded
            // and pre-create all the buffers required when adding files to the group
            //
            if (SUCCEEDED(hr))
            {
                wszPreviousComponent = g_sctAll[i].wszComponent;

                if (!ConcatenateWithBackslash(&wszPath, wszProfilePath, g_sctAll[i].wszComponent))
                    EXIT_ON_FAILURE(L"first ConcatenateWithBackslash call");
                if (!ConcatenateWithBackslash(&wszAlternatePath, wszPath, g_wszAlternatePath))
                    EXIT_ON_FAILURE(L"second ConcatenateWithBackslash call");
            }
        }

        // Add files to the file group and their alternate restore location
        if (SUCCEEDED(hr))
        {
            //
            // The set of files (represented by the wildcard) that we want to add
            // should be grouped by the most recently added component
            // To indicate that logical path and component name provided to this
            // call should match path and name provided to the AddComponent call
            //
            hr = pMetadata->AddFilesToFileGroup(
                wszProfile,                         // logical path
                g_sctAll[i].wszComponent,           // component name
                wszPath,                            // directory containing files
                g_sctAll[i].wszFileGroupMask,       // files mask
                true,                               // recursive
                NULL);

            // Make sure documents are not being overwritten during restore
            if (SUCCEEDED(hr))
            {
                //
                // Critical section acquired in this thread so it is safe
                // to look at the g_bQueueInitialized
                // In more complex Writer case we may recreate the internal
                // state instead of making sure we create it only once
                // It may be important to keep copy of the previous state
                // (or states) in more complex states
                // In order to understand when and why, several things should
                // be taken into account:
                // - Writer can query session ID from any event w/ GetSessionId
                // - Before the OnPrepareBackup call ID will be GUID_NULL and
                //   new ID won't be created until the OnPrepareSnapshot
                // - Sequence from OnPrepareSnapshot to OnPostSnapshot is
                //   always being serialized (no concurrent snapshot sessions
                //   may exist)
                //
                if (g_bQueueInitialized == false)
                {
                    //
                    // First create internal state describing file group
                    // that's being added to the component so it can be used
                    // during PostSnapshot and PostRestore
                    //
                    if (!ConcatenateWithBackslash(&wszPathWithMask, wszPath, g_sctAll[i].wszFileGroupMask))
                        EXIT_ON_FAILURE(L"third ConcatenateWithBackslash call");
                    queue = new CQueue(wszPathWithMask, wszProfile, g_sctAll[i].wszComponent);
                    if (queue == NULL)
                        EXIT_ON_FAILURE(L"CQueue operator new");
                    g_queueRoot = queue->Enqueue(g_queueRoot);
                }

                //
                // This call is not bound to the component or file group
                // and can be performed at any time during the OnIdentify
                // It makes more sense to put it here in this case,
                // but it's perfectly valid to call it once
                //      (e.g. to have one location where the entire
                //      directory structure is being restored to)
                //
                hr = pMetadata->AddAlternateLocationMapping(
                    wszPath,                        // directory containing files
                    g_sctAll[i].wszFileGroupMask,   // files mask
                    true,                           // recursive
                    wszAlternatePath);
            }
        }
    }

    bResult = SUCCEEDED(hr);

_exit:
    // Don't release wszPreviousComponent - it's just a pointer!
    if (wszPath != NULL)
    {
        free(wszPath);
        wszPath = NULL;
    }
    if (wszPathWithMask != NULL)
    {
        free(wszPathWithMask);
        wszPathWithMask = NULL;
    }
    if (wszAlternatePath != NULL)
    {
        free(wszAlternatePath);
        wszAlternatePath = NULL;
    }
    return bResult;
}


//
// Helper method that generates data representing user profile in the system
// and passes it further so the appropriate components can be created
//
STDMETHODIMP_(bool) SampleWriter::AddComponentForUserProfile(
    IVssCreateWriterMetadata    *pMetadata,
    PCWSTR                      wszSid
)
{
    PWSTR   wszKeyName              = NULL;
    DWORD   cchProfilePathLength    = 100;
    PWSTR   wszProfilePath          = NULL;
    DWORD   cchExpandedPathLength   = 0;
    PWSTR   wszExpandedPath         = NULL;
    PWSTR   wszAccount              = NULL;
    DWORD   dwProfileFlags          = 0;
    DWORD   dwSize                  = sizeof(DWORD);    // initialized upfront to the dwProfileFlags size
    HKEY    hkeyProfile             = NULL;
    LONG    lStatus                 = 0;
    PSID    pSid                    = NULL;
    bool    bResult                 = false;

    if (!ConcatenateWithBackslash(&wszKeyName, g_wszProfileList, wszSid))
        EXIT_ON_FAILURE(L"ConcatenateWithBackslash call");

    if (ERROR_SUCCESS != RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        wszKeyName,
        0,
        KEY_READ,
        &hkeyProfile))
        EXIT_ON_FAILURE(L"RegOpenKeyEx call");

    lStatus = RegQueryValueExW(
        hkeyProfile,
        L"Flags",
        NULL,
        NULL,
        (LPBYTE)&dwProfileFlags,
        &dwSize);

    // Skip atypical profiles and keys without flags value
    if ((dwProfileFlags != 0) || (lStatus != ERROR_SUCCESS))
    {
        bResult = true;
        goto _exit;
    }

    //
    // Query for the user's local profile location
    //
    wszProfilePath = (WCHAR *)malloc(cchProfilePathLength * sizeof(WCHAR));
    if (wszProfilePath == NULL)
        EXIT_ON_FAILURE(L"first malloc call");
    ZeroMemory(wszProfilePath, cchProfilePathLength * sizeof(WCHAR));

    do
    {
        dwSize = (cchProfilePathLength - 1) * sizeof(WCHAR);
        lStatus = RegQueryValueExW(
            hkeyProfile,
            L"ProfileImagePath",
            NULL,
            NULL,
            (LPBYTE)wszProfilePath,
            &dwSize);

        // Resize the buffer to accommodate the data
        if (lStatus == ERROR_MORE_DATA)
        {
            cchProfilePathLength *= 2;
            wszProfilePath = (WCHAR *)realloc(wszProfilePath, cchProfilePathLength * sizeof(WCHAR));
            if (wszProfilePath == NULL)
                EXIT_ON_FAILURE(L"realloc call");
            ZeroMemory(wszProfilePath, cchProfilePathLength * sizeof(WCHAR));
        }
    } while(lStatus == ERROR_MORE_DATA);

    if (lStatus != ERROR_SUCCESS)
        EXIT_ON_FAILURE(L"first RegQueryValueExW call");

    //
    // Expand environment variables in the profile path
    //
    cchExpandedPathLength = cchProfilePathLength + 100;
    wszExpandedPath = (WCHAR *)malloc(cchExpandedPathLength * sizeof(WCHAR));
    if (wszExpandedPath == NULL)
        EXIT_ON_FAILURE(L"second malloc call");
    ZeroMemory(wszExpandedPath, cchExpandedPathLength * sizeof(WCHAR));

    dwSize = ExpandEnvironmentStringsW(wszProfilePath, wszExpandedPath, cchExpandedPathLength);
    if (!dwSize || dwSize > cchExpandedPathLength)
        EXIT_ON_FAILURE(L"ExpandEnvironmentStringsW call");

    //
    // Get SID for the account using this profile
    //
    if (ERROR_SUCCESS != RegQueryValueExW(
        hkeyProfile,
        L"Sid",
        NULL,
        NULL,
        NULL,
        &dwSize))
    {
        // Some keys have no Sid value - those are not interesting to us
        bResult = true;
        goto _exit;
    }

    pSid = LocalAlloc(LPTR, dwSize);

    if (!pSid)
        EXIT_ON_FAILURE(L"LocalAlloc call");

    if (ERROR_SUCCESS != RegQueryValueExW(
        hkeyProfile,
        L"Sid",
        NULL,
        NULL,
        (LPBYTE)pSid,
        &dwSize))
        EXIT_ON_FAILURE(L"third RegQueryValueExW call");

    //
    // Get the friendly account name
    // Account name will contain both domain and user name
    // Since it is prohibited for the component name to contain
    // backslash, user@domain format will be used
    //
    if (!CreateAccountName(&wszAccount, pSid))
        EXIT_ON_FAILURE(L"CreateAccountName call");

    if (!AddComponent(pMetadata, wszAccount, wszExpandedPath))
        EXIT_ON_FAILURE(L"AddComponent call");

    bResult = true;

_exit:
    if (wszKeyName != NULL)
    {
        free(wszKeyName);
        wszKeyName = NULL;
    }

    if (wszProfilePath != NULL)
    {
        free(wszProfilePath);
        wszProfilePath = NULL;
    }

    if (wszExpandedPath != NULL)
    {
        free(wszExpandedPath);
        wszExpandedPath = NULL;
    }

    if (wszAccount != NULL)
    {
        free(wszAccount);
        wszAccount = NULL;
    }

    if (hkeyProfile != NULL)
    {
        RegCloseKey(hkeyProfile);
        hkeyProfile = NULL;
    }

    if (pSid != NULL)
    {
        LocalFree(pSid);
        pSid = NULL;
    }

    return bResult;
}


//
// Helper method that generates the list of all the user profiles existing
// in the system and hand component generation over to another helper method
//
STDMETHODIMP_(bool) SampleWriter::AddComponents(
    IVssCreateWriterMetadata    *pMetadata
)
{
    DWORD       cchKeyNameLength    = 100;
    PWSTR       wszKeyName          = NULL;
    DWORD       dwKeyIndex          = 0;
    HKEY        hkeyProfiles        = NULL;
    LONG        lStatus             = 0;
    bool        bResult             = false;

    // Open the profile list
    lStatus = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        g_wszProfileList,
        0,
        KEY_READ,
        &hkeyProfiles);

    if (lStatus != ERROR_SUCCESS)
        EXIT_ON_FAILURE(L"RegOpenKeyExW call");

    // Allocate the buffer for the key name
    wszKeyName = (WCHAR *)malloc(cchKeyNameLength * sizeof(WCHAR));
    if (wszKeyName == NULL)
        EXIT_ON_FAILURE(L"malloc call");
    ZeroMemory(wszKeyName, cchKeyNameLength * sizeof(WCHAR));

    for (dwKeyIndex = 0; ; ++dwKeyIndex)
    {
        //
        // Get the next key name
        //
        do
        {
            lStatus = RegEnumKeyW(hkeyProfiles, dwKeyIndex, wszKeyName, cchKeyNameLength);

            // Resize the buffer to accommodate the data
            if (lStatus == ERROR_MORE_DATA)
            {
                cchKeyNameLength *= 2;
                wszKeyName = (WCHAR *)realloc(wszKeyName, cchKeyNameLength * sizeof(WCHAR));
                if (wszKeyName == NULL)
                    EXIT_ON_FAILURE(L"realloc call");
                ZeroMemory(wszKeyName, cchKeyNameLength * sizeof(WCHAR));
            }
        } while(lStatus == ERROR_MORE_DATA);


        // Exit 'for' loop if done
        if (lStatus == ERROR_NO_MORE_ITEMS)
            break;

        if (lStatus != ERROR_SUCCESS)
            EXIT_ON_FAILURE(L"RegEnumKeyW call");

        // Adding a profile component is best effort operation
        if (!AddComponentForUserProfile(pMetadata, wszKeyName))
            EXIT_ON_FAILURE(L"AddComponentForUserProfile call");
    }

    // Passed all the logic, success
    bResult = true;

    //
    // Exit with cleanup
    //
_exit:
    if (wszKeyName != NULL)
    {
        free(wszKeyName);
        wszKeyName = NULL;
    }
    if (hkeyProfiles != NULL)
    {
        RegCloseKey(hkeyProfiles);
        hkeyProfiles = NULL;
    }

    return bResult;
}


//
// OnIdentify event handler
//
// It is called as a result of the Requester calling GatherWriterMetadata
// Writer should express its metadata by describing how Requesters should
// deal with it during backup and restore and which files it covers
//
STDMETHODIMP_(bool) SampleWriter::OnIdentify(
    IVssCreateWriterMetadata    *pMetadata
)
{
    HRESULT hr      = S_OK;
    bool    bResult = false;

    wprintf(L"    SampleWriter::OnIdentify called\n");

    //
    // Set the restore method to restore to alternate location
    // We will use AddAlternateLocationMapping method in the AddComponent
    // to detail alternate restore locations
    //
    hr = pMetadata->SetRestoreMethod(
        VSS_RME_RESTORE_TO_ALTERNATE_LOCATION,
        NULL,
        NULL,
        VSS_WRE_ALWAYS,
        false);

    if (FAILED(hr))
        EXIT_ON_FAILURE(L"SetRestoreMethod call");

    // Protect the global queue and its flag with critical section
    EnterCriticalSection(&g_cs);

    //
    // Add components and files that they cover
    // Please note files for deletion may be added as well
    // In that case it is expected that Writer will delete them from
    // the snapshot during the OnPostSnapshot event
    // It is important to make sure that no files are exposed twice
    // by the same component, different components of the Writer or
    // different Writers altogether
    // Furthermore same files should not be exposed as part of
    // the component and as files for deletion
    // This is important when deciding on the wildcards for the file groups
    //
    if (!AddComponents(pMetadata))
        EXIT_ON_FAILURE(L"AddComponents call");

    //
    // Returning false from OnIdentify has very strong implications
    // Unlike other events it does not indicate error on the Requester side
    // Instead it makes Writer completely invisible to the Requester
    // For example if one executes vssadmin list Writers from the command line,
    // this Writer will not be included in the list at all if we return false
    //
    // For code brevity this Writer does not register itself with the EventLog
    // Production Writers however are obliged to report details of the failure
    // in the EventLog if false is being returned from OnIdentify
    // Despite the fact that Writer will become invisible to the Requesters,
    // some Requesters may be tweaked to work with this specific Writer
    // Those Requesters are expected to point users to the EvenLog for
    // further details on the problem
    //
    g_bQueueInitialized = true;
    LeaveCriticalSection(&g_cs);
    bResult = true;

_exit:
    return bResult;
}


//
// OnPrepareBackup event handler
//
// This is called as a result of the Requester calling PrepareForBackup
// This indicates to the Writer that a backup sequence has started
//
STDMETHODIMP_(bool) SampleWriter::OnPrepareBackup(
    IVssWriterComponents    *pComponents
)
{
    UNREFERENCED_PARAMETER(pComponents);
    wprintf(L"    SampleWriter::OnPrepareBackup called\n");
    return true;
}


//
// OnPrepareSnapshot event handler
//
// This is called as a result of the Requester calling DoSnapshotSet
// Time consuming actions required before I/O to the disk will be blocked
// prior to the snapshot should be performed here
// Please note that this event will be initiated by the VSS Service
// on behalf of the Requester
//
STDMETHODIMP_(bool) SampleWriter::OnPrepareSnapshot()
{
    wprintf(L"    SampleWriter::OnPrepareSnapshot called\n");
    return true;
}


//
// OnFreeze event handler
//
// This is called as a result of the Requester calling DoSnapshotSet
// At this stage Writer should communicate to its application that I/O
// on the disk will be held for a short period of time
// Writer is expected to handle this event quickly as it is essential for
// the system performance to get through the Freeze-Thaw sequence swiftly
// Please note that this event will be initiated by the VSS Service
// on behalf of the Requester
//
STDMETHODIMP_(bool) SampleWriter::OnFreeze()
{
    wprintf(L"    SampleWriter::OnFreeze called\n");
    return true;
}


//
// OnThaw event handler
//
// This is called as a result of the Requester calling DoSnapshotSet
// At this stage Writer should communicate to its application that I/O
// on the disk has been resumed
// Writer is expected to handle this event quickly
// Please note that this event will be initiated by the VSS Service
// on behalf of the Requester
//
STDMETHODIMP_(bool) SampleWriter::OnThaw()
{
    wprintf(L"    SampleWriter::OnThaw called\n");
    return true;
}


//
// OnPostSnapshot event handler
//
// This is called as a result of the Requester calling DoSnapshotSet
// At this stage Writer gets access to the shadow copy of the volumes that were
// snapshotted and is expected to perform auto recovery if needed
// Please note that this event will be initiated by the VSS Service
// on behalf of the Requester
//
STDMETHODIMP_(bool) SampleWriter::OnPostSnapshot(
    IVssWriterComponents    *pComponents
)
{
    UINT    uiComponentCount    = 0;
    UINT    uiCounter           = 0;
    HRESULT hr                  = S_OK;
    bool    bResult             = false;
    LONG    lContext            = 0;

    wprintf(L"    SampleWriter::OnPostSnapshot called\n");

    lContext = GetContext();

    // Perform auto recovery if possible (not blocked by the Requester)
    if (lContext & VSS_VOLSNAP_ATTR_AUTORECOVER)
    {
        hr = pComponents->GetComponentCount(&uiComponentCount);
        if (FAILED(hr))
            EXIT_ON_FAILURE(L"GetComponentCount call");

        //
        // For the purpose of this sample we will only display the list
        // of components that were backed up and files on the disk
        // corresponding to those components
        //
        // In the real scenario Writer would most likely discover all
        // of the files corresponding to the components that were
        // backed up and if there is some post processing on them needed,
        // appropriate actions would have been taken
        // In case of documents and images from the snapshot there is nothing
        // interesting we can do with them
        //
        // Writer has to match files on the source volume with files on
        // the snapshot volume and perform auto-recovery on the snapshot,
        // which at this point is available read/write
        //
        // To get to the volume name one should call GetVolumePathName
        // on the files that were backed up, followed by the call to
        // GetVolumeNameForVolumeMountPoint to get the original volume
        // VSS API GetSnapshotDeviceName then finds the corresponding
        // snapshot device name that should be used to get to the
        // file system on the snapshot
        //

        for (uiCounter = 0; uiCounter < uiComponentCount; ++uiCounter)
        {
            CComPtr<IVssComponent>  pComponent;
            CComBSTR                bstrComponentName;
            CQueue                  *queue              = NULL;

            hr = pComponents->GetComponent(uiCounter, &pComponent);
            if (FAILED(hr))
                EXIT_ON_FAILURE(L"GetComponent call");

            hr = pComponent->GetComponentName(&bstrComponentName);
            if (FAILED(hr))
                EXIT_ON_FAILURE(L"GetComponentName call");

            // OLE2T should not be used in busy loops as it allocates
            wprintf(L"(+) Component: %s\n", OLE2T(bstrComponentName));

            //
            // Search for all the matching components
            //
            // Our Writer state (queue) does not change during runtime and
            // is global (spans across all the sessions i.e. is not session
            // bound) so we would get away without using this lock
            // For completeness however we'd like to highlight that
            // one of the most important parts of the Writer is its ability
            // to track the state between different events
            //
            EnterCriticalSection(&g_cs);
            queue = g_queueRoot;
            while (queue != NULL)
            {
                //
                // When searching for the match it is important to remember
                // that user potentially selected component with subcomponents
                // In that case we have to match the value received from the
                // GetComponentName call with the entire component path
                //

                // First let's see if there's a direct component name match
                if (_wcsicmp(queue->GetComponentName(), OLE2T(bstrComponentName)) == 0)
                {
                    wprintf(
                        L"    (*) Files in this component (%s): %s\n",
                        queue->GetComponentName(),
                        queue->GetPath());
                }

                //
                // Now let's see if we match one of the subcomponents
                // Please note that this code assumes that only one level below
                // the main component exists and simple path match is sufficient
                // In the rare case of deeper component nesting, it may be
                // required to parse the path looking for the '\' separators
                //
                if (_wcsicmp(queue->GetComponentPath(), OLE2T(bstrComponentName)) == 0)
                {
                    wprintf(
                        L"    (*) Files in subcomponent %s: %s\n",
                        queue->GetComponentPath(),
                        queue->GetPath());
                }

                queue = queue->GetNext();
            }
            LeaveCriticalSection(&g_cs);
        }
    }

    bResult = true;

_exit:
    if (!bResult)
    {
        // Set error
    }
    return bResult;
}


//
// OnAbort event handler
//
// This function is called to abort the Writer's backup sequence
// This should only be called between OnPrepareBackup and OnPostSnapshot
// Writer may perform cleanup of some internal state it holds
//
STDMETHODIMP_(bool) SampleWriter::OnAbort()
{
    wprintf(L"    SampleWriter::OnAbort called\n");
    return true;
}


//
// OnBackupComplete event handler
//
// This function is called as a result of the Requester calling BackupComplete
// This is simply a notification for the Writer that backup is done
// and that it succeeded
// As a notification it is discouraged to raise errors from this event
// by returning false
//
STDMETHODIMP_(bool) SampleWriter::OnBackupComplete(
    IVssWriterComponents    *pComponents
)
{
    UNREFERENCED_PARAMETER(pComponents);
    wprintf(L"    SampleWriter::OnBackupComplete called\n");
    return true;
}


//
// OnBackupShutdown event handler
//
// This function is called at the end of the backup process, most likely after
// the OnBackupComplete call when Requester releases IVssBackupComponent
// This event may also happen as a result of the Requester shutting down
// or as a result of abnormal termination of the Requester
//
bool STDMETHODCALLTYPE SampleWriter::OnBackupShutdown(
    VSS_ID  id
)
{
    UNREFERENCED_PARAMETER(id);
    wprintf(L"    SampleWriter::OnBackupShutdown called\n");
    return true;
}


//
// OnPreRestore event handler
//
// This function is called as a result of the Requester calling PreRestore
// This will be called immediately before resting files
//
bool STDMETHODCALLTYPE SampleWriter::OnPreRestore(
    IVssWriterComponents    *pComponents
)
{
    UNREFERENCED_PARAMETER(pComponents);
    wprintf(L"    SampleWriter::OnPreRestore called\n");
    return true;
}


//
// OnPostRestore event handler
//
// This function is called as a result of the Requester calling PreRestore
// This will be called immediately after files being restored
//
bool STDMETHODCALLTYPE SampleWriter::OnPostRestore(
    IVssWriterComponents    *pComponents
)
{
    UINT    uiComponentCount    = 0;
    UINT    uiCounter           = 0;
    HRESULT hr                  = S_OK;
    bool    bResult             = false;

    wprintf(L"    SampleWriter::OnPostRestore called\n");

    hr = pComponents->GetComponentCount(&uiComponentCount);
    if (FAILED(hr))
        EXIT_ON_FAILURE(L"GetComponentCount call");

    //
    // Similarily to the OnPostSnapshot event we will match components
    // involved in this session with the previously created list of
    // file groups
    //
    // As mentioned in the AddComponent this logic is a simplification
    // of what real Writer would do
    // This simplification comes from the fact that our restore depends
    // on the queue constructed during the OnIdentify event
    // This means that if data in the stores (Documens of Pictures
    // within the user profile directory) has changed, restore may not
    // be complete
    // Alternatively we could depend on the g_sctAll variable
    // However this has its limitations as well
    // In real life Writer comes with the application and that application
    // knows its existing as well as potential stores
    // Our implementation knows only about the user profiles that
    // exist in the registry now so e.g. restoring files that belong
    // to the user who no longer has profile on this machine would be
    // impossible
    //
    // So for the real implementation what you most likely want is
    // the matching the components that were restored with your knowledge
    // of what application can deal with
    //
    for (uiCounter = 0; uiCounter < uiComponentCount; ++uiCounter)
    {
        CComPtr<IVssComponent>  pComponent;
        CComBSTR                bstrComponentName;
        CQueue                  *queue              = NULL;

        hr = pComponents->GetComponent(uiCounter, &pComponent);
        if (FAILED(hr))
            EXIT_ON_FAILURE(L"GetComponent call");

        hr = pComponent->GetComponentName(&bstrComponentName);
        if (FAILED(hr))
            EXIT_ON_FAILURE(L"GetComponentName call");

        // OLE2T should not be used in busy loops as it allocates
        wprintf(L"(+) Component: %s\n", OLE2T(bstrComponentName));

        //
        // Search for all the matching components
        //
        // Our Writer state (queue) does not change during runtime and
        // is global (spans across all the sessions i.e. is not session
        // bound) so we would get away without using this lock
        // For completeness however we'd like to highlight that
        // one of the most important parts of the Writer is its ability
        // to track the state between different events
        //
        EnterCriticalSection(&g_cs);
        queue = g_queueRoot;
        while (queue != NULL)
        {
            //
            // When searching for the match it is important to remember
            // that user potentially selected component with subcomponents
            // In that case we have to match the value received from the
            // GetComponentName call with the entire component path
            //
            // To prevent Writer from overwriting files on your system
            // no actual move will be performed
            // In real life files would be moved from the alternate folder
            // to the main location or in some ways merged, depending on
            // the type of information they contain
            //
            if ((_wcsicmp(queue->GetComponentName(), OLE2T(bstrComponentName)) == 0) ||
                (_wcsicmp(queue->GetComponentPath(), OLE2T(bstrComponentName)) == 0))
            {
                wprintf(
                    L"    (*) Files in the directory: %s\\%s\n",
                    queue->GetPath(),
                    g_wszAlternatePath);
                wprintf(
                    L"        (*) Move to the directory: %s\n",
                    queue->GetPath());
            }

            queue = queue->GetNext();
        }
        LeaveCriticalSection(&g_cs);
    }

    bResult = true;

_exit:
    if (!bResult)
    {
        // Set error
    }
    return bResult;
}


