// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:       
//      MathInputControl.h
//
// Description:
//      The header file for the CMathInputControlHost and 
//      CMathInputControlEventListener classes. 
//
//      CMathInputControlHost class is a host for Math Input Control object and
//      user interface objects which interact with the control (main application
//      window and edit box control).
//      The methods of the class are defined in the MathInputControl.cpp file.
//
//      CMathInputControlEventListener class is the basic implementation of the
//      Math Input Control events. It passes all events to the CMathInputControlHost
//      objeect for processing.
//
//--------------------------------------------------------------------------

#pragma once

/////////////////////////////////////////////////////////////////////////////
// MathInputControlHost

class CMathInputControlHost
{
private:
    // Main application window
    HWND m_hWnd;

    // Edit box for displaying recognition result
    HWND m_hWndEdit;

    // Pointer to Math Input Control object
    IMathInputControl* m_pIMathInputControl;

    // Constants for initial size of the Math Input Control and its position
    // on the screen.
    static const int mc_left = 100;
    static const int mc_top = 100;
    static const int mc_width = 400;
    static const int mc_height = 500;

    // Minimal implementation of the Math Input Control events.
    // All events are redirected to the host object.
    class CMathInputControlEventListener : public IMathInputControlEvents
    {
    public:
        CMathInputControlEventListener(CMathInputControlHost* pMathInputControlHost) 
            : p(pMathInputControlHost)
        {
        }

    private:
        // Pointer to the host objects. All events are redirected to host.
        CMathInputControlHost* p;

        // IMathInputControl events
        HRESULT OnMICInsert(BSTR RecoResult)
        {
            return p->OnMICInsert(RecoResult);
        }
        HRESULT OnMICClose(void)
        {
            return p->OnMICClose();
        }
        HRESULT OnMICClear(void)
        {
            return p->OnMICClear();
        }
    };
    CMathInputControlEventListener* m_pEventListener;

public:
    // Constructor
    CMathInputControlHost()
    {
        m_hWnd = NULL;
        m_hWndEdit = NULL;
        m_pIMathInputControl = NULL;
        m_pEventListener = NULL;
    }

    // Math Input Control initialization.
    // Should be called right after constructor.
    HRESULT Init(HWND hWnd, HWND hWndEdit);

    // Destructor
    ~CMathInputControlHost()
    {
        if (m_pIMathInputControl)
        {
            m_pIMathInputControl->Release();
        }
        if (m_pEventListener)
        {
            delete m_pEventListener;
        }
    }

    // Command handlers
    LRESULT OnMICShow();

    // Accessors
    HWND GetEditWindow() const
    {
        return m_hWndEdit;
    }

private:
    // Helper methods
    HRESULT HideMIC();

    // Actions triggered on Math Input Control events
    HRESULT OnMICInsert(BSTR RecoResult);
    HRESULT OnMICClose(void);
    HRESULT OnMICClear(void);
};
