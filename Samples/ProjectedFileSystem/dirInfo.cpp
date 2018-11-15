#include "stdafx.h"

using namespace regfs;

//////////////////////////////////////////////////////////////////////////
// See dirInfo.h for descriptions of the routines in this module.
//////////////////////////////////////////////////////////////////////////

// A comparison routine for std::sort that wraps PrjFileNameCompare() so that we can sort our DirInfo
// the same way the file system would.
bool FileNameLessThan(DirEntry entry1, DirEntry entry2)
{
    return PrjFileNameCompare(entry1.FileName.c_str(), entry2.FileName.c_str()) < 0;
}

DirInfo::DirInfo(PCWSTR FilePathName) :
    _filePathName(FilePathName),
    _currIndex(0),
    _entriesFilled(false)
{}

void DirInfo::Reset()
{
    _currIndex = 0;
    _entriesFilled = false;
    _entries.clear();
}

bool DirInfo::EntriesFilled()
{
    return _entriesFilled;
}

bool DirInfo::CurrentIsValid()
{
    return _currIndex < _entries.size();
}

PRJ_FILE_BASIC_INFO DirInfo::CurrentBasicInfo()
{
    PRJ_FILE_BASIC_INFO basicInfo = { 0 };
    basicInfo.IsDirectory = _entries[_currIndex].IsDirectory;
    basicInfo.FileSize = _entries[_currIndex].FileSize;

    return basicInfo;
}

PCWSTR DirInfo::CurrentFileName()
{
    return _entries[_currIndex].FileName.c_str();
}

bool DirInfo::MoveNext()
{
    _currIndex++;

    if (_currIndex >= _entries.size())
    {
        return false;
    }
    return true;
}

void DirInfo::FillDirEntry(LPCWSTR DirName)
{
    FillItemEntry(DirName, 0, true);
}

void DirInfo::FillFileEntry(LPCWSTR FileName, INT64 FileSize)
{
    FillItemEntry(FileName, FileSize, false);
}

void DirInfo::FillItemEntry(LPCWSTR FileName, INT64 FileSize, bool IsDirectory)
{
    DirEntry entry;
    auto nameLen = wcslen(FileName);
    if (nameLen > MAX_PATH)
    {
        return;
    }

    entry.FileName = FileName;
    entry.IsDirectory = IsDirectory;
    entry.FileSize = FileSize;

    _entries.push_back(entry);
}

void DirInfo::SortEntriesAndMarkFilled()
{
    _entriesFilled = true;

    std::sort(_entries.begin(),
              _entries.end(),
              FileNameLessThan);
}