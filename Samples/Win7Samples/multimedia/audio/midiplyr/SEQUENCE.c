// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
* Sequence.C
*
* Sequencer engine for MIDI player app
*
*****************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <limits.h>

#include "debug.h"
#include "seq.h"

PRIVATE void FAR PASCAL seqMIDICallback(HMIDISTRM hms, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
PRIVATE MMRESULT FNLOCAL XlatSMFErr(SMFRESULT smfrc);

/***************************************************************************
*  
* seqAllocBuffers
*
* Allocate buffers for this instance.
*
* pSeq                      - The sequencer instance to allocate buffers for.
*
* Returns
*   MMSYSERR_NOERROR If the operation was successful.
*
*   MCIERR_OUT_OF_MEMORY  If there is insufficient memory for
*     the requested number and size of buffers.
*
* seqAllocBuffers allocates playback buffers based on the
* cbBuffer and cBuffer fields of pSeq. cbBuffer specifies the
* number of bytes in each buffer, and cBuffer specifies the
* number of buffers to allocate.
*
* seqAllocBuffers must be called before any other sequencer call
* on a newly allocted SEQUENCE structure. It must be paired with
* a call to seqFreeBuffers, which should be the last call made
* before the SEQUENCE structure is discarded.
*
***************************************************************************/
MMRESULT FNLOCAL seqAllocBuffers(
    PSEQ                    pSeq)
{
    DWORD                   dwEachBufferSize;
    DWORD                   dwAlloc;
    UINT                    i;
    LPBYTE                  lpbWork;

    assert(pSeq != NULL);

    pSeq->uState    = SEQ_S_NOFILE;
    pSeq->lpmhFree  = NULL;
    pSeq->lpbAlloc  = NULL;
    pSeq->hSmf      = (HSMF)NULL;
    
    /* First make sure we can allocate the buffers they asked for
    */
    dwEachBufferSize = sizeof(MIDIHDR) + (DWORD)(pSeq->cbBuffer);
    dwAlloc          = dwEachBufferSize * (DWORD)(pSeq->cBuffer);
    
    pSeq->lpbAlloc = GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE, dwAlloc);
    if (NULL == pSeq->lpbAlloc)
        return MCIERR_OUT_OF_MEMORY;

    /* Initialize all MIDIHDR's and throw them into a free list
    */
    pSeq->lpmhFree = NULL;

    lpbWork = pSeq->lpbAlloc;
    for (i=0; i < pSeq->cBuffer; i++)
    {
        ((LPMIDIHDR)lpbWork)->lpNext            = pSeq->lpmhFree;

        ((LPMIDIHDR)lpbWork)->lpData            = lpbWork + sizeof(MIDIHDR);
        ((LPMIDIHDR)lpbWork)->dwBufferLength    = pSeq->cbBuffer;
        ((LPMIDIHDR)lpbWork)->dwBytesRecorded   = 0;
        ((LPMIDIHDR)lpbWork)->dwUser            = (DWORD)(UINT)pSeq;
        ((LPMIDIHDR)lpbWork)->dwFlags           = 0;

        pSeq->lpmhFree = (LPMIDIHDR)lpbWork;

        lpbWork += dwEachBufferSize;
    }

    return MMSYSERR_NOERROR;
}

/***************************************************************************
*  
* seqFreeBuffers
*
* Free buffers for this instance.
*
* pSeq                      - The sequencer instance to free buffers for.
*   
* seqFreeBuffers frees all allocated memory belonging to the
* given sequencer instance pSeq. It must be the last call
* performed on the instance before it is destroyed.
*       
****************************************************************************/
VOID FNLOCAL seqFreeBuffers(
    PSEQ                    pSeq)
{
    LPMIDIHDR               lpmh;
    
    assert(pSeq != NULL);

    if (NULL != pSeq->lpbAlloc)
    {
        lpmh = (LPMIDIHDR)pSeq->lpbAlloc;
        assert(!(lpmh->dwFlags & MHDR_PREPARED));
        
        GlobalFreePtr(pSeq->lpbAlloc);
    }
}

/***************************************************************************
*  
* seqOpenFile
*
* Associates a MIDI file with the given sequencer instance.
*
* pSeq                      - The sequencer instance.
*
* Returns
*   MMSYSERR_NOERROR If the operation is successful.
*    
*   MCIERR_UNSUPPORTED_FUNCTION If there is already a file open
*     on this instance.
*     
*   MCIERR_OUT_OF_MEMORY If there was insufficient memory to
*     allocate internal buffers on the file.
*
*   MCIERR_INVALID_FILE If initial attempts to parse the file
*     failed (such as the file is not a MIDI or RMI file).
*
* seqOpenFile may only be called if there is no currently open file
* on the instance. It must be paired with a call to seqCloseFile
* when operations on this file are complete.
*
* The pstrFile field of pSeq contains the name of the file
* to open. This name will be passed directly to mmioOpen; it may
* contain a specifcation for a custom MMIO file handler. The task
* context used for all I/O will be the task which calls seqOpenFile.
*
***************************************************************************/
MMRESULT FNLOCAL seqOpenFile(
    PSEQ                    pSeq)
{                            
    MMRESULT                rc      = MMSYSERR_NOERROR;
    SMFOPENFILESTRUCT       sofs;
    SMFFILEINFO             sfi;
    SMFRESULT               smfrc;
    DWORD                   cbBuffer;

    assert(pSeq != NULL);

    if (pSeq->uState != SEQ_S_NOFILE)
    {
        return MCIERR_UNSUPPORTED_FUNCTION;
    }

    assert(pSeq->pstrFile != NULL);
    
    sofs.pstrName     = pSeq->pstrFile;

    smfrc = smfOpenFile(&sofs);
    if (SMF_SUCCESS != smfrc)
    {
        rc = XlatSMFErr(smfrc);
        goto Seq_Open_File_Cleanup;
    }

    pSeq->hSmf = sofs.hSmf;
    smfGetFileInfo(pSeq->hSmf, &sfi);
    
    pSeq->dwTimeDivision = sfi.dwTimeDivision;
    pSeq->tkLength       = sfi.tkLength;
    pSeq->cTrk           = sfi.dwTracks;
               
    /* Track buffers must be big enough to hold the state data returned
    ** by smfSeek()
    */
    cbBuffer = min(pSeq->cbBuffer, smfGetStateMaxSize());
    
Seq_Open_File_Cleanup:    
    if (MMSYSERR_NOERROR != rc)
        seqCloseFile(pSeq);
    else
        pSeq->uState = SEQ_S_OPENED;

    return rc;
}

/***************************************************************************
*  
* seqCloseFile
*
* Deassociates a MIDI file with the given sequencer instance.
*
* pSeq                      -  The sequencer instance.
*
* Returns
*   MMSYSERR_NOERROR If the operation is successful.
*    
*   MCIERR_UNSUPPORTED_FUNCTION If the sequencer instance is not
*     stopped.
*     
* A call to seqCloseFile must be paired with a prior call to
* seqOpenFile. All buffers associated with the file will be
* freed and the file will be closed. The sequencer must be
* stopped before this call will be accepted.
*
***************************************************************************/
MMRESULT FNLOCAL seqCloseFile(
    PSEQ                    pSeq)
{
    LPMIDIHDR               lpmh;
    
    assert(pSeq != NULL);
    
    if (SEQ_S_OPENED != pSeq->uState)
        return MCIERR_UNSUPPORTED_FUNCTION;
    
    if ((HSMF)NULL != pSeq->hSmf)
    {
        smfCloseFile(pSeq->hSmf);
        pSeq->hSmf = (HSMF)NULL;
    }

    /* If we were prerolled, need to clean up -- have an open MIDI handle
    ** and buffers in the ready queue
    */

    for (lpmh = pSeq->lpmhFree; lpmh; lpmh = lpmh->lpNext)
        midiOutUnprepareHeader(pSeq->hmidi, lpmh, sizeof(*lpmh));

    if (pSeq->lpmhPreroll)
        midiOutUnprepareHeader(pSeq->hmidi, pSeq->lpmhPreroll, sizeof(*pSeq->lpmhPreroll));

    if (pSeq->hmidi != NULL)
    {
        midiStreamClose((HMIDISTRM)(pSeq->hmidi));
        pSeq->hmidi = NULL;
    }

    pSeq->uState = SEQ_S_NOFILE;

    return MMSYSERR_NOERROR;
}

/***************************************************************************
*  
* seqPreroll
*
* Prepares the file for playback at the given position.
*
* pSeq                      - The sequencer instance.
*
* lpPreroll                 - Specifies the starting and ending tick
*                             positions to play between.
*
* Returns
*   MMSYSERR_NOERROR If the operation is successful.
*    
*   MCIERR_UNSUPPORTED_FUNCTION If the sequencer instance is not
*     opened or prerolled.
*
* Open the device so we can initialize channels.
*
* Loop through the tracks. For each track, seek to the given position and
* send the init data SMF gives us to the handle.
*
* Wait for all init buffers to finish.
*
* Unprepare the buffers (they're only ever sent here; the sequencer
* engine merges them into a single stream during normal playback) and
* refill them with the first chunk of data from the track. 
*
*     
****************************************************************************/
MMRESULT FNLOCAL seqPreroll(
    PSEQ                    pSeq,
    LPPREROLL               lpPreroll)
{
    SMFRESULT           smfrc;
    MMRESULT            mmrc        = MMSYSERR_NOERROR;
    MIDIPROPTIMEDIV     mptd;
    LPMIDIHDR           lpmh = NULL;
    LPMIDIHDR           lpmhPreroll = NULL;
    DWORD               cbPrerollBuffer;
    UINT                uDeviceID;

    assert(pSeq != NULL);

    pSeq->mmrcLastErr = MMSYSERR_NOERROR;

    if (pSeq->uState != SEQ_S_OPENED &&
        pSeq->uState != SEQ_S_PREROLLED)
        return MCIERR_UNSUPPORTED_FUNCTION;

	pSeq->tkBase = lpPreroll->tkBase;
	pSeq->tkEnd = lpPreroll->tkEnd;

    if (pSeq->hmidi)
    {
        // Recollect buffers from MMSYSTEM back into free queue
        //
        pSeq->uState = SEQ_S_RESET;
        midiOutReset(pSeq->hmidi);

		while (pSeq->uBuffersInMMSYSTEM)
			Sleep(0);
    }
    
    pSeq->uBuffersInMMSYSTEM = 0;
    pSeq->uState = SEQ_S_PREROLLING;
    
    //
    // We've successfully opened the file and all of the tracks; now
    // open the MIDI device and set the time division.
    //
    // NOTE: seqPreroll is equivalent to seek; device might already be open
    //
    if (NULL == pSeq->hmidi)
    {
        uDeviceID = pSeq->uDeviceID;
        if ((mmrc = midiStreamOpen(&(HMIDISTRM)pSeq->hmidi,
                                   &uDeviceID,
                                   1,
                                   (DWORD)seqMIDICallback,
                                   0,
                                   CALLBACK_FUNCTION)) != MMSYSERR_NOERROR)
        {
            pSeq->hmidi = NULL;
            goto seq_Preroll_Cleanup;
        }
        
        mptd.cbStruct  = sizeof(mptd);
        mptd.dwTimeDiv = pSeq->dwTimeDivision;
        if ((mmrc = midiStreamProperty(
                                       (HMIDISTRM)pSeq->hmidi,
                                       (LPBYTE)&mptd,
                                       MIDIPROP_SET|MIDIPROP_TIMEDIV)) != MMSYSERR_NOERROR)
        {
            DPF(1, "midiStreamProperty() -> %04X", (WORD)mmrc);
            midiStreamClose((HMIDISTRM)pSeq->hmidi);
            pSeq->hmidi = NULL;
            mmrc = MCIERR_DEVICE_NOT_READY;
            goto seq_Preroll_Cleanup;
        }
    }

    mmrc = MMSYSERR_NOERROR;

    //
    //  Allocate a preroll buffer.  Then if we don't have enough room for
    //  all the preroll info, we make the buffer larger.  
    //
    if (!pSeq->lpmhPreroll)
    {
        cbPrerollBuffer = 4096;
        lpmhPreroll = (LPMIDIHDR)GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE,
                                                            cbPrerollBuffer);
    }
    else
    {
        cbPrerollBuffer = pSeq->cbPreroll;
        lpmhPreroll = pSeq->lpmhPreroll;
    }

    lpmhPreroll->lpNext            = pSeq->lpmhFree;
    lpmhPreroll->lpData            = (LPBYTE)lpmhPreroll + sizeof(MIDIHDR);
    lpmhPreroll->dwBufferLength    = cbPrerollBuffer - sizeof(MIDIHDR);
    lpmhPreroll->dwBytesRecorded   = 0;
    lpmhPreroll->dwUser            = (DWORD)(UINT)pSeq;
    lpmhPreroll->dwFlags           = 0;

    do
    {
        smfrc = smfSeek(pSeq->hSmf, pSeq->tkBase, lpmhPreroll);
        if( SMF_SUCCESS != smfrc )
        {
            if( ( SMF_NO_MEMORY != smfrc )  ||
                ( cbPrerollBuffer >= 32768L ) )
            {
                DPF(1, "smfSeek() returned %lu", (DWORD)smfrc);

                GlobalFreePtr(lpmhPreroll);
                pSeq->lpmhPreroll = NULL;

                mmrc = XlatSMFErr(smfrc);
                goto seq_Preroll_Cleanup;
            }
            else   //  Try to grow buffer.
            {
                cbPrerollBuffer *= 2;
                lpmh = (LPMIDIHDR)GlobalReAllocPtr( lpmhPreroll, cbPrerollBuffer, 0 );
                if( NULL == lpmh )
                {
                    DPF(2,"seqPreroll - realloc failed, aborting preroll.");
                    mmrc = MCIERR_OUT_OF_MEMORY;
                    goto seq_Preroll_Cleanup;
                }

                lpmhPreroll = lpmh;
                lpmhPreroll->lpData = (LPBYTE)lpmhPreroll + sizeof(MIDIHDR);
                lpmhPreroll->dwBufferLength = cbPrerollBuffer - sizeof(MIDIHDR);

                pSeq->lpmhPreroll = lpmhPreroll;
                pSeq->cbPreroll = cbPrerollBuffer;
            }
        }
    } while( SMF_SUCCESS != smfrc );

    if (MMSYSERR_NOERROR != (mmrc = midiOutPrepareHeader(pSeq->hmidi, lpmhPreroll, sizeof(MIDIHDR))))
    {
        DPF(1, "midiOutPrepare(preroll) -> %lu!", (DWORD)mmrc);

        mmrc = MCIERR_DEVICE_NOT_READY;
        goto seq_Preroll_Cleanup;
    }

    ++pSeq->uBuffersInMMSYSTEM;

    if (MMSYSERR_NOERROR != (mmrc = midiStreamOut((HMIDISTRM)pSeq->hmidi, lpmhPreroll, sizeof(MIDIHDR))))
    {
        DPF(1, "midiStreamOut(preroll) -> %lu!", (DWORD)mmrc);

        mmrc = MCIERR_DEVICE_NOT_READY;
        --pSeq->uBuffersInMMSYSTEM;
        goto seq_Preroll_Cleanup;
    }
    DPF(3,"seqPreroll: midiStreamOut(0x%x,0x%lx,%u) returned %u.",pSeq->hmidi,lpmhPreroll,sizeof(MIDIHDR),mmrc);

    pSeq->fdwSeq &= ~SEQ_F_EOF;
    while (pSeq->lpmhFree)
    {
        lpmh = pSeq->lpmhFree;
        pSeq->lpmhFree = lpmh->lpNext;

        smfrc = smfReadEvents(pSeq->hSmf, lpmh, pSeq->tkEnd);
        if (SMF_SUCCESS != smfrc && SMF_END_OF_FILE != smfrc)
        {
            DPF(1, "SFP: smfReadEvents() -> %u", (UINT)smfrc);
            mmrc = XlatSMFErr(smfrc);
            goto seq_Preroll_Cleanup;
        }

        if (MMSYSERR_NOERROR != (mmrc = midiOutPrepareHeader(pSeq->hmidi, lpmh, sizeof(*lpmh))))
        {
            DPF(1, "SFP: midiOutPrepareHeader failed");
            goto seq_Preroll_Cleanup;
        }

        if (MMSYSERR_NOERROR != (mmrc = midiStreamOut((HMIDISTRM)pSeq->hmidi, lpmh, sizeof(*lpmh))))
        {
            DPF(1, "SFP: midiStreamOut failed");
            goto seq_Preroll_Cleanup;
        }

        ++pSeq->uBuffersInMMSYSTEM; 

        if (SMF_END_OF_FILE == smfrc)
        {
            pSeq->fdwSeq |= SEQ_F_EOF;
            break;
        }
    } 

seq_Preroll_Cleanup:
    if (MMSYSERR_NOERROR != mmrc)
    {
        pSeq->uState = SEQ_S_OPENED;
        pSeq->fdwSeq &= ~SEQ_F_WAITING;
    }
    else
    {
        pSeq->uState = SEQ_S_PREROLLED;
    }

    return mmrc;
}

/***************************************************************************
*  
* seqStart
*
* Starts playback at the current position.
*
* pSeq                      - The sequencer instance.
*
* Returns
*   MMSYSERR_NOERROR If the operation is successful.
*    
*   MCIERR_UNSUPPORTED_FUNCTION If the sequencer instance is not
*     stopped.
*
*   MCIERR_DEVICE_NOT_READY If the underlying MIDI device could
*     not be opened or fails any call.
* 
* The sequencer must be prerolled before seqStart may be called.
*
* Just feed everything in the ready queue to the device.
*       
***************************************************************************/
MMRESULT FNLOCAL seqStart(
    PSEQ                    pSeq)
{
    assert(NULL != pSeq);

    if (SEQ_S_PREROLLED != pSeq->uState)
    {
        DPF(1, "seqStart(): State is wrong! [%u]", pSeq->uState);
        return MCIERR_UNSUPPORTED_FUNCTION;
    }

    pSeq->uState = SEQ_S_PLAYING;

    return midiStreamRestart((HMIDISTRM)pSeq->hmidi);
}

/***************************************************************************
*  
* seqPause
*
* Pauses playback of the instance.
*
* pSeq                      - The sequencer instance.
*
* Returns
*   MMSYSERR_NOERROR If the operation is successful.
*    
*   MCIERR_UNSUPPORTED_FUNCTION If the sequencer instance is not
*     playing.
*
* The sequencer must be playing before seqPause may be called.
* Pausing the sequencer will cause all currently on notes to be turned
* off. This may cause playback to be slightly inaccurate on restart
* due to missing notes.
*       
***************************************************************************/
MMRESULT FNLOCAL seqPause(
    PSEQ                    pSeq)
{
    assert(NULL != pSeq);
    
    if (SEQ_S_PLAYING != pSeq->uState)
        return MCIERR_UNSUPPORTED_FUNCTION;

    pSeq->uState = SEQ_S_PAUSED;
    midiStreamPause((HMIDISTRM)pSeq->hmidi);
    
    return MMSYSERR_NOERROR;
}

/***************************************************************************
*  
* seqRestart
*
* Restarts playback of an instance after a pause.
*
* pSeq                      - The sequencer instance.
*
* Returns
*    MMSYSERR_NOERROR If the operation is successful.
*    
*    MCIERR_UNSUPPORTED_FUNCTION If the sequencer instance is not
*     paused.
*
* The sequencer must be paused before seqRestart may be called.
*
***************************************************************************/
MMRESULT FNLOCAL seqRestart(
    PSEQ                    pSeq)
{
    assert(NULL != pSeq);

    if (SEQ_S_PAUSED != pSeq->uState)
        return MCIERR_UNSUPPORTED_FUNCTION;

    pSeq->uState = SEQ_S_PLAYING;
    midiStreamRestart((HMIDISTRM)pSeq->hmidi);

    return MMSYSERR_NOERROR;
}

/***************************************************************************
*  
* seqStop
*
* Totally stops playback of an instance.
*
* pSeq                      - The sequencer instance.
*
* Returns
*   MMSYSERR_NOERROR If the operation is successful.
*    
*   MCIERR_UNSUPPORTED_FUNCTION If the sequencer instance is not
*     paused or playing.
*
* The sequencer must be paused or playing before seqStop may be called.
*
***************************************************************************/
MMRESULT FNLOCAL seqStop(
    PSEQ                    pSeq)
{
    assert(NULL != pSeq);

    /* Automatic success if we're already stopped
    */
    if (SEQ_S_PLAYING != pSeq->uState &&
        SEQ_S_PAUSED != pSeq->uState)
    {
        pSeq->fdwSeq &= ~SEQ_F_WAITING;
        return MMSYSERR_NOERROR;
    }

    pSeq->uState = SEQ_S_STOPPING;
    pSeq->fdwSeq |= SEQ_F_WAITING;
    
    if (MMSYSERR_NOERROR != (pSeq->mmrcLastErr = midiStreamStop((HMIDISTRM)pSeq->hmidi)))
    {
        DPF(1, "midiOutStop() returned %lu in seqStop()!", (DWORD)pSeq->mmrcLastErr);
        
        pSeq->fdwSeq &= ~SEQ_F_WAITING;
        return MCIERR_DEVICE_NOT_READY;
    }

	while (pSeq->uBuffersInMMSYSTEM)
		Sleep(0);
    
    return MMSYSERR_NOERROR;
}

/***************************************************************************
*  
* seqTime
*
* Determine the current position in playback of an instance.
*
* pSeq                      - The sequencer instance.
*
* pTicks                    - A pointer to a DWORD where the current position
*                             in ticks will be returned.
*
* Returns
*   MMSYSERR_NOERROR If the operation is successful.
*
*   MCIERR_DEVICE_NOT_READY If the underlying device fails to report
*     the position.
*    
*   MCIERR_UNSUPPORTED_FUNCTION If the sequencer instance is not
*     paused or playing.
*
* The sequencer must be paused, playing or prerolled before seqTime
* may be called.
*
***************************************************************************/
MMRESULT FNLOCAL seqTime(
    PSEQ                    pSeq,
    PTICKS                  pTicks)
{
    MMRESULT                mmr;
    MMTIME                  mmt;
    
    assert(pSeq != NULL);

    if (SEQ_S_PLAYING != pSeq->uState &&
        SEQ_S_PAUSED != pSeq->uState &&
        SEQ_S_PREROLLING != pSeq->uState &&
        SEQ_S_PREROLLED != pSeq->uState &&
        SEQ_S_OPENED != pSeq->uState)
    {
        DPF(1, "seqTime(): State wrong! [is %u]", pSeq->uState);
        return MCIERR_UNSUPPORTED_FUNCTION;
    }

    *pTicks = 0;
    if (SEQ_S_OPENED != pSeq->uState)
    {
        *pTicks = pSeq->tkBase;
        if (SEQ_S_PREROLLED != pSeq->uState)
        {
            mmt.wType = TIME_TICKS;
            mmr = midiStreamPosition((HMIDISTRM)pSeq->hmidi, &mmt, sizeof(mmt));
            if (MMSYSERR_NOERROR != mmr)
            {
                DPF(1, "midiStreamPosition() returned %lu", (DWORD)mmr);
                return MCIERR_DEVICE_NOT_READY;
            }

            *pTicks += mmt.u.ticks;
        }
    }

    return MMSYSERR_NOERROR;
}
                              
/***************************************************************************
*  
* seqMillisecsToTicks
*
* Given a millisecond offset in the output stream, returns the associated
* tick position.
*
* pSeq                      - The sequencer instance.
*
* msOffset                  - The millisecond offset into the stream.
*
* Returns the number of ticks into the stream.
*
***************************************************************************/
TICKS FNLOCAL seqMillisecsToTicks(
    PSEQ                    pSeq,
    DWORD                   msOffset)
{
    return smfMillisecsToTicks(pSeq->hSmf, msOffset);
}

/***************************************************************************
*  
* seqTicksToMillisecs
*
* Given a tick offset in the output stream, returns the associated
* millisecond position.
*
* pSeq                      - The sequencer instance.
*
* tkOffset                  - The tick offset into the stream.
*
* Returns the number of milliseconds into the stream.
*
***************************************************************************/
DWORD FNLOCAL seqTicksToMillisecs(
    PSEQ                    pSeq,
    TICKS                   tkOffset)
{
    return smfTicksToMillisecs(pSeq->hSmf, tkOffset);
}

/***************************************************************************
*  
* seqMIDICallback
*
* Called by the system when a buffer is done
*
* dw1                       - The buffer that has completed playback.
*
***************************************************************************/
PRIVATE void FAR PASCAL seqMIDICallback(HMIDISTRM hms, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	LPMIDIHDR					lpmh		= (LPMIDIHDR)dw1;
    PSEQ                    pSeq;
    MMRESULT                mmrc;
    SMFRESULT               smfrc;

	if (uMsg != MOM_DONE)
		return;

	assert(NULL != lpmh);

    pSeq = (PSEQ)(lpmh->dwUser);

    assert(pSeq != NULL);

    --pSeq->uBuffersInMMSYSTEM;
    
    if (SEQ_S_RESET == pSeq->uState)
    {
        // We're recollecting buffers from MMSYSTEM
        //
		if (lpmh != pSeq->lpmhPreroll)
		{
        	lpmh->lpNext   = pSeq->lpmhFree;
        	pSeq->lpmhFree = lpmh;
		}

        return;
    }
    

    if ((SEQ_S_STOPPING == pSeq->uState) || (pSeq->fdwSeq & SEQ_F_EOF))
    {
        /*
        ** Reached EOF, just put the buffer back on the free
        ** list 
        */
		if (lpmh != pSeq->lpmhPreroll)
		{
        	lpmh->lpNext   = pSeq->lpmhFree;
        	pSeq->lpmhFree = lpmh;
		}

        if (MMSYSERR_NOERROR != (mmrc = midiOutUnprepareHeader(pSeq->hmidi, lpmh, sizeof(*lpmh))))
        {
            DPF(1, "midiOutUnprepareHeader failed in seqBufferDone! (%lu)", (DWORD)mmrc);
        }

        if (0 == pSeq->uBuffersInMMSYSTEM)
        {
            DPF(1, "seqBufferDone: normal sequencer shutdown.");
            
            /* Totally done! Free device and notify.
            */
            midiStreamClose((HMIDISTRM)pSeq->hmidi);
            
            pSeq->hmidi = NULL;
            pSeq->uState = SEQ_S_OPENED;
            pSeq->mmrcLastErr = MMSYSERR_NOERROR;
            pSeq->fdwSeq &= ~SEQ_F_WAITING;
        
        	// lParam indicates whether or not to preroll again. Don't if we were explicitly
        	// stopped.
        	//    
            PostMessage(pSeq->hWnd, MMSG_DONE, (WPARAM)pSeq, (LPARAM)(SEQ_S_STOPPING != pSeq->uState));
        }
    }
    else
    {
        /*
        ** Not EOF yet; attempt to fill another buffer
        */
        smfrc = smfReadEvents(pSeq->hSmf, lpmh, pSeq->tkEnd);
        
        switch(smfrc)
        {
            case SMF_SUCCESS:
                break;

            case SMF_END_OF_FILE:
                pSeq->fdwSeq |= SEQ_F_EOF;
                smfrc = SMF_SUCCESS;
                break;

            default:
                DPF(1, "smfReadEvents returned %lu in callback!", (DWORD)smfrc);
                pSeq->uState = SEQ_S_STOPPING;
                break;
        }

        if (SMF_SUCCESS == smfrc)
        {
            ++pSeq->uBuffersInMMSYSTEM;
            mmrc = midiStreamOut((HMIDISTRM)pSeq->hmidi, lpmh, sizeof(*lpmh));
            if (MMSYSERR_NOERROR != mmrc)
            {
                DPF(1, "seqBufferDone(): midiStreamOut() returned %lu!", (DWORD)mmrc);
                
                --pSeq->uBuffersInMMSYSTEM;
                pSeq->uState = SEQ_S_STOPPING;
            }
        }
    }
}

/***************************************************************************
*  
* XlatSMFErr
*
* Translates an error from the SMF layer into an appropriate MCI error.
*
* smfrc                     - The return code from any SMF function.
*
* Returns
*   A parallel error from the MCI error codes.   
*
***************************************************************************/
PRIVATE MMRESULT FNLOCAL XlatSMFErr(
    SMFRESULT               smfrc)
{
    switch(smfrc)
    {
        case SMF_SUCCESS:
            return MMSYSERR_NOERROR;

        case SMF_NO_MEMORY:
            return MCIERR_OUT_OF_MEMORY;

        case SMF_INVALID_FILE:
        case SMF_OPEN_FAILED:
        case SMF_INVALID_TRACK:
            return MCIERR_INVALID_FILE;

        default:
            return MCIERR_UNSUPPORTED_FUNCTION;
    }
}




