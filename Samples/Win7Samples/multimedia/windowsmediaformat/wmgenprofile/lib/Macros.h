//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Macros.h
//
// Abstract:            Contains macros used to perform operations safely on
//                      pointers or handles
//
//*****************************************************************************

#ifndef _MACROS_H_
#define _MACROS_H_

#define SAFE_RELEASE( x )   \
    if ( x )                \
    {                       \
        x->Release();       \
        x = NULL;           \
    }
    
#define SAFE_ADDREF( x )    \
    if ( x )                \
    {                       \
        x->AddRef();        \
    }

#define SAFE_DELETE( x )    \
    if ( x )                \
    {                       \
        delete x;           \
        x = NULL;           \
    }

#define SAFE_ARRAYDELETE( x )   \
    if ( x )                    \
    {                           \
        delete[] x;             \
        x = NULL;               \
    }

#define SAFE_SYSFREESTRING( x ) \
    if ( x )                    \
    {                           \
        SysFreeString( x );     \
        x = NULL;               \
    }

#define SAFE_CLOSEHANDLE( x )               \
    if ( x && INVALID_HANDLE_VALUE != x )   \
    {                                       \
        CloseHandle( x );                   \
        x = NULL;                           \
    }


#endif // _MACROS_H_
