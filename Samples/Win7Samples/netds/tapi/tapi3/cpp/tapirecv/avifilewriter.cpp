/*

Copyright (c) 1999 - 2000  Microsoft Corporation

Module Name:

    AVIFileWriter.cpp

Abstract:

    Implementation of CAVIFileWriter class
 
*/


#include "common.h"
#include "AVIFileWriter.h"


///////////////////////////////////////////////////////////////////////////////
//
// CAVIFileWriter::CAVIFileWriter
//
///////////////////////////////////////////////////////////////////////////////

CAVIFileWriter::CAVIFileWriter()
    :m_pAVIFile(NULL),
    m_pAudioStream(NULL),
    m_nSampleSize(0)
{

    LogMessage("CAVIFileWriter::CAVIFileWriter");

}


///////////////////////////////////////////////////////////////////////////////
//
// CAVIFileWriter::Initialize
//
// initialize avi library. attempt to create a file with the given name and 
// format.
//
// if file creation fails, subsequent calls to write will fail.
//
///////////////////////////////////////////////////////////////////////////////

HRESULT CAVIFileWriter::Initialize(IN const CHAR *pszFileName, 
                                   IN const WAVEFORMATEX &WaveFormat)
{

    HRESULT hr = S_OK;

    
    //
    // if initialization fails, the object's stream and file will be NULL, so 
    // subsequent Write() calls will fail
    //


    //
    // would do a better argument checking in a real-life application
    //

    _ASSERTE(pszFileName);


    //
    // log file name and requested wave format
    //


    LogMessage("CAVIFileWriter::Initialize started.");

    LogMessage("    file [%s] ", pszFileName);
    
    LogFormat(&WaveFormat);
 

    //
    // initialize avi file library.
    // when done with it, call AVIFileExit should be called on the same thread
    //
    
    AVIFileInit();

        
    //
    // create file
    //

    hr = AVIFileOpen(&m_pAVIFile,
                     pszFileName,
                     OF_WRITE | OF_CREATE | OF_SHARE_DENY_WRITE,
                     NULL);

    if (FAILED(hr))
    {
        LogError("CAVIFileWriter::Initialize Failed to create file %s",
                 pszFileName);

        m_pAVIFile = NULL;

        return hr;
    }


    //
    // fill in stream information
    //

    AVISTREAMINFO StreamInfo;

    memset(&StreamInfo, 0, sizeof(StreamInfo));

    StreamInfo.fccType = streamtypeAUDIO;
    StreamInfo.dwQuality = (DWORD)-1;
    m_nSampleSize = StreamInfo.dwSampleSize = WaveFormat.nBlockAlign;

    // ignore the return code -- we will have a null-terminated string no 
    // matter what.
    StringCbCopy(&StreamInfo.szName[0], 64, "test audio stream");


    //
    // add an audio stream to the file
    //

    hr = AVIFileCreateStream(m_pAVIFile, &m_pAudioStream, &StreamInfo);

    if (FAILED(hr))
    {
        LogError("CAVIFileWriter::CAVIFileWriter "
                 "failed to create audio stream");

        AVIFileRelease(m_pAVIFile);
        m_pAVIFile = NULL;

        return hr;
    }


    //
    // set format for the stream
    //

    hr = AVIStreamSetFormat(m_pAudioStream,
                            0,
                            (void*)&WaveFormat,
                            sizeof(WaveFormat) + WaveFormat.cbSize);

    if ( FAILED(hr))
    {
        LogError("Failed to set stream format");

        AVIFileRelease(m_pAVIFile);
        m_pAVIFile = NULL;

        AVIStreamRelease(m_pAudioStream);
        m_pAudioStream = NULL;

        return hr;
    }

    LogMessage("CAVIFileWriter::Initialize succeeded");

    return S_OK;

}


///////////////////////////////////////////////////////////////////////////////
//
// CAVIFileWriter::~CAVIFileWriter
//
// if the file and stream were open, close them.
//
// uninitialize AVI library
//
///////////////////////////////////////////////////////////////////////////////

CAVIFileWriter::~CAVIFileWriter()
{

    LogMessage("CAVIFileWriter::~CAVIFileWriter started");


    if (NULL != m_pAudioStream)
    {
        AVIStreamRelease(m_pAudioStream);
        m_pAudioStream = NULL;
    }


    if (NULL != m_pAVIFile)
    {
        AVIFileRelease(m_pAVIFile);
        m_pAVIFile = NULL;
    }

    
    //
    // uninitialize avi file libraries
    //

    AVIFileExit();

    LogMessage("CAVIFileWriter::~CAVIFileWriter finished");

}


///////////////////////////////////////////////////////////////////////////////
//
// CAVIFileWriter::Write
// 
// write the buffer into the open stream. return the number of bytes recorded.
//
// return S_OK on success, error code on failure
//
///////////////////////////////////////////////////////////////////////////////

HRESULT CAVIFileWriter::Write(IN BYTE *pBuffer,
                              IN ULONG nBytesToWrite,
                              IN OUT ULONG *pnBytesWritten)
{

    if ( (NULL == m_pAudioStream) || (NULL == m_pAVIFile))
    {
        LogError("CAVIFileWriter::Write file or stream is not open");
       
        return E_UNEXPECTED;
    }


    //
    // AVIStreamWrite wants the number of samples we are recording
    //

    LONG nSamples2Write = nBytesToWrite / m_nSampleSize;


    //
    // write to file until dumped the whole buffer of until failure
    //
    
    HRESULT hr = S_OK;

    ULONG nTotalBytesWritten = 0;


    while ((hr == S_OK) && (nTotalBytesWritten < nBytesToWrite))
    {

        //
        // bytes and samples written with the call
        //

        LONG nBytesWritten = 0;

        LONG nSamplesWritten = 0;

        hr = AVIStreamWrite(m_pAudioStream,
                            -1,              // append at the end of the stream
                            nSamples2Write,  // how many samples to write
                            pBuffer,         // where the data is
                            nBytesToWrite,   // how much data do we have
                            AVIIF_KEYFRAME,  // self-sufficient data 
                            &nSamplesWritten,// how many samples were written
                            &nBytesWritten); // how many bytes were written


        //
        // keep track of how many bytes made it into the file
        //

        nTotalBytesWritten += nBytesWritten;
        
    }


    //
    // will return the number of bytes written
    //

    if (NULL != pnBytesWritten) 
    {
        *pnBytesWritten = nTotalBytesWritten;
    }


    //
    // either dumped the whole buffer or failed.
    //

    if (FAILED(hr))
    {

        LogError("CAVIFileWriter::Write Failed to write buffer hr = 0x%lx, "
                 "wrote %ld bytes", hr, nTotalBytesWritten);
    }
    else
    {

        LogMessage("CAVIFileWriter::Write Succeeded. Wrote %ld bytes",
                    nTotalBytesWritten);
    }

    return hr;
    
}