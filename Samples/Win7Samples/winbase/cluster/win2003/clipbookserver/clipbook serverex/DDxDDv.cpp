/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      DDxDDv.cpp
//
//  Description:
//      Implementation of custom dialog data exchange/dialog data validation
//      routines.
//
//  Author:
//      <name> (<e-mail name>) Mmmm DD, 2003
//
//  Revision History:
//
//  Notes:
//      The IDS_REQUIRED_FIELD_EMPTY string resource must be defined in
//      the resource file.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DDxDDv.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//++
//
//  DDX_Number
//
//  Description:
//      Do data exchange between the dialog and the class.
//
//  Arguments:
//      pDXIn
//          Data exchange object.
//
//      nIDCIn
//          Control ID.
//
//      rdwValueInout
//          Value to set or get.
//
//      dwMinIn
//          Minimum value.
//
//      dwMaxIn
//          Maximum value.
//
//      fSignedIn
//          TRUE = value is signed, FALSE = value is unsigned
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void AFXAPI
DDX_Number(
      CDataExchange *   pDXIn
    , int               nIDCIn
    , DWORD &           rdwValueInout
    , DWORD             dwMinIn
    , DWORD             dwMaxIn
    , BOOL              fSignedIn /* =FALSE */
    )
{
    HWND    hwndCtrl = NULL;
    DWORD   dwValue = 0;
    HRESULT hr = S_OK;

    ASSERT( pDXIn != NULL );
#ifdef _DEBUG
    if ( fSignedIn )
    {
        ASSERT( static_cast< LONG >( dwMinIn ) < static_cast< LONG >( dwMaxIn ) );
    }
    else
    {
        ASSERT( dwMinIn < dwMaxIn );
    }
#endif // _DEBUG

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    //
    //  Get the control window handle.
    //

    hwndCtrl = pDXIn->PrepareEditCtrl( nIDCIn );

    if ( pDXIn->m_bSaveAndValidate )
    {
        BOOL    bTranslated;

        //
        //  Get the number from the control.
        //

        dwValue = GetDlgItemInt( pDXIn->m_pDlgWnd->m_hWnd, nIDCIn, &bTranslated, fSignedIn );

        //
        // If the retrival failed, it is a signed number, and the minimum
        // value is the smallest negative value possible, check the string itself.
        //

        if ( ! bTranslated && fSignedIn && (dwMinIn == 0x80000000) )
        {
            UINT    cch = 0;
            TCHAR   szNumber[ 20 ];

            //
            //  See if it is the smallest negative number.
            //

            cch = GetDlgItemText( pDXIn->m_pDlgWnd->m_hWnd, nIDCIn, szNumber, RTL_NUMBER_OF( szNumber ) );
            if ( (cch != 0) && (_tcsnicmp( szNumber, _T("-2147483648"), RTL_NUMBER_OF( szNumber ) ) == 0) )
            {
                dwValue = 0x80000000;
                bTranslated = TRUE;
            } // if:  text retrieved successfully and is highest negative number
        } // if:  error translating number and getting signed number

        //
        // If the retrieval failed or the specified number is
        // out of range, display an error.
        //

        if (    ! bTranslated
            ||  (fSignedIn
                && (    (static_cast< LONG >( dwValue ) < static_cast< LONG >( dwMinIn ))
                    ||  (static_cast< LONG >( dwValue ) > static_cast< LONG >( dwMaxIn ))
                    )
                )
            ||  (!  fSignedIn
                &&  (   (dwValue < dwMinIn)
                    ||  (dwValue > dwMaxIn)
                    )
                )
            )
        {
            TCHAR   szMin[ 32 ];
            TCHAR   szMax[ 32 ];
            CString strPrompt;

            if ( fSignedIn )
            {
                hr = StringCchPrintf( szMin, RTL_NUMBER_OF( szMin ), _T("%d%"), dwMinIn );
                ASSERT( SUCCEEDED( hr ) );
                hr = StringCchPrintf( szMax, RTL_NUMBER_OF( szMax ), _T("%d%"), dwMaxIn );
                ASSERT( SUCCEEDED( hr ) );
            } // if:  signed number
            else
            {
                hr = StringCchPrintf( szMin, RTL_NUMBER_OF( szMin ), _T("%u%"), dwMinIn );
                ASSERT( SUCCEEDED( hr ) );
                hr = StringCchPrintf( szMax, RTL_NUMBER_OF( szMax ), _T("%u%"), dwMaxIn );
                ASSERT( SUCCEEDED( hr ) );
            } // else:  unsigned number
            AfxFormatString2( strPrompt, AFX_IDP_PARSE_INT_RANGE, szMin, szMax );
            AfxMessageBox( strPrompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_INT_RANGE );
            strPrompt.Empty(); // exception prep
            pDXIn->Fail();
        } // if:  invalid string
        else
        {
            rdwValueInout = dwValue;
        } // if:  number is in range
    } // if:  saving data
    else
    {
        CString     strMinValue;
        CString     strMaxValue;
        UINT        cchMax;

        //
        //  Set the maximum number of characters that can be entered.
        //

        if ( fSignedIn )
        {
            strMinValue.Format( _T("%d"), dwMinIn );
            strMaxValue.Format( _T("%d"), dwMaxIn );
        } // if:  signed value
        else
        {
            strMinValue.Format( _T("%u"), dwMinIn );
            strMaxValue.Format( _T("%u"), dwMaxIn );
        } // else:  unsigned value
        cchMax = max( strMinValue.GetLength(), strMaxValue.GetLength() );
        SendMessage( hwndCtrl, EM_LIMITTEXT, cchMax, 0 );

        //
        //  Set the value into the control.
        //

        if ( fSignedIn )
        {
            LONG lValue = static_cast< LONG >( rdwValueInout );
            DDX_Text( pDXIn, nIDCIn, lValue );
        } // if:  signed value
        else
        {
            DDX_Text( pDXIn, nIDCIn, rdwValueInout );
        } // else:
    } // else:  setting data onto the dialog

} //*** DDX_Number

/////////////////////////////////////////////////////////////////////////////
//++
//
//  DDV_RequiredText
//
//  Description:
//      Validate that the dialog string is present.
//
//  Arguments:
//      pDXIn
//          Data exchange object.
//
//      nIDCIn
//          Control ID.
//
//      nIDCLabelIn
//          Label control ID.
//
//      rstrValueIn
//          Value to set or get.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void AFXAPI
DDV_RequiredText(
      CDataExchange *   pDXIn
    , int               nIDCIn
    , int               nIDCLabelIn
    , const CString &   rstrValueIn
    )
{
    ASSERT( pDXIn != NULL );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    if ( pDXIn->m_bSaveAndValidate )
    {
        if ( rstrValueIn.GetLength() == 0 )
        {
            HWND        hwndLabel;
            TCHAR       szLabel[ 1024 ];
            CString     strPrompt;

            // Get the label window handle
            hwndLabel = pDXIn->PrepareEditCtrl( nIDCLabelIn );

            // Get the text of the label.
            GetWindowText( hwndLabel, szLabel, RTL_NUMBER_OF( szLabel ) );

            // Remove ampersands (&) and colons (:).
            CleanupLabel( szLabel );

            // Format and display a message.
            strPrompt.FormatMessage( IDS_REQUIRED_FIELD_EMPTY, szLabel );
            AfxMessageBox( strPrompt, MB_ICONEXCLAMATION );

            // Do this so that the control receives focus.
            (void) pDXIn->PrepareEditCtrl( nIDCIn );

            // Fail the call.
            strPrompt.Empty();  // exception prep
            pDXIn->Fail();
        } // if:  field not specified
    } // if:  saving data

} //*** DDV_RequiredText

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CleanupLabel
//
//  Description:
//      Prepare a label read from a dialog to be used as a string in a
//      message by removing ampersands (&) and colons (:).
//
//  Arguments:
//      pwszLabelIn
//          Label to be cleaned up.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CleanupLabel( LPTSTR pwszLabelIn )
{
    LPTSTR  pin = NULL;
    LPTSTR  pout = NULL;
    LANGID  langid;
    WORD    primarylangid;
    BOOL    fFELanguage;

    //
    //  Get the language ID.
    //

    langid = GetUserDefaultLangID();
    primarylangid = static_cast< WORD >( PRIMARYLANGID( langid ) );
    fFELanguage = ((primarylangid == LANG_JAPANESE)
                || (primarylangid == LANG_CHINESE)
                || (primarylangid == LANG_KOREAN) );

    //
    // Copy the name sans '&' and ':' chars
    //

    pin = pout = pwszLabelIn;
    do
    {
        //
        // Strip FE accelerators with parentheses. e.g. "foo(&F)" -> "foo"
        //

        if (    fFELanguage
            &&  (pin[ 0 ] == _T('('))
            &&  (pin[ 1 ] == _T('&'))
            &&  (pin[ 2 ] != _T('\0'))
            &&  (pin[ 3 ] == _T(')')) )
        {
            pin += 3;
        } // if:  Far East language with accelerator
        else if ( (*pin != _T('&')) && (*pin != _T(':')) )
        {
            *pout++ = *pin;
        } // else if:  accelerator found
    } while ( *pin++ != _T('\0') );

} //*** CleanupLabel
