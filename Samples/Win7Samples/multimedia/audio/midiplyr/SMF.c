// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
* SMF.C
*
* MIDI File access routines.
*
*****************************************************************************/
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <memory.h>
#include <strsafe.h>
#include "muldiv32.h" 
#include "smf.h"
#include "smfi.h"
#include "debug.h"

PRIVATE SMFRESULT FNLOCAL smfInsertParmData(
    PSMF                    pSmf,
    TICKS                   tkDelta,                                            
    LPMIDIHDR               lpmh);

/*****************************************************************************
*
* smfOpenFile
*
* This function opens a MIDI file for access. 
*
* psofs                     - Specifies the file to open and associated
*                             parameters. Contains a valid HSMF handle
*                             on success.
*
* Returns
*   SMF_SUCCESS The specified file was opened.
*
*   SMF_OPEN_FAILED The specified file could not be opened because it
*     did not exist or could not be created on the disk.
*
*   SMF_INVALID_FILE The specified file was corrupt or not a MIDI file.
* 
*   SMF_NO_MEMORY There was insufficient memory to open the file.
*
*   SMF_INVALID_PARM The given flags or time division in the
*     SMFOPENFILESTRUCT were invalid.
* 
*****************************************************************************/
SMFRESULT FNLOCAL smfOpenFile(
    PSMFOPENFILESTRUCT      psofs)
{
    HMMIO                   hmmio = (HMMIO)NULL;
    PSMF                    pSmf;
    SMFRESULT               smfrc = SMF_SUCCESS;
    MMIOINFO                mmioinfo;
    MMCKINFO                ckRIFF;
    MMCKINFO                ckDATA;

    assert(psofs != NULL);
    assert(psofs->pstrName != NULL);
    
    /* Verify that the file can be opened or created
    */
    _fmemset(&mmioinfo, 0, sizeof(mmioinfo));

    hmmio = mmioOpen(psofs->pstrName, &mmioinfo, MMIO_READ|MMIO_ALLOCBUF);
    if ((HMMIO)NULL == hmmio)
    {
        DPF(1, "smfOpenFile: mmioOpen failed!");
        return SMF_OPEN_FAILED;
    }

    /* Now see if we can create the handle structure
    */
    pSmf = (PSMF)LocalAlloc(LPTR, sizeof(SMF));
    if (NULL == pSmf)
    {
        DPF(1, "smfOpenFile: LocalAlloc failed!");
        smfrc = SMF_NO_MEMORY;
        goto smf_Open_File_Cleanup;
    }

    StringCchCopyA(pSmf->szName, 128, psofs->pstrName);  // 128 comes from definition of SMF struct in SMFI.h
    pSmf->fdwSMF = 0;
    pSmf->pTempoMap = NULL;

    /* Pull the entire file into a block of memory. 
    */
    _fmemset(&ckRIFF, 0, sizeof(ckRIFF));
    
    if (0 == mmioDescend(hmmio, &ckRIFF, NULL, MMIO_FINDRIFF) &&
        ckRIFF.fccType == FOURCC_RMID)
    {
        ckDATA.ckid = FOURCC_data;
        
        if (0 == mmioDescend(hmmio, &ckDATA, &ckRIFF, MMIO_FINDCHUNK))
        {
            pSmf->cbImage   = ckDATA.cksize;
        }
        else
        {
            DPF(1, "smfOpenFile: Could not descend into RIFF DATA chunk!");
            smfrc = SMF_INVALID_FILE;
            goto smf_Open_File_Cleanup;
        }
    }
    else
    {
        mmioSeek(hmmio, 0L, SEEK_SET);
        
        pSmf->cbImage = mmioSeek(hmmio, 0L, SEEK_END);
        mmioSeek(hmmio, 0L, SEEK_SET);
    }
    
    if (NULL == (pSmf->hpbImage = GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE, pSmf->cbImage)))
    {
        DPF(1, "smfOpenFile: No memory for image! [%08lX]", pSmf->cbImage);
        smfrc = SMF_NO_MEMORY;
        goto smf_Open_File_Cleanup;
    }
 
    if (pSmf->cbImage != (DWORD)mmioRead(hmmio, pSmf->hpbImage, pSmf->cbImage))
    {
        DPF(1, "smfOpenFile: Read error on image!");
        smfrc = SMF_INVALID_FILE;
        goto smf_Open_File_Cleanup;
    }

    /* If the file exists, parse it just enough to pull out the header and
    ** build a track index.
    */
    smfrc = smfBuildFileIndex((PSMF BSTACK *)&pSmf);
    if (MMSYSERR_NOERROR != smfrc)
    {
        DPF(1, "smfOpenFile: smfBuildFileIndex failed! [%lu]", (DWORD)smfrc);
    }

smf_Open_File_Cleanup:

    mmioClose(hmmio, 0);

    if (SMF_SUCCESS != smfrc)
    {
        if (NULL != pSmf)
        {
            if (NULL != pSmf->hpbImage)
            {
                GlobalFreePtr(pSmf->hpbImage);
            }
            
            LocalFree((HLOCAL)pSmf);
        }
    }
    else
    {
        psofs->hSmf = (HSMF)pSmf;
    }
    
    return smfrc;
}

/*****************************************************************************
*
* smfCloseFile
*
* This function closes an open MIDI file.
*
* hSmf                      - The handle of the open file to close.
*
* Returns
*   SMF_SUCCESS The specified file was closed.
*   SMF_INVALID_PARM The given handle was not valid.
*
* Any track handles opened from this file handle are invalid after this
* call.
*        
*****************************************************************************/
SMFRESULT FNLOCAL smfCloseFile(
    HSMF                    hSmf)
{
    PSMF                    pSmf        = (PSMF)hSmf;
    
    assert(pSmf != NULL);
    
    /*
    ** Free up handle memory 
    */
    
    if (NULL != pSmf->hpbImage)
        GlobalFreePtr(pSmf->hpbImage);
    
    LocalFree((HLOCAL)pSmf);
    
    return SMF_SUCCESS;
}

/******************************************************************************
*
* smfGetFileInfo This function gets information about the MIDI file.
*
* hSmf                      - Specifies the open MIDI file to inquire about.
*
* psfi                      - A structure which will be filled in with
*                             information about the file.
*
* Returns
*   SMF_SUCCESS Information was gotten about the file.
*   SMF_INVALID_PARM The given handle was invalid.
*
*****************************************************************************/
SMFRESULT FNLOCAL smfGetFileInfo(
    HSMF                    hSmf,
    PSMFFILEINFO            psfi)
{
    PSMF                    pSmf = (PSMF)hSmf;

    assert(pSmf != NULL);
    assert(psfi != NULL);

    /* 
    ** Just fill in the structure with useful information.
    */
    psfi->dwTracks      = pSmf->dwTracks;
    psfi->dwFormat      = pSmf->dwFormat;
    psfi->dwTimeDivision= pSmf->dwTimeDivision;
    psfi->tkLength      = pSmf->tkLength;
    
    return SMF_SUCCESS;
}

/******************************************************************************
*
* smfTicksToMillisecs
*
* This function returns the millisecond offset into the file given the
* tick offset.
*
* hSmf                      - Specifies the open MIDI file to perform
*                             the conversion on.
*
* tkOffset                  - Specifies the tick offset into the stream
*                             to convert.
*
* Returns the number of milliseconds from the start of the stream.
*
* The conversion is performed taking into account the file's time division and
* tempo map from the first track. Note that the same millisecond value
* might not be valid at a later time if the tempo track is rewritten.
*
*****************************************************************************/
DWORD FNLOCAL smfTicksToMillisecs(
    HSMF                    hSmf,
    TICKS                   tkOffset)
{
    PSMF                    pSmf            = (PSMF)hSmf;
    PTEMPOMAPENTRY          pTempo;
    UINT                    idx;
    UINT                    uSMPTE;
    DWORD                   dwTicksPerSec;

    assert(pSmf != NULL);

    if (tkOffset > pSmf->tkLength)
    {
        DPF(1, "sTTM: Clipping ticks to file length!");
        tkOffset = pSmf->tkLength;
    }

    /* SMPTE time is easy -- no tempo map, just linear conversion
    ** Note that 30-Drop means nothing to us here since we're not
    ** converting to a colonized format, which is where dropping
    ** happens.
    */
    if (pSmf->dwTimeDivision & 0x8000)
    {
        uSMPTE = -(int)(char)((pSmf->dwTimeDivision >> 8)&0xFF);
        if (29 == uSMPTE)
            uSMPTE = 30;
        
        dwTicksPerSec = (DWORD)uSMPTE *
                        (DWORD)(BYTE)(pSmf->dwTimeDivision & 0xFF);
        
        return (DWORD)muldiv32(tkOffset, 1000L, dwTicksPerSec);
    }
       
    /* Walk the tempo map and find the nearest tick position. Linearly
    ** calculate the rest (using MATH.ASM)
    */

    pTempo = pSmf->pTempoMap;
    assert(pTempo != NULL);
    
    for (idx = 0; idx < pSmf->cTempoMap; idx++, pTempo++)
        if (tkOffset < pTempo->tkTempo)
            break;
    pTempo--;

    /* pTempo is the tempo map entry preceding the requested tick offset.
    */

    return pTempo->msBase + muldiv32(tkOffset-pTempo->tkTempo,
                                     pTempo->dwTempo,
                                     1000L*pSmf->dwTimeDivision);
    
}


/******************************************************************************
*
* smfMillisecsToTicks
*
* This function returns the nearest tick offset into the file given the
* millisecond offset.
*
* hSmf                      - Specifies the open MIDI file to perform the
*                             conversion on.
*
* msOffset                  - Specifies the millisecond offset into the stream
*                             to convert.
*
* Returns the number of ticks from the start of the stream.
*
* The conversion is performed taking into account the file's time division and
* tempo map from the first track. Note that the same tick value
* might not be valid at a later time if the tempo track is rewritten.
* If the millisecond value does not exactly map to a tick value, then
* the tick value will be rounded down.
*
*****************************************************************************/
TICKS FNLOCAL smfMillisecsToTicks(
    HSMF                    hSmf,
    DWORD                   msOffset)
{
    PSMF                    pSmf            = (PSMF)hSmf;
    PTEMPOMAPENTRY          pTempo;
    UINT                    idx;
    UINT                    uSMPTE;
    DWORD                   dwTicksPerSec;
    TICKS                   tkOffset;

    assert(pSmf != NULL);
    
    /* SMPTE time is easy -- no tempo map, just linear conversion
    ** Note that 30-Drop means nothing to us here since we're not
    ** converting to a colonized format, which is where dropping
    ** happens.
    */
    if (pSmf->dwTimeDivision & 0x8000)
    {
        uSMPTE = -(int)(char)((pSmf->dwTimeDivision >> 8)&0xFF);
        if (29 == uSMPTE)
            uSMPTE = 30;
        
        dwTicksPerSec = (DWORD)uSMPTE *
                        (DWORD)(BYTE)(pSmf->dwTimeDivision & 0xFF);

        return (DWORD)muldiv32(msOffset, dwTicksPerSec, 1000L);
    }
    
    /* Walk the tempo map and find the nearest millisecond position. Linearly
    ** calculate the rest (using MATH.ASM)
    */
    pTempo = pSmf->pTempoMap;
    assert(pTempo != NULL);
    
    for (idx = 0; idx < pSmf->cTempoMap; idx++, pTempo++)
        if (msOffset < pTempo->msBase)
            break;
    pTempo--;

    /* pTempo is the tempo map entry preceding the requested tick offset.
    */

    tkOffset = pTempo->tkTempo + muldiv32(msOffset-pTempo->msBase,
                                     1000L*pSmf->dwTimeDivision,
                                     pTempo->dwTempo);
    
    if (tkOffset > pSmf->tkLength)
    {
        DPF(1, "sMTT: Clipping ticks to file length!");
        tkOffset = pSmf->tkLength;
    }

    return tkOffset;
}

/******************************************************************************
*
* smfReadEvents
*
* This function reads events from a track.
*
* hSmf                      - Specifies the file to read data from.
*
* lpmh                      - Contains information about the buffer to fill.
*
* tkMax                     - Specifies a cutoff point in the stream
*                             beyond which events will not be read.        
*
* Return@rdes
*   SMF_SUCCESS The events were successfully read.
*   SMF_END_OF_TRACK There are no more events to read in this track.
*   SMF_INVALID_FILE A disk error occured on the file.
* 
* @xref <f smfWriteEvents>
*****************************************************************************/
SMFRESULT FNLOCAL smfReadEvents(
    HSMF                    hSmf,
    LPMIDIHDR               lpmh,
    TICKS                   tkMax)
{
    PSMF                    pSmf = (PSMF)hSmf;
    SMFRESULT               smfrc;
    EVENT                   event;
    LPDWORD                 lpdw;
    DWORD                   dwTempo;

    assert(pSmf != NULL);
    assert(lpmh != NULL);

    /* 
    ** Read events from the track and pack them into the buffer in polymsg
    ** format.
    ** 
    ** If a SysEx or meta would go over a buffer boundry, split it.
    */ 
    lpmh->dwBytesRecorded = 0;
    if (pSmf->dwPendingUserEvent)
    {
        smfrc = smfInsertParmData(pSmf, (TICKS)0, lpmh);
        if (SMF_SUCCESS != smfrc)
        {
            DPF(1, "smfInsertParmData() -> %u", (UINT)smfrc);
            return smfrc;
        }
    }
    
    lpdw = (LPDWORD)(lpmh->lpData + lpmh->dwBytesRecorded);

    if (pSmf->fdwSMF & SMF_F_EOF)
    {
        return SMF_END_OF_FILE;
    }

    while(TRUE)
    {
        assert(lpmh->dwBytesRecorded <= lpmh->dwBufferLength);
        
        /* If we know ahead of time we won't have room for the
        ** event, just break out now. We need 2 DWORD's for the
        ** terminator event and at least 2 DWORD's for any
        ** event we might store - this will allow us a full
        ** short event or the delta time and stub for a long
        ** event to be split.
        */
        if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 4*sizeof(DWORD))
        {
            break;
        }

        smfrc = smfGetNextEvent(pSmf, (SPEVENT)&event, tkMax);
        if (SMF_SUCCESS != smfrc)
        {
            /* smfGetNextEvent doesn't set this because smfSeek uses it
            ** as well and needs to distinguish between reaching the
            ** seek point and reaching end-of-file.
            **
            ** To the user, however, we present the selection between
            ** their given tkBase and tkEnd as the entire file, therefore
            ** we want to translate this into EOF.
            */
            if (SMF_REACHED_TKMAX == smfrc)
            {
                pSmf->fdwSMF |= SMF_F_EOF;
            }
            
            DPF(1, "smfReadEvents: smfGetNextEvent() -> %u", (UINT)smfrc);
            break;
        }

        
        if (MIDI_SYSEX > EVENT_TYPE(event))
        {
            *lpdw++ = (DWORD)event.tkDelta;
            *lpdw++ = 0;
            *lpdw++ = (((DWORD)MEVT_SHORTMSG)<<24) |
                      ((DWORD)EVENT_TYPE(event)) |
                      (((DWORD)EVENT_CH_B1(event)) << 8) |
                      (((DWORD)EVENT_CH_B2(event)) << 16);
            
            lpmh->dwBytesRecorded += 3*sizeof(DWORD);
        }
        else if (MIDI_META == EVENT_TYPE(event) &&
                 MIDI_META_EOT == EVENT_META_TYPE(event))
        {
            /* These are ignoreable since smfReadNextEvent()
            ** takes care of track merging
            */
        }
        else if (MIDI_META == EVENT_TYPE(event) &&
                 MIDI_META_TEMPO == EVENT_META_TYPE(event))
        {
            if (event.cbParm != 3)
            {
                DPF(1, "smfReadEvents: Corrupt tempo event");
                return SMF_INVALID_FILE;
            }

            dwTempo = (((DWORD)MEVT_TEMPO)<<24)|
                      (((DWORD)event.hpbParm[0])<<16)|
                      (((DWORD)event.hpbParm[1])<<8)|
                      ((DWORD)event.hpbParm[2]);

            *lpdw++ = (DWORD)event.tkDelta;
            *lpdw++ = 0;
            *lpdw++ = dwTempo;

            lpmh->dwBytesRecorded += 3*sizeof(DWORD);
        }
        else if (MIDI_META != EVENT_TYPE(event))
        {
            /* Must be F0 or F7 system exclusive or FF meta
            ** that we didn't recognize
            */
            pSmf->cbPendingUserEvent = event.cbParm;
            pSmf->hpbPendingUserEvent = event.hpbParm;
            pSmf->fdwSMF &= ~SMF_F_INSERTSYSEX;

            switch(EVENT_TYPE(event))
            {
                case MIDI_SYSEX:
                    pSmf->fdwSMF |= SMF_F_INSERTSYSEX;
            
                    ++pSmf->cbPendingUserEvent;

                    /* Falling through...
                    */

                case MIDI_SYSEXEND:
                    pSmf->dwPendingUserEvent = ((DWORD)MEVT_LONGMSG) << 24;
                    break;
            }

            smfrc = smfInsertParmData(pSmf, event.tkDelta, lpmh);
            if (SMF_SUCCESS != smfrc)
            {
                DPF(1, "smfInsertParmData[2] %u", (UINT)smfrc);
                return smfrc;
            }

            lpdw = (LPDWORD)(lpmh->lpData + lpmh->dwBytesRecorded);
        }
    }

    return (pSmf->fdwSMF & SMF_F_EOF) ? SMF_END_OF_FILE : SMF_SUCCESS;
}

/******************************************************************************
*
* smfInsertParmData
*
* Inserts pending long data from a track into the given buffer.
*
* pSmf                      - Specifies the file to read data from.
*
* tkDelta                   - Specfices the tick delta for the data.
*
* lpmh                      - Contains information about the buffer to fill.
*
* Returns
*   SMF_SUCCESS The events were successfully read.
*   SMF_INVALID_FILE A disk error occured on the file.
* 
* Fills as much data as will fit while leaving room for the buffer
* terminator.
*
* If the long data is depleted, resets pSmf->dwPendingUserEvent so
* that the next event may be read.
*
*****************************************************************************/
PRIVATE SMFRESULT FNLOCAL smfInsertParmData(
    PSMF                    pSmf,
    TICKS                   tkDelta,                                            
    LPMIDIHDR               lpmh)
{
    DWORD                   dwLength;
    DWORD                   dwRounded;
    LPDWORD                 lpdw;

    assert(pSmf != NULL);
    assert(lpmh != NULL);
    
    /* Can't fit 4 DWORD's? (tkDelta + stream-id + event + some data)
    ** Can't do anything.
    */
    assert(lpmh->dwBufferLength >= lpmh->dwBytesRecorded);
    
    if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 4*sizeof(DWORD))
    {
        if (0 == tkDelta)
            return SMF_SUCCESS;

        /* If we got here with a real delta, that means smfReadEvents screwed
        ** up calculating left space and we should flag it somehow.
        */
        DPF(1, "Can't fit initial piece of SysEx into buffer!");
        return SMF_INVALID_FILE;
    }

    lpdw = (LPDWORD)(lpmh->lpData + lpmh->dwBytesRecorded);

    dwLength = lpmh->dwBufferLength - lpmh->dwBytesRecorded - 3*sizeof(DWORD);
    dwLength = min(dwLength, pSmf->cbPendingUserEvent);

    *lpdw++ = (DWORD)tkDelta;
    *lpdw++ = 0L;
    *lpdw++ = (pSmf->dwPendingUserEvent & 0xFF000000L) | (dwLength & 0x00FFFFFFL);

    dwRounded = (dwLength + 3) & (~3L);
    
    if (pSmf->fdwSMF & SMF_F_INSERTSYSEX)
    {
        *((LPBYTE)lpdw)++ = MIDI_SYSEX;
        pSmf->fdwSMF &= ~SMF_F_INSERTSYSEX;
        --dwLength;
        --pSmf->cbPendingUserEvent;
    }

    if (dwLength & 0x80000000L)
    {
        DPF(1, "dwLength %08lX  dwBytesRecorded %08lX  dwBufferLength %08lX", dwLength, lpmh->dwBytesRecorded, lpmh->dwBufferLength);
        DPF(1, "cbPendingUserEvent %08lX  dwPendingUserEvent %08lX dwRounded %08lX", pSmf->cbPendingUserEvent, pSmf->dwPendingUserEvent, dwRounded);
        DPF(1, "Offset into MIDI image %08lX", (DWORD)(pSmf->hpbPendingUserEvent - pSmf->hpbImage));
        DPF(1, "!hmemcpy is about to fault");
    }

    hmemcpy(lpdw, pSmf->hpbPendingUserEvent, dwLength);
    if (0 == (pSmf->cbPendingUserEvent -= dwLength))
        pSmf->dwPendingUserEvent = 0;

    lpmh->dwBytesRecorded += 3*sizeof(DWORD) + dwRounded;

    return SMF_SUCCESS;
}

/******************************************************************************
*
* smfSeek
*
* This function moves the file pointer within a track
* and gets the state of the track at the new position. It returns a buffer of
* state information which can be used to set up to play from the new position.
*
* hSmf                      - Handle of file to seek within
*
* tkPosition                - The position to seek to in the track.
*         
* lpmh                      - A buffer to contain the state information.
*
* Returns
*   SMF_SUCCESS | The state was successfully read.
*   SMF_END_OF_TRACK | The pointer was moved to end of track and no state
*     information was returned.
*   SMF_INVALID_PARM | The given handle or buffer was invalid.
*   SMF_NO_MEMORY | There was insufficient memory in the given buffer to
*     contain all of the state data.
*
* The state information in the buffer includes patch changes, tempo changes,
* time signature, key signature, 
* and controller information. Only the most recent of these paramters before
* the current position will be stored. The state buffer will be returned
* in polymsg format so that it may be directly transmitted over the MIDI
* bus to bring the state up to date.
*
* The buffer is mean to be sent as a streaming buffer; i.e. immediately
* followed by the first data buffer. If the requested tick position
* does not exist in the file, the last event in the buffer
* will be a MEVT_NOP with a delta time calculated to make sure that
* the next stream event plays at the proper time.
*
* The meta events (tempo, time signature, key signature) will be the
* first events in the buffer if they exist.
* 
* Use smfGetStateMaxSize to determine the maximum size of the state
* information buffer. State information that will not fit into the given
* buffer will be lost.
*
* On return, the dwBytesRecorded field of lpmh will contain the
* actual number of bytes stored in the buffer.
*
*****************************************************************************/

typedef struct tag_keyframe
{
    /*
    ** Meta events. All FF's indicates never seen.
    */
    BYTE        rbTempo[3];

    /*
    ** MIDI channel messages. FF indicates never seen.
    */
    BYTE        rbProgram[16];
    BYTE        rbControl[16*120];
}   KEYFRAME,
    FAR *PKEYFRAME;

#define KF_EMPTY ((BYTE)0xFF)

SMFRESULT FNLOCAL smfSeek(
    HSMF                    hSmf,
    TICKS                   tkPosition,
    LPMIDIHDR               lpmh)
{
    PSMF                    pSmf    = (PSMF)hSmf;
    PTRACK                  ptrk;
    DWORD                   idxTrack;
    SMFRESULT               smfrc;
    EVENT                   event;
    LPDWORD                 lpdw;
    BYTE                    bEvent;
    UINT                    idx;
    UINT                    idxChannel;
    UINT                    idxController;
    
    static KEYFRAME         kf;

    _fmemset(&kf, 0xFF, sizeof(kf));
    
    pSmf->tkPosition = 0;
    pSmf->fdwSMF &= ~SMF_F_EOF;
    
    for (ptrk = pSmf->rTracks, idxTrack = pSmf->dwTracks; idxTrack--; ptrk++)
    {
        ptrk->pSmf              = pSmf;
        ptrk->tkPosition        = 0;
        ptrk->cbLeft            = ptrk->smti.cbLength;
        ptrk->hpbImage          = pSmf->hpbImage + ptrk->idxTrack;
        ptrk->bRunningStatus    = 0;
        ptrk->fdwTrack          = 0;
    }

    while (SMF_SUCCESS == (smfrc = smfGetNextEvent(pSmf, (SPEVENT)&event, tkPosition)))
    {
        if (MIDI_META == (bEvent = EVENT_TYPE(event)))
        {
            if (EVENT_META_TYPE(event) == MIDI_META_TEMPO)
            {
                if (event.cbParm != sizeof(kf.rbTempo))
                    return SMF_INVALID_FILE;

                hmemcpy((HPBYTE)kf.rbTempo, event.hpbParm, event.cbParm);
            }
        }
        else switch(bEvent & 0xF0)
        {
            case MIDI_PROGRAMCHANGE:
                kf.rbProgram[bEvent & 0x0F] = EVENT_CH_B1(event);
                break;

            case MIDI_CONTROLCHANGE:
                kf.rbControl[(((WORD)bEvent & 0x0F)*120) + EVENT_CH_B1(event)] =
                    EVENT_CH_B2(event);
                break;
        }
    }

    if (SMF_REACHED_TKMAX != smfrc)
    {
        return smfrc;
    }

    /* Build lpmh from keyframe
    */
    lpmh->dwBytesRecorded = 0;
    lpdw = (LPDWORD)lpmh->lpData;

    /* Tempo change event?
    */
    if (KF_EMPTY != kf.rbTempo[0] ||
        KF_EMPTY != kf.rbTempo[1] ||
        KF_EMPTY != kf.rbTempo[2])
    {
        if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 3*sizeof(DWORD))
            return SMF_NO_MEMORY;

        *lpdw++ = 0;
        *lpdw++ = 0;
        *lpdw++ = (((DWORD)kf.rbTempo[0])<<16)|
                  (((DWORD)kf.rbTempo[1])<<8)|
                  ((DWORD)kf.rbTempo[2])|
                  (((DWORD)MEVT_TEMPO) << 24);

        lpmh->dwBytesRecorded += 3*sizeof(DWORD);
    }

    /* Program change events?
    */
    for (idx = 0; idx < 16; idx++)
    {
        if (KF_EMPTY != kf.rbProgram[idx])
        {
            if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 3*sizeof(DWORD))
                return SMF_NO_MEMORY;

            *lpdw++ = 0;
            *lpdw++ = 0;
            *lpdw++ = (((DWORD)MEVT_SHORTMSG) << 24)      |
                      ((DWORD)MIDI_PROGRAMCHANGE)         |
                      ((DWORD)idx)                        |
                      (((DWORD)kf.rbProgram[idx]) << 8);

            lpmh->dwBytesRecorded += 3*sizeof(DWORD);
        }
    }

    /* Controller events?
    */
    idx = 0;
    for (idxChannel = 0; idxChannel < 16; idxChannel++)
    {
        for (idxController = 0; idxController < 120; idxController++)
        {
            if (KF_EMPTY != kf.rbControl[idx])
            {
                if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 3*sizeof(DWORD))
                    return SMF_NO_MEMORY;

                *lpdw++ = 0;
                *lpdw++ = 0;
                *lpdw++ = (((DWORD)MEVT_SHORTMSG << 24)     |
                          ((DWORD)MIDI_CONTROLCHANGE)       |
                          ((DWORD)idxChannel)               |
                          (((DWORD)idxController) << 8)     |
                          (((DWORD)kf.rbControl[idx]) << 16));


                lpmh->dwBytesRecorded += 3*sizeof(DWORD);
            }

            idx++;
        }
    }

    /* Force all tracks to be at tkPosition. We are guaranteed that
    ** all tracks will be past the event immediately preceding tkPosition;
    ** this will force correct delta-ticks to be generated so that events
    ** on all tracks will line up properly on a seek into the middle of the
    ** file.
    */
    for (ptrk = pSmf->rTracks, idxTrack = pSmf->dwTracks; idxTrack--; ptrk++)
    {
        ptrk->tkPosition        = tkPosition;
    }
    
    return SMF_SUCCESS;
}

/******************************************************************************
*
* smfGetStateMaxSize
*
* This function returns the maximum sizeof buffer that is needed to
* hold the state information returned by f smfSeek.
*
* pdwSize                   - Gets the size in bytes that should be allocated
*                             for the state buffer.
*
* Returns the state size in bytes.
*
*****************************************************************************/
DWORD FNLOCAL smfGetStateMaxSize(
    VOID)
{
    return  3*sizeof(DWORD) +           /* Tempo                */
            3*16*sizeof(DWORD) +        /* Patch changes        */  
            3*16*120*sizeof(DWORD) +    /* Controller changes   */
            3*sizeof(DWORD);            /* Time alignment NOP   */
}

