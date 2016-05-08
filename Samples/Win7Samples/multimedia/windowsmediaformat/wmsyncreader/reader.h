//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            reader.h
//
// Abstract:            Declaration of the CReader class.
//
//*****************************************************************************

#if !defined(AFX_READER_H__7F1F622C_A27E_4E47_B7DA_7516F883884C__INCLUDED_)
#define AFX_READER_H__7F1F622C_A27E_4E47_B7DA_7516F883884C__INCLUDED_

#include "stdafx.h"
#include "rostream.h"


class CReader 
{
public:
    CReader();
    virtual ~CReader();

public:
    HRESULT Open( const TCHAR *pwszFile );
    HRESULT Close();
    HRESULT GetStreamNumbers( IWMProfile* pProfile );
    HRESULT ReadSamples();
    HRESULT SetParams(  DWORD cnsStart,
                        DWORD cnsEnd,
                        BOOL fCompressed,
                        BOOL fAudioPresent,
                        BOOL fVideoPresent,
                        BOOL fRangeInFrames );


private :
    IWMSyncReader*  m_pReader;
    WORD            m_wAudioStreamNum;
    WORD            m_wVideoStreamNum;
    QWORD           m_cnsStart;
    QWORD           m_cnsEnd;

    BOOL            m_fCompressed;
    BOOL            m_fAudioStream;
    BOOL            m_fVideoStream;
    BOOL            m_fRangeInFrames;
    CROStream*      m_pStream;


};

#endif // !defined(AFX_READER_H__7F1F622C_A27E_4E47_B7DA_7516F883884C__INCLUDED_)
