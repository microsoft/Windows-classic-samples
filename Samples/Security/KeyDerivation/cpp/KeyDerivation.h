// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:             KeyDerivation.h
//
//  Contents:         This file defines the parameters required for key derivation.
//    
//

#include <ncrypt.h>

#define DERIVED_KEY_LEN 60 

static const 
BYTE Secret[20] = 
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
};

//
// Algorithm name array
//

static
LPCWSTR KdfAlgorithmNameArray[] = {
    BCRYPT_SP800108_CTR_HMAC_ALGORITHM,
    BCRYPT_SP80056A_CONCAT_ALGORITHM,
    BCRYPT_PBKDF2_ALGORITHM,
    BCRYPT_CAPI_KDF_ALGORITHM,
};

//
// Sample Parameters for SP800-108 KDF
// 

static
BYTE Label[] =
{
    0x41,0x4C,0x49,0x43,0x45,0x31,0x32,0x33,0x00
};

static
WCHAR Context[] = L"Context";

static
BCryptBuffer SP800108ParamBuffer[]= {
    {
        sizeof(Label),
        KDF_LABEL,
        (PBYTE)Label,
    },
    {
        sizeof(Context),
        KDF_CONTEXT,
        (PBYTE)Context,
    },
    {
        sizeof(BCRYPT_SHA256_ALGORITHM),
        KDF_HASH_ALGORITHM,
        BCRYPT_SHA256_ALGORITHM,
    }
};

//
// Sample Parameters for SP800-56A KDF
// 

static
BYTE AlgorithmID[]    =
{
    0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0
};

static
BYTE PartyUInfo[] =
{
    0x41,0x4C,0x49,0x43,0x45,0x31,0x32,0x33
};

static
BYTE PartyVInfo[] =
{
    0x42,0x4F,0x42,0x42,0x59,0x34,0x35,0x36
};

static 
BCryptBuffer SP80056AParamBuffer[] =
{
     {
        sizeof(AlgorithmID),
        KDF_ALGORITHMID,
        (PBYTE)AlgorithmID,
    },
    {
        sizeof(PartyUInfo),
        KDF_PARTYUINFO,
        (PBYTE)PartyUInfo,
    },
    {
        sizeof(PartyVInfo),
        KDF_PARTYVINFO,
        (PBYTE)PartyVInfo,
    },
    {
        sizeof(BCRYPT_SHA256_ALGORITHM),
        KDF_HASH_ALGORITHM,
        BCRYPT_SHA256_ALGORITHM,
    }
};

//
// Sample Parameters for PBKDF2
// 

static
BYTE Salt[] =
{
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
};

static
ULONGLONG IterationCount = 12000;

static 
BCryptBuffer PBKDF2ParamBuffer[] =
{
     {
        sizeof(Salt),
        KDF_SALT,
        (PBYTE)Salt,
    },
    {
        sizeof(IterationCount),
        KDF_ITERATION_COUNT,
        (PBYTE)&IterationCount,
    },
    {
        sizeof(BCRYPT_SHA256_ALGORITHM),
        KDF_HASH_ALGORITHM,
        BCRYPT_SHA256_ALGORITHM,
    }
};

//
// Sample Parameters for CAPI_KDF
// 

static 
BCryptBuffer CAPIParamBuffer[] =
{
    {
        sizeof(BCRYPT_SHA256_ALGORITHM),
        KDF_HASH_ALGORITHM,
        BCRYPT_SHA256_ALGORITHM,
    }
};

static
BCryptBufferDesc ParamList[] = 
{
    {
        BCRYPTBUFFER_VERSION,
        3,
        SP800108ParamBuffer
    },
    {
        BCRYPTBUFFER_VERSION,
        4,
        SP80056AParamBuffer
    },
    {
        BCRYPTBUFFER_VERSION,
        3,
        PBKDF2ParamBuffer
    },
    {
        BCRYPTBUFFER_VERSION,
        1,
        CAPIParamBuffer
    },
};
