// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
* SMFRead.C
*
* MIDI File access routines.
*
*****************************************************************************/
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <memory.h>
#include "muldiv32.h" 
#include "smf.h"
#include "smfi.h"
#include "debug.h"

PRIVATE UINT grbChanMsgLen[] =
{ 
    0,                      /* 0x   not a status byte   */
    0,                      /* 1x   not a status byte   */
    0,                      /* 2x   not a status byte   */
    0,                      /* 3x   not a status byte   */
    0,                      /* 4x   not a status byte   */
    0,                      /* 5x   not a status byte   */
    0,                      /* 6x   not a status byte   */
    0,                      /* 7x   not a status byte   */
    3,                      /* 8x   Note off            */
    3,                      /* 9x   Note on             */
    3,                      /* Ax   Poly pressure       */
    3,                      /* Bx   Control change      */
    2,                      /* Cx   Program change      */
    2,                      /* Dx   Chan pressure       */
    3,                      /* Ex   Pitch bend change   */
    0,                      /* Fx   SysEx (see below)   */             
} ;

/******************************************************************************
*
* smfBuildFileIndex
*
* Preliminary parsing of a MIDI file.
*
* ppSmf                     - Pointer to a returned SMF structure if the
*                             file is successfully parsed.
*
* Returns
*   SMF_SUCCESS The events were successfully read.
*   SMF_NO_MEMORY Out of memory to build key frames.
*   SMF_INVALID_FILE A disk or parse error occured on the file.
* 
* This function validates the format of and existing MIDI or RMI file
* and builds the handle structure which will refer to it for the
* lifetime of the instance.
*  
* The file header information will be read and verified, and
* smfBuildTrackIndices will be called on every existing track
* to build keyframes and validate the track format.
*
*****************************************************************************/
SMFRESULT FNLOCAL smfBuildFileIndex(
    PSMF BSTACK *           ppSmf)
{
    SMFRESULT               smfrc;
    UNALIGNED CHUNKHDR *    pCh;
    FILEHDR FAR *           pFh;
    DWORD                   idx;
    PSMF                    pSmf,
                            pSmfTemp;
    PTRACK                  pTrk;
    WORD                    wMemory;
    DWORD                   dwLeft;
    HPBYTE                  hpbImage;
    
    DWORD                   idxTrack;
    EVENT                   event;
    BOOL                    fFirst;
    DWORD                   dwLength;
    HLOCAL                  hLocal;
    PTEMPOMAPENTRY          pTempo;

    assert(ppSmf != NULL);

    pSmf = *ppSmf;

    assert(pSmf != NULL);

    /* MIDI data image is already in hpbImage (already extracted from
    ** RIFF header if necessary).
    */

    /* Validate MIDI header
    */
    dwLeft   = pSmf->cbImage;
    hpbImage = pSmf->hpbImage;
    
    if (dwLeft < sizeof(CHUNKHDR))
        return SMF_INVALID_FILE;

    pCh = (CHUNKHDR FAR *)hpbImage;

    dwLeft   -= sizeof(CHUNKHDR);
    hpbImage += sizeof(CHUNKHDR);
    
    if (pCh->fourccType != FOURCC_MThd)
        return SMF_INVALID_FILE;

    dwLength = DWORDSWAP(pCh->dwLength);
    if (dwLength < sizeof(FILEHDR) || dwLength > dwLeft)
        return SMF_INVALID_FILE;

    pFh = (FILEHDR FAR *)hpbImage;

    dwLeft   -= dwLength;
    hpbImage += dwLength;
    
    pSmf->dwFormat       = (DWORD)(WORDSWAP(pFh->wFormat));
    pSmf->dwTracks       = (DWORD)(WORDSWAP(pFh->wTracks));
    pSmf->dwTimeDivision = (DWORD)(WORDSWAP(pFh->wDivision));

    /*
    ** We've successfully parsed the header. Now try to build the track
    ** index.
    ** 
    ** We only check out the track header chunk here; the track will be
    ** preparsed after we do a quick integretiy check.
    */
    wMemory = sizeof(SMF) + (WORD)(pSmf->dwTracks*sizeof(TRACK));
    pSmfTemp = (PSMF)LocalReAlloc((HLOCAL)pSmf, wMemory, LMEM_MOVEABLE|LMEM_ZEROINIT);

    if (NULL == pSmfTemp)
    {
        DPF(1, "No memory for extended pSmf");
        return SMF_NO_MEMORY;
    }

    pSmf = *ppSmf = pSmfTemp;
    pTrk = pSmf->rTracks;
    
    for (idx=0; idx<pSmf->dwTracks; idx++)
    {
        if (dwLeft < sizeof(CHUNKHDR))
            return SMF_INVALID_FILE;

        pCh = (CHUNKHDR FAR *)hpbImage;

        dwLeft   -= sizeof(CHUNKHDR);
        hpbImage += sizeof(CHUNKHDR);

        if (pCh->fourccType != FOURCC_MTrk)
            return SMF_INVALID_FILE;
        
        pTrk->idxTrack      = (DWORD)(hpbImage - pSmf->hpbImage);
        pTrk->smti.cbLength = DWORDSWAP(pCh->dwLength);

        if (pTrk->smti.cbLength > dwLeft)
        {
            DPF(1, "Track longer than file!");
            return SMF_INVALID_FILE;
        }

        dwLeft   -= pTrk->smti.cbLength;
        hpbImage += pTrk->smti.cbLength;

        pTrk++;
    }

    /* File looks OK. Now preparse, doing the following:
    ** (1) Build tempo map so we can convert to/from ticks quickly
    ** (2) Determine actual tick length of file
    ** (3) Validate all events in all tracks
    */ 
    pSmf->tkPosition = 0;
    pSmf->fdwSMF &= ~SMF_F_EOF;
    
    for (pTrk = pSmf->rTracks, idxTrack = pSmf->dwTracks; idxTrack--; pTrk++)
    {
        pTrk->pSmf              = pSmf;
        pTrk->tkPosition        = 0;
        pTrk->cbLeft            = pTrk->smti.cbLength;
        pTrk->hpbImage          = pSmf->hpbImage + pTrk->idxTrack;
        pTrk->bRunningStatus    = 0;
        pTrk->fdwTrack          = 0;
    }

    while (SMF_SUCCESS == (smfrc = smfGetNextEvent(pSmf, (EVENT BSTACK *)&event, MAX_TICKS)))
    {
        if (MIDI_META == event.abEvent[0] && 
            MIDI_META_TEMPO == event.abEvent[1])
        {
            if (3 != event.cbParm)
            {
                return SMF_INVALID_FILE;
            }

            if (pSmf->cTempoMap == pSmf->cTempoMapAlloc)
            {
                if (NULL != pSmf->hTempoMap)
                {
                    LocalUnlock(pSmf->hTempoMap);
                }
                
                pSmf->cTempoMapAlloc += C_TEMPO_MAP_CHK;
                fFirst = FALSE;
                if (0 == pSmf->cTempoMap)
                {
                    hLocal = LocalAlloc(LHND, (UINT)(pSmf->cTempoMapAlloc*sizeof(TEMPOMAPENTRY)));
                    fFirst = TRUE;
                }
                else
                {
                    hLocal = LocalReAlloc(pSmf->hTempoMap, (UINT)(pSmf->cTempoMapAlloc*sizeof(TEMPOMAPENTRY)), LHND);
                }

                if (NULL == hLocal)
                {
                    return SMF_NO_MEMORY;
                }

                pSmf->pTempoMap = (PTEMPOMAPENTRY)LocalLock(pSmf->hTempoMap = hLocal);
            }

            if (fFirst && pSmf->tkPosition != 0)
            {
                /* Inserting first event and the absolute time is zero.
                ** Use defaults of 500,000 uSec/qn from MIDI spec
                */
                
                pTempo = &pSmf->pTempoMap[pSmf->cTempoMap++];

                pTempo->tkTempo = 0;
                pTempo->msBase  = 0;
                pTempo->dwTempo = MIDI_DEFAULT_TEMPO;

                fFirst = FALSE;
            }

            pTempo = &pSmf->pTempoMap[pSmf->cTempoMap++];

            pTempo->tkTempo = pSmf->tkPosition;
            if (fFirst)
                pTempo->msBase = 0;
            else
            {
                /* NOTE: Better not be here unless we're q/n format!
                */
                pTempo->msBase = (pTempo-1)->msBase +
                                 muldiv32(pTempo->tkTempo-((pTempo-1)->tkTempo),
                                          (pTempo-1)->dwTempo,
                                          1000L*pSmf->dwTimeDivision);
            }
            pTempo->dwTempo = (((DWORD)event.hpbParm[0])<<16)|
                              (((DWORD)event.hpbParm[1])<<8)|
                              ((DWORD)event.hpbParm[2]);
        }
    }

	if (0 == pSmf->cTempoMap)
	{
		DPF(1, "File contains no tempo map! Insert default tempo.");

		hLocal = LocalAlloc(LHND, sizeof(TEMPOMAPENTRY));
		if (!hLocal)
			return SMF_NO_MEMORY;

        pSmf->pTempoMap = (PTEMPOMAPENTRY)LocalLock(pSmf->hTempoMap = hLocal);
		pSmf->cTempoMap = 1;
		pSmf->cTempoMapAlloc = 1;

		pSmf->pTempoMap->tkTempo = 0;
        pSmf->pTempoMap->msBase  = 0;
        pSmf->pTempoMap->dwTempo = MIDI_DEFAULT_TEMPO;
	}

    if (SMF_END_OF_FILE == smfrc || SMF_SUCCESS == smfrc)
    {
        pSmf->tkLength = pSmf->tkPosition;
        smfrc = SMF_SUCCESS;
    }
        
    return smfrc;
}

/******************************************************************************
*
* smfGetNextEvent
*
* Read the next event from the given file.
*
* pSmf                      - File to read the event from.
*
* pEvent                    - Pointer to an event structure which will receive
*                             basic information about the event.
*
* tkMax                     - Tick destination. An attempt to read past this
*                             position in the file will fail.
*
* Returns
*   SMF_SUCCESS The events were successfully read.
*   SMF_END_OF_FILE There are no more events to read in this track.
*   SMF_REACHED_TKMAX No event was read because <p tkMax> was reached.
*   SMF_INVALID_FILE A disk or parse error occured on the file.
*
* This is the lowest level of parsing for a raw MIDI stream. The basic
* information about one event in the file will be returned in pEvent.
*
* Merging data from all tracks into one stream is performed here.
* 
* pEvent->tkDelta will contain the tick delta for the event.
*
* pEvent->abEvent will contain a description of the event.
*  pevent->abEvent[0] will contain
*    F0 or F7 for a System Exclusive message.
*    FF for a MIDI file meta event.
*    The status byte of any other MIDI message. (Running status will
*    be tracked and expanded).
*
* pEvent->cbParm will contain the number of bytes of paramter data
*   which is still in the file behind the event header already read.
*   This data may be read with <f smfGetTrackEventData>. Any unread
*   data will be skipped on the next call to <f smfGetNextTrackEvent>.
*
* Channel messages (0x8? - 0xE?) will always be returned fully in
*   pevent->abEvent.
*
*  Meta events will contain the meta type in pevent->abEvent[1].
*
*  System exclusive events will contain only an 0xF0 or 0xF7 in
*    pevent->abEvent[0].
*
*  The following fields in pTrk are used to maintain state and must
*  be updated if a seek-in-track is performed:
*
*  bRunningStatus contains the last running status message or 0 if
*   there is no valid running status.
*
*  hpbImage is a pointer into the file image of the first byte of
*   the event to follow the event just read.
*
*  dwLeft contains the number of bytes from hpbImage to the end
*   of the track.
*
*
* Get the next due event from all (in-use?) tracks
*
* For all tracks
*  If not end-of-track
*   decode event delta time without advancing through buffer
*   event_absolute_time = track_tick_time + track_event_delta_time
*   relative_time = event_absolute_time - last_stream_time
*   if relative_time is lowest so far
*    save this track as the next to pull from, along with times
*
* If we found a track with a due event
*  Advance track pointer past event, saving ptr to parm data if needed
*  track_tick_time += track_event_delta_time
*  last_stream_time = track_tick_time
* Else
*  Mark and return end_of_file
*
*****************************************************************************/
SMFRESULT FNLOCAL smfGetNextEvent(
    PSMF                    pSmf,
    EVENT BSTACK *          pEvent,
    TICKS                   tkMax)
{
    PTRACK                  pTrk;
    PTRACK                  pTrkFound;
    DWORD                   idxTrack;
    TICKS                   tkEventDelta;
    TICKS                   tkRelTime;
    TICKS                   tkMinRelTime;
    BYTE                    bEvent;
    DWORD                   dwGotTotal;
    DWORD                   dwGot;
    DWORD                   cbEvent;

    assert(pSmf != NULL);
    assert(pEvent != NULL);

    if (pSmf->fdwSMF & SMF_F_EOF)
    {
        return SMF_END_OF_FILE;
    }

    pTrkFound       = NULL;
    tkMinRelTime    = MAX_TICKS;
    
    for (pTrk = pSmf->rTracks, idxTrack = pSmf->dwTracks; idxTrack--; pTrk++)
    {
        if (pTrk->fdwTrack & SMF_TF_EOT)
            continue;

        
        if (!smfGetVDword(pTrk->hpbImage, pTrk->cbLeft, (DWORD BSTACK *)&tkEventDelta))
        {
            DPF(1, "Hit end of track w/o end marker!");
            return SMF_INVALID_FILE;
        }

        tkRelTime = pTrk->tkPosition + tkEventDelta - pSmf->tkPosition;

        if (tkRelTime < tkMinRelTime)
        {
            tkMinRelTime = tkRelTime;
            pTrkFound = pTrk;
        }
    }

    if (!pTrkFound)
    {
        pSmf->fdwSMF |= SMF_F_EOF;
        return SMF_END_OF_FILE;
    }

    pTrk = pTrkFound;

    if (pSmf->tkPosition + tkMinRelTime >= tkMax)
    {
        return SMF_REACHED_TKMAX;
    }
        

    pTrk->hpbImage += (dwGot = smfGetVDword(pTrk->hpbImage, pTrk->cbLeft, (DWORD BSTACK *)&tkEventDelta));
    pTrk->cbLeft   -= dwGot;

    /* We MUST have at least three bytes here (cause we haven't hit
    ** the end-of-track meta yet, which is three bytes long). Checking
    ** against three means we don't have to check how much is left
    ** in the track again for any short event, which is most cases.
    */
    if (pTrk->cbLeft < 3)
    {
        return SMF_INVALID_FILE;
    }

    pTrk->tkPosition += tkEventDelta;
    pEvent->tkDelta = pTrk->tkPosition - pSmf->tkPosition;
    pSmf->tkPosition = pTrk->tkPosition;

    bEvent = *pTrk->hpbImage++;
    
    if (MIDI_MSG > bEvent)
    {
        if (0 == pTrk->bRunningStatus)
        {
            return SMF_INVALID_FILE;
        }

        dwGotTotal = 1;
        pEvent->abEvent[0] = pTrk->bRunningStatus;
        pEvent->abEvent[1] = bEvent;
        if (3 == grbChanMsgLen[(pTrk->bRunningStatus >> 4) & 0x0F])
        {
            pEvent->abEvent[2] = *pTrk->hpbImage++;
            dwGotTotal++;
        }
    }
    else if (MIDI_SYSEX > bEvent)
    {
        pTrk->bRunningStatus = bEvent;
        
        dwGotTotal = 2;
        pEvent->abEvent[0] = bEvent;
        pEvent->abEvent[1] = *pTrk->hpbImage++;
        if (3 == grbChanMsgLen[(bEvent >> 4) & 0x0F])
        {
            pEvent->abEvent[2] = *pTrk->hpbImage++;
            dwGotTotal++;
        }
    }
    else
    {
        pTrk->bRunningStatus = 0;
        if (MIDI_META == bEvent)
        {
            pEvent->abEvent[0] = MIDI_META;
            if (MIDI_META_EOT == (pEvent->abEvent[1] = *pTrk->hpbImage++))
            {
                pTrk->fdwTrack |= SMF_TF_EOT;
            }

            dwGotTotal = 2;
        }
        else if (MIDI_SYSEX == bEvent || MIDI_SYSEXEND == bEvent)
        {
            pEvent->abEvent[0] = bEvent;
            dwGotTotal = 1;
        }
        else
        {
            return SMF_INVALID_FILE;
        }
        
        if (0 == (dwGot = smfGetVDword(pTrk->hpbImage, pTrk->cbLeft - 2, (DWORD BSTACK *)&cbEvent)))
        {
            return SMF_INVALID_FILE;
        }

        pTrk->hpbImage  += dwGot;
        dwGotTotal      += dwGot;

        if (dwGotTotal + cbEvent > pTrk->cbLeft)
        {
            return SMF_INVALID_FILE;
        }

        pEvent->cbParm  = cbEvent;
        pEvent->hpbParm = pTrk->hpbImage;

        pTrk->hpbImage += cbEvent;
        dwGotTotal     += cbEvent;
    }

    assert(pTrk->cbLeft >= dwGotTotal);

    pTrk->cbLeft -= dwGotTotal;

    return SMF_SUCCESS;
}

/******************************************************************************
*
* smfGetVDword
*
* Reads a variable length DWORD from the given file.
*
* hpbImage                  - Pointer to the first byte of the VDWORD.
*
* dwLeft                    - Bytes left in image
*
* pDw                       - Pointer to a DWORD to store the result in.
*                             track.
*
* Returns the number of bytes consumed from the stream.
*
* A variable length DWORD stored in a MIDI file contains one or more
* bytes. Each byte except the last has the high bit set; only the
* low 7 bits are significant.
*  
*****************************************************************************/
DWORD FNLOCAL smfGetVDword(
    HPBYTE                  hpbImage,                                
    DWORD                   dwLeft,                               
    DWORD BSTACK *          pDw)
{
    BYTE                    b;
    DWORD                   dwUsed  = 0;

    assert(hpbImage != NULL);
    assert(pDw != NULL);
    
    *pDw = 0;

    do
    {
        if (!dwLeft)
        {
            return 0;
        }

        b = *hpbImage++;
        dwLeft--;
        dwUsed++;
        
        *pDw = (*pDw << 7) | (b & 0x7F);
    } while (b&0x80);

    return dwUsed;
}
