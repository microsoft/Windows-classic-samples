#include "stdafx.h"

using namespace regfs;

RegfsProvider::RegfsProvider()
{
    // Record that this class implements the optional Notify callback.
    this->SetOptionalMethods(OptionalMethods::Notify);
}

/*++

Description:

    ProjFS invokes this callback to request metadata information for a file or a directory.

    If the file or directory exists in the provider's namespace, the provider calls
    WritePlaceholderInfo() to give ProjFS the info for the requested name.

    The metadata information ProjFS supports includes:

        Mandatory:
            FileBasicInfo.IsDirectory - the requested name is a file or directory.

        Mandatory for files:
            FileBasicInfo.FileSize - file size in bytes.

        Optional:
            VersionInfo - A 256 bytes ID which can be used to distinguish different versions of file content
                          for one file name.
            FileBasicInfo.CreationTime/LastAccessTime/LastWriteTime/ChangeTime - timestamps of the file.
            FileBasicInfo.FileAttributes - File Attributes.

        Optional and less commonly used:
            EaInformation - Extended attribute (EA) information.
            SecurityInformation - Security descriptor information.
            StreamsInformation - Alternate data stream information.

    See also PRJ_PLACEHOLDER_INFORMATION structure in projectedfslib.h for more details.

    If the file or directory doesn't exist in the provider's namespace, this callback returns
    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND).

    If the provider is unable to process the request (e.g. due to network error) or it wants to block
    the request, this callback returns an appropriate HRESULT error code.

    Below is a list of example commands that demonstrate how GetPlaceholderInfo is called.

    Assuming z:\reg doesn't exist, run 'regfs.exe z:\reg' to create the root.
    Now start another command line window, 'cd z:\reg' then run below commands in sequence.

    1) cd HKEY_LOCAL_MACHINE
       The first time you cd into a folder that exists in provider's namespace, GetPlaceholderInfo is
       called with CallbackData->FilePathName = "HKEY_LOCAL_MACHINE".  This callback will cause an
       on-disk placeholder file called "HKEY_LOCAL_MACHINE" to be created under z:\reg.

    2) cd .. & cd HKEY_LOCAL_MACHINE
       The second and subsequent time you cd into a folder that exists in provider's namespace, this
       callback will not be called because the on-disk placeholder for HKEY_LOCAL_MACHINE already exists.

    3) mkdir newfolder
       If _readonlyNamespace is true, GetPlaceholderInfo returns ERROR_ACCESS_DENIED, so the mkdir command
       reports "Access is denied" and the placeholder is not created.  If _readonlyNamespace is false,
       GetPlaceholderInfo returns ERROR_FILE_NOT_FOUND so the command succeeds and newfolder is created.

    4) cd SOFTWARE\Microsoft\.NETFramework
       The first time you cd into a deep path, GetPlaceholderInfo is called repeatedly with the
       following CallbackData->FilePathName values:
       1) "HKEY_LOCAL_MACHINE\SOFTWARE"
       2) "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft"
       3) "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework"

--*/

HRESULT RegfsProvider::GetPlaceholderInfo(
    _In_    const PRJ_CALLBACK_DATA*    CallbackData
)
{
    wprintf(L"\n----> %hs: Path [%s] triggered by [%s] \n",
            __FUNCTION__, CallbackData->FilePathName, CallbackData->TriggeringProcessImageFileName);

    bool isKey;
    INT64 valSize = 0;

    // Find out whether the specified path exists in the registry, and whether it is a key or a value.
    if (_regOps.DoesKeyExist(CallbackData->FilePathName))
    {
        isKey = true;
    }
    else if (_regOps.DoesValueExist(CallbackData->FilePathName, valSize))
    {
        isKey = false;
    }
    else
    {
        wprintf(L"<---- %hs: return 0x%08x\n",
                __FUNCTION__, ERROR_FILE_NOT_FOUND);
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    // Format the PRJ_PLACEHOLDER_INFO structure.  For registry keys we create directories on disk,
    // for values we create files.
    PRJ_PLACEHOLDER_INFO placeholderInfo = {};
    placeholderInfo.FileBasicInfo.IsDirectory = isKey;
    placeholderInfo.FileBasicInfo.FileSize = valSize;

    // Create the on-disk placeholder.
    HRESULT hr = this->WritePlaceholderInfo(CallbackData->FilePathName,
                                            &placeholderInfo,
                                            sizeof(placeholderInfo));

    wprintf(L"<---- %hs: return 0x%08x\n",
            __FUNCTION__, hr);

    return hr;
}

/*++

Description:

    ProjFS invokes this callback to tell the provider that a directory enumeration is starting.

    A user-mode tool usually uses FindFirstFile/FindNextFile APIs to enumerate a directory.  Those
    APIs send QueryDirectory requests to the file system.  If the enumeration is for a placeholder
    folder, ProjFS intercepts and blocks those requests.  Then ProjFS invokes the registered directory
    enumeration callbacks (StartDirEnum, GetDirEnum, EndDirEnum) to get a list of names in provider's
    namespace, merges those names with the names physically on disk under that folder, then unblocks
    the enumeration requests and returns the merged list to the caller.

--*/

HRESULT RegfsProvider::StartDirEnum(
    _In_     const PRJ_CALLBACK_DATA*   CallbackData,
    _In_     const GUID*                EnumerationId
)
{
    wprintf(L"\n----> %hs: Path [%s] triggerred by [%s]\n",
            __FUNCTION__, CallbackData->FilePathName, CallbackData->TriggeringProcessImageFileName);

    // For each dir enum session, ProjFS sends:
    //      one StartEnumCallback
    //      one or more GetEnumCallbacks
    //      one EndEnumCallback
    // These callbacks will use the same value for EnumerationId for the same session.
    // Here we map the EnumerationId to a new DirInfo object.
    _activeEnumSessions[*EnumerationId] = std::make_unique<DirInfo>(CallbackData->FilePathName);

    wprintf(L"<---- %hs: return 0x%08x\n",
            __FUNCTION__, S_OK);

    return S_OK;
};

/*++

Description:

    ProjFS invokes this callback to tell the provider that a directory enumeration is over.  This
    gives the provider the opportunity to release any resources it set up for the enumeration.

--*/

HRESULT RegfsProvider::EndDirEnum(
    _In_     const PRJ_CALLBACK_DATA*   CallbackData,
    _In_     const GUID*                EnumerationId
)
{
    wprintf(L"\n----> %hs\n",
            __FUNCTION__);

    // Get rid of the DirInfo object we created in StartDirEnum.
    _activeEnumSessions.erase(*EnumerationId);

    wprintf(L"<---- %hs: return 0x%08x\n",
            __FUNCTION__, S_OK);

    return S_OK;
};

/*++

Description:

    ProjFS invokes this callback to request a list of files and directories under the given directory.

    To handle this callback, RegFS calls DirInfo->FillFileEntry/FillDirEntry for each matching file
    or directory.

    If the SearchExpression argument specifies something that doesn't exist in provider's namespace,
    or if the directory being enumerated is empty, the provider just returns S_OK without storing
    anything in DirEntryBufferHandle.  ProjFS will return the correct error code to the caller.

    Below is a list of example commands that will invoke GetDirectoryEntries callbacks.
    These assume you've cd'd into the virtualization root folder.

    Command                  CallbackData->FilePathName    SearchExpression
    ------------------------------------------------------------------------
    dir                      ""(root folder)               *
    dir foo*                 ""(root folder)               foo*
    dir H + TAB              ""(root folder)               H*
    dir HKEY_LOCAL_MACHINE   ""(root folder)               HKEY_LOCAL_MACHINE
    dir HKEY_LOCAL_MACHIN?   ""(root folder)               HKEY_LOCAL_MACHIN>

    In the last example, the ">" character is the special wildcard value DOS_QM.  ProjFS handles this
    and other special file system wildcard values in its PrjFileNameMatch and PrjDoesNameContainWildCards
    APIs.

--*/

HRESULT RegfsProvider::GetDirEnum(
    _In_        const PRJ_CALLBACK_DATA*    CallbackData,
    _In_        const GUID*                 EnumerationId,
    _In_opt_    PCWSTR                      SearchExpression,
    _In_        PRJ_DIR_ENTRY_BUFFER_HANDLE DirEntryBufferHandle
)
{
    wprintf(L"\n----> %hs: Path [%s] SearchExpression [%s]\n",
            __FUNCTION__, CallbackData->FilePathName, SearchExpression);

    HRESULT hr = S_OK;

    // Get the correct enumeration session from our map.
    auto it = _activeEnumSessions.find(*EnumerationId);
    if (it == _activeEnumSessions.end())
    {
        // We were asked for an enumeration we don't know about.
        hr = E_INVALIDARG;

        wprintf(L"<---- %hs: Unknown enumeration ID\n",
                __FUNCTION__);

        return hr;
    }

    // Get out our DirInfo helper object, which manages the context for this enumeration.
    auto& dirInfo = it->second;

    // If the enumeration is restarting, reset our bookkeeping information.
    if (CallbackData->Flags & PRJ_CB_DATA_FLAG_ENUM_RESTART_SCAN)
    {
        dirInfo->Reset();
    }

    if (!dirInfo->EntriesFilled())
    {
        // The DirInfo associated with the current session hasn't been initialized yet.  This method
        // will enumerate the subkeys and values in the registry key corresponding to CallbackData->FilePathName.
        // For each one that matches SearchExpression it will create an entry to return to ProjFS
        // and store it in the DirInfo object.
        HRESULT hr = PopulateDirInfoForPath(CallbackData->FilePathName,
                                            dirInfo.get(),
                                            SearchExpression);

        if (FAILED(hr))
        {
            wprintf(L"<---- %hs: Failed to populate dirInfo: 0x%08x\n",
                    __FUNCTION__, hr);
            return hr;
        }

        // This will ensure the entries in the DirInfo are sorted the way the file system expects.
        dirInfo->SortEntriesAndMarkFilled();
    }

    // Return our directory entries to ProjFS.
    while (dirInfo->CurrentIsValid())
    {
        // ProjFS allocates a fixed size buffer then invokes this callback.  The callback needs to
        // call PrjFillDirEntryBuffer to fill as many entries as possible until the buffer is full.
        if (S_OK != PrjFillDirEntryBuffer(dirInfo->CurrentFileName(),
                                          &dirInfo->CurrentBasicInfo(),
                                          DirEntryBufferHandle))
        {
            break;
        }

        // Only move the current entry cursor after the entry was successfully filled, so that we
        // can start from the correct index in the next GetDirEnum callback for this enumeration
        // session.
        dirInfo->MoveNext();
    }

    wprintf(L"<---- %hs: return 0x%08x\n",
            __FUNCTION__, hr);

    return hr;
};

// Populates a DirInfo object with directory and file entires that represent the registry keys and
// values that are under a given key.
HRESULT RegfsProvider::PopulateDirInfoForPath (
    _In_     std::wstring                       relativePath,
    _In_     DirInfo*                           dirInfo,
    _In_     std::wstring                       searchExpression
)
{
    RegEntries entries;

    // Get a list of the registry keys and values under the given key.
    HRESULT hr = _regOps.EnumerateKey(relativePath.c_str(), entries);
    if (FAILED(hr))
    {
        wprintf(L"%hs: Could not enumerate key: 0x%08x",
                __FUNCTION__, hr);
        return hr;
    }

    // Store each registry key that matches searchExpression as a directory entry.
    for (auto subKey : entries.SubKeys)
    {
        if (PrjFileNameMatch(subKey.Name.c_str(), searchExpression.c_str()))
        {
            dirInfo->FillDirEntry(subKey.Name.c_str());
        }
    }

    // Store each registry value that matches searchExpression as a file entry.
    for (auto val : entries.Values)
    {
        if (PrjFileNameMatch(val.Name.c_str(), searchExpression.c_str()))
        {
            dirInfo->FillFileEntry(val.Name.c_str(), val.Size);
        }
    }

    return hr;
}

/*++

Description:

    ProjFS invokes this callback to request the contents of a file.

    To handle this callback, the provider issues one or more calls to WriteFileData() to give
    ProjFS the file content. ProjFS will convert the on-disk placeholder into a hydrated placeholder,
    populated with the file contents.  Afterward, subsequent file reads will no longer invoke the
    GetFileStream callback.

    If multiple threads read the same placeholder file simultaneously, ProjFS ensures that the provider
    receives only one GetFileStream callback.

    If the provider is unable to process the request, it return an appropriate error code.  The caller
    who issued the read will receive an error, and the next file read for the same file will invoke
    GetFileStream again.

    Below is a list of example commands that will invoke GetFileStream callbacks.
    Assume there's a file named 'testfile' in provider's namespace:

    type testfile
    echo 123>>testfile
    echo 123>testfile

--*/

HRESULT RegfsProvider::GetFileData(
    _In_    const PRJ_CALLBACK_DATA*    CallbackData,
    _In_    UINT64                      ByteOffset,
    _In_    UINT32                      Length
)
{
    wprintf(L"\n----> %hs: Path [%s] triggered by [%s]\n",
            __FUNCTION__, CallbackData->FilePathName, CallbackData->TriggeringProcessImageFileName);

    HRESULT hr = S_OK;

    // We're going to need alignment information that is stored in the instance to service this
    // callback.
    PRJ_VIRTUALIZATION_INSTANCE_INFO instanceInfo;
    hr = PrjGetVirtualizationInstanceInfo(_instanceHandle,
                                          &instanceInfo);

    if (FAILED(hr))
    {
        wprintf(L"<---- %hs: PrjGetVirtualizationInstanceInfo: 0x%08x\n",
                __FUNCTION__, hr);
        return hr;
    }

    // Allocate a buffer that adheres to the machine's memory alignment.  We have to do this in case
    // the caller who caused this callback to be invoked is performing non-cached I/O.  For more
    // details, see the topic "Providing File Data" in the ProjFS documentation.
    void* writeBuffer = nullptr;
    writeBuffer = PrjAllocateAlignedBuffer(_instanceHandle,
                                           Length);

    if (writeBuffer == nullptr)
    {
        wprintf(L"<---- %hs: Could not allocate write buffer.\n",
                __FUNCTION__);
        return E_OUTOFMEMORY;
    }

    // Read the data out of the registry.
    if (!_regOps.ReadValue(CallbackData->FilePathName, reinterpret_cast<PBYTE>(writeBuffer), Length))
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

        PrjFreeAlignedBuffer(writeBuffer);
        wprintf(L"<---- %hs: Failed to read from registry.\n",
                __FUNCTION__);

        return hr;
    }

    // Call ProjFS to write the data we read from the registry into the on-disk placeholder.
    hr = this->WriteFileData(&CallbackData->DataStreamId,
                             reinterpret_cast<PVOID>(writeBuffer),
                             ByteOffset,
                             Length);

    if (FAILED(hr))
    {
        // If this callback returns an error, ProjFS will return this error code to the thread that
        // issued the file read, and the target file will remain an empty placeholder.
        wprintf(L"%hs: failed to write file for [%s]: 0x%08x\n",
                __FUNCTION__, CallbackData->FilePathName, hr);
    }

    // Free the memory-aligned buffer we allocated.
    PrjFreeAlignedBuffer(writeBuffer);

    wprintf(L"<---- %hs: return 0x%08x\n",
            __FUNCTION__, hr);

    return hr;
}

/*++

Description:

    ProjFS invokes this callback to deliver notifications of file system operations.

    The provider can specify which notifications it wishes to receive by filling out an array of
    PRJ_NOTIFICATION_MAPPING structures that it feeds to PrjStartVirtualizing in the PRJ_STARTVIRTUALIZING_OPTIONS
    structure.

    For the following notifications the provider can return a failure code.  This will prevent the
    associated file system operation from taking place.

    PRJ_NOTIFICATION_FILE_OPENED
    PRJ_NOTIFICATION_PRE_DELETE
    PRJ_NOTIFICATION_PRE_RENAME
    PRJ_NOTIFICATION_PRE_SET_HARDLINK
    PRJ_NOTIFICATION_FILE_PRE_CONVERT_TO_FULL

    All other notifications are informational only.

    See also the PRJ_NOTIFICATION_TYPE enum for more details about the notification types.

--*/

HRESULT RegfsProvider::Notify (
    _In_        const PRJ_CALLBACK_DATA*        CallbackData,
    _In_        BOOLEAN                         IsDirectory,
    _In_        PRJ_NOTIFICATION                NotificationType,
    _In_opt_    PCWSTR                          DestinationFileName,
    _Inout_     PRJ_NOTIFICATION_PARAMETERS*    NotificationParameters
)
{
    HRESULT hr = S_OK;

    wprintf(L"\n----> %hs: Path [%s] triggered by [%s]",
            __FUNCTION__, CallbackData->FilePathName, CallbackData->TriggeringProcessImageFileName);
    wprintf(L"\n----  Notification: 0x%08x\n", NotificationType);

    switch (NotificationType)
    {
    case PRJ_NOTIFICATION_FILE_OPENED:

        break;

    case PRJ_NOTIFICATION_FILE_HANDLE_CLOSED_FILE_MODIFIED:
    case PRJ_NOTIFICATION_FILE_OVERWRITTEN:

        wprintf(L"\n ----- [%s] was modified\n", CallbackData->FilePathName);
        break;

    case PRJ_NOTIFICATION_NEW_FILE_CREATED:

        wprintf(L"\n ----- [%s] was created\n", CallbackData->FilePathName);
        break;

    case PRJ_NOTIFICATION_FILE_RENAMED:

        wprintf(L"\n ----- [%s] -> [%s]\n", CallbackData->FilePathName,
                DestinationFileName);
        break;

    case PRJ_NOTIFICATION_FILE_HANDLE_CLOSED_FILE_DELETED:

        wprintf(L"\n ----- [%s] was deleted\n", CallbackData->FilePathName);
        break;

    case PRJ_NOTIFICATION_PRE_RENAME:

        if (_readonlyNamespace)
        {
            // Block file renames.
            hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
            wprintf(L"\n ----- rename request for [%s] was rejected \n", CallbackData->FilePathName);
        }
        else
        {
            wprintf(L"\n ----- rename request for [%s] \n", CallbackData->FilePathName);
        }
        break;

    case PRJ_NOTIFICATION_PRE_DELETE:

        if (_readonlyNamespace)
        {
            // Block file deletion.  We must return a particular NTSTATUS to ensure the file system
            // properly recognizes that this is a deny-delete.
            hr = HRESULT_FROM_NT(STATUS_CANNOT_DELETE);
            wprintf(L"\n ----- delete request for [%s] was rejected \n", CallbackData->FilePathName);
        }
        else
        {
            wprintf(L"\n ----- delete request for [%s] \n", CallbackData->FilePathName);
        }
        break;

    case PRJ_NOTIFICATION_FILE_PRE_CONVERT_TO_FULL:

        break;

    default:

        wprintf(L"%hs: Unexpected notification\n",
                __FUNCTION__);
    }

    wprintf(L"<---- %hs: return 0x%08x\n",
            __FUNCTION__, hr);
    return hr;
}