//------------------------------------------------------------------------------
// File: ISynth.h
//
// Desc: DirectShow sample code - custom interface to allow the user to
//       adjust the frequency.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __ISYNTH2__
#define __ISYNTH2__

#ifdef __cplusplus
extern "C" {
#endif


//
// ISynth2's GUID
//
// {00487A78-D875-44b0-ADBB-DECA9CDB51FC}
DEFINE_GUID(IID_ISynth2, 
0x487a78, 0xd875, 0x44b0, 0xad, 0xbb, 0xde, 0xca, 0x9c, 0xdb, 0x51, 0xfc);

enum SYNTH_OUTPUT_FORMAT
{
    SYNTH_OF_PCM,
    SYNTH_OF_MS_ADPCM
};

//
// ISynth2
//
DECLARE_INTERFACE_(ISynth2, IUnknown) {

    STDMETHOD(get_Frequency) (THIS_
                int *Frequency          /* [out] */    // the current frequency
             ) PURE;

    STDMETHOD(put_Frequency) (THIS_
                int    Frequency        /* [in] */    // Change to this frequency
             ) PURE;

    STDMETHOD(get_Waveform) (THIS_
                int *Waveform           /* [out] */    // the current Waveform
             ) PURE;

    STDMETHOD(put_Waveform) (THIS_
                int    Waveform         /* [in] */    // Change to this Waveform
             ) PURE;

    STDMETHOD(get_Channels) (THIS_
                int *Channels           /* [out] */   // the current Channels
             ) PURE;

    STDMETHOD(put_Channels) (THIS_
                int    Channels         /* [in] */    // Change to this Channels
             ) PURE;

    STDMETHOD(get_BitsPerSample) (THIS_
                int *BitsPerSample      /* [out] */   // the current BitsPerSample
             ) PURE;

    STDMETHOD(put_BitsPerSample) (THIS_
                int    BitsPerSample    /* [in] */    // Change to this BitsPerSample
             ) PURE;

    STDMETHOD(get_SamplesPerSec) (THIS_
                 int *SamplesPerSec     /* [out] */   // the current SamplesPerSec
             ) PURE;

    STDMETHOD(put_SamplesPerSec) (THIS_
                  int    SamplesPerSec  /* [in] */    // Change to this SamplesPerSec
             ) PURE;

    STDMETHOD(get_Amplitude) (THIS_
                  int *Amplitude        /* [out] */   // the current Amplitude
             ) PURE;

    STDMETHOD(put_Amplitude) (THIS_
                  int    Amplitude      /* [in] */    // Change to this Amplitude
              ) PURE;

    STDMETHOD(get_SweepRange) (THIS_
                  int *SweepStart,      /* [out] */
                  int *SweepEnd         /* [out] */
             ) PURE;

    STDMETHOD(put_SweepRange) (THIS_
                  int    SweepStart,    /* [in] */
                  int    SweepEnd       /* [in] */
             ) PURE;

    STDMETHOD(get_OutputFormat) (THIS_
                  SYNTH_OUTPUT_FORMAT *pOutputFormat /* [out] */
             ) PURE;

    STDMETHOD(put_OutputFormat) (THIS_
                  SYNTH_OUTPUT_FORMAT ofNewOutputFormat /* [out] */
             ) PURE;
    
};


#ifdef __cplusplus
}
#endif

#endif // __ISYNTH2__


