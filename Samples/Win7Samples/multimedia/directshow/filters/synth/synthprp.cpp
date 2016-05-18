//------------------------------------------------------------------------------
// File: SynthPrp.cpp
//
// Desc: DirectShow sample code - implements property page for synthesizer.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>
#include <streams.h>

#include <commctrl.h>
#include <memory.h>
#include <olectl.h>

#include "isynth.h"
#include "synth.h"
#include "synthprp.h"
#include "resource.h"

#pragma warning(disable:4127)   // C4127: conditional expression is constant

// -------------------------------------------------------------------------
// CSynthProperties
// -------------------------------------------------------------------------

//
// CreateInstance
//

CUnknown * WINAPI CSynthProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CSynthProperties(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}


//
// Constructor
//
// Creaete a Property page object for the synthesizer

CSynthProperties::CSynthProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage(NAME("Synth Property Page"), lpunk,
        IDD_SYNTHPROP1,IDS_SYNTHPROPNAME)
    , m_pSynth(NULL)
    , m_iSweepStart(DefaultSweepStart)
    , m_iSweepEnd(DefaultSweepEnd)
    , m_fWindowInActive(TRUE)
{
    ASSERT(phr);

    InitCommonControls();
}

//
// OnConnect
//
// Give us the filter to communicate with

HRESULT CSynthProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pSynth == NULL);

    // Ask the filter for it's control interface

    HRESULT hr = pUnknown->QueryInterface(IID_ISynth2,(void **)&m_pSynth);
    if (FAILED(hr)) {
        return hr;
    }

    ASSERT(m_pSynth);

    // Get current filter state
    m_pSynth->get_BitsPerSample(&m_iBitsPerSampleOriginal);
    m_pSynth->get_Waveform(&m_iWaveformOriginal);
    m_pSynth->get_Frequency(&m_iFrequencyOriginal);
    m_pSynth->get_Channels(&m_iChannelsOriginal);
    m_pSynth->get_SamplesPerSec(&m_iSamplesPerSecOriginal);
    m_pSynth->get_Amplitude(&m_iAmplitudeOriginal);
    m_pSynth->get_OutputFormat(&m_OutputFormat);

    return NOERROR;
}


//
// OnDisconnect
//
// Release the interface

HRESULT CSynthProperties::OnDisconnect()
{
    // Release the interface

    if (m_pSynth == NULL) {
        return E_UNEXPECTED;
    }

    m_pSynth->put_Waveform(m_iWaveformOriginal);
    m_pSynth->put_Frequency(m_iFrequencyOriginal);
    m_pSynth->put_Amplitude(m_iAmplitudeOriginal);

    m_pSynth->put_Channels(m_iChannelsOriginal);
    m_pSynth->put_BitsPerSample(m_iBitsPerSampleOriginal);
    m_pSynth->put_SamplesPerSec(m_iSamplesPerSecOriginal);

    m_pSynth->put_OutputFormat(m_OutputFormat);

    m_pSynth->Release();
    m_pSynth = NULL;
    return NOERROR;
}


//
// OnActivate
//
// Called on dialog creation

HRESULT CSynthProperties::OnActivate(void)
{
    InitPropertiesDialog(m_hwnd);

    ASSERT(m_hwndFreqSlider);

    m_fWindowInActive = FALSE;
    
    return NOERROR;
}

//
// OnDeactivate
//
// Called on dialog destruction

HRESULT
CSynthProperties::OnDeactivate(void)
{
    m_fWindowInActive = TRUE;
    return NOERROR;
}


//
// OnApplyChanges
//
// User pressed the Apply button, remember the current settings

HRESULT CSynthProperties::OnApplyChanges(void)
{
    m_pSynth->get_BitsPerSample(&m_iBitsPerSampleOriginal);
    m_pSynth->get_Waveform(&m_iWaveformOriginal);
    m_pSynth->get_Frequency(&m_iFrequencyOriginal);
    m_pSynth->get_Channels(&m_iChannelsOriginal);
    m_pSynth->get_SamplesPerSec(&m_iSamplesPerSecOriginal);
    m_pSynth->get_Amplitude(&m_iAmplitudeOriginal);
    m_pSynth->get_OutputFormat(&m_OutputFormat);

    return NOERROR;
}


//
// OnReceiveMessages
//
// Handles the messages for our property window

INT_PTR CSynthProperties::OnReceiveMessage(HWND hwnd
                                         , UINT uMsg
                                         , WPARAM wParam
                                         , LPARAM lParam)
{
    HRESULT hr;

    if(m_fWindowInActive)
        return FALSE;

    switch (uMsg) {

    case WM_PROPERTYPAGE_ENABLE:
        // Our private message that our owning filter sends us when changing to a Run / Stop / Pause
        // state.  if lParam, then enable the controls which affect the format; if not lParam, then
        // disable the controls that affect the format.

        EnableWindow (GetDlgItem (hwnd, IDC_SAMPLINGFREQ11), (BOOL) lParam);
        EnableWindow (GetDlgItem (hwnd, IDC_SAMPLINGFREQ22), (BOOL) lParam);
        EnableWindow (GetDlgItem (hwnd, IDC_SAMPLINGFREQ44), (BOOL) lParam);

        EnableWindow (GetDlgItem (hwnd, IDC_BITSPERSAMPLE8),  (BOOL) lParam);
        EnableWindow (GetDlgItem (hwnd, IDC_BITSPERSAMPLE16), (BOOL) lParam);

        EnableWindow (GetDlgItem (hwnd, IDC_CHANNELS1), (BOOL) lParam);
        EnableWindow (GetDlgItem (hwnd, IDC_CHANNELS2), (BOOL) lParam);
        break;

    case WM_VSCROLL:
        if ((HWND) lParam == m_hwndFreqSlider)
            OnFreqSliderNotification(LOWORD (wParam), HIWORD (wParam));
        else if ((HWND) lParam == m_hwndAmplitudeSlider)
            OnAmpSliderNotification(LOWORD (wParam), HIWORD (wParam));
        SetDirty();
        return TRUE;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDC_FREQUENCYTEXT:
        {
            int iNotify = HIWORD (wParam);

            if (iNotify == EN_KILLFOCUS) {
                BOOL fOK;

                int iPos = GetDlgItemInt (hwnd, IDC_FREQUENCYTEXT, &fOK, FALSE);
                int iMaxFreq;

                m_pSynth->get_SamplesPerSec(&iMaxFreq);
                iMaxFreq /= 2;

                if (!fOK || (iPos > iMaxFreq || iPos < 0)) {
                    int iCurrentFreq;
                    m_pSynth->get_Frequency(&iCurrentFreq);
                    SetDlgItemInt (hwnd, IDC_FREQUENCYTEXT, iCurrentFreq, FALSE);
                    break;
                }

                SendMessage(m_hwndFreqSlider, TBM_SETPOS, TRUE, (LPARAM) iMaxFreq - iPos);
                m_pSynth->put_Frequency(iPos);
                SetDirty();
            }
        }
        break;

        case IDC_AMPLITUDETEXT:
        {
            int iNotify = HIWORD (wParam);

            if (iNotify == EN_KILLFOCUS) {
                BOOL fOK;

                int iPos = GetDlgItemInt (hwnd, IDC_AMPLITUDETEXT, &fOK, FALSE);

                if (!fOK || (iPos > MaxAmplitude || iPos < 0)) {
                    int iCurrentAmplitude;

                    m_pSynth->get_Amplitude(&iCurrentAmplitude);
                    SetDlgItemInt (hwnd, IDC_AMPLITUDETEXT, iCurrentAmplitude, FALSE);
                    break;
                }

                SendMessage(m_hwndAmplitudeSlider, TBM_SETPOS, TRUE, (LPARAM) MaxAmplitude - iPos);
                m_pSynth->put_Amplitude(iPos);
                SetDirty();
            }
        }
        break;

        case IDC_SAMPLINGFREQ11:
            m_pSynth->put_SamplesPerSec(11025);
            RecalcFreqSlider();
            SetDirty();
            break;
        case IDC_SAMPLINGFREQ22:
            m_pSynth->put_SamplesPerSec(22050);
            RecalcFreqSlider();
            SetDirty();
            break;
        case IDC_SAMPLINGFREQ44:
            m_pSynth->put_SamplesPerSec(44100);
            RecalcFreqSlider();
            SetDirty();
            break;


        case IDC_BITSPERSAMPLE8:
            m_pSynth->put_BitsPerSample(8);
            SetDirty();
            break;
        case IDC_BITSPERSAMPLE16:
            m_pSynth->put_BitsPerSample(16);
            SetDirty();
            break;


        case IDC_CHANNELS1:
            m_pSynth->put_Channels(1);
            SetDirty();
            break;
        case IDC_CHANNELS2:
            m_pSynth->put_Channels(2);
            SetDirty();
            break;
        

        case IDC_WAVESINE:
            m_pSynth->put_Waveform(WAVE_SINE);
            SetDirty();
            break;
        case IDC_WAVESQUARE:
            m_pSynth->put_Waveform(WAVE_SQUARE);
            SetDirty();
            break;
        case IDC_WAVESAWTOOTH:
            m_pSynth->put_Waveform(WAVE_SAWTOOTH);
            SetDirty();
            break;
        case IDC_WAVESWEEP:
            m_pSynth->put_Waveform(WAVE_SINESWEEP);
            SetDirty();
            break;

        case IDC_OF_PCM:
            hr = m_pSynth->put_OutputFormat(SYNTH_OF_PCM);
            if (SUCCEEDED(hr)) {
                EnableBitsPerSampleRadioButtons(hwnd, TRUE);
            } else {
                MessageBeep(MB_ICONASTERISK);
            }

            SetDirty();
            break;

        case IDC_OF_MSADPCM:
            hr = m_pSynth->put_OutputFormat(SYNTH_OF_MS_ADPCM);
            if (SUCCEEDED(hr)) {
                EnableBitsPerSampleRadioButtons(hwnd, FALSE);
            } else {
                MessageBeep(MB_ICONASTERISK);
            }
            SetDirty();
            break;

        default:
            break;

        }
        return TRUE;

    case WM_DESTROY:
        return TRUE;

    default:
        return FALSE;

    }
    return TRUE;
}


//
// InitPropertiesDialog
//

void
CSynthProperties::InitPropertiesDialog(HWND hwndParent)
{
    m_hwndFreqSlider = GetDlgItem (hwndParent, IDC_FREQTRACKBAR);
    m_hwndFreqText  = GetDlgItem (hwndParent, IDC_FREQUENCYTEXT);
    m_hwndAmplitudeSlider = GetDlgItem (hwndParent, IDC_AMPLITUDETRACKBAR);
    m_hwndAmplitudeText  = GetDlgItem (hwndParent, IDC_AMPLITUDETEXT);

    // Sampling Frequency
    int i=0;
    switch (m_iSamplesPerSecOriginal) {
        case 11025: i = IDC_SAMPLINGFREQ11; break;
        case 22050: i = IDC_SAMPLINGFREQ22; break;
        case 44100: i = IDC_SAMPLINGFREQ44; break;
        default:
            ASSERT(0);  break;
    }
    CheckRadioButton(hwndParent,
        IDC_SAMPLINGFREQ11,
        IDC_SAMPLINGFREQ44,
        i);

    // BitsPerSample
    CheckRadioButton(hwndParent,
                IDC_BITSPERSAMPLE8,
                IDC_BITSPERSAMPLE16,
                IDC_BITSPERSAMPLE8 + m_iBitsPerSampleOriginal / 8 - 1);

    // Waveform 0 == sine, 1 == square, ...
    CheckRadioButton(hwndParent,
                IDC_WAVESINE,
                IDC_WAVESWEEP,
                IDC_WAVESINE + m_iWaveformOriginal);

    // Channels
    CheckRadioButton(hwndParent,
                IDC_CHANNELS1,
                IDC_CHANNELS2,
                IDC_CHANNELS1 + m_iChannelsOriginal - 1);

    CheckRadioButton(hwndParent,
                IDC_OF_PCM,
                IDC_OF_MSADPCM,
                IDC_OF_PCM + (int)m_OutputFormat);

    if(SYNTH_OF_MS_ADPCM == m_OutputFormat) {
        EnableBitsPerSampleRadioButtons(hwndParent, FALSE); 
    } else {
        // The synth filter only supports two output formats: SYNTH_OF_PCM and SYNTH_OF_MS_ADPCM.
        ASSERT(SYNTH_OF_PCM == m_OutputFormat);
        
        EnableBitsPerSampleRadioButtons(hwndParent, TRUE); 
    }

    //
    // Frequency trackbar
    //

    RecalcFreqSlider();

    //
    //  Amplitude trackbar
    //

    SendMessage(m_hwndAmplitudeSlider, TBM_SETRANGE, TRUE,
        MAKELONG(MinAmplitude, MaxAmplitude) );

    SendMessage(m_hwndAmplitudeSlider, TBM_SETTIC, 0,
        ((MinAmplitude + MaxAmplitude) / 2));

    SendMessage(m_hwndAmplitudeSlider, TBM_SETPOS, TRUE,
        (LPARAM) (MaxAmplitude - m_iAmplitudeOriginal));

    SetDlgItemInt (hwndParent, IDC_AMPLITUDETEXT,
        m_iAmplitudeOriginal, TRUE);
}


//
// RecalcFreqSlider
//
// Set the range, current settings for the Freq scrollbar

void
CSynthProperties::RecalcFreqSlider(void)
{
    int iPos, iMaxFreq;

    // Limit the frequency to one half the sampling frequency

    m_pSynth->get_SamplesPerSec(&iMaxFreq);
    iMaxFreq /= 2;
    m_pSynth->get_Frequency(&iPos);
    if (iPos > iMaxFreq)
        iPos = iMaxFreq;

    SendMessage(m_hwndFreqSlider, TBM_SETRANGE, TRUE,
        MAKELONG(0, iMaxFreq));

    SendMessage(m_hwndFreqSlider, TBM_SETTIC, 0,
        iMaxFreq / 2);

    SendMessage(m_hwndFreqSlider, TBM_SETPOS, TRUE,
        (LPARAM) (iMaxFreq - iPos));

    SendMessage(m_hwndFreqSlider, TBM_SETPAGESIZE, 0, 10);

    SendMessage(m_hwndFreqSlider, TBM_SETSEL, TRUE,
        MAKELONG (iMaxFreq - m_iSweepEnd, iMaxFreq - m_iSweepStart));

    SetDlgItemInt (m_hwnd, IDC_FREQUENCYTEXT,
        iPos, TRUE);

}

//
// OnFreqSliderNotification
//
// Handle the notification meesages from the slider control

void
CSynthProperties::OnFreqSliderNotification(WPARAM wParam, WORD wPosition)
{
    int MaxFreq;
    int Freq;
    int SliderPos;

    switch (wParam) {

    case TB_ENDTRACK:
    case TB_THUMBTRACK:
    case TB_LINEDOWN:
    case TB_LINEUP: {
        // max frequency of slider is half the sampling frequency
        m_pSynth->get_SamplesPerSec (&MaxFreq);
        MaxFreq /= 2;
        SliderPos = (int) SendMessage(m_hwndFreqSlider, TBM_GETPOS, 0, 0L);
        Freq = MaxFreq - SliderPos;
        m_pSynth->put_Frequency (Freq);

        // Set the end of sweep to the current slider pos
        if (!(GetKeyState (VK_SHIFT) & 0x8000)) {
            m_iSweepEnd = Freq;
        }

        // Set the start of the sweep range if SHIFT key is pressed
        if (GetKeyState (VK_SHIFT) & 0x8000) {
            m_iSweepStart = Freq;
        }
        m_pSynth->put_SweepRange (m_iSweepStart, m_iSweepEnd);

        if (m_iSweepEnd > m_iSweepStart)
            SendMessage(m_hwndFreqSlider, TBM_SETSEL, TRUE,
                MAKELONG (MaxFreq - m_iSweepEnd, MaxFreq - m_iSweepStart));
        else
            SendMessage(m_hwndFreqSlider, TBM_SETSEL, TRUE,
                MAKELONG (MaxFreq - m_iSweepStart, MaxFreq - m_iSweepEnd));

        SetDlgItemInt (m_hwnd, IDC_FREQUENCYTEXT, Freq, TRUE);

    }
    break;

    }
}

//
// OnAmpSliderNotification
//
// Handle the notification meesages from the slider control

void
CSynthProperties::OnAmpSliderNotification(WPARAM wParam, WORD wPosition)
{
    switch (wParam) {

    case TB_ENDTRACK:
    case TB_THUMBTRACK:
    case TB_LINEDOWN:
    case TB_LINEUP: {
        int Level = (int) SendMessage(m_hwndAmplitudeSlider, TBM_GETPOS, 0, 0L);
        m_pSynth->put_Amplitude (MaxAmplitude - Level);
        SetDlgItemInt (m_hwnd, IDC_AMPLITUDETEXT, MaxAmplitude - Level, TRUE);
    }
    break;

    }
}

//
// SetDirty
//
// notifies the property page site of changes

void
CSynthProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}

void
CSynthProperties::EnableBitsPerSampleRadioButtons(HWND hwndParent, BOOL fEnable)
{
    EnableWindow (GetDlgItem (hwndParent, IDC_BITSPERSAMPLE8),  fEnable);
    EnableWindow (GetDlgItem (hwndParent, IDC_BITSPERSAMPLE16), fEnable);
}
