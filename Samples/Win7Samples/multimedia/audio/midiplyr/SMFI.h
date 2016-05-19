// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef _SMFI_
#define _SMFI_

/* Handle structure for HSMF
*/ 

#define SMF_TF_EOT          0x00000001L
#define SMF_TF_INVALID      0x00000002L

typedef struct tag_tempomapentry
{
    TICKS           tkTempo;           
    DWORD           msBase;            
    DWORD           dwTempo;           
}   TEMPOMAPENTRY,
    *PTEMPOMAPENTRY;

typedef struct tag_smf *PSMF;

typedef struct tag_track
{
    PSMF            pSmf;

    DWORD           idxTrack;          
    
    TICKS           tkPosition;        
    DWORD           cbLeft;            
    HPBYTE          hpbImage;          
    BYTE            bRunningStatus;    
    
    DWORD           fdwTrack;          

    struct
    {
        TICKS       tkLength;
        DWORD       cbLength;
    }
    smti;                              

}   TRACK,
    *PTRACK;

#define SMF_F_EOF               0x00000001L
#define SMF_F_INSERTSYSEX       0x00000002L

#define C_TEMPO_MAP_CHK     16
typedef struct tag_smf
{
    char            szName[128];
    HPBYTE          hpbImage;
    DWORD           cbImage;
    HTASK           htask;

    TICKS           tkPosition;
    TICKS           tkLength;
    DWORD           dwFormat;
    DWORD           dwTracks;
    DWORD           dwTimeDivision;
    DWORD           fdwSMF;

    DWORD           cTempoMap;
    DWORD           cTempoMapAlloc;
    HLOCAL          hTempoMap;
    PTEMPOMAPENTRY  pTempoMap;

    DWORD           dwPendingUserEvent;
    DWORD           cbPendingUserEvent;
    HPBYTE          hpbPendingUserEvent;
    
    TRACK           rTracks[];
}   SMF;

typedef struct tagEVENT
{
    TICKS           tkDelta;           
    BYTE            abEvent[3];        
                                       
                                       
                                       
    DWORD           cbParm;            
    HPBYTE          hpbParm;           
}   EVENT,
    BSTACK *SPEVENT;

#define EVENT_TYPE(event)       ((event).abEvent[0])
#define EVENT_CH_B1(event)      ((event).abEvent[1])
#define EVENT_CH_B2(event)      ((event).abEvent[2])

#define EVENT_META_TYPE(event)  ((event).abEvent[1])

SMFRESULT FNLOCAL smfBuildFileIndex(
    PSMF BSTACK *       ppsmf);

DWORD FNLOCAL smfGetVDword(
    HPBYTE              hpbImage,
    DWORD               dwLeft,                                
    DWORD BSTACK *      pdw);

SMFRESULT FNLOCAL smfGetNextEvent(
    PSMF                psmf,
    SPEVENT             pevent,
    TICKS               tkMax);

/*
** Useful macros when dealing with hi-lo format integers
*/
#define DWORDSWAP(dw) \
    ((((dw)>>24)&0x000000FFL)|\
    (((dw)>>8)&0x0000FF00L)|\
    (((dw)<<8)&0x00FF0000L)|\
    (((dw)<<24)&0xFF000000L))

#define WORDSWAP(w) \
    ((((w)>>8)&0x00FF)|\
    (((w)<<8)&0xFF00))

#define FOURCC_RMID     mmioFOURCC('R','M','I','D')
#define FOURCC_data     mmioFOURCC('d','a','t','a')
#define FOURCC_MThd     mmioFOURCC('M','T','h','d')
#define FOURCC_MTrk     mmioFOURCC('M','T','r','k')

typedef struct tag_chunkhdr
{
    FOURCC  fourccType;
    DWORD   dwLength;
}   CHUNKHDR,
    *PCHUNKHDR;

#pragma pack(1)	// override cl32 default packing, to match disk file.
typedef struct tag_filehdr
{
    WORD    wFormat;
    WORD    wTracks;
    WORD    wDivision;
}   FILEHDR,
    *PFILEHDR;
#pragma pack()

/* NOTE: This is arbitrary and only used if there is a tempo map but no
** entry at tick 0.
*/
#define MIDI_DEFAULT_TEMPO      (500000L)

#define MIDI_MSG                ((BYTE)0x80)
#define MIDI_NOTEOFF            ((BYTE)0x80)
#define MIDI_NOTEON             ((BYTE)0x90)
#define MIDI_POLYPRESSURE       ((BYTE)0xA0)
#define MIDI_CONTROLCHANGE      ((BYTE)0xB0)
#define MIDI_PROGRAMCHANGE      ((BYTE)0xC0)
#define MIDI_CHANPRESSURE       ((BYTE)0xD0)
#define MIDI_PITCHBEND          ((BYTE)0xE0)
#define MIDI_META               ((BYTE)0xFF)
#define MIDI_SYSEX              ((BYTE)0xF0)
#define MIDI_SYSEXEND           ((BYTE)0xF7)

#define MIDI_META_TRACKNAME     ((BYTE)0x03)
#define MIDI_META_EOT           ((BYTE)0x2F)
#define MIDI_META_TEMPO         ((BYTE)0x51)
#define MIDI_META_TIMESIG       ((BYTE)0x58)
#define MIDI_META_KEYSIG        ((BYTE)0x59)
#define MIDI_META_SEQSPECIFIC   ((BYTE)0x7F)

#endif
