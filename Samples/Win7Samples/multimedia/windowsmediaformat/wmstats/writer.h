//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            writer.h
//
// Abstract:            Definition for CWriter class
//
//*****************************************************************************

#if !defined(AFX_WRITER_H__61F3BB49_9ACB_4AA7_8011_45F99857411C__INCLUDED_)
#define AFX_WRITER_H__61F3BB49_9ACB_4AA7_8011_45F99857411C__INCLUDED_

#include "stdafx.h"


class CWriterStats : public WM_WRITER_STATISTICS
{
public :
    WORD m_nStreamNum;
    const CWriterStats &operator= ( const WM_WRITER_STATISTICS &right );
};

class CWriterStatsEx : public WM_WRITER_STATISTICS_EX
{
public :
    WORD m_nStreamNum;
    const CWriterStatsEx &operator= ( const WM_WRITER_STATISTICS_EX &right );
};

class CWriter 
{
public:
    CWriter();
    virtual ~CWriter();

public:
    HRESULT Init();
    HRESULT Configure( IWMProfile* pProfile );
    HRESULT Start();
    HRESULT Stop();
    HRESULT CreateFileSink( __in LPWSTR pwszFileName );
    HRESULT RemoveFileSink();
    HRESULT CreateNetSink( DWORD dwPortNumber, UINT nMaxClient );
    HRESULT RemoveNetSink();
    HRESULT GetStats( DWORD &dwClientNum, int &nStatsNum, CWriterStats **pStats, CWriterStatsEx **pStatsEx );

    HRESULT WriteStreamSample( WORD wStreamNum, QWORD cnsSampleTime, DWORD msSampleSendTime, QWORD cnsSampleDuration, DWORD dwFlags, INSSBuffer* pSample );
    HRESULT AddScript( __in LPWSTR pwszType, __in LPWSTR pwszCommand, QWORD cnsScriptTime );
    HRESULT SetAttribute( WORD nstreamNum, LPCWSTR  pwszName, WMT_ATTR_DATATYPE type, const BYTE* pValue, WORD cbLength );


private :
    HRESULT SetCodecOff();
    HRESULT CreateStreamNumList( IWMProfile*    pProfile );

private :
    int                     m_fWriterStarted;
    WORD*                   m_pStreamNumList;
    DWORD                   m_nStreamNumListCount;

    IWMWriter*              m_pWriter;
    IWMWriterAdvanced*      m_pWriterAdvanced;
    IWMWriterAdvanced3*     m_pWriterAdvanced3;
    IWMWriterFileSink*      m_pFileSink;
    IWMWriterNetworkSink*   m_pNetSink;
    IWMHeaderInfo*          m_pWriterHeaderInfo;




};

#endif // !defined(AFX_WRITER_H__61F3BB49_9ACB_4AA7_8011_45F99857411C__INCLUDED_)
