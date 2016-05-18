// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
* Seq.H
*
* Public include file for sequencer interface.
*
*****************************************************************************/

#ifndef _SEQ_
#define _SEQ_

#include "global.h"
#include "smf.h"   

#define  VERSION_MINOR              0x00
#define  VERSION_MAJOR              0x04
#define  SEQ_VERSION                ((DWORD)(WORD)((BYTE)VERSION_MINOR | (((WORD)(BYTE)VERSION_MAJOR) << 8)))

#define SEQ_F_EOF           0x00000001L
#define SEQ_F_COLONIZED     0x00000002L
#define SEQ_F_WAITING       0x00000004L   
                                          
#define SEQ_S_NOFILE        0
#define SEQ_S_OPENED        1
#define SEQ_S_PREROLLING    2
#define SEQ_S_PREROLLED     3
#define SEQ_S_PLAYING       4
#define SEQ_S_PAUSED        5
#define SEQ_S_STOPPING      6
#define SEQ_S_RESET         7

#define MMSG_DONE                   (WM_USER+20)

typedef struct tag_preroll
{
    TICKS       tkBase;
    TICKS       tkEnd;
}   PREROLL,
    FAR *LPPREROLL;

typedef struct tag_seq NEAR* PSEQ;
typedef struct tag_seq
{
    DWORD       cBuffer;            /* Number of streaming buffers to alloc         */
    DWORD       cbBuffer;           /* Size of each streaming buffer                */
    LPSTR       pstrFile;           /* Pointer to filename to open                  */
    UINT        uDeviceID;          /* Requested MIDI device ID for MMSYSTEM        */
    UINT        uMCIDeviceID;       /* Our MCI device ID given to us                */
    UINT        uMCITimeFormat;     /* Current time format                          */
    UINT        uMCITimeDiv;        /* MCI_SEQ_DIV_xxx for current file             */
	HWND		hWnd;				/* Where to post MMSG_DONE when done playing	*/

    UINT        uState;             /* Sequencer state (SEQ_S_xxx)                  */
    TICKS       tkLength;           /* Length of longest track                      */
    DWORD       cTrk;               /* Number of tracks                             */
    MMRESULT    mmrcLastErr;        /* Error return from last sequencer operation   */

    PSEQ        pNext;              /* Link to next PSEQ                            */
    HSMF        hSmf;               /* Handle to open file                          */
    HMIDIOUT    hmidi;              /* Handle to open MIDI device                   */
    DWORD       dwTimeDivision;     /* File time division                           */

    LPBYTE      lpbAlloc;           /* Streaming buffers -- initial allocation      */ 
    LPMIDIHDR   lpmhFree;           /* Streaming buffers -- free list               */
    LPMIDIHDR   lpmhPreroll;        /* Streaming buffers -- preroll buffer          */
    DWORD       cbPreroll;          /* Streaming buffers -- size of lpmhPreroll     */
    UINT        uBuffersInMMSYSTEM; /* Streaming buffers -- in use                  */
    
    TICKS       tkBase;             /* Where playback started from in stream        */
    TICKS       tkEnd;              /* Where playback should end                    */
    
    DWORD       fdwSeq;             /* Various sequencer flags                      */

}   SEQ,
    NEAR *PSEQ;

/* sequence.c
*/

MMRESULT FNLOCAL seqAllocBuffers(    
    PSEQ                pseq);

VOID FNLOCAL seqFreeBuffers(         
    PSEQ                pseq);

MMRESULT FNLOCAL seqOpenFile(        
    PSEQ                pseq);

MMRESULT FNLOCAL seqCloseFile(       
    PSEQ                pseq);

MMRESULT FNLOCAL seqPreroll(         
    PSEQ                pseq,
    LPPREROLL           lppreroll);

MMRESULT FNLOCAL seqStart(           
    PSEQ                pseq);

MMRESULT FNLOCAL seqPause(           
    PSEQ                pseq);

MMRESULT FNLOCAL seqRestart(         
    PSEQ                pseq);

MMRESULT FNLOCAL seqStop(            
    PSEQ                pseq);

MMRESULT FNLOCAL seqTime(            
    PSEQ                pseq,
    PTICKS              pTicks);

TICKS FNLOCAL seqMillisecsToTicks(   
    PSEQ                pseq,
    DWORD               msOffset);

DWORD FNLOCAL seqTicksToMillisecs(   
    PSEQ                pseq,
    TICKS               tkOffset);

#endif
