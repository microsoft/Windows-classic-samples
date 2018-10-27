/*++

Copyright (c) Microsoft Corporation

Module Name:

    regfsProvider.h

Abstract:

   This implements the provider class that uses ProjFS to project the Windows registry into the file
   system.  This class and the VirtualizationInstance class comprise the interface to ProjFS.  The
   other classes in this sample are helpers.

   This class understands how to map the Windows registry to the ProjFS callbacks.  It uses the RegInfo
   helper class to access the registry, and it overrides callback methods in the VirtualizationInstance
   class to feed registry data to the file system via ProjFS.

--*/

namespace regfs {

class RegfsProvider : public VirtualizationInstance {

public:

    RegfsProvider();

private:

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Overrides of the virtual callback functions from the VirtualizationInstance base class that
    // this class will implement.
    ///////////////////////////////////////////////////////////////////////////////////////////////

    HRESULT StartDirEnum(
        _In_     const PRJ_CALLBACK_DATA*   CallbackData,
        _In_     const GUID*                EnumerationId
    ) override;

    HRESULT EndDirEnum(
        _In_     const PRJ_CALLBACK_DATA*   CallbackData,
        _In_     const GUID*                EnumerationId
    ) override;

    HRESULT GetDirEnum(
        _In_        const PRJ_CALLBACK_DATA*    CallbackData,
        _In_        const GUID*                 EnumerationId,
        _In_opt_    PCWSTR                      SearchExpression,
        _In_        PRJ_DIR_ENTRY_BUFFER_HANDLE DirEntryBufferHandle
    ) override;

    HRESULT GetPlaceholderInfo(
        _In_    const PRJ_CALLBACK_DATA*    CallbackData
    ) override;

    HRESULT GetFileData(
        _In_    const PRJ_CALLBACK_DATA*    CallbackData,
        _In_    UINT64                      ByteOffset,
        _In_    UINT32                      Length
    ) override;

    HRESULT Notify(
        _In_        const PRJ_CALLBACK_DATA*        CallbackData,
        _In_        BOOLEAN                         IsDirectory,
        _In_        PRJ_NOTIFICATION                NotificationType,
        _In_opt_    PCWSTR                          DestinationFileName,
        _Inout_     PRJ_NOTIFICATION_PARAMETERS*    NotificationParameters
    ) override;

private:

    // Helper routine 
    HRESULT PopulateDirInfoForPath (
        _In_     std::wstring                    relativePath,
        _In_     DirInfo*                        dirInfo,
        _In_     std::wstring                    searchExpression
    );

    RegOps _regOps;

    // If this flag is set to true, RegFS will block the following namespace-altering operations
    // that take place under virtualization root:
    // 1) file or directory deletion
    // 2) file or directory rename
    //
    // New file or folder create cannot be easily blocked due to limitations in ProjFS.
    bool _readonlyNamespace = true;

    // If this flag is set to true, RegFS will block file content modifications for placeholder files.
    bool _readOnlyFileContent = true;

    // An enumeration session starts when StartDirEnum is invoked and ends when EndDirEnum is invoked.
    // This tracks the active enumeration sessions.
    std::map<GUID, std::unique_ptr<DirInfo>, GUIDComparer> _activeEnumSessions;
};

}