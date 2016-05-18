//////////////////////////////////////////////////////////////////////////
// MultiFileSelect.h: Defines the MultiSelectFileList class.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////


#pragma once

//-----------------------------------------------------------------------------
// MultiSelectFileList class
// 
// Helper class to parse the buffer that is returned by GetOpenFileName 
// when the OFN_ALLOWMULTISELECT and OFN_EXPLORER flags are used.
//
// The template parameter is the size of the buffer. 
//-----------------------------------------------------------------------------

// NOTE: In Windows Vista and later, applications are encouraged to use
// IFileOpenDialog instead of GetOpenFileName.

template <DWORD CCH_BUFFER>
class MultiSelectFileList
{
    TCHAR pBuffer[CCH_BUFFER];
    TCHAR *pPath;   // Points into pBuffer
    TCHAR *pName;   // Points into pBuffer

    size_t cchRemaining;
    size_t cchPath;
    size_t cchName;


    //-------------------------------------------------------------------------
    // _Next()
    // 
    // Advances to the next file name.
    // 
    // If there are no more file names, pName is set to NULL and the method
    // returns FALSE.
    //-------------------------------------------------------------------------

    BOOL _Next()
    {
        TCHAR *pNext = pName + cchName + 1;

        size_t cchBuf = _tcsnlen(pNext, cchRemaining);

        if ((cchBuf == cchRemaining) || (cchBuf == 0))
        {
            pName = NULL;
            cchName = 0;
        
            // The buffer is not NULL-terminated, or
            // there are no file names left in the buffer.
            return FALSE;  
        }

        pName = pNext;
        cchName = cchBuf;

        cchRemaining -= (cchName + 1);

        return TRUE;
    }


public:
    MultiSelectFileList() : pPath(NULL), pName(NULL), cchPath(0), cchName(0), cchRemaining(0)
    {
        // Fill the buffer with a non-NULL value, so that we can detect
        // the double-NULL at the end of the last file name.
        memset(pBuffer, 0xFF, sizeof(pBuffer));

        // Set the buffer to the empty string.
        if (CCH_BUFFER > 0)
        {
            pBuffer[0] = TEXT('\0');
        }
    }

    //-------------------------------------------------------------------------
    // BufferSizeCch
    // 
    // Returns the maximum number of characters in the buffer, including the
    // terminating NULL.
    //-------------------------------------------------------------------------
    const DWORD BufferSizeCch() const { return CCH_BUFFER; }

    //-------------------------------------------------------------------------
    // BufferPtr()
    // 
    // Returns a pointer to the buffer that contains the file name(s). This
    // is the raw buffer used by GetOpenFileName. To parse the buffer, call
    // Next().
    //-------------------------------------------------------------------------
    TCHAR* BufferPtr() { return pBuffer; }


    //-------------------------------------------------------------------------
    // ParseBuffer
    // 
    // Parses the file-name buffer. Call this method after GetOpenFileName
    // returns successfully.
    //-------------------------------------------------------------------------

    HRESULT ParseBuffer()
    {
        size_t cchFileName = _tcsnlen(pBuffer, CCH_BUFFER);

        if (cchFileName == CCH_BUFFER)
        {
            return E_FAIL;  // The buffer is not NULL-terminated.
        }
        else if (cchFileName >= CCH_BUFFER - 1)
        {
            // The buffer is not double-NULL-terminated.
            return E_FAIL; 
        }

        cchRemaining = CCH_BUFFER - (cchFileName + 1);

        if (pBuffer[cchFileName + 1] == TEXT('\0'))
        {
            // Case 1: Single file selection.
            // If the user selects a single file, the buffer contains the full 
            // path name with two trailing NULLs.

            // We handle this case by setting pPath to NULL and pName to
            // the start of the buffer.

            pPath = NULL;
            cchPath = 0;

            pName = pBuffer;
            cchName = cchFileName;
        }
        else
        {
            // Case 2: Multi-file selection.
            // If the user selects two or more files, the buffer contains the
            // path (null-terminated), followed by each file name (null-terminated).

            pPath = pBuffer;
            cchPath = cchFileName;
            pName = pBuffer + cchPath;

            _Next();
        }

        return S_OK;
    }


    //-------------------------------------------------------------------------
    // Next
    // 
    // Gets the next file name in the buffer. The method allocates memory for
    // the file name, and returns a pointer to the allocated string in 
    // ppFileName. The caller must free the memory by calling CoTaskMemFree.
    // The file name includes the full path.
    //
    // If there are no more file names, the method returns S_FALSE.
    //-------------------------------------------------------------------------

    HRESULT Next(TCHAR **ppFileName)
    {
        if (pName == NULL)
        {
            return S_FALSE;
        }
        int bAddTrailingSlash = 0;

        // Calculate the length of the returned string.

        // If the files are in the root directory, the path name includes the 
        // trailing backward slash. Otherwise, we need to add it to the path name.
        if ( (pPath != NULL) && (cchPath > 0) && (pPath[cchPath - 1] != TEXT('\\')) )
        {
            bAddTrailingSlash = 1;
        }

        size_t cch = cchPath + cchName + bAddTrailingSlash + 1;

        // Allocate a buffer for the string.
        TCHAR *psz = (TCHAR*)CoTaskMemAlloc(cch * sizeof(TCHAR));
        if (psz == NULL)
        {
            return E_OUTOFMEMORY;
        }

        // Copy the path. This can be NULL; see Init() for details.
        if (pPath)
        {
            CopyMemory(psz, pPath, cchPath * sizeof(TCHAR));
        }

        // Add the trailing back slash if needed.
        if (bAddTrailingSlash)
        {
            psz[cchPath] = TEXT('\\');
        }

        // Copy the file name.
        CopyMemory(psz + (cchPath + bAddTrailingSlash), pName, cchName * sizeof(TCHAR));

        // Add a NULL terminator.
        psz[cch - 1] = TEXT('\0');

        *ppFileName = psz;

        _Next();

        return S_OK;
    }

};

