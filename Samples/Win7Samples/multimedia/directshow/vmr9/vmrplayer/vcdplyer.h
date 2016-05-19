//------------------------------------------------------------------------------
// File: vcdplyer.h
//
// Desc: DirectShow sample code
//       - Class header file for VMRPlayer sample
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

// VMR9 Headers
#include <d3d9.h>
#include <vmr9.h>

/* -------------------------------------------------------------------------
** CMovie - a DirectShow movie playback class.
** -------------------------------------------------------------------------
*/
enum EMovieMode { MOVIE_NOTOPENED = 0x00,
                  MOVIE_OPENED    = 0x01,
                  MOVIE_PLAYING   = 0x02,
                  MOVIE_STOPPED   = 0x03,
                  MOVIE_PAUSED    = 0x04 };

struct IMpegAudioDecoder;
struct IMpegVideoDecoder;
struct IQualProp;

class CMovie
{
private:
    // Our state variable - records whether we are opened, playing etc.
    EMovieMode       m_Mode;
    HANDLE           m_MediaEvent;
    HWND             m_hwndApp;
    GUID             m_TimeFormat;

    IFilterGraph            *m_Fg;
    IGraphBuilder           *m_Gb;
    IMediaControl           *m_Mc;
    IMediaSeeking           *m_Ms;
    IMediaEvent             *m_Me;
    IVMRWindowlessControl9   *m_Wc;

    HRESULT AddVideoMixingRendererToFG();
    HRESULT FindInterfaceFromFilterGraph(
        REFIID iid, // interface to look for
        LPVOID *lp  // place to return interface pointer in
        );

public:
     CMovie(HWND hwndApplication);
    ~CMovie();

    HRESULT         OpenMovie(TCHAR *lpFileName);
    DWORD           CloseMovie();

    BOOL            PlayMovie();
    BOOL            PauseMovie();
    BOOL            StopMovie();

    OAFilterState   GetStateMovie();
    HANDLE          GetMovieEventHandle();
    long            GetMovieEventCode();

    BOOL            PutMoviePosition(LONG x, LONG y, LONG cx, LONG cy);
    BOOL            GetMoviePosition(LONG *x, LONG *y, LONG *cx, LONG *cy);
    BOOL            GetNativeMovieSize(LONG *cx, LONG *cy);
    BOOL            CanMovieFrameStep();
    BOOL            FrameStepMovie();

    REFTIME         GetDuration();
    REFTIME         GetCurrentPosition();
    EMovieMode      StatusMovie();

    BOOL            SeekToPosition(REFTIME rt,BOOL bFlushData);
    BOOL            IsTimeFormatSupported(GUID Format);
    BOOL            IsTimeSupported();
    BOOL            SetTimeFormat(GUID Format);

    GUID            GetTimeFormat();
    void            SetFocus();
    BOOL            ConfigDialog(HWND hwnd);
    BOOL            RepaintVideo(HWND hwnd, HDC hdc);

    HRESULT         SetAppImage(VMR9AlphaBitmap* lpBmpInfo);
    HRESULT         UpdateAppImage(VMR9AlphaBitmap* lpBmpInfo);
    void            SetBorderClr(COLORREF clr);

    void            DisplayModeChanged() {
        m_Wc->DisplayModeChanged();
    }

    HRESULT         GetCurrentImage(LPBYTE* lplpDib)
    {
        return m_Wc->GetCurrentImage(lplpDib);
    }

    HRESULT         RenderSecondFile(TCHAR *lpFileName);

    IVMRMixerControl9 *m_pMixControl;
};

