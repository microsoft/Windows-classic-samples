/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

   virtualizationInstance.h

Abstract:

   The VirtualizationInstance class is a thin C++ wrapper around the ProjFS C API.  The ProjFS API
   is available starting with Windows 10 October 2018 Update (Windows 10 version 1809) and Windows
   Server 2019.

   A ProjFS provider written in C++ can derive from this class.  The provider satisfies requests from
   ProjFS by overriding the virtual methods in this class.

   The provider must override the following pure virtual methods, which represent mandatory ProjFS
   callbacks:

   1.a) StartDirEnum
   1.b) GetDirEnum
   1.c) EndDirEnum
      To answer the question 'what are the files and folders under a given folder'
   2) GetPlaceholderInfo
      To answer the question 'what is the meta data for a given file (file attributes, timestamps etc)'
   3) GetFileData
      To answer the question 'what is the data for a given file'

   The provider may choose to override the following methods to implement extended functionality:

   5) Notify
      If the provider wants to receive notifications of file system operations.
      See PRJ_NOTIFICATION_TYPE enum in projectedfslib.h for a full list of notification types.

   6) QueryFileName
      Called to determine whether a particular file exists in the provider's backing store.  If the
      provider does not implement this callback, ProjFS will call the enumeration callbacks to find
      the file.

   7) CancelCommand
      If the provider wants to handle cancellation of I/O, such as if a user presses CRTL-C or kills
      the process while a callback is being processed.

--*/

#pragma once

namespace regfs {

struct GUIDComparer {
    bool operator()(const GUID & Left, const GUID & Right) const
    {
        return memcmp(&Left, &Right, sizeof(Right)) < 0;
    }
};

enum OptionalMethods {
    None            = 0,
    Notify          = 0x1,
    QueryFileName   = 0x2,
    CancelCommand   = 0x4
};
DEFINE_ENUM_FLAG_OPERATORS(OptionalMethods);

class NotImplemented : public std::logic_error {
public:
    NotImplemented() : std::logic_error("Function not yet implemented")
    {};
};

class VirtualizationInstance {

public:

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Virtualization instance control API wrappers (user mode -> kernel mode).  The derived provider
    // class should not override these methods.
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Starts a virtualization instance at rootPath
    HRESULT Start(LPCWSTR rootPath, PRJ_STARTVIRTUALIZING_OPTIONS* options = nullptr);

    // Stops a virtualization instance
    void Stop();

    // Send file meta data information to ProjFS, ProjFS will create an on-disk placeholder for the path.
    HRESULT WritePlaceholderInfo(LPCWSTR relativePath,
                                 PRJ_PLACEHOLDER_INFO* placeholderInfo,
                                 DWORD length);

    // Send file contents to ProjFS, ProjFS will write the data into the target placeholder file
    // and convert it to hydrated placeholder state.
    HRESULT WriteFileData(LPCGUID streamId,
                          PVOID buffer,
                          ULONGLONG byteOffset,
                          DWORD length);

protected:

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Virtualization instance callbacks (kernel mode -> user mode).  These are the methods the derived
    // provider class overrides.
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // [Mandatory] Inform the provider a directory enumeration is starting.
    // It corresponds to PRJ_START_DIRECTORY_ENUMERATION_CB in projectedfslib.h
    virtual HRESULT StartDirEnum(
        _In_     const PRJ_CALLBACK_DATA*   CallbackData,
        _In_     const GUID*                EnumerationId
    ) = 0;

    // [Mandatory] Inform the provider a directory enumeration is over.
    // It corresponds to PRJ_END_DIRECTORY_ENUMERATION_CB in projectedfslib.h
    virtual HRESULT EndDirEnum(
        _In_     const PRJ_CALLBACK_DATA*   CallbackData,
        _In_     const GUID*                EnumerationId
    ) = 0;

    // [Mandatory] Request directory enumeration information from the provider.
    // It corresponds to PRJ_GET_DIRECTORY_ENUMERATION_CB in projectedfslib.h
    virtual HRESULT GetDirEnum(
        _In_        const PRJ_CALLBACK_DATA*    CallbackData,
        _In_        const GUID*                 EnumerationId,
        _In_opt_    PCWSTR                      SearchExpression,
        _In_        PRJ_DIR_ENTRY_BUFFER_HANDLE DirEntryBufferHandle
    ) = 0;

    // [Mandatory] Request meta data information for a file or directory.
    // It corresponds to PRJ_GET_PLACEHOLDER_INFO_CB in projectedfslib.h
    virtual HRESULT GetPlaceholderInfo(
        _In_    const PRJ_CALLBACK_DATA*    CallbackData
    ) = 0;

    // [Mandatory] Request the contents of a file's primary data stream.
    // It corresponds to PRJ_GET_FILE_DATA_CB in projectedfslib.h
    virtual HRESULT GetFileData(
        _In_    const PRJ_CALLBACK_DATA*    CallbackData,
        _In_    UINT64                      ByteOffset,
        _In_    UINT32                      Length
    ) = 0;

    // [Optional] Deliver notifications to the provider that it has specified 
    // it wishes to receive. It corresponds to PRJ_NOTIFICATION_CB in projectedfslib.h
    virtual HRESULT Notify(
        _In_        const PRJ_CALLBACK_DATA*        CallbackData,
        _In_        BOOLEAN                         IsDirectory,
        _In_        PRJ_NOTIFICATION                NotificationType,
        _In_opt_    PCWSTR                          DestinationFileName,
        _Inout_     PRJ_NOTIFICATION_PARAMETERS*    NotificationParameters
    );

    virtual HRESULT QueryFileName(
        _In_        const PRJ_CALLBACK_DATA*        CallbackData
    );

    virtual void CancelCommand(
        _In_        const PRJ_CALLBACK_DATA*        CallbackData
    );

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Getter/Setter methods.  The derived provider class (RegfsProvider) uses SetOptionalMethods()
    // to indicate which optional callbacks it overrode.  This is required because ProjFS detects
    // whether a provider implemented an optional callback by whether or not the provider supplied
    // a pointer to the callback in the callbacks parameter of PrjStartVirtualizing().
    // VirtualizationInstance::Start() uses GetOptionalMethods() to find out which optional callbacks
    // were implemented, and sets the corresponding C callback into the callbacks parameter.
    ///////////////////////////////////////////////////////////////////////////////////////////////

    virtual OptionalMethods GetOptionalMethods() final;
    virtual void SetOptionalMethods(OptionalMethods optionalMethodsToSet) final;

private:

    PRJ_STARTVIRTUALIZING_OPTIONS _options = {};
    PRJ_CALLBACKS _callbacks = {};
    OptionalMethods _implementedOptionalMethods = OptionalMethods::None;
    const std::wstring _instanceIdFile = LR"(\.regfsId)";

    HRESULT EnsureVirtualizationRoot();

protected:

    std::wstring _rootPath;
    PRJ_NAMESPACE_VIRTUALIZATION_CONTEXT _instanceHandle = nullptr;

private:

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Prototypes of the ProjFS C callbacks. ProjFS will call these, and they in turn will call
    // the VirtualizationInstance class methods.
    ///////////////////////////////////////////////////////////////////////////////////////////////

    static HRESULT StartDirEnumCallback_C(
        _In_     const PRJ_CALLBACK_DATA*   CallbackData,
        _In_     const GUID*                EnumerationId
    );

    static HRESULT EndDirEnumCallback_C(
        _In_    const PRJ_CALLBACK_DATA*    callbackData,
        _In_    const GUID*                 EnumerationId
    );

    static HRESULT GetDirEnumCallback_C(
        _In_        const PRJ_CALLBACK_DATA*    CallbackData,
        _In_        const GUID*                 EnumerationId,
        _In_opt_    PCWSTR                      SearchExpression,
        _In_        PRJ_DIR_ENTRY_BUFFER_HANDLE DirEntryBufferHandle
    );

    static HRESULT GetPlaceholderInfoCallback_C(
        _In_    const PRJ_CALLBACK_DATA*    CallbackData
    );

    static HRESULT GetFileDataCallback_C(
        _In_    const PRJ_CALLBACK_DATA*    CallbackData,
        _In_    UINT64                      ByteOffset,
        _In_    UINT32                      Length
    );

    static HRESULT NotificationCallback_C(
        _In_        const PRJ_CALLBACK_DATA*        CallbackData,
        _In_        BOOLEAN                         IsDirectory,
        _In_        PRJ_NOTIFICATION                NotificationType,
        _In_opt_    PCWSTR                          DestinationFileName,
        _Inout_     PRJ_NOTIFICATION_PARAMETERS*    NotificationParameters
    );

    static HRESULT QueryFileName_C(
        _In_ const PRJ_CALLBACK_DATA* CallbackData
    );

    static void CancelCommand_C(
        _In_ const PRJ_CALLBACK_DATA* CallbackData
    );
};

}