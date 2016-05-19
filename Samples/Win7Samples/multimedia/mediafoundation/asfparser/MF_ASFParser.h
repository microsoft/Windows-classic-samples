//////////////////////////////////////////////////////////////////////////
//
// Common.h : Global header.
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

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.

#ifndef WINVER              // Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400       // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400     // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif                      

#ifndef _WIN32_WINDOWS      // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE           // Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400    // Change this to the appropriate value to target IE 5.0 or later.
#endif

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers


// Windows Header Files:
#include <windows.h>
#include <cderr.h>
#include <assert.h>
#include <tchar.h>
#include <commdlg.h> // OpenFile dialog
#include <strsafe.h>
#include <commctrl.h> //Common controls
#include <windowsx.h> // Windows helper macros


// Media Foundation Header Files:
#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mftransform.h>
#include <mferror.h>
#include <wmcontainer.h>
#include <wmcodecdsp.h>

//Video rendering through GDI+
#include <gdiplus.h>
using namespace Gdiplus;

//WAV play through 
#include <mmsystem.h>

#define USE_LOGGING
#include "common.h" // Common sample files
using namespace MediaFoundationSamples;

#define CHECK_HR(hr) IF_FAILED_GOTO(hr, done)


//Constants
#define MAX_STRING_SIZE         260
#define TEST_AUDIO_DURATION     50000000
#define STREAMING               1
#define NOT_STREAMING           2
#define MIN_ASF_HEADER_SIZE ( MFASF_MIN_HEADER_BYTES + sizeof( WORD ) + sizeof (DWORD))

struct SAMPLE_INFO
{
    UINT32 fSeekedKeyFrame;
    DWORD wStreamNumber;
    DWORD cBufferCount;
    LONGLONG hnsSampleTime;
    DWORD cbTotalLength;


    SAMPLE_INFO()
        : 
    wStreamNumber(0),
    cBufferCount(0),
    hnsSampleTime(0),
    cbTotalLength(0),
    fSeekedKeyFrame(0)
    {}

};

struct FILE_PROPERTIES_OBJECT
{
    GUID guidFileID;
    FILETIME ftCreationTime;
    UINT32 cbMaxBitRate;
    UINT32 cbMaxPacketSize;
    UINT32 cbMinPacketSize;
    UINT32 cbPackets;
    UINT64 cbPlayDuration;
    UINT64 cbSendDuration;
    UINT32 cbFlags;
    UINT64 cbPreroll;
    UINT64 cbPresentationDuration;

    FILE_PROPERTIES_OBJECT()
        : 
    cbMaxBitRate (0),
    cbMaxPacketSize (0),
    cbMinPacketSize (0),
    cbPackets (0),
    cbPlayDuration (0),
    cbSendDuration (0),
    cbPreroll (0),
    cbPresentationDuration (0),
    cbFlags (0)
    {}

};