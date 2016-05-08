/*

Copyright (c) 1999 - 2000  Microsoft Corporation


Module Name:

    AVIFileReader.h


Abstract:

    Declaration for the AVIFileReader class. 

*/

#if !defined(AFX_AVIFILEREADER_H__6AC8BC7B_287D_452A_98BF_1C2B69277AB3__INCLUDED_)
#define AFX_AVIFILEREADER_H__6AC8BC7B_287D_452A_98BF_1C2B69277AB3__INCLUDED_


//
// an abstraction for a media file open for reading
//

class CAVIFileReader  
{

public:

    
    //
    // open stream
    //

    HRESULT Initialize(char *pszFileName);


    //
    // constructor
    //

    CAVIFileReader();


    //
    // close stream
    //

	virtual ~CAVIFileReader();

    
    //
    // read stream data into buffer
    //

    HRESULT Read(BYTE *pBuffer, LONG nBufferSize, LONG *pBytesWritten);

    
    //
    // returns waveformat for the open file. the caller must free 
    // returned memory
    //
    
    HRESULT GetFormat(WAVEFORMATEX **ppWaveFormat);


    //
    // return TRUE if the file was opened and is a valid wave file containing 
    // an audio stream
    //

    BOOL IsValidAudioFile()
    {

        //
        // if we know stream's format, chances are this is a valid audio file
        //

        if (NULL == m_pWaveFormat)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }


private:

    
    //
    // audio stream
    //
    
    PAVISTREAM  m_pAudioStream;

    
    //
    // format of the audio stream
    //

    WAVEFORMATEX *m_pWaveFormat;


    //
    // number of samples read so far. we use it to determine the current
    // position in the stream
    //

    LONG m_nSamplesReadSoFar;

};


#endif // !defined(AFX_AVIFILEREADER_H__6AC8BC7B_287D_452A_98BF_1C2B69277AB3__INCLUDED_)
