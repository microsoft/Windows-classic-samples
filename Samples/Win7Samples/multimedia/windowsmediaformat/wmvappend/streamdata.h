//***************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Streamdata.h
//
// Abstract:            Definition of CStreamData class 
//
//*****************************************************************************

#if !defined(AFX_STREAMDATA_H__7F619823_DAD9_47A2_A1F3_CBDDF90E215C__INCLUDED_)
#define AFX_STREAMDATA_H__7F619823_DAD9_47A2_A1F3_CBDDF90E215C__INCLUDED_

class CStreamData  
{
public:	
    
    CStreamData( DWORD dwStreamCount ) ;
	~CStreamData();
    
    BOOL    MapStreamNums( CStreamData& data2, WORD** ptrNumMap ) ;
	HRESULT SetAllStreamData( IWMProfile* pProfile ) ;
    HRESULT SetAllStreamsBufferWindow( IWMProfile* pProfile );

private:

    BOOL    CompareMediaTypes( WM_MEDIA_TYPE* pMedia1, 
                               WM_MEDIA_TYPE* pMedia2, 
                               BOOL fVBR );

	WORD   GetSameMediaType( WORD* ptrNumMap, 
                             WM_MEDIA_TYPE* pMediaToFind, 
                             BOOL fVBR,
                             CStreamData& data2, 
                             DWORD* pdwBufferWindow );

private:

    WM_MEDIA_TYPE** m_ptrMediaArray ;

    DWORD   m_dwStreamCount ;     // Stream count
    WORD *  m_ptrStreamNumArray ; // Array of stream numbers
    DWORD * m_ptrStreamBufferWindow;
    BOOL *  m_pfVBRStream;

};

#endif // !defined(AFX_STREAMDATA_H__7F619823_DAD9_47A2_A1F3_CBDDF90E215C__INCLUDED_)
