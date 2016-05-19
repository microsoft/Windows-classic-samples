//------------------------------------------------------------------------------
// File: PlaneScene.h
//
// Desc: DirectShow sample code - interface for the CPlaneScene class
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#if !defined(AFX_PLANESCENE_H__B0ED1D62_D626_479A_925D_7488767DF129__INCLUDED_)
#define AFX_PLANESCENE_H__B0ED1D62_D626_479A_925D_7488767DF129__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <d3d9.h>
#include <time.h>

// Common files
#include "SmartPtr.h"


const int NUM_VERTICES = 4;


class CPlaneScene  
{
public:

    CPlaneScene();
    virtual ~CPlaneScene();

    HRESULT Init(IDirect3DDevice9* d3ddev);

    HRESULT DrawScene(  IDirect3DDevice9* d3ddev,
                        IDirect3DTexture9* texture );

    void SetSrcRect( float fTU, float fTV );

private:

    struct CUSTOMVERTEX
    {
        struct Position {
            Position() : 
                x(0.0f),y(0.0f),z(0.0f) {            
            };
            Position(float x_, float y_, float z_) :
                x(x_),y(y_),z(z_) {
            };
            float x,y,z;
        };

        Position    position; // The position
        D3DCOLOR    color;    // The color
        FLOAT       tu, tv;   // The texture coordinates
    };

    CUSTOMVERTEX                    m_vertices[NUM_VERTICES];
    SmartPtr<IDirect3DVertexBuffer9> m_vertexBuffer;

    DWORD  m_time;
};

#endif // !defined(AFX_PLANESCENE_H__B0ED1D62_D626_479A_925D_7488767DF129__INCLUDED_)
