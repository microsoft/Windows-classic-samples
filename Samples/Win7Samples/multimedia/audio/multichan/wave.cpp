// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "stdafx.h"
#include "resource.h"
#include "wave.h"
#include <mmreg.h>
#include "ChildView.h"
#include <msacm.h>

// -----------------------------------------------------------------------------------------
/*
 *	WaveOpenFile	:	opens a wave file, validates format, extracts parameters
 */
HANDLE 
WaveOpenFile
(
    IN  LPCSTR          szFileName,
    OUT WAVEFORMATEX**  ppWaveFormatEx,
    OUT PULONG          pcbData
)
{
    HMMIO	        hmmio;              // mmio style file handle
    MMCKINFO        mmckinfoParent;	    // parent chunk info struct
    MMCKINFO        mmckinfoSubchunk;	// sub chunk
    HWND            hwnd = AfxGetMainWnd()->m_hWnd;

    HANDLE          hFile = INVALID_HANDLE_VALUE;
    ULONG	        cbFormat = 0;
    LONG            lDataPos = 0;

    // open the file
    if(!(hmmio = mmioOpen((LPSTR)szFileName, NULL, MMIO_READ | MMIO_ALLOCBUF))) 
    {
        return FALSE;
    }

    // verify that the file is of type "WAVE" 
    mmckinfoParent.fccType = MAKEFOURCC('W', 'A', 'V', 'E');
    if(mmioDescend(hmmio, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF))
    {
        // not a "WAVE" file!!
        MessageBox(hwnd, "WaveOpenFile:  Missing 'WAVE' chunk.This does not appear to be a wave file.", "MultiChan : Error opening wave file!", MB_ICONEXCLAMATION | MB_OK);
        goto _error_;
    }

    // find the "fmt " chunk (must be a subchunk of the "WAVE" chunk)
    mmckinfoSubchunk.ckid = MAKEFOURCC('f', 'm', 't', ' ');
    if(mmioDescend(hmmio, (LPMMCKINFO)&mmckinfoSubchunk, (LPMMCKINFO)&mmckinfoParent, MMIO_FINDCHUNK))
    {
        // file has no "fmt " chunk
        MessageBox(hwnd, "WaveOpenFile:  Missing 'fmt ' chunk", "MultiChan : Error opening wave file!", MB_ICONEXCLAMATION | MB_OK);
        goto _error_;
    }

    // get the size of the "fmt " chunk
    cbFormat = mmckinfoSubchunk.cksize;

    // allocate waveformat.  The "max" part ensures that the cbSize member of the WAVEFORMATEX exists and = 0
    *ppWaveFormatEx = (LPWAVEFORMATEX)LocalAlloc(LPTR, max(cbFormat, sizeof(WAVEFORMATEX)));
    if(!*ppWaveFormatEx)
    {
        // failed to allocate
        MessageBox(hwnd, "WaveOpenFile:  Failed to allocate memory", "MultiChan : Error opening wave file!", MB_ICONEXCLAMATION | MB_OK);
        goto _error_;
    }

    // read the "fmt " chunk
    if(mmioRead(hmmio, (HPSTR)*ppWaveFormatEx, cbFormat) != (LONG)cbFormat)
    {
        // failed to read the "fmt " chunk
        MessageBox(hwnd, "WaveOpenFile:  Error reading 'fmt ' chunk", "MultiChan : Error opening wave file!", MB_ICONEXCLAMATION | MB_OK);
        goto _error_;
    }

    // ascend out o' the "fmt " chunk
    mmioAscend(hmmio, &mmckinfoSubchunk, 0);

    // and find the data subchunk.  (current file pos should be at the beginning of the 
    // data chunk, but when you assume...)
    mmckinfoSubchunk.ckid = MAKEFOURCC('d', 'a', 't', 'a');
    if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
    {
        // wave file doesn't appear to have a data chunk
        MessageBox(hwnd, "WaveOpenFile:  Missing 'data' chunk", "MultiChan : Error opening wave file!", MB_ICONEXCLAMATION | MB_OK);
        goto _error_;
    }

    // get the size of the "data" subchunk
    *pcbData = mmckinfoSubchunk.cksize;
    if(0L == *pcbData)
    {
        // the data chunk contains no data
        MessageBox(hwnd, "WaveOpenFile:  Error reading 'data' chunk", "MultiChan : Error opening wave file!", MB_ICONEXCLAMATION | MB_OK);
        goto _error_;
    }

    // remember the data 
    lDataPos = mmioSeek(hmmio, 0, SEEK_CUR);
    mmioClose(hmmio, 0);

    //
    // now we are done with the mmio functions.  Reopen the file and seek to the data chunk using normal
    // file io routines
    //

    __try
    {
        hFile = 
            CreateFile
            ( 
                szFileName, 
                GENERIC_READ, 
                FILE_SHARE_READ,
                NULL, 
                OPEN_EXISTING, 
                FILE_ATTRIBUTE_NORMAL,
                NULL 
            );
    }    
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        MessageBox(hwnd, "WaveOpenFile : Handling exception thrown by CreateFile!", "MultiChan : Error opening wave file!", MB_ICONEXCLAMATION | MB_OK);
        goto _error_;
    }

    if (!IsValidHandle(hFile))
    {
        goto _error_;
    }

    SetFilePointer
    (
        hFile,
        lDataPos,  
        NULL, 
        FILE_BEGIN
    ); 

    return hFile;
    
_error_:
    SafeCloseHandle(hFile);
    if(hmmio)
    {
        mmioClose(hmmio, 0);
    }

    return INVALID_HANDLE_VALUE;
}

// -----------------------------------------------------------------------------------------
/*
 *	WaveSaveFile	:	creates and writes the mixed wave; returns success code
 */
BOOL
WaveSaveFile
(
    IN  LPCSTR          szFileName,
    OUT WAVEFORMATEX*   pWaveFormatEx,
    IN  PVOID           pvData,
    IN  ULONG           cbData
)
{
    HANDLE          hFile		= INVALID_HANDLE_VALUE;
    ULONG           cbWritten;
	HRSRC           hrsrc		= (HRSRC)INVALID_HANDLE_VALUE;
    void*           pv			= NULL;

    //
    // now we are done with the mmio functions.  Reopen the file and seek to the data chunk using normal
    // file io routines
    //

    __try
    {
        hFile = 
            CreateFile
            ( 
                szFileName, 
                GENERIC_WRITE, 
                0,
                NULL, 
                CREATE_ALWAYS, 
                FILE_ATTRIBUTE_NORMAL,
                NULL 
            );
    }    
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        MessageBox(AfxGetMainWnd()->m_hWnd, "WaveSaveFile : Handling exception thrown by CreateFile!", "MultiChan : Error creating wave file", MB_ICONEXCLAMATION | MB_OK);
        goto _error_;
    }

    if (!IsValidHandle(hFile))
        goto _error_;

	// 
	// write file header
	//

    // standard wave goo
    struct
    {
        DWORD   fourccData;
        DWORD   dwDataLength;
    } DataHeader;

    DataHeader.fourccData         = MAKEFOURCC('d','a','t','a');
    DataHeader.dwDataLength       = cbData;

    struct 
    {
        DWORD       dwRiff;
        DWORD       dwFileSize;
        DWORD       dwWave;
        DWORD       dwFormat;
        DWORD       dwFormatLength;
    }	FileHeader;

    FileHeader.dwRiff             = MAKEFOURCC('R','I','F','F');
    FileHeader.dwWave             = MAKEFOURCC('W','A','V','E');
    FileHeader.dwFormat           = MAKEFOURCC('f','m','t',' ');
    FileHeader.dwFormatLength     = sizeof(WAVEFORMATEX) + pWaveFormatEx->cbSize;
    FileHeader.dwFileSize         = sizeof(FileHeader) + FileHeader.dwFormatLength + sizeof(DataHeader) + cbData;

    if(!WriteFile(hFile, &FileHeader, sizeof(FileHeader), &cbWritten, NULL))
        goto _error_;

    // wave format
    if(!WriteFile(hFile, pWaveFormatEx, sizeof(WAVEFORMATEX) + pWaveFormatEx->cbSize, &cbWritten, NULL))
        goto _error_;

    // data chuck
    if(!WriteFile(hFile, &DataHeader, sizeof(DataHeader), &cbWritten, NULL))
        goto _error_;

    //
    // Write wave data
    //
    if(!WriteFile(hFile, pvData, cbData, &cbWritten, NULL))
        goto _error_;

    SafeCloseHandle( hFile );
    return TRUE;
    
_error_:
    MessageBox(NULL, "WaveSaveFile : Error Writing File", "MultiChan : Error!", MB_ICONEXCLAMATION | MB_OK);
    SafeCloseHandle( hFile );
    return FALSE;
}



// ===============================================================
/*
 *   WavePlayFileCB  
 *   callback for the waveOut functions
 *   notifies the originating window
 */
void 
CALLBACK   
WavePlayFileCB
(
    HWAVEOUT	hwo,
    UINT		uMsg,
    DWORD_PTR	dwInstance,
    DWORD_PTR	dwParam1,
    DWORD_PTR	dwParam2
)
{
    DWORD       mmr = MMSYSERR_BASE;

    ASSERT( ( CDialog * )dwInstance );

    switch( uMsg )
    {
        case    WOM_OPEN:
            {
                //
                //  notify window
                //
                (( CDialog * )dwInstance)->PostMessage( WM_START_PLAYBACK, 0, 0 );

                break;
            }
        case    WOM_DONE:
            {
                //
                //  notify window
                //
                (( CDialog * )dwInstance)->PostMessage( WM_STOP_PLAYBACK, 0, 0 );

                break;
            }

        case    WOM_CLOSE:
            {

                break;
            }


        default:
            ASSERT(0);
    }
    return;
}

/*
 *	WaveDisablePlayback
 *	prevent other channels from being played at this time
 */
void
WaveTogglePlayback
(
			CDlgSrc	*	pdlgSelf,
	const	BOOL		fSwitch
)
{
	CDlgSrc	*	pdlgSrc	= 0;
	//
	//	fSwitch toggles between disable (FALSE) and allow (TRUE)
	//

	for
	( 
		POSITION	position = g_listSources.GetHeadPosition();
		position;
	)
	{
		pdlgSrc = g_listSources.GetNext( position );
		//
		//	disable/enable the play button. the stop button is unaffected
		//
		pdlgSrc->m_fPlayable = fSwitch;
		(pdlgSrc->m_butPlay).EnableWindow( fSwitch );
	}

	//
	//	check target
	//
	if( pdlgSelf )
	{
		//	allow stop
		(pdlgSelf->m_butStop).EnableWindow( !fSwitch );

		//	destination may not be mixed already
		if( g_pdlgDest->m_fPlayable )
		{
			g_pdlgDest->m_fPlayable = fSwitch;
			(g_pdlgDest->m_butPlay).EnableWindow( fSwitch );
		}
		else
		{
			g_pdlgDest->m_fPlayable = fSwitch & (0 != g_pdlgDest->m_pbData);
			(g_pdlgDest->m_butPlay).EnableWindow( g_pdlgDest->m_fPlayable );
		}
	}
	else
	{
		g_pdlgDest->m_fPlayable = fSwitch;
		(g_pdlgDest->m_butPlay).EnableWindow( fSwitch );
		(g_pdlgDest->m_butStop).EnableWindow( !fSwitch );
	}


}	//	WaveTogglePlayback


// ----------------------------------------------------------------------------------
// TrapMMError
// return:          FALSE if it is an error, TRUE otherwise
// ----------------------------------------------------------------------------------
BOOL 
TrapMMError
(
    MMRESULT mmRes,
    LPCSTR   szAPI
)
{
	//	no error
    if(MMSYSERR_NOERROR == mmRes)
    {
        return( TRUE );
    } 

    HWND    hwndMain        = AfxGetMainWnd()->m_hWnd;

	const size_t ERROR_STRING_SIZE_CCH = 128; 

    char    szError[ERROR_STRING_SIZE_CCH]	= "";
    char    szErrMsg[ERROR_STRING_SIZE_CCH]	= "";

	HRESULT hr = S_OK;

	//	basic mmsys error
	if( MMSYSERR_LASTERROR >= mmRes )
	{
		if( MMSYSERR_NOMEM == waveOutGetErrorText( mmRes, szError, 128 ) )
		{
			hr = StringCchPrintfA( szErrMsg, ERROR_STRING_SIZE_CCH, "Insufficient memory to complete the task.");
		}
        else
        {
    		hr = StringCchPrintfA( szErrMsg, ERROR_STRING_SIZE_CCH, "ERROR : %s returned : %s", szAPI, szError );
        }
	}
    else
    {
	    //	other errors
	    switch( mmRes )
	    {
		    case	WAVERR_BADFORMAT	:
		    {
			    hr = StringCchCopyA( szError, ERROR_STRING_SIZE_CCH, " : Attempt to open an unsupported waveform-audio format." );
			    break;
		    }

		    case	WAVERR_STILLPLAYING	:
		    {
			    hr = StringCchCopyA( szError, ERROR_STRING_SIZE_CCH, " : There is another waveform resource still playing." );
			    break;
		    }

		    case	WAVERR_UNPREPARED	:
		    {
			    hr = StringCchCopyA( szError, ERROR_STRING_SIZE_CCH, " : Header is not prepared." );
			    break;
		    }

		    case	WAVERR_SYNC			:
		    {
			    hr = StringCchCopyA( szError, ERROR_STRING_SIZE_CCH, " : Device is synchronous." );
			    break;
		    }

		    case	ACMERR_NOTPOSSIBLE	:
		    {
			    hr = StringCchCopyA( szError, ERROR_STRING_SIZE_CCH, " : The specified action is not possible in the current context." );
			    break;
		    }

		    case	ACMERR_BUSY	:
		    {
			    hr = StringCchCopyA( szError, ERROR_STRING_SIZE_CCH, " : The driver is currently in use and cannot be reused." );
			    break;
		    }
		    
		    case	ACMERR_UNPREPARED	:
		    {
			    hr = StringCchCopyA( szError, ERROR_STRING_SIZE_CCH, " : The stream header is not currently prepared." );
			    break;
		    }

		    case	ACMERR_CANCELED		:
		    {
			    hr = StringCchCopyA( szError, ERROR_STRING_SIZE_CCH, " : The action has been canceled." );
			    break;
		    }

		    default	:
		    {
			    //	default
			    hr = StringCchPrintfA( szError, ERROR_STRING_SIZE_CCH, "an unrecognized error code (%d).", mmRes );
			    break;
		    }
        }	//	switch

		if (SUCCEEDED(hr))
		{
		    hr = StringCchPrintfA( szErrMsg, ERROR_STRING_SIZE_CCH, "ERROR : %s returned %s.", szAPI, szError );
        }
	}

    // Display the string.
    if (SUCCEEDED(hr))
    {
    	MessageBox(hwndMain, szErrMsg, "MultiChan : Error!", MB_ICONEXCLAMATION | MB_OK);
    }

	return( FALSE );
}
