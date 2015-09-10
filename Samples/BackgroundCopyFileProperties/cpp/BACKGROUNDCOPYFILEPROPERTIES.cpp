// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


/**
 * This sample demonstrates how to use the GetProperty method on the new
 * IBackgroundCopyFile5 interface to obtain the last set HTTP headers 
 * received for each file in a download job.
 *
 * The information in the HTTP headers could be used, fo example, to
 * determine the file type or when it last changed on the server.
 */


#include "stdafx.h"

#include <windows.h>
#include <bits.h>


#define ARRAY_LENGTH(x) (sizeof(x) / sizeof( *(x) ))


/**
 * Definition of constants
 */
static const unsigned int HALF_SECOND_AS_MILLISECONDS = 500;
static const unsigned int TWO_SECOND_LOOP = 2000 / HALF_SECOND_AS_MILLISECONDS;


/**
 * Simple data structure containing the remote and local names for a file.
 */
typedef struct
{
    LPWSTR RemoteFile;
    LPWSTR LocalFile;
} DOWNLOAD_FILE;


/**
 * Array containing multiple DOWNLOAD_FILE data structures representing the files
 * that will be added to the download job.
 */
DOWNLOAD_FILE FileList[] =
{
    { 
        L"http://download.microsoft.com/download/8/5/5/8551E67C-3D6E-4EAA-891B-6B46A97F179F/Live_Meeting_2007_Getting_Started_Guide_Service.doc", 
        L"c:\\temp\\data\\Getting started with Microsoft Office Live Meeting.doc"
    },
    { 
        L"http://download.microsoft.com/download/3/0/9/309778fd-659e-4853-b556-a14931cc3a2a/Live_Meeting_2007_Service_Quick_Reference_Card.doc", 
        L"c:\\temp\\data\\Live_Meeting_2007_Service_Quick_Reference_Card.doc"
    },
    {
        L"http://download.microsoft.com/download/D/2/2/D22D16C3-7637-41D3-99DA-10E7CEBAD290/SQL2008UpgradeTechnicalReferenceGuide.docx",
        L"c:\\temp\\data\\SQL2008UpgradeTechnicalReferenceGuide.docx"
    }
};


/**
 * Forward declaration of functions
 */

HRESULT GetBackgroundCopyManager(_Out_ IBackgroundCopyManager **Manager);
HRESULT CreateDownloadJob(_In_ LPCWSTR Name, _In_ IBackgroundCopyManager *Manager, _Out_ IBackgroundCopyJob **Job);
HRESULT MonitorJobProgress( _In_ IBackgroundCopyJob *Job );
HRESULT DisplayFileHeaders( _In_ IBackgroundCopyJob *Job );
VOID    DisplayProgress( _In_ IBackgroundCopyJob *Job );
VOID    DisplayHeaders( _In_ LPWSTR Headers );
VOID    DisplayError( _In_ IBackgroundCopyJob *Job );



/*
 * Main program entry point
 */
int _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr;
    IBackgroundCopyManager *Manager;

    // Get the BITS Background Copy Manager 
    hr = GetBackgroundCopyManager(&Manager);
    if( SUCCEEDED( hr ) )
    {
        IBackgroundCopyJob *Job;

        // Create a new download job
        hr = CreateDownloadJob( L"MyJob", Manager, &Job );
        if( SUCCEEDED(hr) )
        {
            // Add the files to the job
            for( int i=0; i<ARRAY_LENGTH(FileList); ++i)
            {
                hr = Job->AddFile(
                            FileList[i].RemoteFile,
                            FileList[i].LocalFile
                            );

                if( FAILED(hr) )
                {
                    printf(
                        "Error: Unable to add remote file '%ws' to the download job (error %08X).\n",
                        FileList[i].RemoteFile,
                        hr
                        );
                }
                else
                {
                    printf( 
                        "Downloading remote file '%ws' to local file '%ws'\n",
                        FileList[i].RemoteFile,
                        FileList[i].LocalFile
                        );
                }
            }


            // Start the job and display its progress
            hr = Job->Resume();
            if( FAILED(hr) )
            {
                printf( "ERROR: Unable to start the BITS download job (error code %08X).\n", hr );
            }
            else
            {
                MonitorJobProgress( Job );
            }

            // Release the BITS IBackgroundCopyJob interface
            Job->Release();
            Job = NULL;
        }

        // Release the IBackgroundCopyManager interface
        Manager->Release();
        Manager = NULL;
    }

    return 0;
}



/**
 * Gets a pointer to the BITS Background Copy Manager.
 *
 * If successful, it returns a success code and sets the
 * referenced IBackgroundCopyFileManager interface pointer
 * to a reference counted instance of the Background Copy Manager
 * interface.
 */
HRESULT
GetBackgroundCopyManager(_Out_ IBackgroundCopyManager **Manager)
{
    HRESULT hr;

    //Specify the appropriate COM threading model for your application.
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(
                    __uuidof(BackgroundCopyManager), 
                    NULL,
                    CLSCTX_LOCAL_SERVER,
                    __uuidof(IBackgroundCopyManager),
                    (void**) Manager
                    );
    }
    else
    {
        printf( "ERROR: Unable to initialize COM (error code %08X).\n", hr );
    }

    return hr;
}



/**
 * Creates a new download job with the specified name.
 */
HRESULT
CreateDownloadJob(_In_ LPCWSTR Name, _In_ IBackgroundCopyManager *Manager, _Out_ IBackgroundCopyJob **Job)
{
    HRESULT hr;
    GUID guid;

    hr = Manager->CreateJob(
                        Name,
                        BG_JOB_TYPE_DOWNLOAD,
                        &guid,
                        Job
                        );

    return hr;
}



/**
 * Monitors and displays the progress of the download job.
 *
 * A new status message is output whenever the job's status changes or,
 * when transferring data, every 2 seconds displays how much data
 * has been transferred.
 */
HRESULT
MonitorJobProgress( _In_ IBackgroundCopyJob *Job )
{
    HRESULT hr;
    LPWSTR JobName;
    BG_JOB_STATE State;
    int PreviousState = -1;
    bool Exit = false;
    int ProgressCounter = 0;

    hr = Job->GetDisplayName( &JobName );
    printf( "Progress report for download job '%ws'.\n", JobName );

    // display the download progress
    while( !Exit )
    {
        hr = Job->GetState( &State );

        if( State != PreviousState )
        {

            switch( State )
            {
            case BG_JOB_STATE_QUEUED:
                printf( "Job is in the queue and waiting to run.\n" );
                break;

            case BG_JOB_STATE_CONNECTING:
                printf( "BITS is trying to connect to the remote server.\n" );
                break;

            case BG_JOB_STATE_TRANSFERRING:
                printf( "BITS has started downloading data.\n" );
                DisplayProgress( Job );
                break;

            case BG_JOB_STATE_ERROR:
                printf( "ERROR: BITS has encountered a non-recoverable error (error code %08X).\n", GetLastError() );
                printf( "       Exiting job.\n" );
                Exit = true;
                break;

            case BG_JOB_STATE_TRANSIENT_ERROR:
                printf( "ERROR: BITS has encountered a recoverable error.\n" );
                DisplayError( Job );
                printf( "       Continuing to retry.\n" );
                break;

            case BG_JOB_STATE_TRANSFERRED:
                DisplayProgress( Job );
                printf( "The job has been successfully completed.\n" );
                printf( "Finalizing local files.\n" );
                Job->Complete();
                break;

            case BG_JOB_STATE_ACKNOWLEDGED:
                printf( "Finalization complete.\n" );
                Exit = true;
                break;

            case BG_JOB_STATE_CANCELLED:
                printf( "WARNING: The job has been cancelled.\n" );
                Exit = true;
                break;

            default:
                printf( "WARNING: Unknown BITS state %d.\n", State );
                Exit = true;
            }

            PreviousState = State;
        }

        else if (State == BG_JOB_STATE_TRANSFERRING )
        {
            // display job progress every 2 seconds
            if( ++ProgressCounter % TWO_SECOND_LOOP == 0 )
            {
                DisplayProgress( Job );
            }
        }

        Sleep(HALF_SECOND_AS_MILLISECONDS);
    }

    printf("\n");

    if( SUCCEEDED(hr) )
    {
        hr = DisplayFileHeaders( Job );
    }

    return hr;
}



/**
 * For each file in the job, obtains the (final) HTTP headers received from the
 * remote server that hosts the files and then displays the HTTP headers.
 */
HRESULT
DisplayFileHeaders( _In_ IBackgroundCopyJob *Job )
{
    HRESULT hr;
    IEnumBackgroundCopyFiles *FileEnumerator;

    printf( "Individual file information.\n" );

    hr = Job->EnumFiles( &FileEnumerator );
    if( FAILED(hr) )
    {
        printf( "WARNING: Unable to obtain an IEnumBackgroundCopyFiles interface.\n");
        printf( "         No further information can be provided about the files in the job.\n");
    }
    else
    {
        ULONG Count;

        hr = FileEnumerator->GetCount( &Count );
        if( FAILED(hr) )
        {
            printf("WARNING: Unable to obtain a count of the number of files in the job.\n" );
            printf( "        No further information can be provided about the files in the job.\n");
        }
        else
        {
            for( ULONG i=0; i < Count; ++i )
            {
                IBackgroundCopyFile *TempFile;

                hr = FileEnumerator->Next(1, &TempFile, NULL);
                if( FAILED(hr) )
                {
                    printf("WARNING: Unable to obtain an IBackgroundCopyFile interface for the next file in the job.\n" );
                    printf( "        No further information can be provided about this file.\n");
                }
                else
                {
                    IBackgroundCopyFile5 *File;
                    hr = TempFile->QueryInterface( __uuidof( IBackgroundCopyFile5 ), (void **) &File );
                    if( FAILED(hr) )
                    {
                        printf("WARNING: Unable to obtain an IBackgroundCopyFile5 interface for the file.\n" );
                        printf( "        No further information can be provided about this file.\n");
                    }
                    else
                    {
                        LPWSTR RemoteFileName;
                        hr = File->GetRemoteName( &RemoteFileName );
                        if( FAILED(hr) )
                        {
                            printf("WARNING: Unable to obtain the remote file name for this file.\n" );
                        }
                        else
                        {
                            printf("HTTP headers for remote file '%ws'\n", RemoteFileName );
                            CoTaskMemFree( RemoteFileName );
                            RemoteFileName = NULL;
                        }

                        BITS_FILE_PROPERTY_VALUE Value;
                        hr = File->GetProperty(
                                        BITS_FILE_PROPERTY_ID_HTTP_RESPONSE_HEADERS,
                                        &Value
                                        );
                        if( FAILED(hr) )
                        {
                            printf("WARNING: Unable to obtain the HTTP headers for this file.\n" );
                        }
                        else
                        {
                            if(Value.String)
							{
								DisplayHeaders( Value.String );
								CoTaskMemFree( Value.String );
								Value.String = NULL;
							}
                        }

                        File->Release();
                        File = NULL;
                    }

                    TempFile->Release();
                    TempFile = NULL;
                }
            }
        }

        FileEnumerator->Release();
        FileEnumerator = NULL;
    }

    return S_OK;
}


/**
 * Displays the current progress of the job in terms of the amount of data
 * and number of files transferred.
 */
VOID
DisplayProgress( _In_ IBackgroundCopyJob *Job )
{
    HRESULT hr;
    BG_JOB_PROGRESS Progress;

    hr = Job->GetProgress( &Progress );
    if( SUCCEEDED(hr) )
    {
        printf(
            "%llu of %llu bytes transferred (%lu of %lu files).\n",
            Progress.BytesTransferred, Progress.BytesTotal,
            Progress.FilesTransferred, Progress.FilesTotal
            );
    }
    else
    {
        printf( "ERROR: Unable to get job progress (error code %08X).\n", hr );
    }
}



/**
 * Parses the provided string containing HTTP headers,
 * splits them apart and displays them to the user.
 */
VOID
DisplayHeaders( _In_ LPWSTR Headers )
{
    printf("Headers: %ws\n", Headers );
}


VOID 
DisplayError( _In_ IBackgroundCopyJob *Job )
{
    HRESULT hr;
    IBackgroundCopyError *Error;
    LPWSTR ErrorDescription;

    hr = Job->GetError( &Error );
    if( FAILED(hr) )
    {
        printf( "WARNING: Error details are not available.\n");
    }
    else
    {
        hr = Error->GetErrorDescription( LANGIDFROMLCID(GetThreadLocale()), &ErrorDescription );
        if( SUCCEEDED(hr) )
        {
            printf( "   Error details: %ws\n", ErrorDescription );
            CoTaskMemFree( ErrorDescription );
        }
        Error->Release();
    }
}

