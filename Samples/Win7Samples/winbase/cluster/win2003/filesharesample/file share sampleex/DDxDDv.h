/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      DDxDDv.h
//
//  Implementation File:
//      DDxDDv.cpp
//
//  Description:
//      Definition of custom dialog data exchange/dialog data validation
//      routines.
//
//  Author:
//      <name> (<e-mail name>) Mmmm DD, 2003
//
//  Revision History:
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Include Files
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Global Function Prototypes
/////////////////////////////////////////////////////////////////////////////

void AFXAPI
DDX_Number(
      CDataExchange *   pDXIn
    , int               nIDCIn
    , DWORD &           rdwValueInout
    , DWORD             dwMinIn
    , DWORD             dwMaxIn
    , BOOL              fSignedIn = FALSE
    );

void AFXAPI
DDV_RequiredText(
      CDataExchange *   pDXIn
    , int               nIDCIn
    , int               nIDCLabelIn
    , const CString &   rstrValueIn
    );

inline void AFXAPI
DDX_Number(
      CDataExchange *  pDXIn
    , int              nIDCIn
    , LONG &           rnValueInout
    , LONG             nMinIn
    , LONG             nMaxIn
    , BOOL             fSignedIn
    )
{
    DDX_Number(
          pDXIn
        , nIDCIn
        , reinterpret_cast< DWORD & >( rnValueInout )
        , static_cast< DWORD >( nMinIn )
        , static_cast< DWORD >( nMaxIn )
        , fSignedIn
        );

} //*** DDXNumber

void
CleanupLabel( LPTSTR pwszIn );

/////////////////////////////////////////////////////////////////////////////
