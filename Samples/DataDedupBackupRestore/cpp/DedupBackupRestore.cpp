// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/////////////////////////////////////////////////////////////////////
//
// The following backup/restore scenarios are addressed in the code
//   1. Optimized backup
//   2. Selective restore from optimized backup
//   3. Full volume restore from optimized backup
//
/////////////////////////////////////////////////////////////////////

#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>

#include "ddpbackup.h"
#include <intsafe.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <atlbase.h>
#include <sal.h>
#include <wbemcli.h>
#include <comutil.h>
#include <wbemtime.h>
#include <winevt.h>

using namespace std;

// Backup/restore constants
const PCWSTR SYSTEM_VOLUME_INFORMATION = L"\\System Volume Information";
const PCWSTR DEDUP_FOLDER = L"\\Dedup";
const PCWSTR BACKUP_METADATA_FILE_NAME = L"dedupBackupMetadata.{741309a8-a42a-4830-b530-fad823933e6d}";
const PCWSTR BACKUP_METADATA_FORMAT = L"%s\r\n%s\r\n";
const PCWSTR LONG_PATH_PREFIX =  L"\\\\?\\";
const PCWSTR DIRECTORY_BACKUP_FILE = 
    L"directoryBackup.{741309a8-a42a-4830-b530-fad823933e6d}";
const ULONG DEDUP_STORE_FOLDERS_COUNT = 3;
const PCWSTR DEDUP_STORE_FOLDERS[DEDUP_STORE_FOLDERS_COUNT] =
    {
        L"\\ChunkStore",
        L"\\Settings",
        L"\\State"
    };

// WMI constants from Data Deduplication MOF schema
const PCWSTR CIM_V2_NAMESPACE = L"root\\cimv2";
const PCWSTR CIM_DEDUP_NAMESPACE = L"root\\Microsoft\\Windows\\Deduplication";
const PCWSTR CIM_DEDUP_CLASS_VOLUME = L"MSFT_DedupVolume";
const PCWSTR CIM_DEDUP_CLASS_VOLUME_METADATA = L"MSFT_DedupVolumeMetadata";
const PCWSTR CIM_DEDUP_CLASS_JOB = L"MSFT_DedupJob";
const PCWSTR CIM_DEDUP_METHOD_ENABLE = L"Enable";
const PCWSTR CIM_DEDUP_METHOD_DISABLE = L"Disable";
const PCWSTR CIM_DEDUP_METHOD_STOP = L"Stop";
const PCWSTR CIM_DEDUP_METHOD_START = L"Start";
const PCWSTR CIM_DEDUP_PROP_STOREID = L"StoreId";
const PCWSTR CIM_DEDUP_PROP_DATAACCESS = L"DataAccess";
const PCWSTR CIM_DEDUP_PROP_VOLUME = L"Volume";
const PCWSTR CIM_DEDUP_PROP_VOLUMEID = L"VolumeId";
const PCWSTR CIM_DEDUP_PROP_RETURNVALUE = L"ReturnValue";
const PCWSTR CIM_DEDUP_PROP_TYPE = L"Type";
const PCWSTR CIM_DEDUP_PROP_TIMESTAMP = L"Timestamp";
const PCWSTR CIM_DEDUP_PROP_WAIT = L"Wait";
const PCWSTR CIM_DEDUP_PROP_JOB = L"DedupJob";
const PCWSTR CIM_DEDUP_PROP_ID = L"Id";
const PCWSTR CIM_DEDUP_PROP_ERRORCODE = L"error_Code";
const PCWSTR CIM_DEDUP_PROP_ERRORMESSAGE = L"Message";
const PCWSTR CIM_DEDUP_PROP_PATH = L"__PATH";
const ULONG CIM_DEDUP_JOB_TYPE_UNOPT = 4;
const ULONG CIM_DEDUP_JOB_TYPE_GC = 2;

const PCWSTR DEDUP_OPERATIONAL_EVENT_CHANNEL_NAME = 
    L"Microsoft-Windows-Deduplication/Operational";

#define DDP_E_NOT_FOUND                  ((HRESULT)0x80565301L)
#define DDP_E_PATH_NOT_FOUND             ((HRESULT)0x80565304L)
#define DDP_E_VOLUME_DEDUP_DISABLED      ((HRESULT)0x80565323L)

#define ARRAY_LEN(A) (sizeof(A)/sizeof((A)[0]))

// Global variables

// Enums, classes
enum Action
{
    BackupAction,
    RestoreStubAction,
    RestoreDataAction,
    RestoreFileAction,
    RestoreVolumeAction,
    RestoreFilesAction
};

HRESULT RestoreVolume(_In_ const wstring& source, _In_ const wstring& destination);
HRESULT ToggleDedupJobs(_In_ const wstring& volumeGuidName, _In_ bool enableJobs);
HRESULT ToggleDedupDataAccess(_In_ const wstring& volumeGuidName, _In_ bool enableDataAccess);
HRESULT CancelDedupJobs(_In_ const wstring& volumeGuidName);
HRESULT GetJobInstanceId(_In_ IWbemClassObject* startJobOutParams, _Out_ wstring& jobId);
HRESULT DisplayUnoptimizationFileError(_In_ EVT_HANDLE event);
HRESULT CheckForUnoptimizationFileErrors(_In_ const wstring& jobId, _Out_ bool& foundErrors);
HRESULT UnoptimizeSinceTimestamp(_In_ const wstring& volumeGuidName, _In_ wstring& backupTime);
HRESULT RunGarbageCollection(_In_ const wstring& volumeGuidName);

void DoBackup(_In_ const wstring& source, _In_ const wstring& destination);
HRESULT RestoreStub(_In_ const wstring& source, _In_ const wstring& destination);
HRESULT RestoreData(_In_ const wstring& source, _In_ const wstring& destination);
HRESULT RestoreFilesData(_In_ const wstring& source, _In_ vector<wstring>& restoredFiles);

HRESULT BackupFile(_In_ const wstring& source, _In_ const wstring& destination);
HRESULT BackupDirectory(_In_ const wstring& source, _In_ const wstring& destination);
void BackupDirectoryTree(_In_ const wstring& source, _In_ const wstring& destination);
HRESULT RestoreFile(_In_ const wstring& source, _In_ const wstring& destination, _In_opt_ bool overWriteExisting = false);
HRESULT RestoreDirectory(_In_ const wstring& source, _In_ const wstring& destination);
HRESULT RestoreDedupStoreDirectories(_In_ const wstring& source, _In_ const wstring& destination);
HRESULT RestoreDedupStore(_In_ const wstring& source, _In_ const wstring& destination);
HRESULT RestoreFiles(_In_ const wstring& source, _In_ const wstring& destination, _In_ const bool isVolumeRestore, _Out_opt_ vector<wstring>* pRestoredFiles);
HRESULT RestoreDirectoryTree(_In_ const wstring& source, _In_ const wstring& destination, _In_ const vector<wstring>& sourceExcludePaths, _Out_opt_ vector<wstring>* pRestoredFiles);
HRESULT DeleteDirectoryTree(_In_ const wstring& directory);
HRESULT DeleteDedupStore(_In_ const wstring& volume);

HRESULT GetDedupChunkStoreId(_In_ const wstring& volumeGuidName, _Out_ wstring& chunkStoreId);
HRESULT VolumeHasDedupMetadata(_In_ const wstring& volumeGuidName, _Out_ bool& hasDedupMetadata, _Out_ wstring& chunkStoreId);
void WriteBackupMetadata(_In_ const wstring& source, _In_ const wstring& destination);
HRESULT ReadBackupMetadata(_In_ const wstring& source, _Out_ wstring& chunkStoreId, _Out_ wstring& backupTime);
wstring BuildDedupStorePath(_In_ const wstring& volume);

HRESULT WmiGetWbemServices(_In_ PCWSTR wmiNamespace, _Out_ CComPtr<IWbemServices>& spWmi);
HRESULT WmiGetMethodInputParams(_In_ IWbemServices* pWmi, _In_ PCWSTR className, _In_ PCWSTR methodName, _Out_ CComPtr<IWbemClassObject>& spInParams);
HRESULT WmiAddVolumeInputParameter(_Inout_ IWbemClassObject* pParams, _In_ PCWSTR className, _In_ PCWSTR methodName, _In_ const wstring& volume);
HRESULT WmiAddInputParameter(_Inout_ IWbemClassObject* pParams, _In_ PCWSTR className, _In_ PCWSTR methodName, _In_ PCWSTR propertyName, _In_ variant_t& var);
HRESULT WmiGetErrorInfo(_Out_ HRESULT& hrOperation, _Out_ wstring& errorMessageOperation);
HRESULT WmiExecuteMethod(_In_ IWbemServices* pWmi, _In_ PCWSTR className, _In_ PCWSTR methodName, _In_opt_ IWbemClassObject* pInParams, 
    _Out_ CComPtr<IWbemClassObject>& spOutParams, _In_opt_ PCWSTR context = L"");
HRESULT WmiGetDedupInstanceByVolumeId(PCWSTR className, _In_ const wstring& volumeGuidName, _Out_ CComPtr<IWbemClassObject>& instance);
HRESULT WmiQueryDedupInstancesByVolumeId(_In_ IWbemServices* pWmi, _In_ const wstring& className, _In_ const wstring& volumeGuidName, _Out_ CComPtr<IEnumWbemClassObject>& instances);

void StringReplace(_Inout_ wstring& stringValue, _In_ const wstring& matchValue, _In_ const wstring& replaceValue);
wstring TrimTrailingSeparator(_In_ const wstring& str, _In_ WCHAR separator);
bool IsRootPath(_In_ const wstring& path);
void PrintUsage(_In_ LPCWSTR programName);
bool ParseCommandLine(_In_ int argc,  _In_reads_(argc) _TCHAR* argv[], _Out_ Action *action, _Out_ wstring* source, _Out_ wstring* destination);
HRESULT ModifyPrivilege(_In_ LPCTSTR szPrivilege, _In_ BOOL fEnable);
HRESULT GetVolumeGuidNameForPath(_In_ const wstring& path, _Out_ wstring& volumeGuidName);
HRESULT GetEventData(_In_ EVT_HANDLE event, _In_ PCWSTR dataName, _Out_ variant_t& varData);
HRESULT GetFileSize(_In_ const std::wstring& FilePath, _Out_ LARGE_INTEGER& FileSize);



/////////////////////////////////////////////////////////////////////
//
// This class is for selective restore from optimized backup
//
/////////////////////////////////////////////////////////////////////
class CBackupStore: public IDedupReadFileCallback
{
private:
    ULONGLONG m_refCount;
    std::wstring m_backupLocation;
    std::map<wstring, ULONGLONG> m_dataStreamMap;

public:
    CBackupStore(_In_ const wstring& backupLocation)
    {
        m_refCount = 1;
        m_backupLocation = backupLocation;
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ __RPC__in REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
    {
        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, __uuidof(IDedupReadFileCallback)))
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef( void)
    {
        return (ULONG) InterlockedIncrement(&m_refCount);
    }

    virtual ULONG STDMETHODCALLTYPE Release( void)
    {
        ULONGLONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0) delete this;
        return (ULONG) ref;
    }

    virtual HRESULT STDMETHODCALLTYPE ReadBackupFile( 
        /* [in] */ __RPC__in BSTR FileFullPath,
        /* [in] */ hyper FileOffset,
        /* [in] */ ULONG SizeToRead,
        /* [length_is][size_is][out] */ __RPC__out_ecount_part(SizeToRead, *ReturnedSize) BYTE *FileBuffer,
        /* [out] */ __RPC__out ULONG *ReturnedSize,
        /* [in] */ DWORD Flags)
    {
        // This method is called by the backup support COM object to read
        // the backup database from the backup medium

        UNREFERENCED_PARAMETER(Flags);
        HRESULT hr = S_OK;
        wstring filePath = m_backupLocation;
        filePath += FileFullPath;
        *ReturnedSize = 0;
        // FileBuffer contents can be uninitialized after byte *ReturnedSize

        HANDLE hFile = ::CreateFile(
            filePath.c_str(), 
            GENERIC_READ, 
            FILE_SHARE_READ, 
            NULL, 
            OPEN_EXISTING, 
            FILE_FLAG_BACKUP_SEMANTICS, 
            NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            wcout << L"Cannot open file " << filePath << 
                L". Did we back it up? Error: " << GetLastError() << endl;
        }
        else 
        {
            // This file was saved with BackupRead, so its contents are actually a series of 
            // WIN32_STREAM_ID structures that describe the original file
            // To read data as we would read from the original file, we first need to find 
            // the DATA stream.
            // If the file was not backed up with BackupRead you would not need this

            LONGLONG dataStreamOffset = 0;
            hr = FindDataStream(hFile, filePath, &dataStreamOffset);
            if (SUCCEEDED(hr))
            {
                LONGLONG actualFileOffset = FileOffset + dataStreamOffset;
                OVERLAPPED overlapped = {};

                overlapped.Offset = LODWORD(actualFileOffset);
                overlapped.OffsetHigh = HIDWORD(actualFileOffset);

                if (!ReadFile(hFile, FileBuffer, SizeToRead, ReturnedSize, &overlapped))
                {
                    wcout << L"Cannot read from file " << filePath << 
                        L". Did we back it up? gle= " << GetLastError() << endl;
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }

            CloseHandle(hFile);
        }

        return hr;
    }
        
    virtual HRESULT STDMETHODCALLTYPE OrderContainersRestore( 
        /* [in] */ ULONG NumberOfContainers,
        /* [size_is][in] */ __RPC__in_ecount_full(NumberOfContainers) BSTR *ContainerPaths,
        /* [out] */ __RPC__out ULONG *ReadPlanEntries,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*ReadPlanEntries) DEDUP_CONTAINER_EXTENT  **ReadPlan)
    {
        UNREFERENCED_PARAMETER(ContainerPaths);

        // If you backed up to multiple tapes and parts of every file are split between the tapes
        // you want avoid switching tapes back and forth as the restore engine the backup database files
        // To implement this, you need to return the order in which the restore engine should read the files
        // In this example, we tell the restore engine to read first 64k of every file first, then
        // the remainder of the file. In a real tape backup application you would need to read the backup 
        // catalog, then return an array containing the file ranges on the first tape, 
        // then the files on the second tape, and so on
        // If you are backing up on fixed disk you don't need to implement this, just uncomment the following 2 
        // lines and delete the rest:
        //*ReadPlanEntries = 0;
        //*ReadPlan = NULL;

        if (ReadPlan == NULL)
        {
            *ReadPlanEntries = 0;
            return S_OK;
        }

        ULONG fileFragments = 2 * NumberOfContainers;
        *ReadPlan = (DEDUP_CONTAINER_EXTENT*)MIDL_user_allocate(fileFragments * sizeof(DEDUP_CONTAINER_EXTENT));
        if (*ReadPlan != NULL) 
        {
            *ReadPlanEntries = fileFragments;

            for (ULONG i = 0; i < 2 * NumberOfContainers; i++) 
            {
                (*ReadPlan)[i].ContainerIndex = i % NumberOfContainers;

                wstring filePath = m_backupLocation;
                wstring relativePath = ContainerPaths[i % NumberOfContainers];
                filePath += relativePath;
                
                // Chop the file at % of file size, to demonstrate extents
                LARGE_INTEGER fileSize = {};
                GetFileSize(filePath, fileSize);

                ULONG extentBoundary = (fileSize.LowPart * 10) / 100;
                
                if (i < NumberOfContainers)
                {
                    (*ReadPlan)[i].StartOffset = 0;
                    (*ReadPlan)[i].Length = extentBoundary;
                }
                else
                {
                    (*ReadPlan)[i].StartOffset = extentBoundary;
                    (*ReadPlan)[i].Length = LONGLONG_MAX; // this just says "until the end of the file"
                }
            }
        }
        else
        {
            // If allocation fails, we will just not optimize
            *ReadPlanEntries = 0;
        }
        return S_OK;
    }
        
    virtual HRESULT STDMETHODCALLTYPE PreviewContainerRead( 
        /* [in] */ __RPC__in BSTR FileFullPath,
        /* [in] */ ULONG NumberOfReads,
        /* [size_is][in] */ __RPC__in_ecount_full(NumberOfReads) DDP_FILE_EXTENT *ReadOffsets)
    {
        UNREFERENCED_PARAMETER(FileFullPath);
        UNREFERENCED_PARAMETER(NumberOfReads);
        UNREFERENCED_PARAMETER(ReadOffsets);

        // This will be called before the actual read, so you can optimize and do bigger reads instead
        // of smaller ones. If you decide you want to do this, examine the ReadOffsets and do a bigger read 
        // into a big buffer, then satisfy the next reads from the buffer you allocated.

        return S_OK;
    }

private:
    HRESULT FindDataStream(_In_ HANDLE hFile, _In_ const wstring& filePath, _Out_ LONGLONG* result)
    {
        // Cache the results per file path so we don't have to do this for every read
        if (m_dataStreamMap.find(filePath) == m_dataStreamMap.end())
        {
            WIN32_STREAM_ID streamId = {};
            DWORD bytesRead = 0;
            while (streamId.dwStreamId != BACKUP_DATA)
            {
                ::SetFilePointerEx(hFile, streamId.Size, NULL, FILE_CURRENT);
                // The Size field is the actual size starting from the cStreamName field, so we only want to read the header
                if (!ReadFile(hFile, &streamId, FIELD_OFFSET(WIN32_STREAM_ID, cStreamName), &bytesRead, NULL) ||
                    bytesRead != FIELD_OFFSET(WIN32_STREAM_ID, cStreamName))
                {
                    wcout << "Cannot find the data stream in file. Did you use something other than BackupRead? Error: " << 
                        GetLastError() << endl;
                    return E_UNEXPECTED;
                }
            }

            // Get the current position
            LARGE_INTEGER zero = {};
            LARGE_INTEGER currentPosition;
            ::SetFilePointerEx(hFile, zero, &currentPosition, FILE_CURRENT);
            m_dataStreamMap[filePath] = currentPosition.QuadPart;
        }
        *result = m_dataStreamMap[filePath];
        return S_OK;
    }
};

/////////////////////////////////////////////////////////////////////
//
// Selective restore
//
/////////////////////////////////////////////////////////////////////


HRESULT RestoreStub(_In_ const wstring& source, _In_ const wstring& destination)
{
    return RestoreFile(source, destination);
}

HRESULT RestoreData(_In_ const wstring& source, _In_ const wstring& destination)
{
    // Source is a file name, but we need the directory name so we can read
    // the backup database inside the backup store
    size_t lastSeparator = source.rfind('\\');
    if (lastSeparator == wstring::npos) 
    {
        wcout << L"source is not a file path" << endl;
        return E_UNEXPECTED;
    }
    wstring backupLocation = source.substr(0, lastSeparator);
    CBackupStore* pStore = new(nothrow) CBackupStore(backupLocation);

    if (pStore == NULL)
    {
        wcout << L"Not enough resources" << endl;
        return E_OUTOFMEMORY;
    }

    CComPtr<IDedupBackupSupport> backupSupport;
    HRESULT hr = backupSupport.CoCreateInstance(__uuidof(DedupBackupSupport), NULL);
    if (FAILED(hr)) 
    {
        wcout << L"Cannot instantiate the restore engine, hr = 0x" << hex << hr << endl;
    }
    else
    {
        // NOTE: destination was already created by RestoreStub
        BSTR bstrFile = SysAllocString(destination.c_str()); 
        if (bstrFile == NULL)
        {
            wcout << L"Not enough resources" << endl;
            delete pStore;
            return E_OUTOFMEMORY;
        }
        hr = backupSupport->RestoreFiles(1, &bstrFile, pStore, DEDUP_RECONSTRUCT_UNOPTIMIZED, NULL);
        SysFreeString(bstrFile);
    }

    if (FAILED(hr)) 
    {
        wcout << L"Restore failed, hr = 0x" << hex << hr << endl;
        // We need to clean up on failure
        DeleteFile(destination.c_str());
    }

    if (hr == S_FALSE)
    {
        wcout << L"Destination file is not a Data Deduplication file" << endl;
    }
    delete pStore;

    return hr;
}

HRESULT RestoreFilesData(_In_ const wstring& source, _In_ vector<wstring>& restoredFiles)
{
    HRESULT hr = S_OK;

    size_t fileCount = restoredFiles.size();
    if (fileCount == 0)
    {
        wcout << L"No files to restore" << endl;
        return S_OK;
    }

    BSTR* bstrFiles = NULL;
    CBackupStore* pStore = NULL;
    HRESULT* hrRestoreResults = NULL;
    CComPtr<IDedupBackupSupport> backupSupport;
    
    bstrFiles = new BSTR[fileCount];
    if (bstrFiles == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    ZeroMemory(bstrFiles, sizeof(BSTR*) * fileCount);

    size_t index = 0;
    for ( ; index < fileCount; ++index)
    {
        bstrFiles[index] = SysAllocString(restoredFiles[index].c_str());
        if (bstrFiles[index] == NULL)
        {
            hr =  E_OUTOFMEMORY;
            goto Cleanup;
        }
    }
    
    hrRestoreResults = new HRESULT[fileCount];
    if (hrRestoreResults == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    pStore = new(nothrow) CBackupStore(source);
    if (pStore == NULL)
    {
        hr =  E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = backupSupport.CoCreateInstance(__uuidof(DedupBackupSupport), NULL);
    if (FAILED(hr)) 
    {
        wcout << L"Cannot instantiate the restore engine, hr = 0x" << hex << hr << endl;
        goto Cleanup;
    }

    // NOTE: destination stubs were already created by RestoreFiles
    hr = backupSupport->RestoreFiles((ULONG)fileCount, bstrFiles, pStore, DEDUP_RECONSTRUCT_UNOPTIMIZED, hrRestoreResults);

    if (FAILED(hr)) 
    {
        wcout << L"Restore failed, hr = 0x" << hex << hr << endl;

        // Files not restored successfully will have deduplication reparse points
        // Non-dedup files were completely restored when restoring stubs
        // When error code is DDP_E_JOB_COMPLETED_PARTIAL_SUCCESS some deduplicated files were also fully restored
        // Cleanup failed restores
        for (index = 0; index < fileCount; ++index)
        {
            if (FAILED(hrRestoreResults[index]))
            {
                wcout << L"Failed to restore file " << bstrFiles[index] << L", hr = 0x" << hex << hr << endl;
                DeleteFile(bstrFiles[index]);
            }
        }
    }

Cleanup:

    if (hr == E_OUTOFMEMORY)
    {
        wcout << L"Not enough resources" << endl;
    }

    if (pStore != NULL)
    {
        delete pStore;
    }

    if (hrRestoreResults != NULL)
    {
        delete [] hrRestoreResults;
    }
    
    if (bstrFiles != NULL)
    {
        for (index = 0; index < fileCount; ++index)
        {
            if (bstrFiles[index] != NULL)
            {
                SysFreeString(bstrFiles[index]);
            }
        }
        delete [] bstrFiles;
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////
//
// Backup full or selective volume
//
/////////////////////////////////////////////////////////////////////

void DoBackup(_In_ const wstring& source, _In_ const wstring& destination)
{
    // Recursivelly back up the specified directory
    BackupDirectoryTree(source, destination);

    WCHAR volumePathName[MAX_PATH];
    GetVolumePathName(source.c_str(), volumePathName, MAX_PATH);

    if (source != volumePathName)
    {
        // Important: you must always ensure you back up the deduplication database, which is under
        // System Volume Information\Dedup and contains the actual file data
        // since we didn't back up the whole volume we need to back it up now

        wstring dedupDatabase;
        dedupDatabase = volumePathName;
        dedupDatabase = TrimTrailingSeparator(dedupDatabase, L'\\');
        dedupDatabase += SYSTEM_VOLUME_INFORMATION;

        wstring databaseDestination = destination;
        databaseDestination += SYSTEM_VOLUME_INFORMATION;

        // Backup SVI folder
        BackupDirectory(dedupDatabase, databaseDestination);
        
        databaseDestination += DEDUP_FOLDER;
        dedupDatabase += DEDUP_FOLDER;

        // Backup the deduplication store
        BackupDirectoryTree(dedupDatabase, databaseDestination);
    }

    WriteBackupMetadata(source, destination);
}



/////////////////////////////////////////////////////////////////////
//
// Full volume restore core
//
/////////////////////////////////////////////////////////////////////

HRESULT RestoreVolume(_In_ const wstring& source, _In_ const wstring& destination)
{
    HRESULT hr = S_OK;
    wstring destinationDedupStoreId;
    wstring destinationVolumeGuidName;
    wstring sourceDedupStoreId;
    wstring backupTime;
    bool destinationHasDedupMetadata = false;

    wcout << L"Restoring files from '" << source << L"' to volume '" << destination << L"'" << endl;
    
    hr = GetVolumeGuidNameForPath(destination, destinationVolumeGuidName);

    // Get the chunk store ID and backup timestamp from the backup metadata
    if (SUCCEEDED(hr))
    {
        hr = ReadBackupMetadata(source, sourceDedupStoreId, backupTime);
    }

    // Check for deduplication metadata
    if (SUCCEEDED(hr))
    {
        hr = VolumeHasDedupMetadata(destinationVolumeGuidName, destinationHasDedupMetadata, destinationDedupStoreId);
    }

    if (SUCCEEDED(hr) && destinationHasDedupMetadata)
    {
        if (_wcsicmp(sourceDedupStoreId.c_str(), destinationDedupStoreId.c_str()) != 0)
        {
            wcout << L"Restore is unsupported. Source deduplication store ID '" << sourceDedupStoreId << 
                L"' does not match destination ID '" << destinationDedupStoreId << L"'." << endl;
            hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        }
        
        if (SUCCEEDED(hr))
        {
            // Disable deduplication jobs on the volume to avoid disruption during restore
            hr = ToggleDedupJobs(destinationVolumeGuidName, false);
        }
        
        // Cancel deduplication jobs running/queued on the volume
        if (SUCCEEDED(hr))
        {
            hr = CancelDedupJobs(destinationVolumeGuidName);
        }
        
        // Unoptimize files that changed since backup timestamp
        if (SUCCEEDED(hr))
        {
            hr = UnoptimizeSinceTimestamp(destinationVolumeGuidName, backupTime);
        }

        // Disable deduplication data access on the volume during restore
        // NOTE: this operation causes the destination volume to dismount/remount
        if (SUCCEEDED(hr))
        {
            hr = ToggleDedupDataAccess(destinationVolumeGuidName, false);
        }

        // Delete the deduplication store
        if (SUCCEEDED(hr))
        {
            hr = DeleteDedupStore(destinationVolumeGuidName);
        }
    }
    
    // Restore deduplication store
    if (SUCCEEDED(hr))
    {
        hr = RestoreDedupStore(source, destination);
    }

    if (SUCCEEDED(hr) && !destinationHasDedupMetadata)
    {
        // Deduplication store is restored to a fresh volume
        // Jobs and data access need to be disabled to avoid disruption during restore

        // Disable deduplication jobs on the volume
        hr = ToggleDedupJobs(destinationVolumeGuidName, false);

        // Disable deduplication data access on the volume
        // NOTE: this operation causes the destination volume to dismount/remount
        if (SUCCEEDED(hr))
        {
            hr = ToggleDedupDataAccess(destinationVolumeGuidName, false);
        }

        // Set state such that deduplication is reenabled after restore
        destinationHasDedupMetadata = true;
    }
    
    // Restore files on the volume
    if (SUCCEEDED(hr))
    {
        // This sample restores files with source priority in cases where a file/directory
        // exists in both the backup store and the target volume
        // A real backup application might offer the option of target volume priority semantics
        hr = RestoreFiles(source, destination, true, NULL);
    }

    // Reenable deduplication jobs and data access
    if (SUCCEEDED(hr) && destinationHasDedupMetadata)
    {
        // Enable deduplication data access on the volume
        // NOTE: this operation causes the destination volume to dismount/remount
        hr = ToggleDedupDataAccess(destinationVolumeGuidName, true);
        
        if (SUCCEEDED(hr))
        {
            // Enable deduplication jobs on the volume
            hr = ToggleDedupJobs(destinationVolumeGuidName, true);
        }
    }
    
    // Run GC job
    if (SUCCEEDED(hr))
    {
        hr = RunGarbageCollection(destinationVolumeGuidName);
    }

    if (SUCCEEDED(hr))
    {
        wcout << L"Restore completed" << endl;
    }
    else
    {
        wcout << L"Restore completed with error 0x" << hex << hr << endl;
    }
    
    return hr;
}



HRESULT GetDedupChunkStoreId(_In_ const wstring& volumeGuidName, _Out_ wstring& chunkStoreId)
{

    HRESULT hr = S_OK;
    CComPtr<IWbemClassObject> spInstance;

    chunkStoreId.clear();
    
    // Returns S_FALSE if not found
    hr = WmiGetDedupInstanceByVolumeId(CIM_DEDUP_CLASS_VOLUME_METADATA, volumeGuidName, spInstance);

    if (hr == S_OK)
    {
        _variant_t var;
        
        // Get the value of the StoreId property
        hr = spInstance->Get(CIM_DEDUP_PROP_STOREID, 0, &var, 0, 0);
        if (FAILED(hr))
        {
            wcout << L"IWbemClassObject::Get for property StoreId failed with error 0x" << hex << hr << endl;
        }
        else
        {
            chunkStoreId = var.bstrVal;
        }
    }

    return hr;
}


HRESULT ToggleDedupJobs(_In_ const wstring& volumeGuidName, _In_ bool enableJobs)
{
    wstring methodName = enableJobs ? CIM_DEDUP_METHOD_ENABLE : CIM_DEDUP_METHOD_DISABLE;

    // Setup for WMI method call - get WBEM services, input parameter object
    
    CComPtr<IWbemServices> spWmi;
    HRESULT hr = WmiGetWbemServices(CIM_DEDUP_NAMESPACE, spWmi);

    CComPtr<IWbemClassObject> spInParams;
    if (SUCCEEDED(hr))
    {
        hr = WmiGetMethodInputParams(spWmi, CIM_DEDUP_CLASS_VOLUME, methodName.c_str(), spInParams);
    }

    if (SUCCEEDED(hr))
    {
        hr = WmiAddVolumeInputParameter(spInParams, CIM_DEDUP_CLASS_VOLUME, methodName.c_str(), volumeGuidName);
    }

    if (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> spOutParams;
        hr = WmiExecuteMethod(spWmi, CIM_DEDUP_CLASS_VOLUME, methodName.c_str(), spInParams, spOutParams, L"Jobs");
    }
    
    return hr;
}

HRESULT ToggleDedupDataAccess(_In_ const wstring& volumeGuidName, _In_ bool enableDataAccess)
{
    wstring methodName = enableDataAccess ? CIM_DEDUP_METHOD_ENABLE : CIM_DEDUP_METHOD_DISABLE;

    // Setup for WMI method call - get WBEM services, input parameter object
    CComPtr<IWbemServices> spWmi;
    HRESULT hr = WmiGetWbemServices(CIM_DEDUP_NAMESPACE, spWmi);

    CComPtr<IWbemClassObject> spInParams;
    if (SUCCEEDED(hr))
    {
        hr = WmiGetMethodInputParams(spWmi, CIM_DEDUP_CLASS_VOLUME, methodName.c_str(), spInParams);
    }

    // Volume name parameter
    if (SUCCEEDED(hr))
    {
        hr = WmiAddVolumeInputParameter(spInParams, CIM_DEDUP_CLASS_VOLUME, methodName.c_str(), volumeGuidName);
    }

    // DataAccess parameter
    if (SUCCEEDED(hr))
    {
        variant_t dataAccessTrigger = true;;
        
        hr = WmiAddInputParameter(spInParams, CIM_DEDUP_CLASS_VOLUME, methodName.c_str(), CIM_DEDUP_PROP_DATAACCESS, dataAccessTrigger);
    }

    // Execute method
    if (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> spOutParams;
        hr = WmiExecuteMethod(spWmi, CIM_DEDUP_CLASS_VOLUME, methodName.c_str(), spInParams, spOutParams, L"DataAccess");
    }
    
    return hr;
}

HRESULT CancelDedupJobs(_In_ const wstring& volumeGuidName)
{
    CComPtr<IWbemServices> spWmi;
    HRESULT hr = WmiGetWbemServices(CIM_DEDUP_NAMESPACE, spWmi);

    // Get the job instance queued/running for the specified volume
    CComPtr<IEnumWbemClassObject> spInstances;
    if (SUCCEEDED(hr))
    {
        hr = WmiQueryDedupInstancesByVolumeId(spWmi, CIM_DEDUP_CLASS_JOB, volumeGuidName.c_str(), spInstances);
    }

    // Stop each job
    while (SUCCEEDED(hr))
    {

        CComPtr<IWbemClassObject> spInstance;
        ULONG uReturn = 0;

        // Get a job instance
        hr = spInstances->Next(WBEM_INFINITE, 1, &spInstance, &uReturn);
        if (FAILED(hr))
        {
           wcout << L"IEnumWbemClassObject::Next failed with error 0x" << hex << hr << endl;
           break;
        }

        if (uReturn == 0)
        {
            // All done
            break;
        }

        // Get the __PATH property (used as input to ExecMethod)
        variant_t varObjectPath;
        hr = spInstance->Get(CIM_DEDUP_PROP_PATH, 0, &varObjectPath, 0, 0);
        if (FAILED(hr))
        {
            wcout << L"IWbemClassObject::Get for property " << CIM_DEDUP_PROP_PATH << L" failed with error 0x" << hex << hr << endl;
            break;
        }

        // Call the Stop method
        if (SUCCEEDED(hr))
        {
            // NOTE: the MSFT_DedupJob.Stop method takes no input parameters
            CComPtr<IWbemClassObject> spOutParams;
            hr = WmiExecuteMethod(spWmi, varObjectPath.bstrVal, CIM_DEDUP_METHOD_STOP, NULL, spOutParams);
            if (FAILED(hr))
            {
                break;
            }
        }
    }
    
    return hr;
}

HRESULT RestoreDedupStoreDirectories(_In_ const wstring& source, _In_ const wstring& destination)
{
    wstring sourcePath = source;
    wstring destinationPath = destination;

    sourcePath += SYSTEM_VOLUME_INFORMATION;
    destinationPath += SYSTEM_VOLUME_INFORMATION;
    
    HRESULT hr = RestoreDirectory(sourcePath, destinationPath);

    if (SUCCEEDED(hr) || hr == HRESULT_FROM_WIN32(ERROR_FILE_EXISTS))
    {
        sourcePath += DEDUP_FOLDER;
        destinationPath += DEDUP_FOLDER;
        
        hr = RestoreDirectory(sourcePath, destinationPath);
    }

    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_EXISTS))
    {
        hr = S_OK;
    }
    
    return hr;
}

HRESULT RestoreDedupStore(_In_ const wstring& source, _In_ const wstring& destination)
{
    HRESULT hr = RestoreDedupStoreDirectories(source, destination);

    if (SUCCEEDED(hr))
    {
        wstring sourceDedupStorePath = BuildDedupStorePath(source);
        wstring destinationDedupStore = BuildDedupStorePath(destination);

        for (int index = 0; SUCCEEDED(hr) && index < DEDUP_STORE_FOLDERS_COUNT; index++)
        {
            wstring sourceDedupPath = sourceDedupStorePath;
            sourceDedupPath.append(DEDUP_STORE_FOLDERS[index]);

            wstring destinationDedupPath = destinationDedupStore;
            destinationDedupPath.append(DEDUP_STORE_FOLDERS[index]);

            vector<wstring> excludePaths;
            hr = RestoreDirectoryTree(sourceDedupPath, destinationDedupPath, excludePaths, NULL);
        }
    }

    return hr;
}

HRESULT RestoreFiles(_In_ const wstring& source, _In_ const wstring& destination, _In_ const bool isVolumeRestore, _Out_opt_ vector<wstring>* pRestoredFiles)
{
    HRESULT hr = S_OK;

    // Restore files (exclude deduplication store)
    
    vector<wstring> excludePaths;
    excludePaths.push_back(BuildDedupStorePath(source));

    if (SUCCEEDED(hr))
    {
        hr = RestoreDirectoryTree(source, destination, excludePaths, pRestoredFiles);
    }

    if (SUCCEEDED(hr) && !isVolumeRestore)
    {
        // NOTE: RestoreDirectoryTree also restores SVI folder
        // If the backed up directory was a volume root there may be files to restore under SVI folder
        // If the backed up directory was a folder then SVI should be cleaned up
        // Directory deletion failures ignored intentionally
        wstring dedupStorePath = TrimTrailingSeparator(destination, L'\\');
        dedupStorePath.append(SYSTEM_VOLUME_INFORMATION);
        RemoveDirectory(dedupStorePath.c_str());
    }

    return hr;
}

HRESULT GetJobInstanceId(_In_ IWbemClassObject* startJobOutParams, _Out_ wstring& jobId)
{
    UNREFERENCED_PARAMETER(jobId);

    _variant_t varArray;

    jobId.clear();
    
    // Get the value of the StoreId property
    HRESULT hr = startJobOutParams->Get(CIM_DEDUP_PROP_JOB, 0, &varArray, 0, 0);
    if (FAILED(hr))
    {
        wcout << L"IWbemClassObject::Get for property " << CIM_DEDUP_PROP_JOB << L" failed with error 0x" << hex << hr << endl;
    }
    
    if (varArray.vt != (VT_ARRAY | VT_UNKNOWN))
    {
        wcout << L"VT Type is unexpected for property " << CIM_DEDUP_PROP_JOB << endl;
        hr = E_FAIL;
    }

    if (varArray.ppunkVal == NULL || *(varArray.ppunkVal) == NULL)
    {
        wcout << L"Property " << CIM_DEDUP_PROP_JOB << L" has a NULL value" << endl;
        hr = E_FAIL;
    }

    IUnknown* unknown = NULL;

    if (SUCCEEDED(hr))
    {
        long index = 0;
        SAFEARRAY *sa = varArray.parray;
        hr = SafeArrayGetElement(sa, &index, &unknown);
        if (FAILED(hr))
        {
            wcout << L"SafeArrayGetElement failed with error 0x" << hex << hr << endl;
        }

        if (unknown == NULL)
        {
            wcout << L"Property " << CIM_DEDUP_PROP_JOB << L" has a NULL value" << endl;
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> spJob;
        
        CComPtr<IUnknown> spUnknown = unknown;
        
        hr = spUnknown->QueryInterface(&spJob);
        if (FAILED(hr))
        {
            wcout << L"IUnknown::QueryInterface for IWbemClassObject failed with error 0x" << hex << hr << endl;
        }

        if (SUCCEEDED(hr))
        {
            variant_t varJobId;
            
            HRESULT hr = spJob->Get(CIM_DEDUP_PROP_ID, 0, &varJobId, 0, 0);
            if (FAILED(hr))
            {
                wcout << L"IWbemClassObject::Get for property " << CIM_DEDUP_PROP_ID << L" failed with error 0x" << hex << hr << endl;
            }

            if (varJobId.vt != VT_BSTR)
            {
                wcout << L"VT Type is unexpected for property " << CIM_DEDUP_PROP_JOB << endl;
                hr = E_FAIL;
            }

            if (SUCCEEDED(hr))
            {
                jobId = varJobId.bstrVal;
            }
        }
        
    }
    
    return hr;
}

HRESULT GetEventData(_In_ EVT_HANDLE event, _In_ PCWSTR dataName, _Out_ variant_t& varData)
{
    HRESULT hr = S_OK;

    wstring eventDataXPath(L"Event/EventData/Data[@Name='");
    eventDataXPath.append(dataName);
    eventDataXPath.append(L"']");

    PCWSTR values[] = { eventDataXPath.c_str() };

    EVT_HANDLE renderContext = EvtCreateRenderContext(
        ARRAYSIZE(values), 
        values, 
        EvtRenderContextValues);

    if (renderContext == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        wcout << L"EvtCreateRenderContext failed with error " << GetLastError() << endl;
    }
    
    if (SUCCEEDED(hr))
    {
        PEVT_VARIANT properties = NULL;
        ULONG bufferUsed = 0;
        ULONG propertyCount = 0;

        BOOL result = EvtRender(
            renderContext,
            event,
            EvtRenderEventValues,
            0,
            NULL,
            &bufferUsed,
            &propertyCount);

        if (!result)
        {
            DWORD status = ::GetLastError();

            if (status == ERROR_INSUFFICIENT_BUFFER)
            {
                WCHAR eventDataBuffer[1024] = {}; // production code should use dynamic memory

                properties = (PEVT_VARIANT)eventDataBuffer;

                result = ::EvtRender(
                    renderContext,
                    event,
                    EvtRenderEventValues,
                    sizeof(eventDataBuffer),
                    properties,
                    &bufferUsed,
                    &propertyCount);
            }
        }

        if (!result)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            wcout << L"EvtRender failed with error " << GetLastError() << endl;
        }

        if (SUCCEEDED(hr))
        {
            if (properties[0].Type == EvtVarTypeString)
            {
                varData = properties[0].StringVal;
            }
            else if (properties[0].Type == EvtVarTypeUInt32)
            {
                varData.ulVal = properties[0].UInt32Val;
                varData.vt = VT_UI4;
            }
            else
            {
                wcout << L"Conversion from  " << properties[0].Type << L" not implemented" << endl;
                hr = E_FAIL;
            }
        }

        if (renderContext != NULL)
        {
            EvtClose(renderContext);
        }
    
    }

    return hr;
}

HRESULT DisplayUnoptimizationFileError(_In_ EVT_HANDLE event)
{
    wstring parentPath = L"";
    wstring fileName = L"";
    wstring errorMessage = L"";
    ULONG errorCode = 0;
    
    variant_t var;
    HRESULT hr = GetEventData(event, L"ParentDirectoryPath", var);

    if (SUCCEEDED(hr))
    {
        parentPath = var.bstrVal;
    }
    
    var.Clear();
    hr = GetEventData(event, L"FileName", var);

    if (SUCCEEDED(hr))
    {
        fileName = var.bstrVal;
    }

    var.Clear();
    hr = GetEventData(event, L"ErrorMessage", var);

    if (SUCCEEDED(hr))
    {
        errorMessage = var.bstrVal;
    }

    var.Clear();
    hr = GetEventData(event, L"ErrorCode", var);

    if (SUCCEEDED(hr))
    {
        errorCode = var.ulVal;
    }

    wstring filePath = parentPath + fileName;
    
    wcout << L"Unoptimization file error" << endl
        << L"    File path: " << filePath << endl
        << L"    Error code: 0x"  << hex << errorCode << endl
        << L"    Error message: " << errorMessage << endl;
    
    return hr;
}

HRESULT CheckForUnoptimizationFileErrors(_In_ const wstring& jobId, _Out_ bool& foundErrors)
{
    HRESULT hr = S_OK;
    foundErrors = false;

    // Query for errors from the specified job ID

    // Build the query string
    const PCWSTR eventQueryPrefix = 
        L"<QueryList> <Query Id=\"0\" Path=\"Microsoft-Windows-Deduplication/Operational\"> "
        L"<Select Path=\"Microsoft-Windows-Deduplication/Operational\">*[System[(EventID=6144)] "
        L"and EventData[Data[(@Name=\"JobInstanceId\")]=\"";
    
    const PCWSTR eventQuerySuffix = 
        L"\"]] </Select> </Query> </QueryList>";

    wstring queryXml = eventQueryPrefix;
    queryXml.append(jobId);
    queryXml.append(eventQuerySuffix);

    // Execute the query
    EVT_HANDLE queryResult = EvtQuery(
        NULL,
        DEDUP_OPERATIONAL_EVENT_CHANNEL_NAME,
        queryXml.c_str(),
        //L"Event/System[EventID=6144]",
        EvtQueryChannelPath | EvtQueryReverseDirection);

    if (queryResult == NULL)
    {
        DWORD error = ::GetLastError();
        hr = HRESULT_FROM_WIN32(error);
        wcout << L"EvtQuery for unoptimization file errors failed with error " << error << endl;
    }

    // Process the returned events, if any
    while (SUCCEEDED(hr))
    {    
        DWORD eventsReturned = 0;
        EVT_HANDLE eventHandle = NULL;
        
        BOOL bStatus = EvtNext(
            queryResult,
            1,
            &eventHandle,
            INFINITE,
            0,
            &eventsReturned);

        if (!bStatus)
        {
            DWORD error = ::GetLastError();

            if (error != ERROR_NO_MORE_ITEMS)
            {
                hr = HRESULT_FROM_WIN32(error);
                wcout << L"EvtNext for unoptimization file error failed with error " << error << endl;
            }
            else
            {
                break;
            }
        }
        else if (eventsReturned == 1)
        {
            DisplayUnoptimizationFileError(eventHandle);
            EvtClose(eventHandle);
            foundErrors = true;
        }
    }

    if (queryResult != NULL)
    {
        EvtClose(queryResult);
    }
    
    return hr;
}

HRESULT UnoptimizeSinceTimestamp(_In_ const wstring& volumeGuidName, _In_ wstring& backupTime)
{
    // Setup for WMI method call - get WBEM services, input parameter object
    CComPtr<IWbemServices> spWmi;
    HRESULT hr = WmiGetWbemServices(CIM_DEDUP_NAMESPACE, spWmi);

    CComPtr<IWbemClassObject> spInParams;
    if (SUCCEEDED(hr))
    {
        hr = WmiGetMethodInputParams(spWmi, CIM_DEDUP_CLASS_JOB, CIM_DEDUP_METHOD_START, spInParams);
    }

    // Volume parameter
    if (SUCCEEDED(hr))
    {
        hr = WmiAddVolumeInputParameter(spInParams, CIM_DEDUP_CLASS_JOB, CIM_DEDUP_METHOD_START, volumeGuidName);
    }

    // Job type parameter
    if (SUCCEEDED(hr))
    {
        // Unoptimization job is type=4 (from Data Deduplication MOF schema)
        variant_t var;
        var.lVal = CIM_DEDUP_JOB_TYPE_UNOPT;
        var.vt = VT_I4;
        hr = WmiAddInputParameter(spInParams, CIM_DEDUP_CLASS_JOB, CIM_DEDUP_METHOD_START, CIM_DEDUP_PROP_TYPE, var);
    }

    // Timestamp parameter
    if (SUCCEEDED(hr))
    {
        variant_t var = backupTime.c_str();
        hr = WmiAddInputParameter(spInParams, CIM_DEDUP_CLASS_JOB, CIM_DEDUP_METHOD_START, CIM_DEDUP_PROP_TIMESTAMP, var);
    }

    // Job wait parameter for synchronous job
    // NOTE: real backup applications might run optimization job asynchronously, track progress, etc.
    if (SUCCEEDED(hr))
    {
        variant_t var = (bool) true;
        hr = WmiAddInputParameter(spInParams, CIM_DEDUP_CLASS_JOB, CIM_DEDUP_METHOD_START, CIM_DEDUP_PROP_WAIT, var);
    }

    // Execute the unoptimization job
    wstring jobId;
    if (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> spOutParams;
        hr = WmiExecuteMethod(spWmi, CIM_DEDUP_CLASS_JOB, CIM_DEDUP_METHOD_START, spInParams, spOutParams, L"Unoptimization");

        // Get the job id for error tracking
        if (SUCCEEDED(hr))
        {
            hr = GetJobInstanceId(spOutParams, jobId);
        }
    }

    // Check for file errors
    if (SUCCEEDED(hr))
    {
        bool foundErrors = false;
        hr = CheckForUnoptimizationFileErrors(jobId, foundErrors);

        if (foundErrors)
        {
            wcout << L"One or more files failed to unoptimize. Resolve the problems and retry the restore." << endl;
            hr = E_FAIL;
        }
    }
    
    return hr;
}

HRESULT RunGarbageCollection(_In_ const wstring& volumeGuidName)
{
    // Setup for WMI method call - get WBEM services, input parameter object
    CComPtr<IWbemServices> spWmi;
    HRESULT hr = WmiGetWbemServices(CIM_DEDUP_NAMESPACE, spWmi);

    CComPtr<IWbemClassObject> spInParams;
    if (SUCCEEDED(hr))
    {
        hr = WmiGetMethodInputParams(spWmi, CIM_DEDUP_CLASS_JOB, CIM_DEDUP_METHOD_START, spInParams);
    }

    // Volume parameter
    if (SUCCEEDED(hr))
    {
        hr = WmiAddVolumeInputParameter(spInParams, CIM_DEDUP_CLASS_JOB, CIM_DEDUP_METHOD_START, volumeGuidName);
    }

    // Job type parameter
    if (SUCCEEDED(hr))
    {
        // GC job is type=2 (from Data Deduplication MOF schema)
        variant_t var;
        var.lVal = CIM_DEDUP_JOB_TYPE_GC;
        var.vt = VT_I4;
        hr = WmiAddInputParameter(spInParams, CIM_DEDUP_CLASS_JOB, CIM_DEDUP_METHOD_START, CIM_DEDUP_PROP_TYPE, var);
    }

    // Job wait parameter for synchronous job
    // NOTE: Backup applications are not required to wait for GC completion
    if (SUCCEEDED(hr))
    {
        variant_t var = (bool) true;
        hr = WmiAddInputParameter(spInParams, CIM_DEDUP_CLASS_JOB, CIM_DEDUP_METHOD_START, CIM_DEDUP_PROP_WAIT, var);
    }


    // Execute the method
    // Job will be queued and execute asynchronously
    if (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> spOutParams;
        hr = WmiExecuteMethod(spWmi, CIM_DEDUP_CLASS_JOB, CIM_DEDUP_METHOD_START, spInParams, spOutParams, L"GarbageCollection");
    }
    return hr;
}


HRESULT DeleteDedupStore(_In_ const wstring& volume)
{
    HRESULT hr = S_OK;

    wstring dedupStorePath = BuildDedupStorePath(volume);
    
    hr = DeleteDirectoryTree(dedupStorePath);
    
    return hr;
}

/////////////////////////////////////////////////////////////////////
//
// Program main
//
/////////////////////////////////////////////////////////////////////

int __cdecl _tmain(_In_ int argc, _In_reads_(argc) _TCHAR* argv[])
{
    wstring source, destination;
    Action action;

    if (!ParseCommandLine(argc, argv, &action, &source, &destination))
    {
        PrintUsage(argv[0]);
        return 1;
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        hr =  CoInitializeSecurity(
            NULL, 
            -1,                          // COM authentication
            NULL,                        // Authentication services
            NULL,                        // Reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
            RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
            NULL,                        // Authentication info
            EOAC_NONE,                   // Additional capabilities 
            NULL                         // Reserved
            );

        if (SUCCEEDED(hr))
        {
            hr = ModifyPrivilege(SE_BACKUP_NAME, TRUE);
        }
        
        if (SUCCEEDED(hr))
        {
            hr = ModifyPrivilege(SE_RESTORE_NAME, TRUE);
        }

        // Permit paths longer than MAX_PATH
        source = wstring(LONG_PATH_PREFIX) + source;
        destination = wstring(LONG_PATH_PREFIX) + destination;
        
        if (SUCCEEDED(hr))
        {
            switch (action)
            {
            case BackupAction:
                DoBackup(source, destination);
                break;
            case RestoreStubAction:
                hr = RestoreStub(source, destination);
                break;
            case RestoreDataAction:
                hr = RestoreData(source, destination);
                break;
            case RestoreFileAction:
                hr = RestoreStub(source, destination);
                if (SUCCEEDED(hr))
                {
                    hr = RestoreData(source, destination);
                }
                break;
            case RestoreVolumeAction:
                hr = RestoreVolume(source, destination);
                break;
            case RestoreFilesAction:
                vector<wstring> restoredFiles;
                hr = RestoreFiles(source, destination, false, &restoredFiles);
                if (SUCCEEDED(hr))
                {
                    hr = RestoreFilesData(source, restoredFiles);
                }
                restoredFiles.clear();
                break;
            }
        }

        CoUninitialize();
    }

    if (FAILED(hr)) return 1;
    return 0;
}

/////////////////////////////////////////////////////////////////////
//
// Backup/restore utilities
//
/////////////////////////////////////////////////////////////////////

wstring BuildDedupStorePath(_In_ const wstring& volume)
{
    wstring dedupStorePath = TrimTrailingSeparator(volume, L'\\');
        
    dedupStorePath.append(SYSTEM_VOLUME_INFORMATION);
    dedupStorePath.append(DEDUP_FOLDER);

    return dedupStorePath;
}

/////////////////////////////////////////////////////////////////////
//
// Backup related methods
//
/////////////////////////////////////////////////////////////////////

HRESULT BackupFile(_In_ const wstring& source, _In_ const wstring& destination)
{
    // Open the source file
    HANDLE hSourceFile = ::CreateFile(
                        source.c_str(),
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                        NULL);
    if (hSourceFile == INVALID_HANDLE_VALUE) 
    {
        wcout << L"CreateFile(" << source << L" failed with error " << GetLastError() << endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Open the backup medium
    // in this example the medium is another file, but it could be tape, network server, etc...
    HANDLE hDestinationFile = ::CreateFile(
                        destination.c_str(),
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_FLAG_BACKUP_SEMANTICS,
                        NULL);
    if (hDestinationFile == INVALID_HANDLE_VALUE) 
    {
        wcout << L"CreateFile(" << destination << L") failed with error " << GetLastError() << endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // 4k is the default NTFS cluster size, and small enough to fit into the stack
    // we could also readk 8k if we can afford the extra stack cost or allocate a heap buffer
    // and read 64k or more
    const DWORD DEFAULT_BUFFER_SIZE = 4086;  
    DWORD bytesRead = 0, bytesWritten = 0;
    BYTE buffer[DEFAULT_BUFFER_SIZE];
    LPVOID context = NULL;
    HRESULT hr = S_OK;

    // Read 4k at a time. BackupRead will return attributes, security, and reparse point information
    while (BackupRead(hSourceFile, buffer, DEFAULT_BUFFER_SIZE, &bytesRead, FALSE, TRUE, &context) && bytesRead > 0)
    {
        // Save the data describing the source file to the destination medium.
        // we do a write file here, but if this would be a network server you could send on a socket
        if (!WriteFile(hDestinationFile, buffer, bytesRead, &bytesWritten, NULL))
        {
            wcout << L"WriteFile(" << destination << L") failed with error " << GetLastError() << endl;
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        if (bytesRead != bytesWritten) 
        {
            wcout << L"WriteFile(" << destination << L") unexpectedly wrote less bytes than expected (expected:" << bytesRead << L" written:" << bytesWritten << L")" << endl;
            hr = E_UNEXPECTED;
            break;
        }
    }

    // Call BackupRead one more time to clean up the context
    BackupRead(hSourceFile, NULL, 0, NULL, TRUE, TRUE, &context);

    // Close the source file
    CloseHandle(hSourceFile);

    // Close the backup medium
    CloseHandle(hDestinationFile);

    return hr;
}

HRESULT BackupDirectory(_In_ const wstring& source, _In_ const wstring& destination)
{
    HRESULT hr = S_OK;

    // Create the corresponding directory in the destination
    if (!::CreateDirectory(destination.c_str(), NULL))
    {
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS)
        {
            wcout << L"CreateDirectory(" << destination << L") failed with error " << GetLastError() << endl;
            hr =  HRESULT_FROM_WIN32(error);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Backup the directory
        wstring directoryBackupPath = destination;
        directoryBackupPath.append(L"\\");
        directoryBackupPath.append(DIRECTORY_BACKUP_FILE);

        hr = BackupFile(source, directoryBackupPath);
    }

    return hr;
}

void BackupDirectoryTree(_In_ const wstring& source, _In_ const wstring& destination)
{
    // Backup the directory
    HRESULT hr = BackupDirectory(source, destination);
    if (FAILED(hr))
    {
        return;
    }
    
    // Walk through all the files and subdirectories
    WIN32_FIND_DATA findData;
    wstring pattern = source;
    pattern += L"\\*";
    HANDLE hFind = FindFirstFile(pattern.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do 
        {
            // If not . or ..
            if (findData.cFileName[0] != '.')
            {
                wstring newSource = source;
                newSource += '\\';
                newSource += findData.cFileName;
                wstring newDestination = destination;
                newDestination += '\\';
                newDestination += findData.cFileName;
                // Backup the source file or directory
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                {
                    // NOTE: This code is using recursion for simplicity, however a real backup application
                    // should avoid recursion since it will overflow the stack for a deep directory tree
                    BackupDirectoryTree(newSource, newDestination);
                }
                else 
                {
                    // Do BackupRead and backup the file
                    hr = BackupFile(newSource, newDestination);
                    if (FAILED(hr))
                    {
                        // NOTE: This code ignores BackupFile errors,
                        // but a backup app might handle it differently
                        wcout << L"BackupFile failed, hr = 0x" << hex << hr << endl;
                    }
                }
            }
        } while (FindNextFile(hFind, &findData));

        FindClose(hFind);
    }
}

HRESULT VolumeHasDedupMetadata(_In_ const wstring& volumeGuidName, _Out_ bool& hasDedupMetadata, _Out_ wstring& chunkStoreId)
{
    chunkStoreId.clear();
    hasDedupMetadata = false;
    
    HRESULT hr = GetDedupChunkStoreId(volumeGuidName, chunkStoreId);

    if (SUCCEEDED(hr) && !chunkStoreId.empty())
    {
        hasDedupMetadata = true;
    }

    return hr;
}

void WriteBackupMetadata(_In_ const wstring& source, _In_ const wstring& destination)
{
    wstring volumeGuidName;
    wstring chunkStoreId;
    
    HRESULT hr = GetVolumeGuidNameForPath(source, volumeGuidName);

    // Get the deduplication chunk store ID
    if (SUCCEEDED(hr))
    {
        hr = GetDedupChunkStoreId(volumeGuidName.c_str(), chunkStoreId);
    }

    // Write the store ID and backup timestamp to the backup metadata file
    if (SUCCEEDED(hr) && !chunkStoreId.empty())
    {
        wstring filePath = destination;
        filePath.append(L"\\");
        filePath.append(BACKUP_METADATA_FILE_NAME);
        
        FILE* metadataFile = NULL;
        errno_t err = _wfopen_s(&metadataFile, filePath.c_str(), L"w");
        if (err != 0)
        {
            wcout << L"Unable to create backup metadata file: " << filePath << endl;
        }
        else
        {
            // A real backup application would use the VSS snapshot timestamp
            FILETIME ftNow = {};
            GetSystemTimeAsFileTime(&ftNow);

            // Convert time to wbem time string
            WBEMTime backupTime(ftNow);            
            bstr_t bstrTime = backupTime.GetDMTF();
            
            _ftprintf(metadataFile, BACKUP_METADATA_FORMAT, chunkStoreId.c_str(), (WCHAR*)bstrTime);
            fclose(metadataFile);
        }
    }
}

HRESULT ReadBackupMetadata(_In_ const wstring& source, _Out_ wstring& chunkStoreId, _Out_ wstring& backupTime)
{
    HRESULT hr = S_OK;
    wstring filePath = source;

    backupTime.clear();
    chunkStoreId.clear();
    
    filePath.append(L"\\");
    filePath.append(BACKUP_METADATA_FILE_NAME);
    
    FILE* metadataFile = NULL;
    errno_t err = _wfopen_s(&metadataFile, filePath.c_str(), L"r");
    if (err != 0)
    {
        wcout << L"Unable to open backup metadata file: " << filePath << endl;
        hr = E_FAIL;
    }
    else
    {
        WCHAR storeId[] = L"{00000000-0000-0000-0000-000000000000}";
        WCHAR timestamp[] = L"yyyymmddhhmmss.nnnnnn-ggg";

        int converted = fwscanf_s(metadataFile, BACKUP_METADATA_FORMAT, storeId, ARRAY_LEN(storeId), timestamp, ARRAY_LEN(timestamp));
        if (converted != 2)
        {
            wcout << L"Unable to read chunk store ID and backup time from backup metadata file: " << filePath << endl;            
        }
        else
        {
            chunkStoreId = storeId;
            backupTime = timestamp;
        }
        fclose(metadataFile);
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////
//
// Restore related methods
//
/////////////////////////////////////////////////////////////////////

HRESULT RestoreFile(_In_ const wstring& source, _In_ const wstring& destination, _In_opt_ bool overWriteExisting)
{
    // Create destination dir if it doesn't exist
    size_t lastSeparator = destination.rfind('\\');
    if (lastSeparator == wstring::npos) 
    {
        wcout << L"destination is not a file path" << endl;
        return E_UNEXPECTED;
    }
    wstring destinationLocation = destination.substr(0, lastSeparator);
    ::CreateDirectory(destinationLocation.c_str(), NULL);

    // Open the backup medium
    // In this example the medium is another file, but it could be tape, network server, etc...
    HANDLE hSourceFile = ::CreateFile(
                        source.c_str(),
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                        NULL);
    if (hSourceFile == INVALID_HANDLE_VALUE) 
    {
        wcout << L"CreateFile(" << source << L") failed with error " << GetLastError() << endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Open the file to be restored
    HANDLE hDestinationFile = ::CreateFile(
                        destination.c_str(),
                        GENERIC_WRITE | WRITE_OWNER | WRITE_DAC,
                        FILE_SHARE_READ,
                        NULL,
                        overWriteExisting ? OPEN_EXISTING : CREATE_ALWAYS,
                        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                        NULL);
    if (hDestinationFile == INVALID_HANDLE_VALUE) 
    {
        wcout << L"CreateFile(" << destination << L") failed with error " << GetLastError() << endl;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const DWORD DEFAULT_BUFFER_SIZE = 4086;
    DWORD bytesRead, bytesWritten;
    BYTE buffer[DEFAULT_BUFFER_SIZE];
    LPVOID context = NULL;
    HRESULT hr = S_OK;

    // Read 4k at a time from the backup medium. BackupRead will return attributes, security, and reparse point information
    while (ReadFile(hSourceFile, buffer, DEFAULT_BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0)
    {
        // Call BackupWrite to restore the file, including security, attributes and reparse point
        if (!BackupWrite(hDestinationFile, buffer, bytesRead, &bytesWritten, FALSE, TRUE, &context))
        {
            wcout << L"BackupWrite(" << destination << L") failed with error " << GetLastError() << endl;
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        if (bytesRead != bytesWritten) 
        {
            wcout << L"BackupWrite(" << destination << L") unexpectedly wrote less bytes than expected (expected:" << bytesRead << L" written:" << bytesWritten << L")" << endl;
            hr = E_UNEXPECTED;
            break;
        }
    }

    // Call BackupWrite one more time to clean up the context
    BackupWrite(hDestinationFile, NULL, 0, NULL, TRUE, TRUE, &context);

    // Close the backup medium
    CloseHandle(hSourceFile);

    // Close the destination file
    CloseHandle(hDestinationFile);

    return hr;
}

HRESULT RestoreDirectory(_In_ const wstring& source, _In_ const wstring& destination)
{
    HRESULT hr = S_OK;
    
    wstring directorySourcePath = source;
    directorySourcePath.append(L"\\");
    directorySourcePath.append(DIRECTORY_BACKUP_FILE);

    // Create the corresponding directory in the destination, if not already present
    if (!::CreateDirectory(destination.c_str(), NULL))
    {
        DWORD error = GetLastError();
        // Access denied error can occur for volume root directory
        // The sample code also exempts access denied error for all other directories
        // A real backup application may handle this condition differently
        if (error != ERROR_ALREADY_EXISTS && error != ERROR_ACCESS_DENIED)
        {
            wcout << L"CreateDirectory(" << destination << L") failed with error " << GetLastError() << endl;
            hr =  HRESULT_FROM_WIN32(error);
        }
    }
    
    if (SUCCEEDED(hr))
    {
        // Restore the directory
        RestoreFile(directorySourcePath, destination, true);
    }
    
    return hr;
}


HRESULT DeleteDirectoryTree(_In_ const wstring& directory)
{
    HRESULT hr = S_OK;
    
    // Walk through all the files and subdirectories
    WIN32_FIND_DATA findData;
    wstring pattern = directory;
    pattern += L"\\*";
    HANDLE hFind = FindFirstFile(pattern.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do 
        {
            // If not . or ..
            if (findData.cFileName[0] != '.')
            {
                wstring newPath = directory;
                newPath += '\\';
                newPath += findData.cFileName;
                
                // Backup the source file or directory
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                {
                    // NOTE: This code is using recursion for simplicity, however a real backup application
                    // should avoid recursion since it will overflow the stack for a deep directory tree
                    hr = DeleteDirectoryTree(newPath);
                    if (FAILED(hr))
                    {
                        break;
                    }

                    // NOTE: This code is ignoring directory deletion failures since the same tree will be restored anyway.
                    // real backup applications migh choose to handle this differently.
                    RemoveDirectory(newPath.c_str());
                }
                else 
                {
                    // Do BackupRead and backup the file
                    BOOL result = DeleteFile(newPath.c_str());
                    if (!result)
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        wcout << L"DeleteFile failed, hr = 0x" << hex << hr << endl;
                        break;
                    }
                }
            }
        } while (FindNextFile(hFind, &findData));

        FindClose(hFind);
    }

    return hr;
}

HRESULT RestoreDirectoryTree(_In_ const wstring& source, _In_ const wstring& destination, _In_ const vector<wstring>& sourceExcludePaths, _Out_opt_ vector<wstring>* pRestoredFiles)
{
    HRESULT hr = S_OK;

    if (pRestoredFiles != NULL)
    {
        pRestoredFiles->clear();
    }
    
    // Check for exclusion
    for (size_t index = 0; index < sourceExcludePaths.size(); index++)
    {
        wstring excludePath = sourceExcludePaths[index];
        if (wcscmp(source.c_str(), excludePath.c_str()) == 0)
        {
            return hr = S_FALSE;
        }
    }

    // Restore the directory
    hr = RestoreDirectory(source, destination);
    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) && hr != HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
    {
        return hr;
    }
    hr = S_OK;
    
    wstring trimmedSource = TrimTrailingSeparator(source, L'\\');
    
    // Walk through all the files and subdirectories
    WIN32_FIND_DATA findData;
    wstring pattern = trimmedSource;
    pattern += L"\\*";
    HANDLE hFind = FindFirstFile(pattern.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do 
        {
            // If not . or ..
            if (findData.cFileName[0] != '.')
            {
                wstring newSource = trimmedSource;
                newSource += '\\';
                newSource += findData.cFileName;
                wstring newDestination = destination;
                newDestination += '\\';
                newDestination += findData.cFileName;
                // Restore the file or directory
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                {

                    // NOTE: This code is using recursion for simplicity, however a real backup application
                    // should avoid recursion since it will overflow the stack for a deep directory tree
                    hr = RestoreDirectoryTree(newSource, newDestination, sourceExcludePaths, pRestoredFiles);
                    if (FAILED(hr))
                    {
                        break;
                    }
                }
                else if ((_wcsicmp(findData.cFileName, DIRECTORY_BACKUP_FILE) == 0) ||
                         (_wcsicmp(findData.cFileName, BACKUP_METADATA_FILE_NAME) == 0))
                {
                    // This file is backup metadata, not original volume data
                    continue;
                }
                else 
                {
                    // Restore the file
                    hr = RestoreFile(newSource, newDestination);
                    if (FAILED(hr))
                    {
                        if (hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) || hr == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION))
                        {
                            // Some files may be in busy, protected by a filter, etc.
                            // An online restore may not be able to replace every file
                            wcout << L"Warning: continuing restore after RestoreFile failed, hr = 0x" << hex << hr << endl;
                            hr = S_OK;
                        }
                        else
                        {
                            wcout << L"RestoreFile failed, hr = 0x" << hex << hr << endl;
                            break;
                        }
                    }
                    if (pRestoredFiles)
                    {
                        pRestoredFiles->push_back(newDestination);
                    }
                }
            }
        } while (FindNextFile(hFind, &findData));

        FindClose(hFind);
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////
//
// WMI utilities
//
/////////////////////////////////////////////////////////////////////

HRESULT WmiGetWbemServices(_In_ PCWSTR wmiNamespace, _Out_ CComPtr<IWbemServices>& spWmi)
{
    CComPtr<IWbemLocator> spLocator = NULL;

    bstr_t bstrNamespace = wmiNamespace;

    HRESULT hr = CoCreateInstance(
        CLSID_WbemLocator, 
        0, 
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, 
        (void**) &spLocator
        );
    if (FAILED(hr))
    {
        wcout << L"CoCreateInstance(IWbemLocator) failed, hr = 0x" << hex << hr << endl;
    }

    if (SUCCEEDED(hr))
    {
        hr = spLocator->ConnectServer(
            bstrNamespace,
            NULL, 
            NULL, 
            NULL, 
            0L,
            NULL,
            NULL,
            &spWmi
            );
        if (FAILED(hr))
        {
            wcout << L"unable to connect to WMI; namespace " << CIM_DEDUP_NAMESPACE << L", hr = 0x" << hex << hr << endl;
        }
    }
    
    if (SUCCEEDED(hr))
    {
        hr = CoSetProxyBlanket(
            spWmi,
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            NULL,
            RPC_C_AUTHN_LEVEL_PKT,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, 
            EOAC_NONE
            );

        if (FAILED(hr))
        {
            wcout << L"CoSetProxyBlanket failed, hr = 0x" << hex << hr << endl;
        }
    }
    
    return hr;
}

HRESULT WmiGetMethodInputParams(_In_ IWbemServices* pWmi, _In_ PCWSTR className, _In_ PCWSTR methodName, _Out_ CComPtr<IWbemClassObject>& spInParams)
{
    CComPtr<IWbemClassObject> spClass;

    spInParams = NULL;
    
    HRESULT hr = pWmi->GetObject(bstr_t(className), 0, NULL, &spClass, NULL);
    if (FAILED(hr))
    {
        wcout << L"WMI query for class " << className << L" failed with error 0x" << hex << hr << endl;
    }

    CComPtr<IWbemClassObject> spInParamsDefinition;
    if (SUCCEEDED(hr))
    {
        hr = spClass->GetMethod(methodName, 0, &spInParamsDefinition, NULL);
        if (FAILED(hr))
        {
            wcout << L"WMI query for method " << className << L"." << (PCWSTR)methodName << L" failed with error 0x" << hex << hr << endl;
        }

        // GetMethod returns NULL for method that takes no input parameters.
    }

    if (SUCCEEDED(hr) && (spInParamsDefinition != NULL))
    {
        hr = spInParamsDefinition->SpawnInstance(0, &spInParams);
        if (FAILED(hr))
        {
            wcout << L"WMI input parameter creation for method " << className << L"." << (PCWSTR)methodName << L" failed with error 0x" << hex << hr << endl;
        }
    }
    else if (spInParamsDefinition == NULL)
    {
        hr = S_FALSE;
    }
    
    return hr;
}

HRESULT WmiAddVolumeInputParameter(_Inout_ IWbemClassObject* pParams, _In_ PCWSTR className, _In_ PCWSTR methodName, _In_ const wstring& volume)
{
    HRESULT hr = S_OK;
    SAFEARRAY* psa = NULL;
    SAFEARRAYBOUND saBound = {};
    saBound.lLbound = 0;
    saBound.cElements = 1;
    
    psa = SafeArrayCreate(VT_BSTR, 1, &saBound);
    if (psa == NULL)
    {
        wcout << L"Out of memory creating safe-array" << endl;
        hr = E_OUTOFMEMORY;
    }
    
    if (SUCCEEDED(hr))
    {
        
        long index = 0;
        hr = SafeArrayPutElement(psa, &index, (BSTR)bstr_t(volume.c_str()));
        if (FAILED(hr))
        {
            wcout << L"SafeArrayPutElement(0, " << volume << L") failed with error 0x" << hex << hr << endl;
        }
    }
    
    VARIANT volumeArray;
    VariantInit(&volumeArray);
    volumeArray.vt = VT_ARRAY | VT_BSTR;
    volumeArray.parray = psa;
    
    if (SUCCEEDED(hr))
    {
        hr = pParams->Put(bstr_t(CIM_DEDUP_PROP_VOLUME), 0, &volumeArray, 0);
        if (FAILED(hr))
        {
            wcout << L"Setting property " << className << L"." << methodName << L"." << CIM_DEDUP_PROP_VOLUME << L" failed with error 0x" << hex << hr << endl;
        }
    }

    if (psa != NULL)
    {
        SafeArrayDestroy(psa);
    }

    return hr;
}

HRESULT WmiAddInputParameter(_Inout_ IWbemClassObject* pParams, _In_ PCWSTR className, _In_ PCWSTR methodName, _In_ PCWSTR propertyName, _In_ variant_t& var)
{
    HRESULT hr = S_OK;
    
    if (SUCCEEDED(hr))
    {
        hr = pParams->Put(bstr_t(propertyName), 0, &var, 0);
        if (FAILED(hr))
        {
            wcout << L"Setting property " << className << L"." << methodName << L"." << propertyName << L" failed with error 0x" << hex << hr << endl;
        }
    }

    return hr;
}

HRESULT WmiGetErrorInfo(_Out_ HRESULT& hrOperation, _Out_ wstring& errorMessageOperation)
{
    CComPtr<IErrorInfo> spErrorInfo;
    CComPtr<IWbemClassObject> spWmiError;

    HRESULT hr = GetErrorInfo(0, &spErrorInfo);  
    if (FAILED(hr))
    {
        wcout << L"GetErrorInfo failed with error 0x" << hex << hr << endl;                
    }

    if (SUCCEEDED(hr))
    {
        hr = spErrorInfo->QueryInterface(&spWmiError);
        if (FAILED(hr))
        {
            wcout << L"IErrorInfo::QueryInterface failed with error 0x" << hex << hr << endl;                
        }
    }

    if (SUCCEEDED(hr))
    {
        variant_t var;
        
        // Get the value of the ErrorCode property
        hr = spWmiError->Get(CIM_DEDUP_PROP_ERRORCODE, 0, &var, 0, 0);
        if (FAILED(hr))
        {
            wcout << L"IWbemClassObject::Get for property " << CIM_DEDUP_PROP_ERRORCODE << L" failed with error 0x" << hex << hr << endl;
        }
        else
        {
            // This is the root cause error
            hrOperation = var.ulVal;

            // Get the error message
            hr = spWmiError->Get(CIM_DEDUP_PROP_ERRORMESSAGE, 0, &var, 0, 0);
            if (FAILED(hr))
            {
                wcout << L"IWbemClassObject::Get for property " << CIM_DEDUP_PROP_ERRORMESSAGE << L" failed with error 0x" << hex << hr << endl;
            }
            else
            {
                errorMessageOperation = var.bstrVal;
            }
        }
    }

    return hr;
}

HRESULT WmiExecuteMethod(_In_ IWbemServices* pWmi, _In_ PCWSTR className, _In_ PCWSTR methodName, _In_opt_ IWbemClassObject* pInParams, 
    _Out_ CComPtr<IWbemClassObject>& spOutParams, _In_opt_ PCWSTR context)
{
    // Make the method call
    HRESULT hr = pWmi->ExecMethod(bstr_t(className), bstr_t(methodName), 0, NULL, pInParams, &spOutParams, NULL);
    
    // Evaluate the output parameter object
    if (SUCCEEDED(hr))
    {
        variant_t var;
        hr = spOutParams->Get(bstr_t(CIM_DEDUP_PROP_RETURNVALUE), 0, &var, NULL, 0);
        if (FAILED(hr))
        {
            wcout << L"Get method return value for " << className << L"." << methodName << L"(" << context << L")" << L" failed with error 0x" << hex << hr << endl;
        }
    
        if (SUCCEEDED(hr) && FAILED(var.ulVal))
        {
            hr = var.ulVal;
            wcout << L"WMI method " << className << L"." << methodName << L"(" << context << L")" << L" failed with error 0x" << hex << hr << endl;
        }
    }
    else
    {
        // Get the root cause error
        HRESULT hrLocal = S_OK;
        HRESULT hrOperation = S_OK;
        wstring errorMessage;
        
        hrLocal = WmiGetErrorInfo(hrOperation, errorMessage);

        if (SUCCEEDED(hrLocal) && FAILED(hrOperation))
        {
            hr = hrOperation;
            wcout << L"WMI method execution " << className << L"." << methodName << L"(" << context << L")" << L" failed with error 0x" << hex << hr << endl;
            wcout << L"    Error message: " << errorMessage << endl;
        }
    }

    return hr;
}

HRESULT WmiGetDedupInstanceByVolumeId(PCWSTR className, _In_ const wstring& volumeGuidName, _Out_ CComPtr<IWbemClassObject>& instance)
{
    CComPtr<IWbemServices> spWmi;
    CComPtr<IWbemClassObject> spInstance;

    HRESULT hr = WmiGetWbemServices(CIM_DEDUP_NAMESPACE, spWmi);

    if (SUCCEEDED(hr))
    {
        wstring objectPath = className;
        objectPath.append(L".VolumeId='");
        objectPath.append(volumeGuidName);
        objectPath.append(L"'");

        // Get the specified object
        hr = spWmi->GetObject(
            bstr_t(objectPath.c_str()),
            WBEM_FLAG_RETURN_WBEM_COMPLETE, 
            NULL, // context
            &spInstance,
            NULL // synchronous call; not needed
            );

        if (FAILED(hr))
        {
            // Get the root cause error
            HRESULT hrLocal = S_OK;
            HRESULT hrOperation = S_OK;
            wstring errorMessage;
            
            hrLocal = WmiGetErrorInfo(hrOperation, errorMessage);

            if (SUCCEEDED(hrLocal) && FAILED(hrOperation))
            {
                if (hrOperation == DDP_E_NOT_FOUND || hrOperation == DDP_E_PATH_NOT_FOUND || hrOperation == DDP_E_VOLUME_DEDUP_DISABLED)
                {
                    // The object is not found
                    hr = S_FALSE;
                }
                else
                {
                    hr = hrOperation;
                    wcout << L"WMI query for " << objectPath << L" failed with error 0x" << hex << hr << endl;
                    wcout << L"    Error message: " << errorMessage << endl;
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        instance = spInstance;
    }
    
    return hr;
}

HRESULT WmiQueryDedupInstancesByVolumeId(_In_ IWbemServices* pWmi, _In_ const wstring& className, _In_ const wstring& volumeGuidName, _Out_ CComPtr<IEnumWbemClassObject>& instances)
{
    CComPtr<IWbemServices> spWmi;

    HRESULT hr = WmiGetWbemServices(CIM_DEDUP_NAMESPACE, spWmi);

    // Backslash chars need to be escaped in WMI queries
    wstring quotedVolumeGuidName = volumeGuidName;
    StringReplace(quotedVolumeGuidName, L"\\", L"\\\\");
    
    wstring query = L"select * from ";
    query.append(className);
    query.append(L" where VolumeId='");
    query.append(quotedVolumeGuidName);
    query.append(L"'");
    
    hr = pWmi->ExecQuery(
        bstr_t("WQL"), 
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &instances
        );

    if (FAILED(hr))
    {
        wcout << L"WMI query failed with error 0x" << hex << hr << L", query: " << query << endl;
    }

    return hr;
}


/////////////////////////////////////////////////////////////////////
//
// Common utilities
//
/////////////////////////////////////////////////////////////////////


void StringReplace(_Inout_ wstring& stringValue, _In_ const wstring& matchValue, _In_ const wstring& replaceValue)
{
    if (!stringValue.empty()) 
    {
        if (matchValue.compare(replaceValue) != 0) 
        {
            wstring::size_type pos = stringValue.find(matchValue, 0);
            
            while (pos != wstring::npos)
            {

                stringValue.replace(
                    pos, 
                    matchValue.length(), 
                    replaceValue);

                pos = stringValue.find(
                    matchValue, 
                    pos + replaceValue.length()
                    );
            }
        }
        else
        {
            // If the two values are the same, we don't need to replace anything
        }
    }
}

wstring TrimTrailingSeparator(_In_ const wstring& str, _In_ WCHAR separator)
{
    wstring returnString = str;
    
    std::wstring::size_type pos = returnString.find_last_not_of(separator);

    if (pos != std::wstring::npos)
    {
        ++pos;
    }
    else
    {
        pos = 0;
    }

    if (pos < returnString.length())
    {
        returnString.erase(pos);
    }

    return returnString;    
}

bool IsRootPath(_In_ const wstring& path)
{

    WCHAR volumePath[MAX_PATH];
    bool isRootPath = false;
    
    if (GetVolumePathName(path.c_str(), volumePath, MAX_PATH))
    {
        // For volume root, input length will equal output length (ignoring 
        // trailing backslash if any)

        size_t cchPath = wcslen(path.c_str());
        size_t cchRoot = wcslen(volumePath);
        
        if (volumePath[cchRoot - 1] == L'\\')
            cchRoot--;
        if (path.c_str()[cchPath - 1] == L'\\')
            cchPath--;

        if (cchPath == cchRoot)
        {
           isRootPath = true;
        }
    }

    return isRootPath;
}

void PrintUsage(_In_ LPCWSTR programName)
{
    wcout << L"BACKUP" << endl;
    wcout << L"Backup a directory to a destination directory:" << endl <<
             L"\t" << programName << L" -backup <directory-or-volume-path> -destination <directory-path>" << endl << endl;
    wcout << L"EXAMPLE: " << programName << L" -backup d:\\mydirectory -destination f:\\mydirectorybackup" << endl;
    wcout << L"EXAMPLE: " << programName << L" -backup d:\\ -destination f:\\mydirectorybackup" << endl;

    wcout << endl << L"RESTORE" << endl;
    wcout << L"Restore a backed up directory to a destination directory:" << endl <<
             L"\t" << programName << L" -restore <backup-directory-path> -destination <directory-path>" << endl << endl;
    wcout << L"EXAMPLE: " << programName << L" -restore f:\\mydirectorybackup -destination d:\\mydirectory" << endl;

    wcout << endl << L"SINGLE FILE RESTORE" << endl;
    wcout << L"Restore the reparse point to the destination:" << endl <<
             L"\t" << programName << L" -restorestub  <backup-file-path> -destination <file-path>" << endl << endl;
    wcout << L"Restore the data for the reparse point restored with the -restorestub option above:" << endl <<
             L"\t" << programName << L" -restoredata  <backup-file-path> -destination <stub-path>" << endl << endl;
    wcout << L"Restore the reparse point and data in one operation (-restorestub + -restoredata):" << endl <<
             L"\t" << programName << L" -restorefile <backup-file-path> -destination <file-path>" << endl << endl;
    wcout << L"EXAMPLE: " << programName << L" -restorefile f:\\mydirectorybackup\\myfile -destination d:\\temp\\myfile" << endl;

    wcout << endl << L"VOLUME RESTORE" << endl;
    wcout << L"Restore the entire volume to the destination:" << endl <<
             L"\t" << programName << L" -restorevolume <backup-directory-path> -destination <volume-path>" << endl << endl;
    wcout << L"EXAMPLE: " << programName << L" -restorevolume f:\\mydirectorybackup -destination d:\\" << endl;
}

bool ParseCommandLine(_In_ int argc,  _In_reads_(argc) _TCHAR* argv[], _Out_ Action *action, _Out_ wstring* source, _Out_ wstring* destination)
{

    if (action == NULL || source == NULL || destination == NULL)
    {
        return false;
    }

    *action = BackupAction;
    *source = L"";
    *destination = L"";

    if (argc < 5) return false;
    // argv[0] is the program name, skip it

    // argv[1] is the command, must be one of the following
    wstring actionString = argv[1];
    if (actionString == L"-backup") 
    {
        *action = BackupAction;
    }
    else if (actionString == L"-restorestub") 
    {
        *action = RestoreStubAction;
    }
    else if (actionString == L"-restoredata") 
    {
        *action = RestoreDataAction;
    }
    else if (actionString == L"-restorefile") 
    {
        *action = RestoreFileAction;
    }
    else if (actionString == L"-restorevolume") 
    {
        *action = RestoreVolumeAction;
    }
    else if (actionString == L"-restore") 
    {
        *action = RestoreFilesAction;
    }
    else
    {
        return false;
    }

    // argv[2] is the source
    *source = argv[2];
    // argv[4] is the destination
    *destination = argv[4];

    // argv[3] is always "-destination", check it for completeness
    wstring arg3 = argv[3];
    if (arg3 != L"-destination") return false;
    
    return true;
}

HRESULT ModifyPrivilege(_In_ LPCTSTR szPrivilege, _In_ BOOL fEnable)
{
    HRESULT hr = S_OK;
    TOKEN_PRIVILEGES NewState = {};
    LUID luid = {};
    HANDLE hToken = NULL;

    // Open the process token for this process.
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hToken ))
    {
        wcout << L"Failed OpenProcessToken" << endl;
        return ERROR_FUNCTION_FAILED;
    }

    // Get the local unique ID for the privilege.
    if ( !LookupPrivilegeValue( NULL,
                                szPrivilege,
                                &luid ))
    {
        CloseHandle( hToken );
        wcout << L"Failed LookupPrivilegeValue" << endl;
        return ERROR_FUNCTION_FAILED;
    }

    // Assign values to the TOKEN_PRIVILEGE structure.
    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = luid;
    NewState.Privileges[0].Attributes = 
              (fEnable ? SE_PRIVILEGE_ENABLED : 0);

    // Adjust the token privilege.
    if (!AdjustTokenPrivileges(hToken,
                               FALSE,
                               &NewState,
                               0,
                               NULL,
                               NULL))
    {
        wcout << L"Failed AdjustTokenPrivileges" << endl;
        hr = ERROR_FUNCTION_FAILED;
    }

    // Close the handle.
    CloseHandle(hToken);

    return hr;
}

HRESULT GetVolumeGuidNameForPath(_In_ const wstring& path, _Out_ wstring& volumeGuidName)
{
    HRESULT hr = S_OK;
    
    WCHAR volumePathName[MAX_PATH];
    BOOL result = GetVolumePathName(path.c_str(), volumePathName, MAX_PATH);
    if (!result)
    {
        wcout << L"GetVolumePathName(" << path << L") failed with error " << GetLastError() << endl;
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        WCHAR tempVolumeGuidName[MAX_PATH];
        result = GetVolumeNameForVolumeMountPoint(volumePathName, tempVolumeGuidName, MAX_PATH);
        if (!result)
        {
            wcout << L"GetVolumeNameForVolumePathName(" << volumePathName << L") failed with error " << GetLastError() << endl;
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        volumeGuidName = tempVolumeGuidName;
    }
    
    return hr;
}

// Required by RPC
_Must_inspect_result_
_Ret_maybenull_ _Post_writable_byte_size_(size)
void  * __RPC_USER MIDL_user_allocate(size_t size)
{    
    return LocalAlloc(0, size);
}

HRESULT
GetFileSize(
    __in const std::wstring& FilePath,
    __out LARGE_INTEGER& FileSize
    )
{
    HRESULT hr = S_OK;
    
    HANDLE fileHandle =
        ::CreateFile(
            FilePath.c_str(),
            FILE_READ_ATTRIBUTES,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
            NULL);

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        
        wcout << L"CreateFile(%s" << FilePath << L") failed with error " << GetLastError() << endl;
    }
    else
    {
        if (!::GetFileSizeEx(fileHandle, &FileSize))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            wcout << L"GetFileSizeEx(%s" << FilePath << L") failed with error " << GetLastError() << endl;
        }
    }

    return hr;
}



