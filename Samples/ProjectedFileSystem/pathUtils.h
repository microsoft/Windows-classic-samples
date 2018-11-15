/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    pathUtils.h

Abstract:

    Utility class for file system path operations.

--*/

#pragma once

namespace regfs {

class PathUtils {

public:

    // Returns true if the given path is for the virtualization root.  The path must be expressed
    // relative to the virtualization root
    static bool IsVirtualizationRoot(const PCWSTR filePathName)
    {
        if ((filePathName == nullptr) ||
            (*filePathName == 0) ||
            (0 == wcscmp(filePathName, L"\\")))
        {
            return true;
        }

        return false;
    };

    // Returns the last component and the parent path for the given path.
    // Example:
    // 
    //      std::wstring parentPath;
    //      std::wstring fileName;
    //      fileName = GetLastComponent(L"foo\bar\a.txt", parentPath);
    //  
    // Result:
    //
    //      parentPath: "foo\bar"
    //      fileName:   "a.txt"
    static std::wstring GetLastComponent(const std::wstring& path, std::wstring& parentPath)
    {
        std::wstring fileName;

        const size_t last_slash_idx = path.find_last_of(L"\\");
        if (std::wstring::npos != last_slash_idx)
        {
            parentPath = path.substr(0, last_slash_idx);
            fileName = path.substr(last_slash_idx + 1);
        }
        else
        {
            parentPath = L"\\";
            fileName = path;
        }

        return fileName;
    }

    // Combines two paths and returns a properly formatted path.
    // Example:
    //
    //      std::wstring combinedPath = CombinePath(L"foo", L"bar");
    //
    // Result:
    // 
    //      combinedPath: "foo\bar"
    static std::wstring CombinePath(const std::wstring& root, const std::wstring& relPath)
    {
        std::wstring fullPath = root;

        if (root.empty() || root == L"\\")
        {
            return relPath;
        }

        if (fullPath.back() == L'\\')
        {
            fullPath.pop_back();
        }

        if (!relPath.empty())
        {
            fullPath += L'\\';
            fullPath += relPath;
        }

        if (fullPath.back() == L'\\')
        {
            fullPath.pop_back();
        }

        return fullPath;
    }
};

}