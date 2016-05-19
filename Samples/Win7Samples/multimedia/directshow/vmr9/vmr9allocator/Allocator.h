//------------------------------------------------------------------------------
// File: Allocator.h
//
// Desc: DirectShow sample code - interface for the CAllocator class
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_ALLOCATOR_H__F675D766_1E57_4269_A4B9_C33FB672B856__INCLUDED_)
#define AFX_ALLOCATOR_H__F675D766_1E57_4269_A4B9_C33FB672B856__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vmr9.h>
//#include <Wxutil.h>

#pragma warning(push, 2)

// C4995'function': name was marked as #pragma deprecated
//
// The version of vector which shipped with Visual Studio .NET 2003 
// indirectly uses some deprecated functions.  Warning C4995 is disabled 
// because the file cannot be changed and we do not want to 
// display warnings which the user cannot fix.
#pragma warning(disable : 4995)

#include <vector>
#pragma warning(pop)
using namespace std;

#include "PlaneScene.h"


class CAllocator  : public  IVMRSurfaceAllocator9, 
                            IVMRImagePresenter9
{
public:
    CAllocator(HRESULT& hr, HWND wnd, IDirect3D9* d3d = NULL, IDirect3DDevice9* d3dd = NULL);
    virtual ~CAllocator();

    // IVMRSurfaceAllocator9
    virtual HRESULT STDMETHODCALLTYPE InitializeDevice( 
            /* [in] */ DWORD_PTR dwUserID,
            /* [in] */ VMR9AllocationInfo *lpAllocInfo,
            /* [out][in] */ DWORD *lpNumBuffers);
            
    virtual HRESULT STDMETHODCALLTYPE TerminateDevice( 
        /* [in] */ DWORD_PTR dwID);
    
    virtual HRESULT STDMETHODCALLTYPE GetSurface( 
        /* [in] */ DWORD_PTR dwUserID,
        /* [in] */ DWORD SurfaceIndex,
        /* [in] */ DWORD SurfaceFlags,
        /* [out] */ IDirect3DSurface9 **lplpSurface);
    
    virtual HRESULT STDMETHODCALLTYPE AdviseNotify( 
        /* [in] */ IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify);

    // IVMRImagePresenter9
    virtual HRESULT STDMETHODCALLTYPE StartPresenting( 
        /* [in] */ DWORD_PTR dwUserID);
    
    virtual HRESULT STDMETHODCALLTYPE StopPresenting( 
        /* [in] */ DWORD_PTR dwUserID);
    
    virtual HRESULT STDMETHODCALLTYPE PresentImage( 
        /* [in] */ DWORD_PTR dwUserID,
        /* [in] */ VMR9PresentationInfo *lpPresInfo);
    
    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        REFIID riid,
        void** ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

protected:
    HRESULT CreateDevice();

    // a helper function to erase every surface in the vector
    void DeleteSurfaces();

    bool NeedToHandleDisplayChange();

    // This function is here so we can catch the loss of surfaces.
    // All the functions are using the FAIL_RET macro so that they exit
    // with the last error code.  When this returns with the surface lost
    // error code we can restore the surfaces.
    HRESULT PresentHelper(VMR9PresentationInfo *lpPresInfo);

private:
    // needed to make this a thread safe object
    CCritSec    m_ObjectLock;
    HWND        m_window;
    long        m_refCount;

    SmartPtr<IDirect3D9>                     m_D3D;
    SmartPtr<IDirect3DDevice9>               m_D3DDev;
    SmartPtr<IVMRSurfaceAllocatorNotify9>    m_lpIVMRSurfAllocNotify;
    vector<SmartPtr<IDirect3DSurface9> >     m_surfaces;
    SmartPtr<IDirect3DSurface9>              m_renderTarget;
    SmartPtr<IDirect3DTexture9>              m_privateTexture;
    CPlaneScene                             m_scene;
};

#endif // !defined(AFX_ALLOCATOR_H__F675D766_1E57_4269_A4B9_C33FB672B856__INCLUDED_)
