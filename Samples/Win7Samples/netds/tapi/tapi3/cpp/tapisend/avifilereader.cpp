/*

Copyright (c) 1999 - 2000  Microsoft Corporation


Module Name:

    AVIFileReader.cpp


Abstract:

    Implementation of CAVIFileReader class
 
*/

#include "common.h"
#include "AVIFileReader.h"


///////////////////////////////////////////////////////////////////////////////
//
// CAVIFileReader::CAVIFileReader
//
///////////////////////////////////////////////////////////////////////////////

CAVIFileReader::CAVIFileReader()
    :m_pAudioStream(NULL),
    m_pWaveFormat(NULL),
    m_nSamplesReadSoFar(0)
{
    LogMessage("CAVIFileReader::CAVIFileReader");
}



///////////////////////////////////////////////////////////////////////////////
//
// CAVIFileReader::Initialize
//
// open the first audio stream contained in file pszFileName. 
// if successful, also get the stream's format
//
// if the file does not exist, is not valid, or does not contain
// any audio streams, subsequent calls to Read() will fail
//
///////////////////////////////////////////////////////////////////////////////

HRESULT CAVIFileReader::Initialize(IN char *pszFileName)
{

    HRESULT hr = S_OK;

    
    //
    // would do better argument checking in a real-life application
    //

    _ASSERTE(pszFileName);


    //
    // log file name and requested wave format
    //

    LogMessage("CAVIFileReader::Initialize started. file [%s]", 
                pszFileName);


    //
    // initialize avi file library. AVIFileExit should be called on the same 
    // thread when AVI library is no longer needed
    //
    // AVIFileExit is called in the destructor. 
    //
    // Assuption: instances of CAVIFileReader are initialized and destroyed on 
    // the same thread
    //
    
    AVIFileInit();

    
    //
    // open the first audio stream in the file
    //

    hr = AVIStreamOpenFromFile(&m_pAudioStream,
                               pszFileName,
                               streamtypeAUDIO,
                               0,
                               OF_READ | OF_SHARE_DENY_WRITE,
                               NULL);

    if (FAILED(hr))
    {
        LogError("CAVIFileReader::Initialize "
                 "Failed to open stream from the file");

        m_pAudioStream = NULL;

        return hr;
    }


    //
    // read the size of the stream's format
    //

    LONG nFormatSize = 0;

    hr = AVIStreamReadFormat(m_pAudioStream, 0, NULL, &nFormatSize);

    if ( FAILED(hr) || (0 == nFormatSize) )
    {
        LogError("CAVIFileReader::Initialize"
                 "Failed to get stream format's size");

        m_pAudioStream->Release();
        m_pAudioStream = NULL;

        return E_FAIL;
    }


    //
    // allocate memory for audio format. keep it in a waveformatex structure. 
    // if the structure returned is waveformat, still allocate waveformatex
    //

    nFormatSize = max((LONG)sizeof(WAVEFORMATEX), nFormatSize);
       
    m_pWaveFormat = (WAVEFORMATEX*)AllocateMemory(nFormatSize);

    if (NULL == m_pWaveFormat)
    {
        LogError("CAVIFileReader::Initialize "
                 "Failed to allocate memory for wave format, size %ld", 
                 nFormatSize);

        m_pAudioStream->Release();
        m_pAudioStream = NULL;

        return E_OUTOFMEMORY;

    }


    //
    // read stream format into allocated structure
    //

    hr = AVIStreamReadFormat(m_pAudioStream, 0, m_pWaveFormat, &nFormatSize);

    if (FAILED(hr))
    {
        LogError("CAVIFileReader::Initialize "
                 "Failed to read stream format");

        m_pAudioStream->Release();
        m_pAudioStream = NULL;

        FreeMemory(m_pWaveFormat);
        m_pWaveFormat = NULL;

        return hr;
    }


    //
    // log stream's format
    //

    LogMessage("CAVIFileReader::CAVIFileReader stream opened");
    LogFormat(m_pWaveFormat);


    LogMessage("CAVIFileReader::CAVIFileReader finished");

    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CAVIFileReader::~CAVIFileReader
//
// if we succeeded opening the stream, close it now.
//
///////////////////////////////////////////////////////////////////////////////

CAVIFileReader::~CAVIFileReader()
{
    LogMessage("CAVIFileReader::~CAVIFileReader started");

    if (NULL != m_pAudioStream)
    {
        AVIStreamRelease(m_pAudioStream);
        m_pAudioStream = NULL;
    }

    if (NULL != m_pWaveFormat)
    {
        FreeMemory(m_pWaveFormat);
        m_pWaveFormat = NULL;
    }


    //
    // AVIFileExit must be called on the same thread as AVIFileInit.
    // therefore this object must be created and deleted on the same thread
    //

    AVIFileExit();

    LogMessage("CAVIFileReader::~CAVIFileReader completed");
}


///////////////////////////////////////////////////////////////////////////////
//
// CAVIFileReader::Read
//
// read the open stream, at the current stream position, 
// into the supplied buffer of specified length. 
// return the number of bytes read
//
// returns: S_OK if succeded, S_FALSE if no more data, failure if failed.
// 
///////////////////////////////////////////////////////////////////////////////

HRESULT CAVIFileReader::Read( IN     BYTE *pBuffer,
                              IN     LONG nBufferSize, 
                              IN OUT LONG *pBytesRead)
{
    
    
    //
    // don't return garbage
    //

    *pBytesRead = 0;


    //
    // fail if the file is not open
    //

    if (NULL == m_pWaveFormat)
    {

        //
        // file is not open
        //

        LogError("CAVIFileReader::Read file is not open");

        return E_FAIL;
    }



    HRESULT hr = E_FAIL;


    //
    // read data into the user-supplied buffer, starting with the current 
    // stream position
    //
    
    LONG nSamplesRead = 0;
    
    hr = AVIStreamRead(m_pAudioStream, 
                        m_nSamplesReadSoFar, 
                        AVISTREAMREAD_CONVENIENT, 
                        pBuffer,
                        nBufferSize,
                        pBytesRead,
                        &nSamplesRead);

    if (FAILED(hr))
    {
        LogError("CAVIFileReader::Read AVIStreamRead failed");

        *pBytesRead = 0;

        return hr;
    }

    
    //
    // keep track of how many samples we have read, so we know where 
    // to start next time
    //

    m_nSamplesReadSoFar += nSamplesRead;

    if (*pBytesRead == 0)
    {
        LogMessage("CAVIFileReader::Read no more data in the file");

        return S_FALSE;
    }


    LogMessage("CAVIFileReader::Read read %ld bytes (%ld samples)", 
                *pBytesRead, nSamplesRead);

    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CAVIFileReader::GetFormat
//
// returns the wave format structure. the caller is responsible for 
// freeing the structure by calling FreeMemory()
// 
// returns S_OK if success or E_FAIL if the file failed to open
//
///////////////////////////////////////////////////////////////////////////////

HRESULT CAVIFileReader::GetFormat(OUT WAVEFORMATEX **ppWaveFormat)
{

    HRESULT hr = E_FAIL;


    //
    // don't return garbage
    //

    *ppWaveFormat = NULL;


    //
    // fail if the file is not open
    //

    if (NULL == m_pWaveFormat)
    {

        //
        // file is not open
        //

        LogError("CAVIFileReader::GetFormat file is not open");

        return E_FAIL;
    }


    //
    // allocate memory for wave format structure
    //

    WAVEFORMATEX *pWaveFormat = 
        (WAVEFORMATEX *)AllocateMemory( sizeof(WAVEFORMATEX) + 
                                m_pWaveFormat->cbSize);

    if (NULL == pWaveFormat)
    {

        LogError("CAVIFileReader::GetFormat "
                 "failed to allocate memory for wave format structure");

        return E_OUTOFMEMORY;
    }

    
    //
    // copy wave format structure into allocated memory
    //

    CopyMemory(pWaveFormat,
               m_pWaveFormat,
               sizeof(WAVEFORMATEX) + m_pWaveFormat->cbSize);

    *ppWaveFormat = pWaveFormat;

    return S_OK;
}