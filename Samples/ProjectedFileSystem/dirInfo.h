/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    dirInfo.h

Abstract:

    A helper class that manages the context data for a directory enumeration session.

--*/

#pragma once

namespace regfs {

// This holds the information the RegFS provider will return for a single directory entry.
//
// Note that RegFS does not supply any timestamps.  This is because the only timestamp the registry
// maintains is the last write time for a key.  It does not maintain creation, last-access, or change
// times for keys, and it does not maintain any timestamps at all for values.  When RegFS calls
// PrjFillDirEntryBuffer(), ProjFS sees that the timestamp values are 0 and uses the current time
// instead.
struct DirEntry {
    std::wstring FileName;
    bool IsDirectory;
    INT64 FileSize;
};

// RegFS uses a DirInfo object to hold directory entries.  When RegFS receives enumeration callbacks
// it populates the DirInfo with a vector of DirEntry structs, one for each key and value in the
// registry key being enumerated.
//
// Refer to RegfsProvider::StartDirEnum, RegfsProvider::GetDirEnum, and RegfsProvider::EndDirEnum
// to see how this class is used.
class DirInfo {

public:

    // Constructs a new empty DirInfo, initializing it with the name of the directory it represents.
    DirInfo(PCWSTR FilePathName);

    // Adds a DirEntry to the list using the given name.  The entry gets marked as a directory.
    void FillDirEntry(LPCWSTR DirName);

    // Adds a DirEntry to the list, using the given name and size.  The entry gets marked as a file.
    void FillFileEntry(LPCWSTR FileName, INT64 FileSize);

    // Sorts the entries in the DirInfo object and marks the object as being fully populated.
    void SortEntriesAndMarkFilled();

    // Returns true if the DirInfo object has been populated with entries.
    bool EntriesFilled();

    // Returns true if CurrentBasicInfo() and CurrentFileName() will return valid values. 
    bool CurrentIsValid();

    // Returns a PRJ_FILE_BASIC_INFO populated with the information for the current item.
    PRJ_FILE_BASIC_INFO CurrentBasicInfo();

    // Returns the file name for the current item.
    PCWSTR CurrentFileName();

    // Moves the internal index to the next DirEntry item.  Returns false if there are no more items.
    bool MoveNext();

    // Deletes all the DirEntry items in the DirInfo object.
    void Reset();

private:

    // Adds a DirEntry to the list, using the given name, size.  Allows the caller to specify whether
    // it should be marked as a directory or not.
    void FillItemEntry(LPCWSTR FileName, INT64 FileSize, bool IsDirectory);

    // Stores the name of the directory this DirInfo represents.
    std::wstring _filePathName;

    // The index of the item in _entries that CurrentBasicInfo() and CurrentFileName() will return.
    int _currIndex;

    // Marks whether or not this DirInfo has been filled with entries.
    bool _entriesFilled;

    // The list of entries in the directory this DirInfo represents.
    std::vector<DirEntry> _entries;
};

}