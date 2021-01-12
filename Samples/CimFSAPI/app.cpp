/*
Copyright (c) Microsoft Corporation

This file contains a sample app showing basic manipulations of a CIM Image.
Topics to cover:
    - Create a new CIM image
    - Commit changes to the image
    - Add a file from the local filesystem to the image
    - Mount and validate image contents
    - Add a Hardlink to an existing file in the image
    - Fork from the base image
    - Delete a file from the forked CIM image
*/

#include "cimfs.h"
#include <aclapi.h>
#include <cstddef>
#include <exception>
#include <iostream>
#include <rpcdce.h>
#include <string>
#include <string_view>
#include <vector>
#include <wil/resource.h>
#include <wil/result.h>
#include <windows.h>
#include <winioctl.h>

// Keep information of a file's alternate streams
struct StreamData
{
    std::wstring Name;
    LONGLONG Size;
};

struct AlternateDataStreams
{
    std::vector<StreamData> StreamData;
};

struct CimFileData
{
    // The handle used to get the information of the file
    wil::unique_hfile FileHandle;

    // While getting the data we also need to keep the sd alive
    wil::unique_hlocal_security_descriptor Sd;

    // A copy from filesystem's attributes and metadata that Cimfs
    // needs when creating a file in the image
    CIMFS_FILE_METADATA MetaData;

    // Alternate streams information
    std::vector<StreamData> StreamData;

    FILE_ID_INFO FileIdInfo;

    // buffer for reparse point data
    std::vector<byte> ReparsePointData;
};

struct MountedCimInformation
{
    // Guid used to mount the volume
    GUID VolumeId;

    // In the form of \\?\Volume{VolumeId}\ 
    std::wstring VolumeRootPath;
};

// RAII adapter for a CIM image handle
using unique_cimfs_image_handle =
    wil::unique_any<CIMFS_IMAGE_HANDLE, decltype(&CimCloseImage), &CimCloseImage>;

// RAII adapter for a CIM stream handle
using unique_cimfs_stream_handle =
    wil::unique_any<CIMFS_STREAM_HANDLE, decltype(&CimCloseStream), &CimCloseStream>;

void
CopyFileContentsToCim(_In_ CIMFS_STREAM_HANDLE streamHandle, _In_ HANDLE file)
//
// Routine Description:
//  Copies the contents of an open file to a Writer.
//
// Parameters:
//  streamHandle - Cim image writer handle, obtained by a call to CimAddFile.
//
//  file - Opened handle to the file to copy data from.
//
{
    std::wcout << "Copying data from file / stream" << std::endl;

    std::vector<byte> buffer(65536);

    for (;;)
    {
        DWORD read;

        if (!ReadFile(file, buffer.data(), static_cast<DWORD>(buffer.size()), &read, nullptr))
        {
            THROW_LAST_ERROR();
        }

        if (read == 0)
        {
            break;
        }

        std::wcout << "\tRead " << read << " bytes, writing in image's stream ..." << std::endl;

        THROW_IF_FAILED(CimWriteStream(streamHandle, buffer.data(), read));
    }
}

AlternateDataStreams
GetAlternateDataStreams(_In_ const std::wstring& filePath)
//
// Routine Description:
//  Gets the list of alternate data streams for an open handle.
//
// Parameters:
//  filePath - Path to the file to get the data from.
//
{
    AlternateDataStreams streams{};
    WIN32_FIND_STREAM_DATA findData{};

    wil::unique_hfind findStream{
        FindFirstStreamW(filePath.c_str(), FindStreamInfoStandard, &findData, 0)};

    if (findStream)
    {
        do
        {
            StreamData data{};
            data.Name = std::wstring(findData.cStreamName);
            data.Size = findData.StreamSize.QuadPart;

            // Skip the default data stream
            if (data.Name != L"::$DATA")
            {
                streams.StreamData.push_back(data);
            }

        } while (FindNextStreamW(findStream.get(), &findData));
    }
    else
    {
        if (GetLastError() != ERROR_HANDLE_EOF)
        {
            THROW_LAST_ERROR();
        }
    }

    return std::move(streams);
}

CimFileData
GetFileData(_In_ const std::wstring& filePath)
//
// Routine Description:
//  Gets the meta data, attributes and stream information from
//  an existing file that will be copied to a Cim image.
//
// Parameters:
//  filePath - Path to the file to get the data from.
//
//  fileData - Structure that will hold the required attributes and stream info.
//
{
    CimFileData fileData{};

    std::wcout << "Getting data for file: " << filePath.c_str() << std::endl;

    // Open the file, ensure in case of a symlink we open the symlink itself
    wil::unique_hfile file{CreateFileW(filePath.c_str(),
                                       GENERIC_READ | ACCESS_SYSTEM_SECURITY,
                                       FILE_SHARE_READ,
                                       nullptr,
                                       OPEN_EXISTING,
                                       FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
                                       nullptr)};

    THROW_LAST_ERROR_IF(!file);

    // Get the in formation we need from the source file that is required when
    // adding a file to the image

    FILE_ID_INFO fileID{};

    THROW_LAST_ERROR_IF(
        !GetFileInformationByHandleEx(file.get(), FileIdInfo, &fileID, sizeof(fileID)));

    FILE_BASIC_INFO basicInfo;

    THROW_LAST_ERROR_IF(
        !GetFileInformationByHandleEx(file.get(), FileBasicInfo, &basicInfo, sizeof(basicInfo)));

    fileData.MetaData.Attributes = basicInfo.FileAttributes;
    fileData.MetaData.CreationTime = basicInfo.CreationTime;
    fileData.MetaData.LastWriteTime = basicInfo.LastWriteTime;
    fileData.MetaData.ChangeTime = basicInfo.ChangeTime;
    fileData.MetaData.LastAccessTime = basicInfo.LastAccessTime;

    fileData.FileIdInfo = fileID;

    if (basicInfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
    {
        std::wcout << "\t\tFile is a reparse point, getting reparse info" << std::endl;

        std::vector<byte> reparseBuffer;
        reparseBuffer.resize(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
        DWORD bytes;

        THROW_LAST_ERROR_IF(!DeviceIoControl(file.get(),
                                             FSCTL_GET_REPARSE_POINT,
                                             nullptr,
                                             0,
                                             reparseBuffer.data(),
                                             static_cast<DWORD>(reparseBuffer.size()),
                                             &bytes,
                                             nullptr));

        fileData.ReparsePointData = std::move(reparseBuffer);
        fileData.MetaData.ReparseDataBuffer = fileData.ReparsePointData.data();
        fileData.MetaData.ReparseDataSize = bytes;
    }

    if (basicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        fileData.MetaData.FileSize = 0;
    }
    else
    {
        LARGE_INTEGER fileSize{};

        THROW_LAST_ERROR_IF(!GetFileSizeEx(file.get(), &fileSize));
        fileData.MetaData.FileSize = fileSize.QuadPart;
    }

    wil::unique_hlocal_security_descriptor sd;

    // Retrieve the security descriptor.
    auto secInfo = DACL_SECURITY_INFORMATION | LABEL_SECURITY_INFORMATION |
                   GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION |
                   SACL_SECURITY_INFORMATION;

    DWORD error = GetSecurityInfo(file.get(),
                                  SE_FILE_OBJECT,
                                  secInfo,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  &sd);

    if (error != ERROR_SUCCESS)
    {
        THROW_WIN32(error);
    }

    fileData.MetaData.SecurityDescriptorBuffer = sd.get();
    fileData.MetaData.SecurityDescriptorSize = GetSecurityDescriptorLength(sd.get());
    fileData.Sd.reset(sd.release());

    // Retrieve Alternate streams info
    fileData.StreamData = std::move(GetAlternateDataStreams(filePath).StreamData);

    // Finally keep the handle used to retrieve the information
    fileData.FileHandle.reset(file.release());

    return std::move(fileData);
}

void
WriteFileEntry(_In_ CIMFS_IMAGE_HANDLE cimHandle,
               _In_ const std::wstring& filePath,
               _In_ const std::wstring& imageRelativePath,
               _Out_ ULONG& fileAttributes)
//
// Routine Description:
//  Retrieves the information of an existing file and writes it into
//  the Cim Image.
//
// Parameters:
//  cimHandle - Opened handle to the Cim Image by calling CimCreateImage.
//
//  filePath - Source Path in the local filesystem to copy the data from.
//
//  imageRelativePath - Destination path in the CIM Image
//
//  fileAttributes - Attributes of the copied file
//
{

    unique_cimfs_stream_handle streamHandle = nullptr;
    CimFileData cimFileData;

    std::wcout << "Adding file:" << filePath.c_str() << " as " << imageRelativePath.c_str()
               << " in image" << std::endl;

    // Get the information we need from the source file
    cimFileData = GetFileData(filePath);

    THROW_IF_FAILED(
        CimCreateFile(cimHandle, imageRelativePath.c_str(), &cimFileData.MetaData, &streamHandle));

    // Write the payload data.
    if (cimFileData.MetaData.FileSize > 0)
    {
        CopyFileContentsToCim(streamHandle.get(), cimFileData.FileHandle.get());
    }

    CimCloseStream(streamHandle.release());

    // Write alternate data streams.

    if (cimFileData.StreamData.size() > 0)
    {

        for (const auto& streamData : cimFileData.StreamData)
        {

            // stream.Name is of the format ":name:$TYPE"
            // no need to check for $DATA as we skip it when
            // collecting data for alternate data streams
            auto end = streamData.Name.find(':', 1);
            if (end != std::wstring::npos)
            {
                auto streamName = filePath + streamData.Name.substr(0, end);

                wil::unique_hfile stream(CreateFileW(streamName.c_str(),
                                                     GENERIC_READ,
                                                     FILE_SHARE_READ,
                                                     nullptr,
                                                     OPEN_EXISTING,
                                                     0,
                                                     nullptr));

                THROW_LAST_ERROR_IF(!stream);


                unique_cimfs_stream_handle alternateStreamHandle;

                // This time we're not creating a new file
                // but adding a stream
                THROW_IF_FAILED(CimCreateAlternateStream(cimHandle,
                                                         streamName.c_str(),
                                                         streamData.Size,
                                                         &alternateStreamHandle));

                if (streamData.Size > 0)
                {
                    CopyFileContentsToCim(alternateStreamHandle.get(), stream.get());
                }
            }
        }
    }

    fileAttributes = cimFileData.MetaData.Attributes;
}

void
AddFileToNewCim(_In_ const std::wstring& cimPath,
                _In_ const std::wstring& imageName,
                _In_ const std::wstring& filePath,
                _In_ const std::wstring& imageRelativePath)
//
// Routine Description:
//  Copies the content of an existing file into a new Cim.
//
// Parameters:
//  cimPath - Path to a directory to contain the CIM image.
//            For example: C:\MyCimImage
//
//  imageName - Name of the image. The image should not already exist
//  cimPath.
//                For example: image0.cim
//
//  filePath - Source Path in the local filesystem to copy the data from.
//             For example: C:\dir\file1.txt
//
//  imageRelativePath - Destination path relative to the root in the CIM Image.
//             For example: dir\file1.txt
//
{
    unique_cimfs_image_handle imageHandle = nullptr;
    ULONG fileAttributes;

    std::wcout << "Creating new image " << imageName << " in directory " << cimPath << std::endl;

    auto hr = CimCreateImage(cimPath.c_str(), nullptr, imageName.c_str(), &imageHandle);

    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_EXISTS))
    {
        std::wcout << "ERROR: image " << imageName << " already exists in directory " << cimPath << std::endl;
    }

    THROW_IF_FAILED(hr);

    WriteFileEntry(imageHandle.get(), filePath, imageRelativePath, fileAttributes);

    if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        std::wcout << "Directory " << filePath << " added as an empty directory in CIM Image as ";
        std::wcout << imageRelativePath << "(add files separately if desired)" << std::endl;
    }

    // Commit changes to a new CIM
    THROW_IF_FAILED(CimCommitImage(imageHandle.get()));

    return;
}

void
AddHardLinkInCim(_In_ const std::wstring& cimPath,
                 _In_ const std::wstring& imageName,
                 _In_ const std::wstring& existingImageRelativePath,
                 _In_ const std::wstring& imageRelativePath)
//
// Routine Description:
//  Adds a Hard Link entry in the CIM image to an existing file in the Cim Image.
//
// Parameters:
//  cimPath - Path to a CIM image directory.
//
//  imageName - Name of the existing image the file will be added to.
//
//  existingImageRelativePath - An existing Path in the CIM image that the hardlink will point to.
//
//  imageRelativePath - Name of the hardlink
//
{
    unique_cimfs_image_handle imageHandle = nullptr;

    std::wcout << "Opening image " << imageName << " in directory " << cimPath
               << " to add a hardlink " << std::endl;

    // extend the parent CIM
    THROW_IF_FAILED(CimCreateImage(cimPath.c_str(),
                                   imageName.c_str(),
                                   imageName.c_str(),
                                   &imageHandle));

    std::wcout << "Adding HardLink" << imageRelativePath.c_str() << " -> " << existingImageRelativePath.c_str();
    std::wcout << std::endl;

    HRESULT hr =  CimCreateHardLink(imageHandle.get(), imageRelativePath.c_str(), existingImageRelativePath.c_str());

    if (FAILED(hr))
    {
        // Can't create a hardlink to a directory
        if (hr == E_ACCESSDENIED)
        {
            std::wcout << "Error, can't add hardlink to a directory" << std::endl;
        }
        else
        {
            THROW_IF_FAILED(hr);
        }
    }
    else
    {
        THROW_IF_FAILED(CimCommitImage(imageHandle.get()));
    }

    return;
}

void
DeletePathFromCimFork(_In_ const std::wstring& cimPath,
                      _In_ const std::wstring& imageName,
                      _In_ const std::wstring& imageRelativePath,
                      _In_ const std::wstring& forkedImageName)
//
// Routine Description:
//  Deletes a file or directory from and existing image creating a fork of the image
//
// Parameters:
//  cimPath - Path to a CIM image directory.
//
//  imageName - Name of the existing image the file will be deleted from.
//
//  imageRelativePath - An existing path in the CIM image that will be deleted.
//
//  forkedImageName - Name of the image fork to be created.
//
{
    unique_cimfs_image_handle imageHandle = nullptr;

    std::wcout << "Opening image " << imageName << " in directory " << cimPath
               << " to delete a file " << std::endl;


    auto hr = CimCreateImage(cimPath.c_str(), imageName.c_str(), forkedImageName.c_str(), &imageHandle);

    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_EXISTS))
    {
        std::wcout << "ERROR: image " << forkedImageName << " already exists in directory " << cimPath << std::endl;
    }

    std::wcout << "Deleting " << imageRelativePath.c_str() << " from image" << std::endl;

    THROW_IF_FAILED(CimDeletePath(imageHandle.get(), imageRelativePath.c_str()));

    // Fork the parent CIM by specifying a new name in the commit

    THROW_IF_FAILED(CimCommitImage(imageHandle.get()));

    return;
}

MountedCimInformation
MountImage(_In_ const std::wstring& cimPath, _In_ const std::wstring& imageName)
//
// Routine Description:
//  Mounts an existing Cim image and returns the path the image was mounted to.
//
// Parameters:
//  cimPath - Path to a CIM image, the image has been created already.
//
//  imageName - Name of the image.
//
{
    MountedCimInformation volumeInfo{};

    GUID uuid;
    wil::unique_rpc_wstr uuidString;

    UuidCreate(&uuid);
    UuidToStringW(&uuid, &uuidString);
    THROW_IF_NULL_ALLOC(uuidString);

    auto uuidWstring = reinterpret_cast<wchar_t*>( uuidString.get() );

    THROW_IF_FAILED(CimMountImage(cimPath.c_str(), imageName.c_str(), CIM_MOUNT_IMAGE_NONE, &uuid));

    volumeInfo.VolumeId = uuid;
    volumeInfo.VolumeRootPath = std::wstring(L"\\\\?\\Volume{") + uuidWstring + L"}\\";

    return std::move(volumeInfo);
}

void
DismountImage(_In_ const GUID& volumeId)
{
    THROW_IF_FAILED(CimDismountImage(&volumeId));
}

bool
CompareStreams(_In_ const std::wstring& source, _In_ const std::wstring& target)
//
// Routine Description:
//  A basic routine that compares the contents of 2 streams.
//  This routine assumes both paths are normal files (no directories or reparse points)
//
// Parameters:
//  source - Path to a source file.
//
//  target - Path to a target file.
//
// Returns:
//  bool, true if both streams have the same content.
//
{
    std::vector<byte> sourceBuffer(4096);
    std::vector<byte> targetBuffer(4096);

    wil::unique_hfile sourceHandle(CreateFileW(source.c_str(),
                                               GENERIC_READ,
                                               FILE_SHARE_READ,
                                               nullptr,
                                               OPEN_EXISTING,
                                               FILE_FLAG_BACKUP_SEMANTICS,
                                               nullptr));

    THROW_LAST_ERROR_IF(!sourceHandle);

    wil::unique_hfile targetHandle(CreateFileW(target.c_str(),
                                               GENERIC_READ,
                                               FILE_SHARE_READ,
                                               nullptr,
                                               OPEN_EXISTING,
                                               FILE_FLAG_BACKUP_SEMANTICS,
                                               nullptr));

    THROW_LAST_ERROR_IF(!targetHandle);

    for (;;)
    {
        DWORD sourceRead, targetRead;

        if (!ReadFile(sourceHandle.get(),
                      sourceBuffer.data(),
                      static_cast<DWORD>(sourceBuffer.size()),
                      &sourceRead,
                      nullptr))
        {
            THROW_LAST_ERROR();
        }

        if (!ReadFile(targetHandle.get(),
                      targetBuffer.data(),
                      static_cast<DWORD>(targetBuffer.size()),
                      &targetRead,
                      nullptr))
        {
            THROW_LAST_ERROR();
        }

        if (targetRead != sourceRead)
        {
            return false;
        }

        if (sourceRead == 0)
        {
            break;
        }

        if (memcmp(sourceBuffer.data(), targetBuffer.data(), sourceRead) != 0)
        {
            std::cerr << "\tContents do not match" << std::endl;
            return false;
        }
    }

    return true;
}

void
CompareFileWithCimFile(_In_ const std::wstring& cimPath,
                       _In_ const std::wstring& imageName,
                       _In_ const std::wstring& imageRelativePath,
                       _In_ const std::wstring& filePath)
//
// Routine Description:
//  Compares the content of a file in the local filesystem against a
//  file in a target CIM image as well as some basic attributes.
//
// Parameters:
//  cimPath - Path to a CIM image directory.
//
//  imageName - Name of the image that will be mounted and used to compare.
//
//  imageRelativePath - Destination path in the CIM Image to compare
//
//  filePath - Source Path in the local filesystem to compare.
//
{
    std::wstring cimFilePath;
    MountedCimInformation volumeInfo;

    volumeInfo = MountImage(cimPath, imageName);
    auto cleanup = wil::scope_exit([&] { DismountImage(volumeInfo.VolumeId); });

    cimFilePath = volumeInfo.VolumeRootPath + imageRelativePath;

    CimFileData fileData{GetFileData(filePath)};
    CimFileData cimFileData{GetFileData(cimFilePath)};

    // start by comparing attributes
    if (fileData.MetaData.Attributes != cimFileData.MetaData.Attributes)
    {
        std::cerr << "Attributes do not match" << std::endl;
    }

    // if file is a reparse point compare reparse buffer
    if (fileData.MetaData.Attributes & FILE_ATTRIBUTE_REPARSE_POINT)
    {
        if (fileData.MetaData.ReparseDataSize != cimFileData.MetaData.ReparseDataSize)
        {
            std::cerr << "\tReparse buffer sizes do not match" << std::endl;
        }

        if (memcmp(fileData.MetaData.ReparseDataBuffer,
                   cimFileData.MetaData.ReparseDataBuffer,
                   fileData.MetaData.ReparseDataSize) != 0)
        {
            std::cerr << "\tReparse buffer contents do not match" << std::endl;
        }
    }

    // if file is not a directory compare file contents
    if (!(fileData.MetaData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        if (fileData.MetaData.FileSize != cimFileData.MetaData.FileSize)
        {
            std::cerr << "File sizes do not match" << std::endl;
        }

        if (fileData.MetaData.CreationTime.QuadPart != cimFileData.MetaData.CreationTime.QuadPart)
        {
            std::cerr << "Creation times do not match" << std::endl;
        }

        // Compare file contents

        if (!CompareStreams(filePath, cimFilePath))
        {
            std::cerr << "Content does not match" << std::endl;
        }
    }
}

void
ValidateHardLinkInCim(_In_ const std::wstring& cimPath,
                      _In_ const std::wstring& imageName,
                      _In_ const std::wstring& existingImagePath,
                      _In_ const std::wstring& imageLinkPath)
//
// Routine Description:
//  Validates if linkPath is a hardlink of filePath.
//
// Parameters:
//  cimPath - Path to a CIM image, the image has been created already.
//
//  imageName - Name of the image that will be mounted and used to compare.
//
//  existingImagePath - An existing Path in the CIM image that the hardlink poins to.
//
//  imageLinkPath - An existing Path in the im age previously added via CimAddLink
//
{
    std::wstring cimFilePath, cimLinkPath;
    MountedCimInformation volumeInfo;

    volumeInfo = MountImage(cimPath, imageName);
    auto cleanup = wil::scope_exit([&] { DismountImage(volumeInfo.VolumeId); });

    cimFilePath = volumeInfo.VolumeRootPath + existingImagePath;
    cimLinkPath = volumeInfo.VolumeRootPath + imageLinkPath;

    {
        CimFileData cimFileData{GetFileData(cimFilePath)};
        CimFileData cimLinkData{GetFileData(cimLinkPath)};

        if (memcmp(cimFileData.FileIdInfo.FileId.Identifier,
                   cimLinkData.FileIdInfo.FileId.Identifier,
                   sizeof(FILE_ID_128)) != 0)
        {
            std::cerr << "did not match" << std::endl;
        }
    }
}

bool
TestFileExistsInCim(_In_ const std::wstring& cimPath,
                    _In_ const std::wstring& imageName,
                    _In_ const std::wstring& imageRelativePath)
// Routine Description:
//  Checks if the file exists or not in the provided image.
//
// Parameters:
//  cimPath - Path to a CIM image.
//
//  imageName - Name of the image that will be mounted and used to check
//
//  imageRelativePath - Relative path from the root in the CIM image
//
{
    MountedCimInformation volumeInfo;

    volumeInfo = MountImage(cimPath, imageName);
    auto cleanup = wil::scope_exit([&] { DismountImage(volumeInfo.VolumeId); });

    std::wstring cimImageFilePath = volumeInfo.VolumeRootPath + imageRelativePath;
    ULONG attributes = GetFileAttributes(cimImageFilePath.c_str());

    // fail for any reason other than is the file was not found
    THROW_LAST_ERROR_IF(attributes == INVALID_FILE_ATTRIBUTES && GetLastError() != ERROR_FILE_NOT_FOUND);

    return (attributes != INVALID_FILE_ATTRIBUTES);
}

bool
TogglePrivilege(std::wstring_view privilegeName, bool enable)
//
// Routine Description:
//  Attempts to enable or disable a given privilege. Returns the previous state for the privilege.
//
{
    wil::unique_handle token;
    THROW_IF_WIN32_BOOL_FALSE(
        OpenProcessToken(GetCurrentProcess(), (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY), &token));

    struct
    {
        union {
            TOKEN_PRIVILEGES TokenPrivileges;
            struct
            {
                DWORD PrivilegeCount;
                LUID_AND_ATTRIBUTES Privileges[1];
            } TokenStruct;
        } TokenUnion;
    } oneTokenPrivilege;

    THROW_IF_WIN32_BOOL_FALSE(
        LookupPrivilegeValue(nullptr,
                             std::wstring{privilegeName}.c_str(),
                             &oneTokenPrivilege.TokenUnion.TokenStruct.Privileges[0].Luid));
    oneTokenPrivilege.TokenUnion.TokenStruct.PrivilegeCount = 1;
    oneTokenPrivilege.TokenUnion.TokenStruct.Privileges[0].Attributes =
        (enable ? SE_PRIVILEGE_ENABLED : 0);

    DWORD cb;
    THROW_IF_WIN32_BOOL_FALSE(
        AdjustTokenPrivileges(token.get(),
                              false,
                              &oneTokenPrivilege.TokenUnion.TokenPrivileges,
                              sizeof(oneTokenPrivilege.TokenUnion.TokenPrivileges),
                              &oneTokenPrivilege.TokenUnion.TokenPrivileges,
                              &cb));


    bool previouslyEnabled = ((cb == sizeof(oneTokenPrivilege.TokenUnion.TokenPrivileges)) &&
                              (oneTokenPrivilege.TokenUnion.TokenStruct.PrivilegeCount == 1) &&
                              (oneTokenPrivilege.TokenUnion.TokenStruct.Privileges[0].Attributes &
                               (SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT)));

    return previouslyEnabled;
}

int __cdecl wmain(int argc, const wchar_t** argv) try
{

    if (argc != 5)
    {
        std::wcerr << "Usage:" << argv[0]
                   << " <cim_path> <image_name> <file_to_add_path> <image_file_path>" << std::endl;
        exit(1);
    }

    TogglePrivilege(SE_SECURITY_NAME, true);
    TogglePrivilege(SE_BACKUP_NAME, true);

    std::wstring cimPath(argv[1]), imageName(argv[2]), filePath(argv[3]), imageRelativePath(argv[4]);

    //  Create a new image and add a file
    AddFileToNewCim(cimPath, imageName, filePath, imageRelativePath);

    CompareFileWithCimFile(cimPath, imageName, imageRelativePath, filePath);

    auto attributes = GetFileAttributes(filePath.c_str());
    std::wstring imageHardLinkPath(L"link");

    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        //  Extend the existing image by adding a hardlink
        AddHardLinkInCim(cimPath, imageName, imageRelativePath, imageHardLinkPath);

        ValidateHardLinkInCim(cimPath, imageName, imageRelativePath, imageHardLinkPath);

        CompareFileWithCimFile(cimPath, imageName, imageHardLinkPath, filePath);
    }

    std::wstring forkImageName = imageName + L"_fork";

    //  Create a fork of the image where the path has been deleted
    DeletePathFromCimFork(cimPath, imageName, imageRelativePath, forkImageName);

    if (TestFileExistsInCim(cimPath, forkImageName, imageRelativePath))
    {
        std::wcerr << "The file " << imageRelativePath << " should have been removed in image "
                  << forkImageName << std::endl;
    }

    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        // check the link still exists in the base image
        if (!TestFileExistsInCim(cimPath, imageName, imageHardLinkPath))
        {
            std::wcerr << "The file " << imageHardLinkPath << " should not have been removed in image "
                    << imageName << std::endl;
        }
    }
}
CATCH_RETURN()
