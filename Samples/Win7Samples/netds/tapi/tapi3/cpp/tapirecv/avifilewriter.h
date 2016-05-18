/*

Copyright (c) 1999 - 2000  Microsoft Corporation

Module Name:

    AVIFileWriter.h

Abstract:

    Declaration for the AVIFileWriter class. 

*/

#if !defined(AFX_AVIFILEWRITER_H__75ED6641_F059_4EC1_AAB2_867109AA7695__INCLUDED_)
#define AFX_AVIFILEWRITER_H__75ED6641_F059_4EC1_AAB2_867109AA7695__INCLUDED_


//
// an abstraction for a media file open for reading
//

class CAVIFileWriter  
{

public:


    //
    // initialization (including opening file)
    //
    
    HRESULT Initialize(IN const CHAR *psFileName, 
                       IN const WAVEFORMATEX &WaveFormat);


    //
    // zero data members
    //
    
    CAVIFileWriter();


    //
    // close file
    //

	virtual ~CAVIFileWriter();


    //
    // write data
    //

    HRESULT Write(IN BYTE *pBuffer,
                  IN ULONG nBytesToWrite,
                  IN OUT ULONG *pnBytesWritten);

private:

    //
    // file handle
    //

    PAVIFILE m_pAVIFile;

    
    //
    // audio stream
    //
    
    PAVISTREAM  m_pAudioStream;

    
    //
    // size of one sample
    //
    
    LONG m_nSampleSize;


};

#endif // !defined(AFX_AVIFILEWRITER_H__75ED6641_F059_4EC1_AAB2_867109AA7695__INCLUDED_)
