/************************************************************************

Copyright (c) Microsoft Corporation

Module Name :

    bits_ie.cpp

Abstract :

    Sample background downloader which uses BITS.

Revision History :

Notes:

    This program is a very simple background downloader which demonstrates
    the use of BITS. The program hooks into the IE context menu, to
    allow the user to schedule the download instead of using the default
    IE downloader.

Concepts Covered:

    1. Basic connection with manager and job submission.
    2. Example presentation to user of job state.
    3. Job control such as suspend/resume/cancel/complete.
    4. Interface based callbacks for updating progress/state.
    5. How to get the text message for a BITS error code.
    
 ***********************************************************************/

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#pragma warning( disable : 4786 )

//
// Allow use of safe string fns where necessary, without rejecting all deprecated functions.
//
#undef STRSAFE_NO_DEPRECATE 
#define STRSAFE_NO_DEPRECATE 

#include <windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <float.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <wininet.h>
#include <shlobj.h>
#include "resource.h"
#include <bits.h>
#include <comdef.h>

template<class T> class SmartRefPointer
{
private:
   T * m_Interface;

   void ReleaseIt()
   {
      if ( m_Interface )
         m_Interface->Release();
      m_Interface = NULL;
   }

   void RefIt()
   {
      if ( m_Interface )
          m_Interface->AddRef();
   }

public:

   SmartRefPointer()
   {
      m_Interface = NULL;
   }

   SmartRefPointer( T * RawInterface )
   {
      m_Interface = RawInterface;
      RefIt();
   }

   SmartRefPointer( SmartRefPointer & Other )
   {
      m_Interface = Other.m_Interface;
      RefIt();
   }

   ~SmartRefPointer()
   {
      ReleaseIt();
   }

   T * Get() const
   {
      return m_Interface;
   }

   T * Release()
   {
      T * temp = m_Interface;
      m_Interface = NULL;
      return temp;
   }

   void Clear()
   {
      ReleaseIt();
   }

   T** GetRecvPointer()
   {
      ReleaseIt();
      return &m_Interface;
   }

   SmartRefPointer & operator=( SmartRefPointer & Other )
   {
      ReleaseIt();
      m_Interface = Other.m_Interface;
      RefIt();
      return *this;
   }

   T* operator->() const
   {
      return m_Interface;
   }

   operator const T*() const
   {
      return m_Interface;
   }
};

typedef SmartRefPointer<IUnknown> SmartIUnknownPointer;
typedef SmartRefPointer<IBackgroundCopyManager> SmartManagerPointer;
typedef SmartRefPointer<IBackgroundCopyJob> SmartJobPointer;
typedef SmartRefPointer<IBackgroundCopyJob2> SmartJob2Pointer;
typedef SmartRefPointer<IBackgroundCopyError> SmartJobErrorPointer;
typedef SmartRefPointer<IBackgroundCopyFile> SmartFilePointer;
typedef SmartRefPointer<IEnumBackgroundCopyFiles> SmartEnumFilesPointer;
typedef SmartRefPointer<IEnumBackgroundCopyJobs> SmartEnumJobsPointer;
typedef SmartRefPointer<IShellLink> SmartShellLinkPointer;
typedef SmartRefPointer<IPersistFile> SmartPersistFilePointer;

// Maxstring size, bump up on problems
#define MAX_STRING 0x800 // 2K

GUID g_JobId;
WCHAR g_szFileName[MAX_PATH];
HWND g_hwndDlg = NULL;

// These two global variables throttle updates.
// The algorithm is to set a timer on the first update request,
// and delay additional updates until after the timer expires.

// The timer is set
bool g_UpdateTimerSet = FALSE;
// Received on update request while timer is active
bool g_RefreshOnTimer = FALSE; 

SmartJobPointer g_pJob;
SmartManagerPointer g_pManager;

void HandleUpdate( );
void CheckHR( HWND hwnd, HRESULT Hr, bool bThrow );

void SafeCopy( __out_ecount( Count ) LPWSTR Dest, LPCWSTR Source, size_t Count )
{
    if ( !Count )
        return;

    while (Count && (*Source != L'\0'))
    {
        *Dest++ = *Source++;
        Count--;
    }

    if (Count == 0)
    {
        // we are going to truncate Dest
        Dest--;
    }

    *Dest= L'\0';
}

void SafeCat( __out_ecount( Count ) LPWSTR Dest, LPCWSTR Source, size_t Count )
{
    size_t DestSize = wcslen( Dest );

    if ( DestSize > Count )
        return;

    SafeCopy( Dest + DestSize,
              Source,
              Count - DestSize );
}

void SafeStringPrintf( __out_ecount(Count) LPWSTR Dest, DWORD Count, LPCWSTR Format, ... )
{
     va_list arglist;
     va_start( arglist, Format );

     if ( !Count )
         return;
     
     //
     // The result is guaranteed to be NULL-terminated, and truncated if necessary.
     //
     (void) StringCchVPrintf( Dest, Count, Format, arglist );
}

const WCHAR * GetString( UINT id )
{

    //
    // Retrieves the localized string for the resource id
    // caching the string when loaded.

    static const WCHAR* pStringArray[ IDS_MAX ];
    static WCHAR TempStringBuffer[ MAX_STRING ];
    const WCHAR * & pStringPointer = pStringArray[ id - 1 ];

    // Cache resource strings
    if ( pStringPointer )
        return pStringPointer;

    // Load string from resource

    int CharsLoaded =
        LoadStringW(
            (HINSTANCE)GetModuleHandle(NULL),
            id,
            TempStringBuffer,
            MAX_STRING );

    if ( !CharsLoaded )
        {
        CheckHR( NULL, HRESULT_FROM_WIN32( GetLastError() ), false );
        return L"";
        }

    WCHAR *pNewString = new WCHAR[ CharsLoaded + 1];
    if ( !pNewString )
        {
        CheckHR( NULL, E_OUTOFMEMORY, false );
        return L"";
        }

    SafeCopy( pNewString, TempStringBuffer, CharsLoaded + 1 );
    return ( pStringPointer = pNewString );

}

void
DeleteStartupLink(
    GUID JobID
    )
{

    //
    // Delete the link in the Startup folder for the job
    //

    WCHAR szLinkFileName[MAX_PATH] = {0};
    WCHAR szGUIDString[MAX_PATH] = {0};

    BOOL bResult =
        SHGetSpecialFolderPath(
            NULL,
            szLinkFileName,
            CSIDL_STARTUP,
            FALSE );

    if ( !bResult )
        return;

    SafeCat( szLinkFileName, L"\\", MAX_PATH );
    SafeCat( szLinkFileName, GetString( IDS_STARTUPLINK ), MAX_PATH );
    SafeCat( szLinkFileName, L" ", MAX_PATH );

    StringFromGUID2( JobID, szGUIDString, MAX_PATH );
    SafeCat( szLinkFileName, szGUIDString, MAX_PATH );
    SafeCat( szLinkFileName, L".lnk", MAX_PATH );

    if (!DeleteFile( szLinkFileName ))
        {
        DWORD dwError = GetLastError();
        if ( ERROR_PATH_NOT_FOUND != dwError &&
             ERROR_FILE_NOT_FOUND != dwError )
            {
            CheckHR( NULL, HRESULT_FROM_WIN32( dwError ), false );
            }
        }

}

void
CreateStartupLink(
    GUID JobID,
    __in LPWSTR pszFileName
    )
{
    //
    // Create a link in the Startup folder for this job.
    //

    SmartShellLinkPointer   ShellLink;
    SmartPersistFilePointer PersistFile;

    WCHAR szLinkFileName[MAX_PATH] = {0};

    BOOL bResult =
        SHGetSpecialFolderPath(
            NULL,
            szLinkFileName,
            CSIDL_STARTUP,
            FALSE );

    if ( !bResult )
        CheckHR( NULL, E_FAIL, true );

    WCHAR szLinkDescription[MAX_PATH] = {0};
    SafeCopy( szLinkDescription, GetString( IDS_STARTUPLINK ), MAX_PATH );
    SafeCat( szLinkDescription, L" ", MAX_PATH );

    WCHAR szGUIDString[MAX_PATH] = {0};
    StringFromGUID2( JobID, szGUIDString, MAX_PATH );
    SafeCat( szLinkDescription, szGUIDString, MAX_PATH );

    WCHAR szArguments[MAX_PATH] = {0};
    SafeCopy( szArguments, L"/RESUMEJOB ", MAX_PATH);
    SafeCat( szArguments, szGUIDString, MAX_PATH );
    SafeCat( szArguments, L" ", MAX_PATH );
    SafeCat( szArguments, pszFileName, MAX_PATH );

    SafeCat( szLinkFileName, L"\\", MAX_PATH );
    SafeCat( szLinkFileName, szLinkDescription, MAX_PATH );
    SafeCat( szLinkFileName, L".lnk", MAX_PATH );

    CheckHR( NULL, 
             CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                               IID_IShellLink, (LPVOID*)ShellLink.GetRecvPointer() ),
             true );

    CheckHR( NULL, ShellLink->SetShowCmd( SW_SHOWMINIMIZED ), true );
    CheckHR( NULL, ShellLink->QueryInterface( IID_IPersistFile, (LPVOID*)PersistFile.GetRecvPointer() ), true );
    CheckHR( NULL, ShellLink->SetPath( L"%windir%\\system32\\bits_ie.exe" ), true );
    CheckHR( NULL, ShellLink->SetArguments( szArguments ), true );
    CheckHR( NULL, ShellLink->SetDescription( szLinkDescription ), true );
    CheckHR( NULL, PersistFile->Save( szLinkFileName, TRUE ), true );

}


void SetWindowTime(
    HWND hwnd,
    FILETIME filetime
    )
{
     // Set the window text to be the text representation
     // of the file time.
     // If an error occurs, set the window text to be error

     FILETIME localtime;
     FileTimeToLocalFileTime( &filetime, &localtime );

     SYSTEMTIME systemtime;
     FileTimeToSystemTime( &localtime, &systemtime );

     WCHAR DateBuffer[ MAX_STRING ];

     int DateSize =
         GetDateFormatW(
             LOCALE_USER_DEFAULT,
             0,
             &systemtime,
             NULL,
             DateBuffer,
             MAX_STRING );

     if (!DateSize)
         {
         SetWindowText( hwnd, GetString( IDS_ERROR ) );
         return;
         }

     WCHAR TimeBuffer[ MAX_STRING ];

     int TimeSize =
        GetTimeFormatW(
            LOCALE_USER_DEFAULT,
            0,
            &systemtime,
            NULL,
            TimeBuffer,
            MAX_STRING );

     if (!TimeSize)
         {
         SetWindowText( hwnd, GetString( IDS_ERROR ) );
         return;
         }

     WCHAR FullTime[ MAX_STRING ];
     SafeStringPrintf( FullTime, MAX_STRING, 
                       L"%s %s", DateBuffer, TimeBuffer );

     SetWindowText( hwnd, FullTime );
}

UINT64
GetSystemTimeAsUINT64()
{

    //
    // Returns the system time as an UINT instead of a FILETIME.
    //

    FILETIME filetime;
    GetSystemTimeAsFileTime( &filetime );

    ULARGE_INTEGER large;
    memcpy( &large, &filetime, sizeof(FILETIME) );

    return large.QuadPart;
}

void SignalAlert(
    HWND hwndDlg,
    UINT Type
    )
{

    //
    // Alert the user that an important event has occurred
    //

    FLASHWINFO FlashInfo;
    FlashInfo.cbSize    = sizeof(FlashInfo);
    FlashInfo.hwnd      = hwndDlg;
    FlashInfo.dwFlags   = FLASHW_ALL | FLASHW_TIMERNOFG;
    FlashInfo.uCount    = 0;
    FlashInfo.dwTimeout = 0;

    FlashWindowEx( &FlashInfo );
    MessageBeep( Type );

}

const WCHAR *
MapStateToString(
    BG_JOB_STATE state
    )
{

   //
   // Maps a BITS job state to a human readable string
   //

   switch( state )
       {

       case BG_JOB_STATE_QUEUED:
           return GetString( IDS_QUEUED );

       case BG_JOB_STATE_CONNECTING:
           return GetString( IDS_CONNECTING );

       case BG_JOB_STATE_TRANSFERRING:
           return GetString( IDS_TRANSFERRING );

       case BG_JOB_STATE_SUSPENDED:
           return GetString( IDS_SUSPENDED );

       case BG_JOB_STATE_ERROR:
           return GetString( IDS_FATALERROR );

       case BG_JOB_STATE_TRANSIENT_ERROR:
           return GetString( IDS_TRANSIENTERROR );

       case BG_JOB_STATE_TRANSFERRED:
           return GetString( IDS_TRANSFERRED );

       case BG_JOB_STATE_ACKNOWLEDGED:
           return GetString( IDS_ACKNOWLEDGED );

       case BG_JOB_STATE_CANCELLED:
           return GetString( IDS_CANCELLED );

       default:

           // NOTE: Always provide a default case
           // since new states may be added in future versions.
           return GetString( IDS_UNKNOWN );

       }
}

double
ScaleDownloadRate(
    double Rate, // rate in seconds
    __out LPCWSTR *pFormat )
{

    //
    // Scales a download rate and selects the correct
    // format to pass to wprintf for printing.
    //

    double RateBounds[] =
    {
       1073741824.0, // Gigabyte
       1048576.0,    // Megabyte
       1024.0,       // Kilobyte
       0             // Byte
    };

    UINT RateFormat[] =
    {
        IDS_GIGAFORMAT,
        IDS_MEGAFORMAT,
        IDS_KILOFORMAT,
        IDS_BYTEFORMAT
    };

    for( unsigned int c = 0;; c++ )
        {
        if ( Rate >= RateBounds[c] )
            {
            *pFormat = GetString( RateFormat[c] );
            double scale = (RateBounds[c] >= 1.0) ? RateBounds[c] : 1.0;
            return Rate / scale;
            }
        }
}

UINT64
ScaleDownloadEstimate(
    double Time, // time in seconds
    __out LPCWSTR *pFormat )
{

    //
    // Scales a download time estimate and selects the correct
    // format to pass to wprintf for printing.
    //


    double TimeBounds[] =
    {
       60.0 * 60.0 * 24.0,        // Days
       60.0 * 60.0,               // Hours
       60.0,                      // Minutes
       0.0                        // Seconds
    };

    UINT TimeFormat[] =
    {
        IDS_DAYSFORMAT,
        IDS_HOURSFORMAT,
        IDS_MINUTESFORMAT,
        IDS_SECONDSFORMAT
    };

    for( unsigned int c = 0;; c++ )
        {
        if ( Time >= TimeBounds[c] )
            {
            *pFormat = GetString( TimeFormat[c] );
            double scale = (TimeBounds[c] >= 1.0) ? TimeBounds[c] : 1.0;
            return (UINT64)floor( ( Time / scale ) + 0.5);
            }
        }

}

void
UpdateDialog(
    HWND hwndDlg
    )
{

   //
   // Main update routine for the dialog box.
   // Retries the job state/properties from
   // BITS and updates the dialog box.
   //

   {
   // update the display name

   HWND hwndDisplayName = GetDlgItem( hwndDlg, IDC_DISPLAYNAME );
   WCHAR * pszDisplayName = NULL;
   if (FAILED( g_pJob->GetDisplayName( &pszDisplayName ) ) ) 
       return; // stop updating on an error
   SetWindowText( hwndDisplayName, pszDisplayName );
   ShowWindow( hwndDisplayName, SW_SHOW );
   CoTaskMemFree( pszDisplayName );

   }

   static BG_JOB_STATE prevstate = BG_JOB_STATE_SUSPENDED;
   BG_JOB_STATE state;

   if (FAILED(g_pJob->GetState( &state )))
       return; // stop updating on an error

   if ( BG_JOB_STATE_ACKNOWLEDGED == state ||
        BG_JOB_STATE_CANCELLED == state )
       {
       // someone else cancelled or completed the job on us,
       // just exist the exit.
       // May happen if job is canceled with bitsadmin

       DeleteStartupLink( g_JobId );
       PostQuitMessage( 0 );
       return;
       }

   BG_JOB_PROGRESS progress;
   if (FAILED(g_pJob->GetProgress( &progress )))
       return; // stop updating on an error

   {
      // update the title, progress bar, and progress description
      WCHAR szProgress[MAX_STRING];
      WCHAR szTitle[MAX_STRING];
      WPARAM newpos = 0;

      if ( progress.BytesTotal &&
           ( progress.BytesTotal != BG_SIZE_UNKNOWN ) )
          {
          SafeStringPrintf( 
              szProgress, MAX_STRING, GetString( IDS_LONGPROGRESS ), 
              progress.BytesTransferred, progress.BytesTotal );

          double Percent = (double)(__int64)progress.BytesTransferred /
                           (double)(__int64)progress.BytesTotal;
          Percent *= 100.0;
          SafeStringPrintf( 
              szTitle, MAX_STRING, L"%u%% %s", 
              (unsigned int)Percent, g_szFileName );
          newpos = (WPARAM)Percent;

          }
      else
          {
          SafeStringPrintf( 
              szProgress, MAX_STRING, GetString( IDS_SHORTPROGRESS ), 
              progress.BytesTransferred );
          SafeCopy( szTitle, g_szFileName, MAX_PATH );
          newpos = 0;
          }

      SendDlgItemMessage( hwndDlg, IDC_PROGRESSBAR, PBM_SETPOS, newpos, 0 );
      SetWindowText( GetDlgItem( hwndDlg, IDC_PROGRESSINFO ), szProgress );
      ShowWindow( GetDlgItem( hwndDlg, IDC_PROGRESSINFO ), SW_SHOW );
      EnableWindow( GetDlgItem( hwndDlg, IDC_PROGRESSINFOTXT ), TRUE );
      SetWindowText( hwndDlg, szTitle );

   }

   {
   // update the status
   HWND hwndStatus = GetDlgItem( hwndDlg, IDC_STATUS );

   SetWindowText( hwndStatus, MapStateToString( state ) );
   ShowWindow( hwndStatus, SW_SHOW );

   // Only enable the finish button if the job is finished.
   EnableWindow( GetDlgItem( hwndDlg, IDC_FINISH ), ( state == BG_JOB_STATE_TRANSFERRED ) );

   // Only enable the suspend button if the job is not finished or transferred
   BOOL EnableSuspend =
       ( state != BG_JOB_STATE_SUSPENDED ) && ( state != BG_JOB_STATE_TRANSFERRED );
   EnableWindow( GetDlgItem( hwndDlg, IDC_SUSPEND ), EnableSuspend );

   // Only enable the resume button if the job is suspended
   BOOL EnableResume = ( BG_JOB_STATE_SUSPENDED == state );
   EnableWindow( GetDlgItem( hwndDlg, IDC_RESUME ), EnableResume );

   // Alert the user when something important happens
   // such as the job completes or a unrecoverable error occurs
   if ( BG_JOB_STATE_TRANSFERRED == state &&
        BG_JOB_STATE_TRANSFERRED != prevstate )
       SignalAlert( hwndDlg, MB_OK );

   else if ( BG_JOB_STATE_ERROR == state &&
        BG_JOB_STATE_ERROR != prevstate )
       SignalAlert( hwndDlg, MB_ICONEXCLAMATION );

   }

   {
   // update times
   BG_JOB_TIMES times;
   if (FAILED(g_pJob->GetTimes( &times )))
       return;

   HWND hwndCreationTime = GetDlgItem( hwndDlg, IDC_STARTTIME );
   SetWindowTime( hwndCreationTime, times.CreationTime );
   ShowWindow( hwndCreationTime, SW_SHOW );

   HWND hwndModificationTime = GetDlgItem( hwndDlg, IDC_MODIFICATIONTIME );
   SetWindowTime( hwndModificationTime, times.ModificationTime );
   ShowWindow( hwndModificationTime, SW_SHOW );

   HWND hwndCompletionTime = GetDlgItem( hwndDlg, IDC_COMPLETIONTIME );
   if ( !times.TransferCompletionTime.dwLowDateTime && !times.TransferCompletionTime.dwHighDateTime )
       {

       // BITS sets the CompletionTime to all zeros
       // if the job is incomplete

       ShowWindow( hwndCompletionTime, SW_HIDE );
       EnableWindow( GetDlgItem( hwndDlg, IDC_COMPLETIONTIMETXT ), FALSE );
       }
   else
       {
       SetWindowTime( hwndCompletionTime, times.TransferCompletionTime );
       ShowWindow( hwndCompletionTime, SW_SHOW );
       EnableWindow( GetDlgItem( hwndDlg, IDC_COMPLETIONTIMETXT ), TRUE );
       }
   }

   {
   // update the error message
   IBackgroundCopyError *pError;
   HRESULT Hr = g_pJob->GetError( &pError );

   if ( FAILED(Hr) )
       {
       ShowWindow( GetDlgItem( hwndDlg, IDC_ERRORMSG ), SW_HIDE );
       EnableWindow( GetDlgItem( hwndDlg, IDC_ERRORMSGTXT ), FALSE );
       }
   else
       {

       WCHAR* pszDescription = NULL;
       WCHAR* pszContext = NULL;

       // If these APIs fail, we should get back
       // a NULL string. So everything should be harmless.

       pError->GetErrorDescription(
           LANGIDFROMLCID( GetThreadLocale() ),
           &pszDescription );
       pError->GetErrorContextDescription(
           LANGIDFROMLCID( GetThreadLocale() ),
           &pszContext );

       WCHAR FullText[ MAX_STRING ];
       FullText[0] = L'\0';

       if ( pszDescription )
           SafeCopy( FullText, pszDescription, MAX_STRING );
       if ( pszContext )
           SafeCat( FullText, pszContext, MAX_STRING );
       CoTaskMemFree( pszDescription );
       CoTaskMemFree( pszContext );

       HWND hwndErrorText = GetDlgItem( hwndDlg, IDC_ERRORMSG );
       SetWindowText( hwndErrorText, FullText );
       ShowWindow( hwndErrorText, SW_SHOW );
       EnableWindow( GetDlgItem( hwndDlg, IDC_ERRORMSGTXT ), TRUE );

       }

   }

   if (!SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_GETDROPPEDSTATE, 0, 0) )
       {
       // set the priority, but only do it if user isn't trying to
       // set the priority.
       BG_JOB_PRIORITY priority;
       g_pJob->GetPriority( &priority );
       SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_SETCURSEL, (WPARAM)priority, 0 );
       }

   {

   //
   // This large block of text computes the average transfer rate
   // and estimated completion time.  This code has much
   // room for improvement.
   //

   static BOOL HasRates = FALSE;
   static UINT64 LastMeasurementTime;
   static UINT64 LastMeasurementBytes;
   static double LastMeasurementRate;

   WCHAR szRateText[MAX_STRING];
   BOOL EnableRate = FALSE;

   if ( !( BG_JOB_STATE_QUEUED == state ) &&
        !( BG_JOB_STATE_CONNECTING == state ) &&
        !( BG_JOB_STATE_TRANSFERRING == state ) )
       {
       // If the job isn't running, then rate values won't
       // make any sense. Don't display them.
       HasRates = FALSE;
       }
   else
       {

       if ( !HasRates )
           {
           LastMeasurementTime = GetSystemTimeAsUINT64();
           LastMeasurementBytes = progress.BytesTransferred;
           LastMeasurementRate = 0;
           HasRates = TRUE;
           }
       else
           {

           UINT64 CurrentTime = GetSystemTimeAsUINT64();
           UINT64 NewTotalBytes = progress.BytesTransferred;

           UINT64 NewTimeDiff = CurrentTime - LastMeasurementTime;
           UINT64 NewBytesDiff = NewTotalBytes - LastMeasurementBytes;
           double NewInstantRate = (double)(__int64)NewBytesDiff /
                                   (double)(__int64)NewTimeDiff;
           double NewAvgRate = (0.3 * LastMeasurementRate) +
                               (0.7 * NewInstantRate );

           if ( !_finite(NewInstantRate) || !_finite(NewAvgRate) )
               {
               NewInstantRate = 0;
               NewAvgRate = LastMeasurementRate;
               }

           LastMeasurementTime = CurrentTime;
           LastMeasurementBytes = NewTotalBytes;
           LastMeasurementRate = NewAvgRate;

           // convert from FILETIME units to seconds
           double NewDisplayRate = NewAvgRate * 10000000;

           const WCHAR *pRateFormat = NULL;
           double ScaledRate = ScaleDownloadRate( NewDisplayRate, &pRateFormat );
           SafeStringPrintf( szRateText, MAX_STRING, pRateFormat, ScaledRate );
           
           EnableRate = TRUE;
           }

       }

   if (!EnableRate)
       {
       ShowWindow( GetDlgItem( hwndDlg, IDC_TRANSFERRATE ), SW_HIDE );
       EnableWindow( GetDlgItem( hwndDlg, IDC_TRANSFERRATETXT ), FALSE );
       }
   else
       {
       SetWindowText( GetDlgItem( hwndDlg, IDC_TRANSFERRATE ), szRateText );
       ShowWindow( GetDlgItem( hwndDlg, IDC_TRANSFERRATE ), SW_SHOW );
       EnableWindow( GetDlgItem( hwndDlg, IDC_TRANSFERRATETXT ), TRUE );
       }

   BOOL EnableEstimate = FALSE;
   WCHAR szEstimateText[MAX_STRING];

   if ( EnableRate )
       {

       if ( progress.BytesTotal != 0 &&
            progress.BytesTotal != BG_SIZE_UNKNOWN )
           {

           double TimeRemaining =
               ( (__int64)progress.BytesTotal - (__int64)LastMeasurementBytes ) / LastMeasurementRate;

           // convert from FILETIME units to seconds
           TimeRemaining = TimeRemaining / 10000000.0;

           static const double SecsPer30Days = 60.0 * 60.0 * 24.0 * 30.0;

           // Don't estimate if estimate is larger then 30 days.
           if ( TimeRemaining < SecsPer30Days )
               {

               const WCHAR *pFormat = NULL;
               UINT64 Time = ScaleDownloadEstimate( TimeRemaining, &pFormat );
               SafeStringPrintf( szEstimateText, MAX_STRING, pFormat, Time );
               EnableEstimate = TRUE;
               }
           }
       }

   if (!EnableEstimate)
       {
       ShowWindow( GetDlgItem( hwndDlg, IDC_ESTIMATEDTIME ), SW_HIDE );
       EnableWindow( GetDlgItem( hwndDlg, IDC_ESTIMATEDTIMETXT ), FALSE );
       }
   else
       {
       SetWindowText( GetDlgItem( hwndDlg, IDC_ESTIMATEDTIME ), szEstimateText );
       ShowWindow( GetDlgItem( hwndDlg, IDC_ESTIMATEDTIME ), SW_SHOW );
       EnableWindow( GetDlgItem( hwndDlg, IDC_ESTIMATEDTIMETXT ), TRUE );
       }

   }

   prevstate = state;
}

void
InitDialog(
    HWND hwndDlg
    )
{

   //
   // Populate the priority list with priority descriptions
   //

   const WCHAR *Foreground    = GetString( IDS_FOREGROUND );
   const WCHAR *High          = GetString( IDS_HIGH );
   const WCHAR *Normal        = GetString( IDS_NORMAL );
   const WCHAR *Low           = GetString( IDS_LOW );

   SendDlgItemMessage( hwndDlg, IDC_PROGRESSBAR, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
   SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)Foreground );
   SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)High );
   SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)Normal );
   SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)Low );

}

void CheckHR( HWND hwnd, HRESULT Hr, bool bThrow )
{
    //
    // Provides automatic error code checking and dialog
    // for generic system errors
    //

    if (SUCCEEDED(Hr))
        return;

    WCHAR * pszError = NULL;

    DWORD dwFormatError =
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD)Hr,
        LANGIDFROMLCID( GetThreadLocale() ),
        (WCHAR*)&pszError,
        0,
        NULL );

    if ( !dwFormatError ) 
       {
       WCHAR ErrorMsg[ MAX_STRING ];
       SafeStringPrintf( ErrorMsg, MAX_STRING, GetString( IDS_DISPLAYERRORCODE ), Hr );
       
       MessageBox( hwnd, ErrorMsg, GetString( IDS_ERRORBOXTITLE ),
                   MB_OK | MB_ICONSTOP | MB_APPLMODAL );
       }
    else
       {
       MessageBox( hwnd, pszError, GetString( IDS_ERRORBOXTITLE ),
                   MB_OK | MB_ICONSTOP | MB_APPLMODAL );
       LocalFree( pszError );
       }

    if ( bThrow )
        throw _com_error( Hr );

}

void BITSCheckHR( HWND hwnd, HRESULT Hr, bool bThrow )
{

   //
   // Provides automatic error code checking and dialog
   // for BITS specific errors
   //


   if (SUCCEEDED(Hr))
       return;

   WCHAR * pszError = NULL;
   HRESULT hErrorHr = 
   g_pManager->GetErrorDescription(
       Hr,
       LANGIDFROMLCID( GetThreadLocale() ),
       &pszError );

   if ( FAILED(hErrorHr) || !pszError )
       {

       WCHAR ErrorMsg[ MAX_STRING ];
       SafeStringPrintf( ErrorMsg, MAX_STRING, GetString( IDS_DISPLAYERRORCODE ), Hr );

       MessageBox( hwnd, ErrorMsg, GetString( IDS_ERRORBOXTITLE ),
                   MB_OK | MB_ICONSTOP | MB_APPLMODAL );

       }

   else
       {
      
       MessageBox( hwnd, pszError, GetString( IDS_ERRORBOXTITLE ),
                   MB_OK | MB_ICONSTOP | MB_APPLMODAL );
       CoTaskMemFree( pszError );

       }


   if ( bThrow )
       throw _com_error( Hr );
}

void
DoCancel(
    HWND hwndDlg,
    bool PromptUser
    )
{

   //
   // Handle all the operations required to cancel the job.
   // This includes asking the user for confirmation.
   //

   if ( PromptUser )
       {

       int Result =
           MessageBox(
               hwndDlg,
               GetString( IDS_CANCELTEXT ),
               GetString( IDS_CANCELCAPTION ),
               MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL |
               MB_SETFOREGROUND | MB_TOPMOST );


       if ( IDYES != Result )
           return;

       }

   try
   {
       BITSCheckHR( hwndDlg, g_pJob->Cancel(), true );
   }
   catch( _com_error Error )
   {
       // If we can't cancel for some unknown reason,
       // don't exit
       return;
   }

   DeleteStartupLink( g_JobId );
   PostQuitMessage( 0 );
}

void
DoFinish(
    HWND hwndDlg
    )
{

   //
   // Handles all the necessary work to complete
   // the download.
   //

   try
   {
       BITSCheckHR( hwndDlg, g_pJob->Complete(), true );
   }
   catch( _com_error Error )
   {
       // If we can't finish/complete for some unknown reason,
       // don't exit
       return;
   }

   DeleteStartupLink( g_JobId );
   PostQuitMessage( 0 );

}

void
DoClose(
    HWND hwndDlg
    )
{
    //
    // Handles an attempt by the user to close the sample.
    //

    // Check to see if the download has finished,
    // if so don't let the user exit.

    BG_JOB_STATE state;
    HRESULT hResult = g_pJob->GetState( &state );

    if (FAILED( hResult ))
        {
        BITSCheckHR( hwndDlg, hResult, false );
        return;
        }

    if ( BG_JOB_STATE_ERROR == state ||
         BG_JOB_STATE_TRANSFERRED == state )
        {

        MessageBox(
            hwndDlg,
            GetString( IDS_ALREADYFINISHED ),
            GetString( IDS_ALREADYFINISHEDCAPTION ),
            MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_APPLMODAL |
            MB_SETFOREGROUND | MB_TOPMOST );


        return;
        }


    //
    // Inform the user that he selected close and ask
    // confirm the intention to exit.  Explain that the job 
    // will be canceled.

    int Result =
        MessageBox(
            hwndDlg,
            GetString( IDS_CLOSETEXT ),
            GetString( IDS_CLOSECAPTION ),
            MB_OKCANCEL | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL |
            MB_SETFOREGROUND | MB_TOPMOST );

    if ( IDOK == Result )
        {
        
        // User confirmed the cancel, just do it.

        DoCancel( hwndDlg, false );
        return;
        }

    // The user didn't really want to exit, so ignore him
    else
        return;

}

void
HandleTimerTick( HWND hwndDlg )
{

    //
    // Handle the throttling timer event 
    // and update the dialog if needed
    //

    if ( g_RefreshOnTimer )
        {
        // The timer fired, handle all updates at once.
        UpdateDialog( hwndDlg );
        g_RefreshOnTimer = FALSE;
        }
    else
        {
        // The timer expired with an additional modification
        // notification.  Just kill the timer.
        KillTimer( hwndDlg, 0 );
        g_RefreshOnTimer = g_UpdateTimerSet = FALSE;
        }

}

void
HandleUpdate()
{

    //
    // Handle a update request, batching the update if needed
    //

    if ( !g_UpdateTimerSet )
        {
        // We aren't currently batching updates,
        // so do this one update but prevent
        // further updates until the timer expires.
        SetTimer( g_hwndDlg, 0, 1000, NULL );
        g_UpdateTimerSet = TRUE;
        UpdateDialog( g_hwndDlg );
        }
    else
        {
        // We've started batching and yet received
        // another update request.  Delay this
        // update until the timer fires.
        g_RefreshOnTimer = TRUE;
        }

}

INT_PTR CALLBACK
DialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
  )
{

  //
  // Dialog proc for main dialog window
  //

  switch( uMsg )
      {

      case WM_INITDIALOG:
          g_hwndDlg = hwndDlg;
          InitDialog( hwndDlg );
          return TRUE;

      case WM_TIMER:
          HandleTimerTick( hwndDlg );
          return TRUE;

      case WM_CLOSE:
          DoClose( hwndDlg );
          return TRUE;

      case WM_COMMAND:

          switch( LOWORD( wParam ) )
              {

              case IDC_RESUME:
                  BITSCheckHR( hwndDlg, g_pJob->Resume(), false );
                  return TRUE;

              case IDC_SUSPEND:
                  BITSCheckHR( hwndDlg, g_pJob->Suspend(), false );
                  return TRUE;

              case IDC_CANCEL:
                  DoCancel( hwndDlg, true );
                  return TRUE;

              case IDC_FINISH:
                  DoFinish( hwndDlg );
                  return TRUE;

              case IDC_PRIORITY:
                  switch( HIWORD( wParam ) )
                      {

                      case CBN_SELENDOK:

                          // User clicked on priority,
                          // update it.

                          BITSCheckHR( hwndDlg,
                              g_pJob->SetPriority( (BG_JOB_PRIORITY)
                                  SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_GETCURSEL, 0, 0 ) ), false );
                          return TRUE;

                      case CBN_SELENDCANCEL:
                          return TRUE;

                      default:
                          return FALSE;
                      }

              default:
                  return FALSE;
              }
      default:
          return FALSE;
      }
}

HRESULT
HandleCOMCallback(
    IBackgroundCopyJob* pJob,
    bool CriticalEvent
    );

class CBackgroundCopyCallback : public IBackgroundCopyCallback
{

    //
    // Callback class.   Used for change notifications.
    //

public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
    {

        *ppvObject = NULL;
        if ( riid == _uuidof(IUnknown) )
            {
            *ppvObject = (IUnknown*)(IBackgroundCopyCallback*)this;
            return S_OK;
            }

        else if ( riid == _uuidof(IBackgroundCopyCallback) )
            {
            *ppvObject = (IBackgroundCopyCallback*)this;
            return S_OK;
            }

        else
            return E_NOINTERFACE;

    }

    virtual HRESULT STDMETHODCALLTYPE CreateInstance(
        IUnknown *pUnkOuter,
        REFIID riid,
        void **ppvObject )
    {

        if ( pUnkOuter )
            return CLASS_E_NOAGGREGATION;

        return QueryInterface( riid, ppvObject );

    }

    // We are cheating on COM here, but we
    // are forcing the lifetime of the callback object
    // to be the same of the lifetime of the exe.

    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return 0;
    }
    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        return 0;
    }

    virtual HRESULT STDMETHODCALLTYPE JobTransferred(IBackgroundCopyJob *pJob)
    {
        return HandleCOMCallback( pJob, true );
    }

    virtual HRESULT STDMETHODCALLTYPE JobError(IBackgroundCopyJob *pJob, IBackgroundCopyError *pError)
    {
        return HandleCOMCallback( pJob, true );
    }

    virtual HRESULT STDMETHODCALLTYPE JobModification( IBackgroundCopyJob *pJob, DWORD dwReserved )
    {
        return HandleCOMCallback( pJob, true );
    }
} g_Callback;

HRESULT
HandleCOMCallback(
    IBackgroundCopyJob* pJob,
    bool CriticalEvent
    )
{

    // In addition to the work of HandleUpdate,
    // this function checks to see if we've
    // already initialized the manager.  If not,
    // do it now.

    if ( !g_pManager )
        {

        try
        {
            CheckHR( NULL,
                     CoCreateInstance( CLSID_BackgroundCopyManager,
                         NULL,
                         CLSCTX_LOCAL_SERVER,
                         IID_IBackgroundCopyManager,
                         (void**)g_pManager.GetRecvPointer() ), true );

            *g_pJob.GetRecvPointer() = pJob;

            BITSCheckHR( NULL, g_pJob->SetNotifyFlags( BG_NOTIFY_JOB_MODIFICATION ), true );

            // As an optimization, set the notification interface to be the callback
            // It shouldn't matter if this fails
            g_pJob->SetNotifyInterface( (IBackgroundCopyCallback*)&g_Callback );

            HandleUpdate();

            ShowWindow( g_hwndDlg, CriticalEvent ? SW_NORMAL : SW_MINIMIZE );

        }
        catch(_com_error error )
        {
            g_pManager.Release();
            g_pJob.Release();

            return error.Error();
        }

        }

    HandleUpdate();
    return S_OK; 
}

void
CreateUI( int nShowCmd )
{

    //
    // Creates the dialog box for the sample.
    //

    g_hwndDlg =
      CreateDialog(
         (HINSTANCE)GetModuleHandle(NULL),
         MAKEINTRESOURCE( IDD_DIALOG ),
         GetDesktopWindow(),
         DialogProc );

    if ( !g_hwndDlg )
        {
        CheckHR( NULL, HRESULT_FROM_WIN32(GetLastError()), true );
        return;
        }

    ShowWindow( g_hwndDlg, nShowCmd );
}

void CreateJob(
    __in LPWSTR szJobURL
    )
{
    //
    // Request a destination file name from the user
    // and submit a new job.
    //

    try
    {

        // crack the URL and get the filename
        WCHAR szURLFilePath[MAX_PATH] = {L'\0'};
        URL_COMPONENTS UrlComponents;

        memset( &UrlComponents, 0, sizeof(UrlComponents) );
        UrlComponents.dwStructSize = sizeof(URL_COMPONENTS);
        UrlComponents.lpszUrlPath = szURLFilePath;
        UrlComponents.dwUrlPathLength =
            sizeof(szURLFilePath)/sizeof(*szURLFilePath);

        BOOL CrackResult =
            InternetCrackUrl(
                szJobURL,
                0,
                0,
                &UrlComponents );

        if (!CrackResult)
            CheckHR( NULL, HRESULT_FROM_WIN32( GetLastError() ), false );

        if ( UrlComponents.nScheme != INTERNET_SCHEME_HTTP &&
             UrlComponents.nScheme != INTERNET_SCHEME_HTTPS
             )
            {

            MessageBox(
                NULL,
                GetString( IDS_NOHTTPORHTTPS ),
                GetString( IDS_ERRORBOXTITLE ),
                MB_OK | MB_ICONERROR | MB_APPLMODAL |
                MB_SETFOREGROUND | MB_TOPMOST );


            throw _com_error( E_INVALIDARG );

            }

        const WCHAR *szURLFileName =
            szURLFilePath + wcslen( szURLFilePath );

        // parse out the filename part of the URL
        while( szURLFileName != szURLFilePath )
            {

            if ( L'/' == *szURLFileName ||
                 L'\\' == *szURLFileName )
                {
                szURLFileName++;
                break;
                }

            szURLFileName--;
            }
       
		// This is needed in case the first
		// character is a slash.
        if ( L'/' == *szURLFileName ||
			 L'\\' == *szURLFileName )
			 szURLFileName++;

        // parse out the extension from the name
        const WCHAR *szURLFileExtension =
            szURLFileName + wcslen( szURLFileName );

        while( szURLFileName != szURLFileExtension )
            {
            if ( L'.' == *szURLFileExtension )
                break;
            szURLFileExtension--;
            }

        // build the extension list

        WCHAR *szExtensionList = NULL;
        const WCHAR *szAllFiles = GetString( IDS_ALLFILES );
        const size_t AllFilesSize = wcslen( szAllFiles ) + 1;
        const WCHAR *szAllFilesPattern = L"*";
        const size_t AllFilesPatternSize = sizeof(L"*")/sizeof(WCHAR);

        WCHAR *p;

        if ( szURLFileExtension == szURLFileName &&
             *szURLFileExtension != L'.' )
            {
            size_t StringSize = sizeof(WCHAR) * ( AllFilesSize + AllFilesPatternSize + 2 );
            szExtensionList = (WCHAR*)_alloca( StringSize );
            p = szExtensionList;
            }
        else
            {
            size_t ExtensionSize = wcslen( szURLFileExtension ) + 1;
            size_t StringSize =
                sizeof(WCHAR) * ( ExtensionSize + ExtensionSize + 1 + AllFilesSize
                                  + AllFilesPatternSize + 2 );
            szExtensionList = (WCHAR*)_alloca( StringSize );
            p = szExtensionList;

            memcpy( p, szURLFileExtension, ExtensionSize * sizeof(WCHAR) );
            p += ExtensionSize;
            *p++ = L'*';
            memcpy( p, szURLFileExtension, ExtensionSize * sizeof(WCHAR) );
            p += ExtensionSize;
            }

        memcpy( p, szAllFiles, AllFilesSize * sizeof(WCHAR) );
        p += AllFilesSize;
        memcpy( p, szAllFilesPattern, AllFilesPatternSize * sizeof(WCHAR) );
        p += AllFilesPatternSize;
        memset( p, 0, sizeof(WCHAR) * 2 );


        OPENFILENAME ofn;
        memset( &ofn, 0, sizeof( ofn ) );

        WCHAR szFileName[MAX_PATH];
        WCHAR szFileTitle[MAX_PATH];

        SafeCopy( szFileName, szURLFileName, MAX_PATH );
        SafeCopy( szFileTitle, szURLFileName, MAX_PATH );

        /* fill in non-variant fields of OPENFILENAME struct. */
        ofn.lStructSize       = sizeof(OPENFILENAME);
        ofn.hwndOwner         = g_hwndDlg;
        ofn.lpstrFilter       = szExtensionList;
        ofn.lpstrCustomFilter = NULL;
        ofn.nMaxCustFilter    = 0;
        ofn.nFilterIndex      = 0;
        ofn.lpstrFile         = szFileName;
        ofn.nMaxFile          = MAX_PATH;
        ofn.lpstrInitialDir   = NULL;
        ofn.lpstrFileTitle    = szFileTitle;
        ofn.nMaxFileTitle     = MAX_PATH;
        ofn.lpstrTitle        = GetString( IDS_FILEDLGTITLE );
        ofn.lpstrDefExt       = NULL;
        ofn.Flags             = 0;

        /* Use standard open dialog */
        BOOL bResult = GetSaveFileName ((LPOPENFILENAME)&ofn);

        if ( !bResult )
            {
            if ( !CommDlgExtendedError() )
                {
                // user canceled the box
                PostQuitMessage( 0 );
                return;
                }
            else
                CheckHR( NULL, HRESULT_FROM_WIN32( GetLastError() ), true );
            }


        SafeCopy( g_szFileName, szFileTitle, MAX_PATH );

        CheckHR( NULL,
                 CoCreateInstance( CLSID_BackgroundCopyManager,
                     NULL,
                     CLSCTX_LOCAL_SERVER,
                     IID_IBackgroundCopyManager,
                     (void**)g_pManager.GetRecvPointer() ), true );

        GUID guid;
        BITSCheckHR( NULL,
            g_pManager->CreateJob( szJobURL,
                                 BG_JOB_TYPE_DOWNLOAD,
                                 &guid,
                                 g_pJob.GetRecvPointer() ),
                        true );

        memset( &g_JobId, 0, sizeof(GUID) );
        BITSCheckHR( NULL, g_pJob->GetId( &g_JobId ), true );
        BITSCheckHR( NULL, g_pJob->AddFile( szJobURL, szFileName ), true );

        CreateStartupLink( g_JobId, g_szFileName );

        BITSCheckHR( NULL, g_pJob->SetNotifyFlags( BG_NOTIFY_JOB_MODIFICATION ), true );

        BITSCheckHR( NULL,
                     g_pJob->SetNotifyInterface( (IBackgroundCopyCallback*)&g_Callback ),
                     true );
        BITSCheckHR( NULL, g_pJob->Resume(), true );

        HandleUpdate();

    }
    catch( const _com_error &error )
    {
        if ( g_pJob )
            {
            g_pJob->Cancel();
            DeleteStartupLink( g_JobId );
            }

        throw error;
    }

}

void ResumeJob(
    __in LPWSTR szJobGUID,
    __in LPWSTR szJobFileName
    )
{

    //
    // Resume the display on an existing job
    //

    SafeCopy( g_szFileName, szJobFileName, MAX_PATH );
    CheckHR( NULL, IIDFromString( szJobGUID, &g_JobId ), true );

    CheckHR( NULL,
             CoCreateInstance( CLSID_BackgroundCopyManager,
                 NULL,
                 CLSCTX_LOCAL_SERVER,
                 IID_IBackgroundCopyManager,
                 (void**)g_pManager.GetRecvPointer() ), true );

    BITSCheckHR( NULL, g_pManager->GetJob( g_JobId, g_pJob.GetRecvPointer() ), true );
    BITSCheckHR( NULL,
                 g_pJob->SetNotifyInterface( (IBackgroundCopyCallback*)&g_Callback ),
                 true );
    BITSCheckHR( NULL, g_pJob->SetNotifyFlags( BG_NOTIFY_JOB_MODIFICATION ), true );

    ShowWindow( g_hwndDlg, SW_MINIMIZE );
    HandleUpdate();

}

int WINAPI WinMain(
  __in HINSTANCE hInstance,      // handle to current instance
  __in_opt HINSTANCE hPrevInstance,  // handle to previous instance
  __in_opt LPSTR lpCmdLine,      // command line
  __in int nCmdShow)             // show state
{

  //
  // Expected syntax:
  // bits_ie /CREATEJOB URL
  // bits_ie /RESUMEJOB JobGUID DestinationFile

  // /CREATEJOB - Called from the script which is run when 
  //              "Background Download As" is selected.
  // /RESUMEJOB - Called from the link in the startup directory
  //              to resume a job when it is restarted.

  try
  {
      CheckHR( NULL, CoInitialize(NULL), true );

      InitCommonControls();

      CreateUI( nCmdShow );

      LPTSTR lpCommandLine = GetCommandLine();

      int argc;
      WCHAR **argv =
          CommandLineToArgvW(
              lpCommandLine,
              &argc );

      if ( argc < 2 )
          CheckHR( NULL, E_INVALIDARG, true );

      if ( argc == 3 &&
           _wcsicmp( L"/CREATEJOB", argv[1] ) == 0 )
          CreateJob( argv[2] );

      else if ( argc == 4 &&
                _wcsicmp( L"/RESUMEJOB", argv[1] ) == 0 )
          ResumeJob( argv[2], argv[3] );

      else
          CheckHR( NULL, E_INVALIDARG, true );

      MSG msg;
      while( GetMessage( &msg, NULL, 0, 0 ) )
      {
          TranslateMessage( &msg );
          DispatchMessage( &msg );
      }

  }
  catch(_com_error error )
  {
      return -1;
  }

  return 0;
}



