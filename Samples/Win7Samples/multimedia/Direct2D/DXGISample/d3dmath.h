// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved



/******************************************************************
*                                                                 *
*  d3dmath.h                                                      *
*                                                                 *
*  3D utilities                                                   *
*                                                                 *
******************************************************************/

#pragma once

inline D3DXMATRIX* D3DMatrixIdentity(D3DXMATRIX *pOut)
{
    pOut->m[0][1] = pOut->m[0][2] = pOut->m[0][3] =
    pOut->m[1][0] = pOut->m[1][2] = pOut->m[1][3] =
    pOut->m[2][0] = pOut->m[2][1] = pOut->m[2][3] =
    pOut->m[3][0] = pOut->m[3][1] = pOut->m[3][2] = 0.0f;

    pOut->m[0][0] = pOut->m[1][1] = pOut->m[2][2] = pOut->m[3][3] = 1.0f;
    return pOut;
}

static inline void sincosf(float angle, float *psin, float *pcos)
{
    *psin = sinf(angle);
    *pcos = cosf(angle);
}

inline D3DXMATRIX* D3DMatrixRotationY(D3DXMATRIX* pOut, float angle)
{
    float s, c;
    sincosf(angle, &s, &c);

    pOut->_11 =    c; pOut->_12 = 0.0f; pOut->_13 =   -s; pOut->_14 = 0.0f;
    pOut->_21 = 0.0f; pOut->_22 = 1.0f; pOut->_23 = 0.0f; pOut->_24 = 0.0f;
    pOut->_31 =    s; pOut->_32 = 0.0f; pOut->_33 =    c; pOut->_34 = 0.0f;
    pOut->_41 = 0.0f; pOut->_42 = 0.0f; pOut->_43 = 0.0f; pOut->_44 = 1.0f;

    return pOut;
}

inline D3DXVECTOR3* D3DVec3Subtract(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2)
{
    pOut->x = pV1->x - pV2->x;
    pOut->y = pV1->y - pV2->y;
    pOut->z = pV1->z - pV2->z;
    return pOut;
}

inline FLOAT D3DVec3LengthSq(CONST D3DXVECTOR3 *pV)
{
    return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z;
}


static inline BOOL WithinEpsilon(float a, float b)
{
    float f = a - b;
    return -FLT_EPSILON <= f && f <= FLT_EPSILON;
}


inline D3DXVECTOR3* D3DVec3Normalize(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV)
{
    float f = D3DVec3LengthSq(pV);

    if(WithinEpsilon(f, 1.0f))
    {
        if(pOut != pV)
            *pOut = *pV;
    }
    else if(f > FLT_MIN)
    {
        *pOut = *pV / sqrtf(f);
    }
    else
    {
        pOut->x = 0.0f;
        pOut->y = 0.0f;
        pOut->z = 0.0f;
    }

    return pOut;
}


inline D3DXVECTOR3* D3DVec3Cross(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2)
{
    D3DXVECTOR3 v;

    v.x = pV1->y * pV2->z - pV1->z * pV2->y;
    v.y = pV1->z * pV2->x - pV1->x * pV2->z;
    v.z = pV1->x * pV2->y - pV1->y * pV2->x;

    *pOut = v;
    return pOut;
}

inline FLOAT D3DVec3Dot(CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2)
{
    return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

inline D3DXMATRIX* D3DMatrixLookAtLH(D3DXMATRIX *pOut, const D3DXVECTOR3 *pEye, const D3DXVECTOR3 *pAt, const D3DXVECTOR3 *pUp)
{
    D3DXVECTOR3 XAxis, YAxis, ZAxis;

    // Compute direction of gaze. (+Z)
    D3DVec3Subtract(&ZAxis, pAt, pEye);
    D3DVec3Normalize(&ZAxis, &ZAxis);

    // Compute orthogonal axes from cross product of gaze and pUp vector.
    D3DVec3Cross(&XAxis, pUp, &ZAxis);
    D3DVec3Normalize(&XAxis, &XAxis);
    D3DVec3Cross(&YAxis, &ZAxis, &XAxis);

    // Set rotation and translate by pEye
    pOut->_11 = XAxis.x;
    pOut->_21 = XAxis.y;
    pOut->_31 = XAxis.z;
    pOut->_41 = -D3DVec3Dot(&XAxis, pEye);

    pOut->_12 = YAxis.x;
    pOut->_22 = YAxis.y;
    pOut->_32 = YAxis.z;
    pOut->_42 = -D3DVec3Dot(&YAxis, pEye);

    pOut->_13 = ZAxis.x;
    pOut->_23 = ZAxis.y;
    pOut->_33 = ZAxis.z;
    pOut->_43 = -D3DVec3Dot(&ZAxis, pEye);

    pOut->_14 = 0.0f;
    pOut->_24 = 0.0f;
    pOut->_34 = 0.0f;
    pOut->_44 = 1.0f;

    return pOut;
}

inline D3DXMATRIX* D3DMatrixPerspectiveFovLH(D3DXMATRIX *pOut, float fovy, float aspect, float zn, float zf)
{
    float s, c;
    sincosf(0.5f * fovy, &s, &c);

    float h = c / s;
    float w = h / aspect;

    pOut->_11 = w;
    pOut->_12 = 0.0f;
    pOut->_13 = 0.0f;
    pOut->_14 = 0.0f;

    pOut->_21 = 0.0f;
    pOut->_22 = h;
    pOut->_23 = 0.0f;
    pOut->_24 = 0.0f;

    pOut->_31 = 0.0f;
    pOut->_32 = 0.0f;
    pOut->_33 = zf / (zf - zn);
    pOut->_34 = 1.0f;

    pOut->_41 = 0.0f;
    pOut->_42 = 0.0f;
    pOut->_43 = -pOut->_33 * zn;
    pOut->_44 = 0.0f;

    return pOut;
}
