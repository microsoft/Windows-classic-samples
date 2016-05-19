// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//------------------------------------------------------------------------------
// File: Wave.h
//
// Desc: DirectShow sample code - wave header file.
//------------------------------------------------------------------------------


#ifndef __WAVE_INCLUDED__
#define __WAVE_INCLUDED__
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WAVEVERSION 1

#ifndef ER_MEM
#define ER_MEM              0xe000
#endif

#ifndef ER_CANNOTOPEN
#define ER_CANNOTOPEN       0xe100
#endif

#ifndef ER_NOTWAVEFILE
#define ER_NOTWAVEFILE      0xe101
#endif

#ifndef ER_CANNOTREAD
#define ER_CANNOTREAD       0xe102
#endif

#ifndef ER_CORRUPTWAVEFILE
#define ER_CORRUPTWAVEFILE  0xe103
#endif

#ifndef ER_CANNOTWRITE
#define ER_CANNOTWRITE      0xe104
#endif



int WaveOpenFile(TCHAR*, HMMIO *, WAVEFORMATEX **, MMCKINFO *);
int WaveStartDataRead(HMMIO *, MMCKINFO *, MMCKINFO *);
int WaveReadFile(HMMIO, UINT, BYTE *, MMCKINFO *, UINT *);
int WaveCloseReadFile(HMMIO *, WAVEFORMATEX **);

int WaveCreateFile(TCHAR*, HMMIO *, WAVEFORMATEX *, MMCKINFO *, MMCKINFO *);
int WaveStartDataWrite(HMMIO *, MMCKINFO *, MMIOINFO *);
int WaveWriteFile(HMMIO, UINT, BYTE *, MMCKINFO *, UINT *, MMIOINFO *);
int WaveCloseWriteFile(HMMIO *, MMCKINFO *, MMCKINFO *, MMIOINFO *, DWORD);

int WaveLoadFile(TCHAR*, UINT *, WAVEFORMATEX **, BYTE **);
int WaveSaveFile(TCHAR*, UINT, DWORD, WAVEFORMATEX *, BYTE *);

int WaveCopyUselessChunks(HMMIO *, MMCKINFO *, MMCKINFO *, HMMIO *, MMCKINFO *, MMCKINFO *);
BOOL riffCopyChunk(HMMIO, HMMIO, const LPMMCKINFO);


#ifdef __cplusplus
}
#endif

#endif

