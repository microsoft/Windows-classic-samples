// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//------------------------------------------------------------------------------
// File: Wave.c
//
// Desc: DirectShow sample code - Wave library routines.
//------------------------------------------------------------------------------


/*==========================================================================
 *
 *      This file is used for loading/saving waves, and reading and
 *      writing waves in smaller blocks.
 *      Uses WaveOpenFile, WaveReadFile and WaveCloseReadFile for
 *      single block access to reading wave files.
 *      Uses WaveCreateFile, WaveWriteFile, WaveCloseWriteFile for
 *      single block access for writing  wave files.
 *      Uses WaveLoadFile to load a whole wave file into memory.
 *      Uses WaveSaveFile to save a whole wave file into memory.
 *
 ***************************************************************************/

#pragma warning(disable:4100)  // Disable C4100: unreferenced formal parameter
#pragma warning(disable:4213)  // Disable C4213: cast on lvalue
#pragma warning(disable:4115)  // Disable C4115: (from VC6.0 header rpsasync.h)

#include <windows.h>
#include <mmsystem.h>
#include "wave.h"
#include "debug.h"
#include "windowsx.h"

#pragma warning(default:4115)  // Reset to default behavior for C4115


/* ROUTINES */
/* -------------------------------------------------------*/

/* This function will open a wave input file and prepare it for reading,
 * so the data can be easily
 * read with WaveReadFile. Returns 0 if successful, the error code if not.
 *      pszFileName - Input filename to load.
 *      phmmioIn    - Pointer to handle which will be used
 *          for further mmio routines.
 *      ppwfxInfo   - Ptr to ptr to WaveFormatEx structure
 *          with all info about the file.                        
 *      
*/
int WaveOpenFile(
    TCHAR*pszFileName,                              // (IN)
    HMMIO *phmmioIn,                                // (OUT)
    WAVEFORMATEX **ppwfxInfo,                       // (OUT)
    MMCKINFO *pckInRIFF                             // (OUT)
            )
{
    HMMIO           hmmioIn;
    MMCKINFO        ckIn;           // chunk info. for general use.
    PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.       
    WORD            cbExtraAlloc;   // Extra bytes for waveformatex 
    int             nError;         // Return value.

    // Initialization
    *ppwfxInfo = NULL;
    nError = 0;
    hmmioIn = NULL;

    if ((hmmioIn = mmioOpen(pszFileName, NULL, MMIO_ALLOCBUF | MMIO_READ)) == NULL)
    {
        nError = ER_CANNOTOPEN;
        goto ERROR_READING_WAVE;
    }

    if ((nError = (int)mmioDescend(hmmioIn, pckInRIFF, NULL, 0)) != 0)
    {
        goto ERROR_READING_WAVE;
    }

    if ((pckInRIFF->ckid != FOURCC_RIFF) || (pckInRIFF->fccType != mmioFOURCC('W', 'A', 'V', 'E')))
    {
        nError = ER_NOTWAVEFILE;
        goto ERROR_READING_WAVE;
    }

    /* Search the input file for for the 'fmt ' chunk.     */
    ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if ((nError = (int)mmioDescend(hmmioIn, &ckIn, pckInRIFF, MMIO_FINDCHUNK)) != 0)
    {
        goto ERROR_READING_WAVE;                
    }

    /* Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
     * if there are extra parameters at the end, we'll ignore them */
    if (ckIn.cksize < (long) sizeof(PCMWAVEFORMAT))
    {
        nError = ER_NOTWAVEFILE;
        goto ERROR_READING_WAVE;
    }

    /* Read the 'fmt ' chunk into <pcmWaveFormat>.*/
    if (mmioRead(hmmioIn, (HPSTR) &pcmWaveFormat, (long) sizeof(pcmWaveFormat)) != (long) sizeof(pcmWaveFormat))
    {
        nError = ER_CANNOTREAD;
        goto ERROR_READING_WAVE;
    }

    // Allocate the waveformatex, but if its not pcm
    // format, read the next word, and thats how many extra
    // bytes to allocate.
    if (pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM)
        cbExtraAlloc = 0;                               
    else
    {
        // Read in length of extra bytes.
        if (mmioRead(hmmioIn, (HPSTR) &cbExtraAlloc,
            (long) sizeof(cbExtraAlloc)) != (long) sizeof(cbExtraAlloc))
        {
            nError = ER_CANNOTREAD;
            goto ERROR_READING_WAVE;
        }
    }

    // Now allocate that waveformatex structure.
    if ((*ppwfxInfo = GlobalAlloc(GMEM_FIXED, sizeof(WAVEFORMATEX)+cbExtraAlloc)) == NULL)
    {
        nError = ER_MEM;
        goto ERROR_READING_WAVE;
    }

    // Copy the bytes from the pcm structure to the waveformatex structure
    memcpy(*ppwfxInfo, &pcmWaveFormat, sizeof(pcmWaveFormat));
    (*ppwfxInfo)->cbSize = cbExtraAlloc;

    // Read those extra bytes into the structure, if cbExtraAlloc != 0.
    if (cbExtraAlloc != 0)
    {
        if (mmioRead(hmmioIn, (HPSTR) (((BYTE*)&((*ppwfxInfo)->cbSize))+sizeof(cbExtraAlloc)),
            (long) (cbExtraAlloc)) != (long) (cbExtraAlloc))
        {
            nError = ER_NOTWAVEFILE;
            goto ERROR_READING_WAVE;
        }
    }

    /* Ascend the input file out of the 'fmt ' chunk. */
    if ((nError = mmioAscend(hmmioIn, &ckIn, 0)) != 0)
    {
        goto ERROR_READING_WAVE;
    }

    goto TEMPCLEANUP;               

ERROR_READING_WAVE:
    if (*ppwfxInfo != NULL)
    {
        GlobalFree(*ppwfxInfo);
        *ppwfxInfo = NULL;
    }               

    if (hmmioIn != NULL)
    {
        mmioClose(hmmioIn, 0);
        hmmioIn = NULL;
    }

TEMPCLEANUP:
    *phmmioIn = hmmioIn;

    return(nError);

}

/*  This routine has to be called before WaveReadFile as it searchs for the 
    chunk to descend into for reading, that is, the 'data' chunk.  For 
    simplicity, this used to be in the open routine, but was taken out and 
    moved to a separate routine so there was more control on the chunks that 
    are before the data chunk, such as 'fact', etc... */

int WaveStartDataRead(
                    HMMIO *phmmioIn,
                    MMCKINFO *pckIn,
                    MMCKINFO *pckInRIFF
                    )
{
    int nError=0;

    // Perform a seek...
    if ((nError = mmioSeek(*phmmioIn, 
                           pckInRIFF->dwDataOffset + sizeof(FOURCC), SEEK_SET)) == -1)
    {
        // ASSERT(FALSE);
    }

    nError = 0;

    // Search the input file for for the 'data' chunk.
    pckIn->ckid = mmioFOURCC('d', 'a', 't', 'a');
    nError = mmioDescend(*phmmioIn, pckIn, pckInRIFF, MMIO_FINDCHUNK);

    return(nError);
}


/*  This will read wave data from the wave file.  Makre sure we're descended into
    the data chunk, else this will fail bigtime!
    hmmioIn         - Handle to mmio.
    cbRead          - # of bytes to read.   
    pbDest          - Destination buffer to put bytes.
    cbActualRead- # of bytes actually read.     
*/

int WaveReadFile(
        HMMIO hmmioIn,                          // IN
        UINT cbRead,                            // IN           
        BYTE *pbDest,                           // IN
        MMCKINFO *pckIn,                        // IN.
        UINT *cbActualRead                      // OUT.        
        )
{
    MMIOINFO    mmioinfoIn;         // current status of <hmmioIn>
    int         nError=0;
    UINT        cT, cbDataIn;

    if ((nError = mmioGetInfo(hmmioIn, &mmioinfoIn, 0)) != 0)
    {
        goto ERROR_CANNOT_READ;
    }

    cbDataIn = cbRead;
    if (cbDataIn > pckIn->cksize) 
        cbDataIn = pckIn->cksize;       

    pckIn->cksize -= cbDataIn;

    for (cT = 0; cT < cbDataIn; cT++)
    {
        /* Copy the bytes from the io to the buffer. */
        if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
        {
            if ((nError = mmioAdvance(hmmioIn, &mmioinfoIn, MMIO_READ)) != 0)
            {
                goto ERROR_CANNOT_READ;
            } 
            if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
            {
                nError = ER_CORRUPTWAVEFILE;
                goto ERROR_CANNOT_READ;
            }
        }

        // Actual copy.
        *((BYTE*)pbDest+cT) = *((BYTE*)mmioinfoIn.pchNext)++;                                                                                                   
    }

    if ((nError = mmioSetInfo(hmmioIn, &mmioinfoIn, 0)) != 0)
    {
        goto ERROR_CANNOT_READ;
    }

    *cbActualRead = cbDataIn;
    goto FINISHED_READING;

ERROR_CANNOT_READ:
    *cbActualRead = 0;

FINISHED_READING:
    return(nError);

}

/*  This will close the wave file openned with WaveOpenFile.  
    phmmioIn - Pointer to the handle to input MMIO.
    ppwfxSrc - Pointer to pointer to WaveFormatEx structure.

    Returns 0 if successful, non-zero if there was a warning.

*/

int WaveCloseReadFile(
            HMMIO *phmmio,                          // IN
            WAVEFORMATEX **ppwfxSrc                 // IN
            )
{
    if (*ppwfxSrc != NULL)
    {
        GlobalFree(*ppwfxSrc);
        *ppwfxSrc = NULL;
    }

    if (*phmmio != NULL)
    {
        mmioClose(*phmmio, 0);
        *phmmio = NULL;
    }

    return(0);
}


/*  This routine will create a wave file for writing.  This will automatically overwrite any
    existing file with the same name, so be careful and check before hand!!!
    pszFileName     - Pointer to filename to write.
    phmmioOut               - Pointer to HMMIO handle that is used for further writes
    pwfxDest                - Valid waveformatex destination structure.
    pckOut                  - Pointer to be set with the MMCKINFO.
    pckOutRIFF              - Pointer to be set with the RIFF info.

*/

int WaveCreateFile(
            TCHAR*pszFileName,                      // (IN)
            HMMIO *phmmioOut,                       // (OUT)
            WAVEFORMATEX *pwfxDest,                 // (IN)
            MMCKINFO *pckOut,                       // (OUT)
            MMCKINFO *pckOutRIFF                    // (OUT)
            )
{
    int       nError;        // Return value
    DWORD     dwFactChunk;   // Contains the actual fact chunk. Garbage until WaveCloseWriteFile.
    MMCKINFO  ckOut1;

    dwFactChunk = (DWORD)-1;
    nError = 0;

    *phmmioOut = mmioOpen(pszFileName, NULL,
        MMIO_ALLOCBUF | MMIO_READWRITE|MMIO_CREATE);

    if (*phmmioOut == NULL)
    {
        nError = ER_CANNOTWRITE;
        goto ERROR_CANNOT_WRITE;    // cannot save WAVE file
    }

    /* Create the output file RIFF chunk of form type 'WAVE'. */
    pckOutRIFF->fccType = mmioFOURCC('W', 'A', 'V', 'E');       
    pckOutRIFF->cksize = 0; 
    if ((nError = mmioCreateChunk(*phmmioOut, pckOutRIFF, MMIO_CREATERIFF)) != 0)
    {
        goto ERROR_CANNOT_WRITE;    // cannot write file, probably
    }

    /* We are now descended into the 'RIFF' chunk we just created.
     * Now create the 'fmt ' chunk. Since we know the size of this chunk,
     * specify it in the MMCKINFO structure so MMIO doesn't have to seek
     * back and set the chunk size after ascending from the chunk.
     */
    pckOut->ckid = mmioFOURCC('f', 'm', 't', ' ');
    pckOut->cksize = sizeof(PCMWAVEFORMAT);   // we know the size of this ck.
    if ((nError = mmioCreateChunk(*phmmioOut, pckOut, 0)) != 0)
    {
        goto ERROR_CANNOT_WRITE;    // cannot write file, probably
    }

    /* Write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type. */
    if (pwfxDest->wFormatTag == WAVE_FORMAT_PCM)
    {
        if (mmioWrite(*phmmioOut, (HPSTR) pwfxDest, sizeof(PCMWAVEFORMAT))
            != sizeof(PCMWAVEFORMAT))
        {
            nError = ER_CANNOTWRITE;
            goto ERROR_CANNOT_WRITE;    // cannot write file, probably
        }
    }
    else
    {
        // Write the variable length size.
        if ((UINT)mmioWrite(*phmmioOut, (HPSTR) pwfxDest, sizeof(*pwfxDest)+pwfxDest->cbSize)
            != (sizeof(*pwfxDest)+pwfxDest->cbSize))
        {
            nError = ER_CANNOTWRITE;
            goto ERROR_CANNOT_WRITE;    // cannot write file, probably
        } 
    }  

    /* Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk. */
    if ((nError = mmioAscend(*phmmioOut, pckOut, 0)) != 0)
    {
        goto ERROR_CANNOT_WRITE;    // cannot write file, probably
    }

    // Now create the fact chunk, not required for PCM but nice to have.  This is filled
    // in when the close routine is called.
    ckOut1.ckid = mmioFOURCC('f', 'a', 'c', 't');
    ckOut1.cksize = 0;
    if ((nError = mmioCreateChunk(*phmmioOut, &ckOut1, 0)) != 0)
    {
        goto ERROR_CANNOT_WRITE;    // cannot write file, probably
    }

    if (mmioWrite(*phmmioOut, (HPSTR)&dwFactChunk, sizeof(dwFactChunk)) != sizeof(dwFactChunk))
    {
        nError = ER_CANNOTWRITE;
        goto ERROR_CANNOT_WRITE;
    }

    // Now ascend out of the fact chunk...
    if ((nError = mmioAscend(*phmmioOut, &ckOut1, 0)) != 0)
    {
        nError = ER_CANNOTWRITE;        // cannot write file, probably
        goto ERROR_CANNOT_WRITE;
    }

    goto DONE_CREATE;

ERROR_CANNOT_WRITE:
    // Maybe delete the half-written file?  For now, its good to leave the
    // file there for debugging.

DONE_CREATE:
    return(nError);
}


/*  This routine has to be called before any data is written to the wave output file, via wavewritefile.  This
    sets up the data to write, and creates the data chunk.
*/

int WaveStartDataWrite(
                HMMIO *phmmioOut,                       // (IN)
                MMCKINFO *pckOut,                       // (IN)
                MMIOINFO *pmmioinfoOut                  // (OUT)
                )
{
    int nError=0;

    /* Create the 'data' chunk that holds the waveform samples.  */
    pckOut->ckid = mmioFOURCC('d', 'a', 't', 'a');
    pckOut->cksize = 0;

    if ((nError = mmioCreateChunk(*phmmioOut, pckOut, 0)) != 0)
    {
        return nError;
    }

    nError = mmioGetInfo(*phmmioOut, pmmioinfoOut, 0);
    return(nError);
}


/* This routine will write out data to a wave file. 
    hmmioOut                - Handle to hmmioOut filled by WaveCreateFile
    cbWrite                 - # of bytes to write out.
    pbSrc                   - Pointer to source.
    pckOut                  - pointer to ckOut filled by WaveCreateFile
    cbActualWrite   - # of actual bytes written.
    pmmioinfoOut    - Pointer to mmioinfoOut filled by WaveCreateFile.

    Returns 0 if successful, else the error code.

 */

int WaveWriteFile(
        HMMIO hmmioOut,                         // (IN)
        UINT cbWrite,                           // (IN)
        BYTE *pbSrc,                            // (IN)
        MMCKINFO *pckOut,                       // (IN)
        UINT *cbActualWrite,                    // (OUT)
        MMIOINFO *pmmioinfoOut                  // (IN)
        )
{
    int  nError=0;
    UINT cT;

    *cbActualWrite = 0;

    for (cT=0; cT < cbWrite; cT++)
    {       
        if (pmmioinfoOut->pchNext == pmmioinfoOut->pchEndWrite)
        {
            pmmioinfoOut->dwFlags |= MMIO_DIRTY;
            if ((nError = mmioAdvance(hmmioOut, pmmioinfoOut, MMIO_WRITE)) != 0)
            {
                goto ERROR_CANNOT_WRITE;
            }
        }

        *((BYTE*)pmmioinfoOut->pchNext)++ = *((BYTE*)pbSrc+cT);
        (*cbActualWrite)++;
    }

ERROR_CANNOT_WRITE:
    // What to do here?  Well, for now, nothing, just return that error.
    // (maybe delete the file later?)

    return(nError);
}


/*  This routine will close a wave file used for writing.  Returns 0 if successful, else
    the error code.
    phmmioOut       - Pointer to mmio handle for saving.
    pckOut          - Pointer to the MMCKINFO for saving.
    pckOutRiff      - Pointer to the riff MMCKINFO for saving.
    pmmioinfoOut    - Pointer to mmioinfo for saving.
    cSamples        - # of samples saved, for the fact chunk.  For PCM, this isn't used but
                      will be written anyway, so this can be zero as long as programs ignore
                      this field when they load PCM formats.
*/

int WaveCloseWriteFile(
            HMMIO *phmmioOut,               // (IN)
            MMCKINFO *pckOut,               // (IN)
            MMCKINFO *pckOutRIFF,           // (IN)
            MMIOINFO *pmmioinfoOut,         // (IN)
            DWORD cSamples                  // (IN)
            )
{
    int nError=0;

    if (*phmmioOut == NULL)
        return(0);

    pmmioinfoOut->dwFlags |= MMIO_DIRTY;
    if ((nError = mmioSetInfo(*phmmioOut, pmmioinfoOut, 0)) != 0)
    {
        // cannot flush, probably...
        goto ERROR_CANNOT_WRITE;                                
    }

    /* Ascend the output file out of the 'data' chunk -- this will cause
    * the chunk size of the 'data' chunk to be written.
    */
    if ((nError = mmioAscend(*phmmioOut, pckOut, 0)) != 0)
        goto ERROR_CANNOT_WRITE;    // cannot write file, probably


    // Do this here instead...
    if ((nError = mmioAscend(*phmmioOut, pckOutRIFF, 0)) != 0)
        goto ERROR_CANNOT_WRITE;    // cannot write file, probably


    nError = mmioSeek(*phmmioOut, 0, SEEK_SET);
    if ((nError = (int)mmioDescend(*phmmioOut, pckOutRIFF, NULL, 0)) != 0)
    {
        goto ERROR_CANNOT_WRITE;
    }

    nError = 0;
    pckOut->ckid = mmioFOURCC('f', 'a', 'c', 't');
    if ((nError = mmioDescend(*phmmioOut, pckOut, pckOutRIFF, MMIO_FINDCHUNK)) == 0)
    {
        // If it didn't fail, write the fact chunk out, if it failed, not critical, just
        // assert (below).
        nError = mmioWrite(*phmmioOut, (HPSTR)&cSamples, sizeof(DWORD));
        nError = mmioAscend(*phmmioOut, pckOut, 0); 
        nError = 0;
    }
    else
    {
        nError = 0;
    }

    /* Ascend the output file out of the 'RIFF' chunk -- this will cause
    * the chunk size of the 'RIFF' chunk to be written.
    */
    if ((nError = mmioAscend(*phmmioOut, pckOutRIFF, 0)) != 0)
        goto ERROR_CANNOT_WRITE;    // cannot write file, probably


ERROR_CANNOT_WRITE:
    if (*phmmioOut != NULL)
    {
        mmioClose(*phmmioOut, 0);
        *phmmioOut = NULL;
    }

    return(nError);

}


/*  This routine will copy from a source wave file to a destination wave file all those useless chunks
    (well, the ones useless to conversions, etc --> apparently people use them!).  The source will be
    seeked to the begining, but the destination has to be at a current pointer to put the new chunks.
    This will also seek     back to the start of the wave riff header at the end of the routine.

    phmmioIn                - Pointer to input mmio file handle.
    pckIn                   - Pointer to a nice ckIn to use.
    pckInRiff               - Pointer to the main riff.
    phmmioOut               - Pointer to output mmio file handle.
    pckOut                  - Pointer to nice ckOut to use.
    pckOutRiff              - Pointer to the main riff.


    Returns 0 if successful, else the error code.  If this routine fails, it still attemps to seek back to
    the start of the wave riff header, though this too could be unsuccessful.
*/

int WaveCopyUselessChunks(
                    HMMIO *phmmioIn, 
                    MMCKINFO *pckIn, 
                    MMCKINFO *pckInRiff, 
                    HMMIO *phmmioOut, 
                    MMCKINFO *pckOut, 
                    MMCKINFO *pckOutRiff)
{
    int  nError=0;

    // First seek to the start of the file, not including the riff header...
    if ((nError = mmioSeek(*phmmioIn, pckInRiff->dwDataOffset + sizeof(FOURCC), SEEK_SET)) == -1)
    {
        nError = ER_CANNOTREAD;
        goto ERROR_IN_PROC;
    }

    nError = 0;                     

    while (mmioDescend(*phmmioIn, pckIn, pckInRiff, 0) == 0)
    {
        //  quickly check for corrupt RIFF file--don't ascend past end!        
        if ((pckIn->dwDataOffset + pckIn->cksize) > 
            (pckInRiff->dwDataOffset + pckInRiff->cksize))
            goto ERROR_IN_PROC;

        switch (pckIn->ckid)
        {                   
            //  explicitly skip these...            
            case mmioFOURCC('f', 'm', 't', ' '):
                break;

            case mmioFOURCC('d', 'a', 't', 'a'):
                break;

            case mmioFOURCC('f', 'a', 'c', 't'):
                break;

            case mmioFOURCC('J', 'U', 'N', 'K'):
                break;

            case mmioFOURCC('P', 'A', 'D', ' '):
                break;

            case mmioFOURCC('c', 'u', 'e', ' '):
                break;                                                  

            //  copy chunks that are OK to copy            
            case mmioFOURCC('p', 'l', 's', 't'):
                // although without the 'cue' chunk, it doesn't make much sense
                riffCopyChunk(*phmmioIn, *phmmioOut, pckIn);
                break;

            case mmioFOURCC('D', 'I', 'S', 'P'):
                riffCopyChunk(*phmmioIn, *phmmioOut, pckIn);
                break;

            //  don't copy unknown chunks
            default:
                break;
        }


        //  step up to prepare for next chunk..        
        mmioAscend(*phmmioIn, pckIn, 0);
    }

    ERROR_IN_PROC:
    {
        int nErrorT;
        // Seek back to riff header     
        nErrorT = mmioSeek(*phmmioIn, pckInRiff->dwDataOffset + sizeof(FOURCC), SEEK_SET);
    }

    return(nError);
}


/** BOOL RIFFAPI riffCopyChunk(HMMIO hmmioSrc, HMMIO hmmioDst, const LPMMCKINFO lpck)
 *
 *  DESCRIPTION:
 *      
 *
 *  ARGUMENTS:
 *      (LPWAVECONVCB lpwc, LPMMCKINFO lpck)
 *
 *  RETURN (BOOL NEAR PASCAL):
 *
 *
 *  NOTES:
 *
 **  */

BOOL riffCopyChunk(HMMIO hmmioSrc, HMMIO hmmioDst, const LPMMCKINFO lpck)
{
    MMCKINFO    ck;
    HPSTR       hpBuf=0;

    hpBuf = (HPSTR)GlobalAllocPtr(GHND, lpck->cksize);
    if (!hpBuf)
        return (FALSE);

    ck.ckid   = lpck->ckid;
    ck.cksize = lpck->cksize;
    if (mmioCreateChunk(hmmioDst, &ck, 0))
        goto rscc_Error;

    if (mmioRead(hmmioSrc, hpBuf, lpck->cksize) != (LONG)lpck->cksize)
        goto rscc_Error;

    if (mmioWrite(hmmioDst, hpBuf, lpck->cksize) != (LONG)lpck->cksize)
        goto rscc_Error;

    if (mmioAscend(hmmioDst, &ck, 0))
        goto rscc_Error;

    if (hpBuf)
        GlobalFreePtr(hpBuf);

    return (TRUE);

rscc_Error:

    if (hpBuf)
        GlobalFreePtr(hpBuf);

    return (FALSE);
}



/*  This routine loads a full wave file into memory.
    szFileName      -       sz Filename
    cbSize          -       Size of loaded wave (returned)
    cSamples        -       # of samples loaded.
    ppwfxInfo       -       Pointer to pointer to waveformatex structure.  The wfx structure
                            IS ALLOCATED by this routine!  Make sure to free it!
    ppbData         -       Pointer to a byte pointer (globalalloc) which is allocated by this 
                            routine.  Make sure to free it!

    Returns 0 if successful, else the error code.
*/

int WaveLoadFile(
            TCHAR*pszFileName,                      // (IN)
            UINT *cbSize,                           // (OUT)
            WAVEFORMATEX **ppwfxInfo,               // (OUT)
            BYTE **ppbData                          // (OUT)
            )
{
    HMMIO     hmmioIn;        
    MMCKINFO  ckInRiff;
    MMCKINFO  ckIn;
    int       nError;
    UINT      cbActualRead;

    *ppbData = NULL;
    *ppwfxInfo = NULL;
    *cbSize = 0;

    if ((nError = WaveOpenFile(pszFileName, &hmmioIn, ppwfxInfo, &ckInRiff)) != 0)
    {
        goto ERROR_LOADING;
    }

    if ((nError = WaveStartDataRead(&hmmioIn, &ckIn, &ckInRiff)) != 0)
    {
        goto ERROR_LOADING;
    }

    // Ok, size of wave data is in ckIn, allocate that buffer.
    if ((*ppbData = (BYTE *)GlobalAlloc(GMEM_FIXED, ckIn.cksize)) == NULL)
    {
        nError = ER_MEM;
        goto ERROR_LOADING;
    }

    if ((nError = WaveReadFile(hmmioIn, ckIn.cksize, *ppbData, &ckIn, &cbActualRead)) != 0)
    {
        goto ERROR_LOADING;
    }        

    *cbSize = cbActualRead;
    goto DONE_LOADING;

ERROR_LOADING:
    if (*ppbData != NULL)
    {
        GlobalFree(*ppbData);
        *ppbData = NULL;
    }
    if (*ppwfxInfo != NULL)
    {
        GlobalFree(*ppwfxInfo);
        *ppwfxInfo = NULL;
    }

DONE_LOADING:
    // Close the wave file. 
    if (hmmioIn != NULL)
    {
        mmioClose(hmmioIn, 0);
        hmmioIn = NULL;
    }

    return(nError);
}

/*  This routine saves a wave file in currently in memory.
    pszFileName     -   FileName to save to.  Automatically overwritten, be careful!
    cbSize          -       Size in bytes to write.
    cSamples        -       # of samples to write, used to make the fact chunk. (if !PCM)
    pwfxDest        -       Pointer to waveformatex structure.
    pbData          -       Pointer to the data.
*/      

int WaveSaveFile(
                TCHAR*pszFileName,                      // (IN)
                UINT cbSize,                            // (IN)
                DWORD cSamples,                         // (IN) 
                WAVEFORMATEX *pwfxDest,                 // (IN)
                BYTE *pbData                            // (IN)
                )
{

    HMMIO     hmmioOut;
    MMCKINFO  ckOut;
    MMCKINFO  ckOutRIFF;
    MMIOINFO  mmioinfoOut;
    UINT      cbActualWrite;
    int       nError;

    if ((nError = WaveCreateFile(pszFileName, &hmmioOut, pwfxDest, &ckOut, &ckOutRIFF)) != 0)
    {
        goto ERROR_SAVING;
    }

    if ((nError = WaveStartDataWrite(&hmmioOut, &ckOut, &mmioinfoOut)) != 0)
    {
        goto ERROR_SAVING;
    }

    if ((nError = WaveWriteFile(hmmioOut, cbSize, pbData, &ckOut, &cbActualWrite, &mmioinfoOut)) != 0)
    {
        goto ERROR_SAVING;
    }

    if ((nError = WaveCloseWriteFile(&hmmioOut, &ckOut, &ckOutRIFF, &mmioinfoOut, cSamples)) != 0)
    {
        goto ERROR_SAVING;
    }       

ERROR_SAVING:

    return(nError);         
}


