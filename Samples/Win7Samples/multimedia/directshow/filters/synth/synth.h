//------------------------------------------------------------------------------
// File: Synth.h
//
// Desc: DirectShow sample code - header file for audio signal generator 
//       source filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __AUDIOSYNTH__
#define __AUDIOSYNTH__

//CLSID_SynthFilter
//{79A98DE0-BC00-11ce-AC2E-444553540000}
DEFINE_GUID(CLSID_SynthFilter,
0x79a98de0, 0xbc00, 0x11ce, 0xac, 0x2e, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

//CLSID_SynthFilterPropertyPage
//{79A98DE1-BC00-11ce-AC2E-444553540000}
DEFINE_GUID(CLSID_SynthPropertyPage,
0x79a98de1, 0xbc00, 0x11ce, 0xac, 0x2e, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

const double TWOPI = 6.283185308;
const int MaxFrequency = 20000;
const int MinFrequency = 0;
const int DefaultFrequency = 440;       // A-440
const int MaxAmplitude = 100;
const int MinAmplitude = 0;
const int DefaultSweepStart = DefaultFrequency;
const int DefaultSweepEnd = 5000;
const int WaveBufferSize = 16*1024;     // Size of each allocated buffer
                                        // Originally used to be 2K, but at
                                        // 44khz/16bit/stereo you would get
                                        // audio breaks with a transform in the
                                        // middle.

enum Waveforms {
    WAVE_SINE = 0,
    WAVE_SQUARE,
    WAVE_SAWTOOTH,
    WAVE_SINESWEEP,
    WAVE_LAST           // Always keep this entry last
};

#define WM_PROPERTYPAGE_ENABLE  (WM_USER + 100)

// below stuff is implementation-only....
#ifdef _AUDIOSYNTH_IMPLEMENTATION_

class CSynthStream;

// -------------------------------------------------------------------------
// CAudioSynth
// -------------------------------------------------------------------------

class CAudioSynth {

public:

    CAudioSynth(
                CCritSec* pStateLock,
                int Frequency = DefaultFrequency,
                int Waveform = WAVE_SINE,
                int iBitsPerSample = 8,
                int iChannels = 1,
                int iSamplesPerSec = 11025,
                int iAmplitude = 100
                );

    ~CAudioSynth();

    // Load the buffer with the current waveform
    void FillPCMAudioBuffer(const WAVEFORMATEX& wfex, BYTE pBuf[], int iSize);

    // Set the "current" format and allocate temporary memory
    HRESULT AllocWaveCache(const WAVEFORMATEX& wfex);

    void GetPCMFormatStructure(WAVEFORMATEX* pwfex);

    STDMETHODIMP get_Frequency(int *Frequency);
    STDMETHODIMP put_Frequency(int  Frequency);
    STDMETHODIMP get_Waveform(int *Waveform);
    STDMETHODIMP put_Waveform(int  Waveform);
    STDMETHODIMP get_Channels(int *Channels);
    STDMETHODIMP put_Channels(int Channels);
    STDMETHODIMP get_BitsPerSample(int *BitsPerSample);
    STDMETHODIMP put_BitsPerSample(int BitsPerSample);
    STDMETHODIMP get_SamplesPerSec(int *SamplesPerSec);
    STDMETHODIMP put_SamplesPerSec(int SamplesPerSec);
    STDMETHODIMP put_SynthFormat(int Channels, int BitsPerSample, int SamplesPerSec);
    STDMETHODIMP get_Amplitude(int *Amplitude);
    STDMETHODIMP put_Amplitude(int  Amplitude);
    STDMETHODIMP get_SweepRange(int *SweepStart, int *SweepEnd);
    STDMETHODIMP put_SweepRange(int  SweepStart, int  SweepEnd);
    STDMETHODIMP get_OutputFormat(SYNTH_OUTPUT_FORMAT *pOutputFormat);
    STDMETHODIMP put_OutputFormat(SYNTH_OUTPUT_FORMAT ofOutputFormat);

private:
    CCritSec* m_pStateLock;

    WORD  m_wChannels;          // The output format's current number of channels.
    WORD  m_wFormatTag;         // The output format.  This can be PCM audio or MS ADPCM audio.
    DWORD m_dwSamplesPerSec;    // The number of samples produced in one second by the synth filter.
    WORD  m_wBitsPerSample;     // The number of bits in each sample.  This member is only valid if the
                                // current format is PCM audio.

    int m_iWaveform;            // WAVE_SINE ...
    int m_iFrequency;           // if not using sweep, this is the frequency
    int m_iAmplitude;           // 0 to 100

    int m_iWaveformLast;        // keep track of the last known format
    int m_iFrequencyLast;       // so we can flush the cache if necessary
    int m_iAmplitudeLast;
    int m_iCurrentSample;       // 0 to iSamplesPerSec-1

    BYTE * m_bWaveCache;        // Wave Cache as BYTEs.  This cache ALWAYS holds PCM audio data.
    WORD * m_wWaveCache;        // Wave Cache as WORDs.  This cache ALWAYS holds PCM audio data.

    int m_iWaveCacheSize;       // how big is the cache?
    int m_iWaveCacheCycles;     // how many cycles are in the cache
    int m_iWaveCacheIndex;

    int m_iSweepStart;           // start of sweep
    int m_iSweepEnd;             // end of sweep

    void CalcCache         (const WAVEFORMATEX& wfex);
    void CalcCacheSine     (const WAVEFORMATEX& wfex);
    void CalcCacheSquare   (const WAVEFORMATEX& wfex);
    void CalcCacheSawtooth (const WAVEFORMATEX& wfex);
    void CalcCacheSweep    (const WAVEFORMATEX& wfex);

};



// -------------------------------------------------------------------------
// CSynthFilter
// -------------------------------------------------------------------------
// CSynthFilter manages filter level stuff

class CSynthFilter :    public ISynth2,
                        public CPersistStream,
                        public ISpecifyPropertyPages,
                        public CDynamicSource {

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    ~CSynthFilter();

    DECLARE_IUNKNOWN;

    // override this to reveal our property interface
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // --- ISpecifyPropertyPages ---

    // return our property pages
    STDMETHODIMP GetPages(CAUUID * pPages);

    // --- IPersistStream Interface

    STDMETHODIMP GetClassID(CLSID *pClsid);
    int SizeMax();
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    DWORD GetSoftwareVersion(void);

    //
    // --- ISynth2 ---
    //

    STDMETHODIMP get_Frequency(int *Frequency);
    STDMETHODIMP put_Frequency(int Frequency);
    STDMETHODIMP get_Waveform(int *Waveform);
    STDMETHODIMP put_Waveform(int Waveform);
    STDMETHODIMP get_Channels(int *Channels);
    STDMETHODIMP get_BitsPerSample(int *BitsPerSample);
    STDMETHODIMP get_SamplesPerSec(int *SamplesPerSec);
    STDMETHODIMP put_Channels(int Channels);
    STDMETHODIMP put_BitsPerSample(int BitsPersample);
    STDMETHODIMP put_SamplesPerSec(int SamplesPerSec);
    STDMETHODIMP get_Amplitude(int *Amplitude);
    STDMETHODIMP put_Amplitude(int Amplitude);
    STDMETHODIMP get_SweepRange(int *SweepStart, int *SweepEnd);
    STDMETHODIMP put_SweepRange(int  SweepStart, int  SweepEnd);
    STDMETHODIMP get_OutputFormat(SYNTH_OUTPUT_FORMAT *pOutputFormat);
    STDMETHODIMP put_OutputFormat(SYNTH_OUTPUT_FORMAT ofOutputFormat);

    CAudioSynth *m_Synth;           // the current synthesizer

private:

    // it is only allowed to to create these objects with CreateInstance
    CSynthFilter(LPUNKNOWN lpunk, HRESULT *phr);

    // When the format changes, reconnect...
    void ReconnectWithNewFormat(void);

};


// -------------------------------------------------------------------------
// CSynthStream
// -------------------------------------------------------------------------
// CSynthStream manages the data flow from the output pin.

class CSynthStream : public CDynamicSourceStream {

public:

    CSynthStream(HRESULT *phr, CSynthFilter *pParent, LPCWSTR pPinName);
    ~CSynthStream();

    BOOL ReadyToStop(void) {return FALSE;}

    // stuff an audio buffer with the current format
    HRESULT FillBuffer(IMediaSample *pms);

    // ask for buffers of the size appropriate to the agreed media type.
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

    HRESULT GetMediaType(CMediaType *pmt);

    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT BreakConnect(void);

    // resets the stream time to zero.
    HRESULT Active(void);

private:

    void DerivePCMFormatFromADPCMFormatStructure(const WAVEFORMATEX& wfexADPCM, WAVEFORMATEX* pwfexPCM);

    // Access to this state information should be serialized with the filters
    // critical section (m_pFilter->pStateLock())

    // This lock protects: m_dwTempPCMBufferSize, m_hPCMToMSADPCMConversionStream,
    // m_rtSampleTime, m_fFirstSampleDelivered and m_llSampleMediaTimeStart
    CCritSec    m_cSharedState;     

    CRefTime     m_rtSampleTime;    // The time to be stamped on each sample
    HACMSTREAM m_hPCMToMSADPCMConversionStream;

    DWORD m_dwTempPCMBufferSize;
    bool m_fFirstSampleDelivered;
    LONGLONG m_llSampleMediaTimeStart;

    CAudioSynth  *m_Synth;          // the current synthesizer
    CSynthFilter *m_pParent;
};

#endif // _AUDIOSYNTH_IMPLEMENTATION_ implementation only....

#endif /* __AUDIOSYNTH__ */



