//------------------------------------------------------------------------------
// File: ProgProp.h
//
// Desc: DirectShow sample code - definition of property page class for
//       the PSI Parser filter
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#define NUM_OF_PROGRAM_SUBITEM_COLUMN   2
#define NUM_OF_ES_SUBITEM_COLUMN        3
#define MAX_LEN                         255

#define APPNAME         TEXT("Program Property Page")
#define MBOX(s)         MessageBox(m_hwndDialog, s, APPNAME, MB_OK);

static
BYTE
g_MPEG1AudioFormat [] = {
    0x50, 0x00, 0x02, 0x00, 0x80, 0xBB, 0x00, 0x00,
    0x00, 0x7D, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x16, 0x00, 0x02, 0x00, 0x00, 0xE8, 0x03, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x1C, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
} ;

static 
BYTE
g_Mpeg2ProgramVideo [] = {
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcSource.left              = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcSource.top               = 0x00000000
    0xD0, 0x02, 0x00, 0x00,                         //  .hdr.rcSource.right             = 0x000002d0
    0xE0, 0x01, 0x00, 0x00,                         //  .hdr.rcSource.bottom            = 0x000001e0
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcTarget.left              = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcTarget.top               = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcTarget.right             = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcTarget.bottom            = 0x00000000
    0x00, 0x09, 0x3D, 0x00,                         //  .hdr.dwBitRate                  = 0x003d0900
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.dwBitErrorRate             = 0x00000000
    0x63, 0x17, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, //  .hdr.AvgTimePerFrame            = 0x0000000000051763
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.dwInterlaceFlags           = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.dwCopyProtectFlags         = 0x00000000
    0x04, 0x00, 0x00, 0x00,                         //  .hdr.dwPictAspectRatioX         = 0x00000004
    0x03, 0x00, 0x00, 0x00,                         //  .hdr.dwPictAspectRatioY         = 0x00000003
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.dwReserved1                = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.dwReserved2                = 0x00000000
    0x28, 0x00, 0x00, 0x00,                         //  .hdr.bmiHeader.biSize           = 0x00000028
    0xD0, 0x02, 0x00, 0x00,                         //  .hdr.bmiHeader.biWidth          = 0x000002d0
    0xE0, 0x01, 0x00, 0x00,                         //  .hdr.bmiHeader.biHeight         = 0x00000000
    0x00, 0x00,                                     //  .hdr.bmiHeader.biPlanes         = 0x0000
    0x00, 0x00,                                     //  .hdr.bmiHeader.biBitCount       = 0x0000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.bmiHeader.biCompression    = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.bmiHeader.biSizeImage      = 0x00000000
    0xD0, 0x07, 0x00, 0x00,                         //  .hdr.bmiHeader.biXPelsPerMeter  = 0x000007d0
    0x27, 0xCF, 0x00, 0x00,                         //  .hdr.bmiHeader.biYPelsPerMeter  = 0x0000cf27
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.bmiHeader.biClrUsed        = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.bmiHeader.biClrImportant   = 0x00000000
    0x98, 0xF4, 0x06, 0x00,                         //  .dwStartTimeCode                = 0x0006f498
    0x56, 0x00, 0x00, 0x00,                         //  .cbSequenceHeader               = 0x00000056
    0x02, 0x00, 0x00, 0x00,                         //  .dwProfile                      = 0x00000002
    0x02, 0x00, 0x00, 0x00,                         //  .dwLevel                        = 0x00000002
    0x00, 0x00, 0x00, 0x00,                         //  .Flags                          = 0x00000000
                                                    //  .dwSequenceHeader [1]
    0x00, 0x00, 0x01, 0xB3, 0x2D, 0x01, 0xE0, 0x24,
    0x09, 0xC4, 0x23, 0x81, 0x10, 0x11, 0x11, 0x12, 
    0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x14, 0x14, 
    0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15, 0x15, 
    0x15, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 
    0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 
    0x18, 0x18, 0x18, 0x19, 0x18, 0x18, 0x18, 0x19, 
    0x1A, 0x1A, 0x1A, 0x1A, 0x19, 0x1B, 0x1B, 0x1B, 
    0x1B, 0x1B, 0x1C, 0x1C, 0x1C, 0x1C, 0x1E, 0x1E, 
    0x1E, 0x1F, 0x1F, 0x21, 0x00, 0x00, 0x01, 0xB5, 
    0x14, 0x82, 0x00, 0x01, 0x00, 0x00
} ;



class CProgramProperties : public CBasePropertyPage
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    CProgramProperties(LPUNKNOWN lpunk, HRESULT *phr);


private:

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    void SetDirty();
    BOOL OnUpdate();

    // UI
    BOOL SetListColumns();
    BOOL AddListViewItems();
    BOOL SetEsListColumns();
    BOOL AddEsListViewItems(WORD dwProgramNumber);
    BOOL PopulateTransportStreamInfo( );
    BOOL OnViewProgram () ;
    BOOL OnRefreshProgram () ;
    void GetStreamTypeDescription(BYTE stream_type, TCHAR* sz, size_t len);

    // configure demux
    BOOL GetSelectedPids(WORD *AudPid, WORD *vidPid);
    BOOL AvPinsExisted();
    BOOL CreateAndRenderAvOutPins();
    BOOL PidsChanged(WORD audPid, WORD vidPid);
    BOOL MapAvPIDs();
    BOOL UnmapAvPIDs();

    HWND    m_hwndDialog;
    HWND    m_hwndProgList;
    HWND    m_hwndEsList;
    HWND    m_hwndMpeg2Radio;
    HWND    m_hwndATSCRadio;

    BOOL    m_bAvStreaming; // A/V streaming or not

    IBaseFilter   * m_pDemux;
    IGraphBuilder * m_pGraphBuilder;
    IMediaControl * m_pMediaControl;

    IMpeg2PsiParser    *m_pProgram; // pointer to the IMpeg2PsiParser interface of the
                                    // parser filter.  Set up in OnConnect.

    WORD            m_stream_id;
    BYTE            m_pat_version;
    int             m_number_of_programs;

    IPin            * m_pVideoOutPin;
    IPin            * m_pAudioOutPin;
    IMPEG2PIDMap    * m_pIVideoPIDMap;
    IMPEG2PIDMap    * m_pIAudioPIDMap;
    WORD            m_mappedAudPid;
    WORD            m_mappedVidPid;         


}; // CProgramProperties


