//////////////////////////////////////////////////////////////////////
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#pragma once

const UINT BACK_BUFFER_COUNT = 1;


//-------------------------------------------------------------------
// D3DHelper
//
// Manages the Direct3D device.
//-------------------------------------------------------------------

struct D3DHelper
{
    IDirect3D9Ex*       m_pD3D;
    IDirect3DDevice9Ex* m_pDevice;

    D3DPRESENT_PARAMETERS   m_d3dpp;

    BOOL                m_bWindowed;
    BOOL                m_bInModeChange;    // If TRUE, a mode change is in progress.
    RECT                m_rcWindow;

    BOOL                m_bAllowHWDevice;   // Allow hardware Direct3D device?
    BOOL                m_bAllowSWDevice;   // Allow software Direct3D device?
    BOOL                m_bDXVA_SW;         // Software DXVA-HD device?

    HMODULE             m_hSWRastDLL;
    PVOID               m_pfnD3D9GetSWInfo;

    D3DHelper() : 
        m_bWindowed(TRUE),
        m_bInModeChange(FALSE),
        m_bAllowSWDevice(TRUE),
        m_bAllowHWDevice(TRUE),
        m_bDXVA_SW(FALSE),
        m_pD3D(NULL),
        m_pDevice(NULL),
        m_hSWRastDLL(NULL),
        m_pfnD3D9GetSWInfo(NULL)
    {
        ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));

        SetRectEmpty(&m_rcWindow);
    }

    ~D3DHelper()
    {
        DestroyD3D9();
    }

    BOOL    InitializeD3D9(HWND hwnd);
    BOOL    RegisterSoftwareRasterizer();
    BOOL    ResetDevice(BOOL bChangeWindowMode);
    BOOL    SetWindowedMode(BOOL bWindowMode);
    HRESULT TestCooperativeLevel();

    void DestroyD3D9()
    {
        SafeRelease(&m_pD3D);
        SafeRelease(&m_pDevice);
    }

};
