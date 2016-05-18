//------------------------------------------------------------------------------
// File: IMpeg2PsiParser.h
//
// Desc: DirectShow sample code - custom interface allowing the user
//       to view the program information and change the program
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef __IMPEG2PSIPARSER__
#define __IMPEG2PSIPARSER__

#ifdef __cplusplus
extern "C" {
#endif

#include "psiobj.h"
#define EC_PROGRAM_CHANGED EC_USER + 100


// {AE1A2884-540E-4077-B1AB-67A34A72298C}
DEFINE_GUID(IID_IMpeg2PsiParser, 
0xae1a2884, 0x540e, 0x4077, 0xb1, 0xab, 0x67, 0xa3, 0x4a, 0x72, 0x29, 0x8c);


    DECLARE_INTERFACE_(IMpeg2PsiParser, IUnknown)
    {

        STDMETHOD(GetTransportStreamId) (THIS_
            WORD *pStreamId      
        ) PURE;

        STDMETHOD(GetPatVersionNumber) (THIS_
            BYTE *pPatVersion        
        ) PURE;

        STDMETHOD(GetCountOfPrograms) (THIS_
            int *pNumOfPrograms        
        ) PURE;

        STDMETHOD(GetRecordProgramNumber) (THIS_
            DWORD dwIndex, 
            WORD * pwVal     
        ) PURE;

        STDMETHOD(GetRecordProgramMapPid) (THIS_
            DWORD dwIndex, 
            WORD * pwVal      
        ) PURE;

        STDMETHOD(FindRecordProgramMapPid) (THIS_
            WORD wProgramNumber, 
            WORD * pwVal       
        ) PURE;


        STDMETHOD(GetPmtVersionNumber) (THIS_
            WORD wProgramNumber, 
            BYTE *pPmtVersion
            )PURE;


        STDMETHOD(GetCountOfElementaryStreams) (THIS_
            WORD wProgramNumber, 
            WORD *pwVal
            )PURE;

        STDMETHOD(GetRecordStreamType) (THIS_
            WORD wProgramNumber,
            DWORD dwRecordIndex, 
            BYTE *pbVal
            )PURE;

        STDMETHOD(GetRecordElementaryPid) (THIS_
            WORD wProgramNumber,
            DWORD dwRecordIndex, 
            WORD *pwVal
            )PURE;

        };


#ifdef __cplusplus
}
#endif

#endif // __IMPEG2PSIPARSER__

