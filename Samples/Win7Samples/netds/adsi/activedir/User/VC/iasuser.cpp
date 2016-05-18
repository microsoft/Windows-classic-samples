// userprop.cpp : Defines the entry point for the console application.
//

//For the pow function to calculate powers of 2
#include <math.h>
#include <wchar.h>
#include <objbase.h>
#include <activeds.h>

//Make sure you define UNICODE
//Need to define version 5 for Windows 2000
#undef  _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#include <sddl.h>



//Function to use the get_* methods of IADsUser.
HRESULT GetUserPropertyMethods(IADsUser *pUser);

HRESULT FindUserByName(IDirectorySearch *pSearchBase, //Container to search
                       LPWSTR szFindUser, //Name of user to find.
                       IADs **ppUser); //Return a pointer to the user


void wmain( int argc, wchar_t *argv[ ])
{

//Handle the command line arguments.

    wchar_t pszBuffer[MAX_PATH*2] ;    
    if ( argc != 2 )
    {
        wprintf( L"This program finds a user in the current domain\n" ) ;
        wprintf( L"(Windows 2000 or higher), and displays all the IADsUser\n" ) ;
        wprintf( L"properties.\n\n" ) ;        
        wprintf(L"useage:  iadsuser <username>\n");
        wprintf( L"where <username> is the DN of the user.\n" ) ;
        wprintf( L"Enclose in quotes as necessary.\n" ) ;
        return ;
    }
    wcsncpy_s( pszBuffer, sizeof(pszBuffer)/sizeof(pszBuffer[0]),argv[1], MAX_PATH * 2 ) ;
    pszBuffer[(MAX_PATH*2)-1] = 0 ;    
    //if empty string, exit.
    if ( 0==wcscmp(L"", pszBuffer) )
        return ;

    wprintf(L"\nFinding user: %ws...\n",pszBuffer);

//Intialize COM
    CoInitialize(NULL);
    HRESULT hr = S_OK;
//Get rootDSE and the domain container's DN.
    IADs *pObject = NULL;
    IDirectorySearch *pDS = NULL;
    wchar_t szPath[MAX_PATH] ;
    VARIANT var;
    hr = ADsOpenObject(L"LDAP://rootDSE",
                       NULL,
                       NULL,
                       ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
                       IID_IADs,
                       (void**)&pObject);
    if ( FAILED(hr) )
    {
        wprintf(L"Not Found. Could not bind to the domain.\n");
        if ( pObject )
            pObject->Release();
        return;
    }

    VariantInit( &var ) ;
    hr = pObject->Get(L"defaultNamingContext",&var);
    if ( SUCCEEDED(hr) )
    {
        _snwprintf_s( szPath, sizeof(szPath)/sizeof(szPath[0]),MAX_PATH, L"LDAP://%ws", var.bstrVal ) ;
        szPath[MAX_PATH-1] = 0 ;    
        VariantClear(&var);
        if ( pObject )
        {
            pObject->Release();
            pObject = NULL;
        }
        //Bind to the root of the current domain.
        hr = ADsOpenObject(szPath,
                           NULL,
                           NULL,
                           ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
                           IID_IDirectorySearch,
                           (void**)&pDS);
        if ( SUCCEEDED(hr) )
        {
            hr =  FindUserByName(pDS, //Container to search
                                 pszBuffer, //Name of user to find.
                                 &pObject); //Return a pointer to the user
            if ( SUCCEEDED(hr) )
            {
                IADsUser *pUser = NULL;
                hr = pObject->QueryInterface( IID_IADsUser, (void**) &pUser );
                if ( SUCCEEDED(hr) )
                {
                    wprintf (L"----------------------------------------------\n");
                    wprintf (L"--------Call GetUserPropertyMethods-----------\n");
                    hr = GetUserPropertyMethods(pUser);
                    wprintf (L"GetUserPropertyMethods HR: %x\n", hr);
                }
                if ( pUser )
                    pUser->Release();
            }
            else
            {
                wprintf(L"User \"%ws\" not Found.\n",pszBuffer);
                wprintf (L"FindUserByName failed with the following HR: %x\n", hr);
            }
            if ( pObject )
                pObject->Release();
        }

        if ( pDS )
            pDS->Release();
    }

//Uninitalize COM
    CoUninitialize();

    return;
}

HRESULT GetUserPropertyMethods( IADsUser *pUser )
{
    VARIANT_BOOL bBool ;
    DATE date ;
    BSTR bstr ;
    LONG lProp ;
    VARIANT var ;
    HRESULT hr ;
    long lBound, uBound ;

    if ( NULL == pUser ) {
        return( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER ) ) ;
    }
    VariantInit( &var ) ;

    hr = pUser->get_AccountDisabled( &bBool ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"AccountDisabled: %ws\n", ( bBool ? L"TRUE" : L"FALSE" ) ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"AccountDisabled property not found.\n" ) ;
    }
    else {
        wprintf( L"AccountDisabled failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_AccountExpirationDate( &date ) ;
    if ( SUCCEEDED( hr ) ) {
        var.vt = VT_DATE ;
        var.date = date ;
        hr = VariantChangeType( &var, &var, VARIANT_NOVALUEPROP, VT_BSTR ) ;
        if ( SUCCEEDED( hr ) ) {
            wprintf( L"AccountExpirationDate: %ws\n", var.bstrVal ) ;
        }
        else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
            wprintf( L"AccountExpirationDate property not found.\n" ) ;
        }
        else {
            wprintf( L"AccountExpirationDate failed with hr: 0x%08x\n", hr ) ;
        }
        VariantClear( &var ) ;
    }

    hr = pUser->get_BadLoginAddress( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"BadLoginAddress: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else {
        if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
            wprintf( L"BadLoginAddress property not found.\n" ) ;
        }
        else if ( E_ADS_PROPERTY_NOT_SUPPORTED == hr ) {
            // Property not supported on all platforms, check documentation.
            wprintf( L"BadLoginAddress property is not supported on this platform.\n" ) ;
        }
        else {
            wprintf( L"BadLoginAddress failed with hr: 0x%08x\n", hr ) ;
        }
    }

    hr = pUser->get_BadLoginCount( &lProp ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"BadLoginCount: %d\n", lProp ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"BadLoginCount property not found.\n" ) ;
    }
    else {
        wprintf( L"BadLoginCount failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_Department( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"Department: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"Department property not found.\n" ) ;
    }
    else {
        wprintf( L"Department failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_Description( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"Description: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"Description property not found.\n" ) ;
    }
    else {
        wprintf( L"Description failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_Division( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"Division: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"Division property not found.\n" ) ;
    }
    else {
        wprintf( L"Division failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_EmailAddress( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"EmailAddress: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"EmailAddress property not found.\n" ) ;
    }
    else {
        wprintf( L"EmailAddress failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_EmployeeID( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"EmployeeID: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"EmployeeID property not found.\n" ) ;
    }
    else {
        wprintf( L"EmployeeID failed with hr: 0x%08x\n", hr ) ;
    }
     
    hr = pUser->get_FaxNumber( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        // Note:  This property method can return either a VARIANT with
        // type VT_BSTR and a single string, or a VARIANT with type
        // VT_ARRAY | VT_BSTR and multiple strings, depending on whether
        // the target directory's property is single- or multi-valued.
        // Generic code should probably handle both cases.                                                                        
        if ( VT_BSTR == var.vt ) {        
            wprintf( L"FaxNumber: %ws\n", var.bstrVal ) ;        
        }
        else if ( ( VT_ARRAY | VT_BSTR ) == var.vt ) {
            VARIANT elem ;
            VariantInit( &elem ) ;
            if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var), 1, &lBound ) ) &&
                 SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var), 1, &uBound ) ) ) {
                while ( lBound <= uBound ) {
                    if ( SUCCEEDED( SafeArrayGetElement( V_ARRAY(&var), &lBound, &elem ) ) ) {
                        wprintf( L"FaxNumber: %ws\n", elem.bstrVal ) ;
                        VariantClear( &elem ) ;
                    }
                    ++lBound ;
                }
            }
        }
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"FaxNumber property not found.\n" ) ;
    }
    else {
        wprintf( L"FaxNumber failed with hr: 0x%08x\n", hr ) ;
    }        

    hr = pUser->get_FirstName( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"FirstName: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"FirstName property not found.\n" ) ;
    }
    else {
        wprintf( L"FirstName failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_FullName( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"FullName: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"FullName property not found.\n" ) ;
    }
    else {
        wprintf( L"FullName failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_GraceLoginsAllowed( &lProp ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"GraceLoginsAllowed: %d\n", lProp ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_SUPPORTED == hr ) {
        // Property not supported on all platforms, check documentation.
        wprintf( L"GraceLoginsAllowed property is not supported on this platform.\n" ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"GraceLoginsAllowed property not found.\n" ) ;
    }
    else {
        wprintf( L"GraceLoginsAllowed failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_GraceLoginsRemaining( &lProp ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"GraceLoginsRemaining: %d\n", lProp ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_SUPPORTED == hr ) {
        // Property not supported on all platforms, check documentation.
        wprintf( L"GraceLoginsRemaining property is not supported on this platform.\n" ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"GraceLoginsRemaining property not found.\n" ) ;
    }
    else {
        wprintf( L"GraceLoginsRemaining failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_HomeDirectory( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"HomeDirectory: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"HomeDirectory property not found.\n" ) ;
    }
    else {
        wprintf( L"HomeDirectory failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_HomePage( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"HomePage: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"HomePage property not found.\n" ) ;
    }
    else {
        wprintf( L"HomePage failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_IsAccountLocked( &bBool ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"IsAccountLocked: %ws\n", ( bBool ? L"TRUE" : L"FALSE" ) ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"IsAccountLocked property not found.\n" ) ;
    }
    else {
        wprintf( L"IsAccountLocked failed with hr: 0x%08x\n", hr ) ;
    }
    
    hr = pUser->get_Languages( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        // Note:  This property method can return either a VARIANT with
        // type VT_BSTR and a single string, or a VARIANT with type
        // VT_ARRAY | VT_BSTR and multiple strings, depending on whether
        // the target directory's property is single- or multi-valued.
        // Generic code should probably handle both cases.                                                                        
        if ( VT_BSTR == var.vt ) {        
            wprintf( L"Languages: %ws\n", var.bstrVal ) ;        
        }
        else if ( ( VT_ARRAY | VT_BSTR ) == var.vt ) {
            VARIANT elem ;
            VariantInit( &elem ) ;
            if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var), 1, &lBound ) ) &&
                 SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var), 1, &uBound ) ) ) {
                while ( lBound <= uBound ) {
                    if ( SUCCEEDED( SafeArrayGetElement( V_ARRAY(&var), &lBound, &elem ) ) ) {
                        wprintf( L"Languages: %ws\n", elem.bstrVal ) ;
                        VariantClear( &elem ) ;
                    }
                    ++lBound ;
                }
            }
        }
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"Languages property not found.\n" ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_SUPPORTED == hr ) {
        // Property not supported on all platforms, check documentation.
        wprintf( L"Languages property is not supported on this platform.\n" ) ;
    }
    else {
        wprintf( L"Languages failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_LastFailedLogin( &date ) ;
    if ( SUCCEEDED( hr ) ) {
        var.vt = VT_DATE ;
        var.date = date ;
        hr = VariantChangeType( &var, &var, VARIANT_NOVALUEPROP, VT_BSTR ) ;
        if ( SUCCEEDED( hr ) ) {
            wprintf( L"LastFailedLogin: %ws\n", var.bstrVal ) ;
        }
        else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
            wprintf( L"LastFailedLogin property not found.\n" ) ;
        }
        else {
            wprintf( L"LastFailedLogin failed with hr: 0x%08x\n", hr ) ;
        }
        VariantClear( &var ) ;
    }

    hr = pUser->get_LastLogin( &date ) ;
    if ( SUCCEEDED( hr ) ) {
        var.vt = VT_DATE ;
        var.date = date ;
        hr = VariantChangeType( &var, &var, VARIANT_NOVALUEPROP, VT_BSTR ) ;
        if ( SUCCEEDED( hr ) ) {
            wprintf( L"LastLogin: %ws\n", var.bstrVal ) ;
        }
        else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
            wprintf( L"LastLogin property not found.\n" ) ;
        }
        else {
            wprintf( L"LastLogin failed with hr: 0x%08x\n", hr ) ;
        }
        VariantClear( &var ) ;
    }

    hr = pUser->get_LastLogoff( &date ) ;
    if ( SUCCEEDED( hr ) ) {
        var.vt = VT_DATE ;
        var.date = date ;
        hr = VariantChangeType( &var, &var, VARIANT_NOVALUEPROP, VT_BSTR ) ;
        if ( SUCCEEDED( hr ) ) {
            wprintf( L"LastLogoff: %ws\n", var.bstrVal ) ;
        }
        else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
            wprintf( L"LastLogoff property not found.\n" ) ;
        }
        else {
            wprintf( L"LastLogoff failed with hr: 0x%08x\n", hr ) ;
        }
        VariantClear( &var ) ;
    }

    hr = pUser->get_LastName( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"LastName: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"LastName property not found.\n" ) ;
    }
    else {
        wprintf( L"LastName failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_LoginHours( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf(L"LoginHours:\n");
        //For Octet String
        void HUGEP *pArray;
        ULONG dwSLBound;
        ULONG dwSUBound;
        if ( var.vt==(VT_UI1|VT_ARRAY) )
        {
            if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var),
                                                1,
                                                (long FAR *) &dwSLBound ) ) &&
                 SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var),
                                                1,
                                                (long FAR *) &dwSUBound ) ) ) {
                hr = SafeArrayAccessData( V_ARRAY(&var),
                                          &pArray );

                //21 byte array representing 8 hour blocks for each day.
                //First byte is 16:00-23:59 on saturday.
                //Second byte is 0:00-07:59 on sunday,
                //Third byte is 8:00-15:59 on sunday,
                //and so on up to 8:00-15:59 on saturday.
                PBYTE pLogonHours = (PBYTE)pArray;
				const DWORD dTimeStrLen = 12;
				const DWORD dDayStrLen = 12;
                wchar_t szDay[dDayStrLen] ;
                wchar_t szTime[dTimeStrLen] ;
                double base = 2;
                //8 bits in a byte.
                DWORD iBitMax = 8;
                DWORD dVal, j;
                int iTotalBytes = 21;
                int iOffset;
                for ( int k = 1; k <= iTotalBytes; k++ )
                {
                    if ( k!=iTotalBytes )
                        iOffset = k;
                    else
                        iOffset = 0;
                    //Get the block of time during the day.
                    switch ( k-1 )
                    {
                    case 0:
                    case 1:
                    case 2:
                        wcscpy_s(szDay, dDayStrLen , L"Sunday" ) ;
                        break;
                    case 3:
                    case 4:
                    case 5:
                        wcscpy_s(szDay, dDayStrLen , L"Monday" ) ;
                        break;
                    case 6:
                    case 7:
                    case 8:
                        wcscpy_s(szDay, dDayStrLen,  L"Tuesday" );
                        break;
                    case 9:
                    case 10:
                    case 11:
                        wcscpy_s(szDay, dDayStrLen , L"Wednesday" );
                        break;
                    case 12:
                    case 13:
                    case 14:
                        wcscpy_s(szDay, dDayStrLen , L"Thursday" );
                        break;
                    case 15:
                    case 16:
                    case 17:
                        wcscpy_s(szDay, dDayStrLen , L"Friday" );
                        break;
                    case 18:
                    case 19:
                    case 20:
                        wcscpy_s(szDay, dDayStrLen , L"Saturday");
                        break;
                    }

                    switch ( k-1 )
                    {
                    case 0:
                    case 3:
                    case 6:
                    case 9:
                    case 12:
                    case 15:
                    case 18:
                        for ( j = 0; j < iBitMax; j++ )
                        {
                            dVal = (DWORD)pow(base, (double)j);
                            if ( (pLogonHours[iOffset] & dVal)==dVal )
                            {
                                switch ( j )
                                {
                                case 0:
                                    wcscpy_s(szTime, dTimeStrLen, L"00:00-59");
                                    break;
                                case 1:
                                    wcscpy_s(szTime, dTimeStrLen, L"01:00-59");
                                    break;
                                case 2:
                                    wcscpy_s(szTime, dTimeStrLen, L"02:00-59");
                                    break;
                                case 3:
                                    wcscpy_s(szTime, dTimeStrLen, L"03:00-59");
                                    break;
                                case 4:
                                    wcscpy_s(szTime, dTimeStrLen, L"04:00-59");
                                    break;
                                case 5:
                                    wcscpy_s(szTime, dTimeStrLen, L"05:00-59");
                                    break;
                                case 6:
                                    wcscpy_s(szTime, dTimeStrLen, L"06:00-59");
                                    break;
                                case 7:
                                    wcscpy_s(szTime, dTimeStrLen, L"07:00-59");
                                    break;
                                }
                                wprintf(L"   %ws %ws\n",szDay,szTime);
                            }
                        }
                        break;
                    case 1:
                    case 4:
                    case 7:
                    case 10:
                    case 13:
                    case 16:
                    case 19:
                        for ( j = 0; j < iBitMax; j++ )
                        {
                            dVal = (DWORD)pow(base, (double)j);
                            if ( (pLogonHours[iOffset] & dVal)==dVal )
                            {
                                switch ( j )
                                {
                                case 0:
                                    wcscpy_s(szTime, dTimeStrLen, L"08:00-59");
                                    break;
                                case 1:
                                    wcscpy_s(szTime, dTimeStrLen, L"09:00-59");
                                    break;
                                case 2:
                                    wcscpy_s(szTime, dTimeStrLen, L"10:00-59");
                                    break;
                                case 3:
                                    wcscpy_s(szTime, dTimeStrLen, L"11:00-59");
                                    break;
                                case 4:
                                    wcscpy_s(szTime, dTimeStrLen, L"12:00-59");
                                    break;
                                case 5:
                                    wcscpy_s(szTime, dTimeStrLen, L"13:00-59");
                                    break;
                                case 6:
                                    wcscpy_s(szTime, dTimeStrLen, L"14:00-59");
                                    break;
                                case 7:
                                    wcscpy_s(szTime, dTimeStrLen, L"15:00-59");
                                    break;
                                }
                                wprintf(L"   %ws %ws\n",szDay,szTime);
                            }
                        }
                        break;
                    case 2:
                    case 5:
                    case 8:
                    case 11:
                    case 14:
                    case 17:
                    case 20:
                        for ( j = 0; j < iBitMax; j++ )
                        {
                            dVal = (DWORD)pow(base, (double)j);
                            if ( (pLogonHours[iOffset] & dVal)==dVal )
                            {
                                switch ( j )
                                {
                                case 0:
                                    wcscpy_s(szTime, dTimeStrLen, L"16:00-59");
                                    break;
                                case 1:
                                    wcscpy_s(szTime, dTimeStrLen, L"17:00-59");
                                    break;
                                case 2:
                                    wcscpy_s(szTime, dTimeStrLen, L"18:00-59");
                                    break;
                                case 3:
                                    wcscpy_s(szTime, dTimeStrLen, L"19:00-59");
                                    break;
                                case 4:
                                    wcscpy_s(szTime, dTimeStrLen, L"20:00-59");
                                    break;
                                case 5:
                                    wcscpy_s(szTime, dTimeStrLen, L"21:00-59");
                                    break;
                                case 6:
                                    wcscpy_s(szTime, dTimeStrLen, L"22:00-59");
                                    break;
                                case 7:
                                    wcscpy_s(szTime, dTimeStrLen, L"23:00-59");
                                    break;
                                }
                                wprintf(L"   %ws %ws\n",szDay,szTime);
                            }
                        }
                        break;
                    }
                }            

                //Decrement the access count for the array.
                SafeArrayUnaccessData( V_ARRAY(&var) ); 
            }
        }
        VariantClear(&var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"LoginHours property not found.\n" ) ;
    }
    else {
        wprintf( L"LoginHours failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_LoginScript( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"LoginScript: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"LoginScript property not found.\n" ) ;
    }
    else {
        wprintf( L"LoginScript failed with hr: 0x%08x\n", hr ) ;
    }
        
    hr = pUser->get_LoginWorkstations( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var), 1, &lBound ) ) &&
             SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var), 1, &uBound ) ) ) {
            VARIANT elem ;
            while ( lBound <= uBound ) {
                hr = SafeArrayGetElement( V_ARRAY(&var), &lBound, &elem ) ;
                if ( SUCCEEDED( hr ) ) {
                    wprintf( L"LoginWorkstations: %ws\n", elem.bstrVal ) ;
                    VariantClear( &elem ) ;
                }
                ++lBound ;
            }
        }
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"LoginWorkstations property not found.\n" ) ;
    }
    else {
        wprintf( L"LoginWorkstations failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_Manager( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"Manager: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"Manager property not found.\n" ) ;
    }
    else {
        wprintf( L"Manager failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_MaxLogins( &lProp ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"MaxLogins: %d\n", lProp ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_SUPPORTED == hr ) {
        // Property not supported on all platforms, check documentation.
        wprintf( L"MaxLogins property is not supported on this platform.\n" ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"MaxLogins property not found.\n" ) ;
    }
    else {
        wprintf( L"MaxLogins failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_MaxStorage( &lProp ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"MaxStorage: %d\n", lProp ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"MaxStorage property not found.\n" ) ;
    }
    else {
        wprintf( L"MaxStorage failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_NamePrefix( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"NamePrefix: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"NamePrefix property not found.\n" ) ;
    }
    else {
        wprintf( L"NamePrefix failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_NameSuffix( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"NameSuffix: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"NameSuffix property not found.\n" ) ;
    }
    else {
        wprintf( L"NameSuffix failed with hr: 0x%08x\n", hr ) ;
    }
  
    hr = pUser->get_OfficeLocations( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        // Note:  This property method can return either a VARIANT with
        // type VT_BSTR and a single string, or a VARIANT with type
        // VT_ARRAY | VT_BSTR and multiple strings, depending on whether
        // the target directory's property is single- or multi-valued.
        // Generic code should probably handle both cases.                                                                        
        if ( VT_BSTR == var.vt ) {        
            wprintf( L"OfficeLocations: %ws\n", var.bstrVal ) ;        
        }
        else if ( ( VT_ARRAY | VT_BSTR ) == var.vt ) {
            VARIANT elem ;
            VariantInit( &elem ) ;
            if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var), 1, &lBound ) ) &&
                 SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var), 1, &uBound ) ) ) {
                while ( lBound <= uBound ) {
                    if ( SUCCEEDED( SafeArrayGetElement( V_ARRAY(&var), &lBound, &elem ) ) ) {
                        wprintf( L"OfficeLocations: %ws\n", elem.bstrVal ) ;
                        VariantClear( &elem ) ;
                    }
                    ++lBound ;
                }
            }
        }
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"OfficeLocations property not found.\n" ) ;
    }
    else {
        wprintf( L"OfficeLocations failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_OtherName( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"OtherName: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"OtherName property not found.\n" ) ;
    }
    else {
        wprintf( L"OtherName failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_PasswordExpirationDate( &date ) ;
    if ( SUCCEEDED( hr ) ) {
        var.vt = VT_DATE ;
        var.date = date ;
        hr = VariantChangeType( &var, &var, VARIANT_NOVALUEPROP, VT_BSTR ) ;
        if ( SUCCEEDED( hr ) ) {
            wprintf( L"PasswordExpirationDate: %ws\n", var.bstrVal ) ;
        }
        else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
            wprintf( L"PasswordExpirationDate property not found.\n" ) ;
        }
        else {
            wprintf( L"PasswordExpirationDate failed with hr: 0x%08x\n", hr ) ;
        }
        VariantClear( &var ) ;
    }

    hr = pUser->get_PasswordLastChanged( &date ) ;
    if ( SUCCEEDED( hr ) ) {
        var.vt = VT_DATE ;
        var.date = date ;
        hr = VariantChangeType( &var, &var, VARIANT_NOVALUEPROP, VT_BSTR ) ;
        if ( SUCCEEDED( hr ) ) {
            wprintf( L"PasswordLastChanged: %ws\n", var.bstrVal ) ;
        }
        else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
            wprintf( L"PasswordLastChanged property not found.\n" ) ;
        }
        else {
            wprintf( L"PasswordLastChanged failed with hr: 0x%08x\n", hr ) ;
        }
        VariantClear( &var ) ;
    }

    hr = pUser->get_PasswordMinimumLength( &lProp ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"PasswordMinimumLength: %d\n", lProp ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"PasswordMinimumLength property not found.\n" ) ;
    }
    else {
        wprintf( L"PasswordMinimumLength failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_PasswordRequired( &bBool ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"PasswordRequired: %ws\n", ( bBool ? L"TRUE" : L"FALSE" ) ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"PasswordRequired property not found.\n" ) ;
    }
    else {
        wprintf( L"PasswordRequired failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_Picture( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"Picture is set (not displayed)\n" ) ;
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"Picture property not found.\n" ) ;
    }
    else {
        wprintf( L"Picture failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_PostalAddresses( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var), 1, &lBound ) ) &&
             SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var), 1, &uBound ) ) ) {
            VARIANT elem ;
            while ( lBound <= uBound ) {
                hr = SafeArrayGetElement( V_ARRAY(&var), &lBound, &elem ) ;
                if ( SUCCEEDED( hr ) ) {
                    wprintf( L"PostalAddresses: %ws\n", elem.bstrVal ) ;
                    VariantClear( &elem ) ;
                }
                ++lBound ;
            }
        }
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"PostalAddresses property not found.\n" ) ;
    }
    else {
        wprintf( L"PostalAddresses failed with hr: 0x%08x\n", hr ) ;
    }
       
    hr = pUser->get_PostalCodes( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        // Note:  This property method can return either a VARIANT with
        // type VT_BSTR and a single string, or a VARIANT with type
        // VT_ARRAY | VT_BSTR and multiple strings, depending on whether
        // the target directory's property is single- or multi-valued.
        // Generic code should probably handle both cases.                                                                        
        if ( VT_BSTR == var.vt ) {        
            wprintf( L"PostalCodes: %ws\n", var.bstrVal ) ;        
        }
        else if ( ( VT_ARRAY | VT_BSTR ) == var.vt ) {
            VARIANT elem ;
            VariantInit( &elem ) ;
            if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var), 1, &lBound ) ) &&
                 SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var), 1, &uBound ) ) ) {
                while ( lBound <= uBound ) {
                    if ( SUCCEEDED( SafeArrayGetElement( V_ARRAY(&var), &lBound, &elem ) ) ) {
                        wprintf( L"PostalCodes: %ws\n", elem.bstrVal ) ;
                        VariantClear( &elem ) ;
                    }
                    ++lBound ;
                }
            }
        }
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"PostalCodes property not found.\n" ) ;
    }
    else {
        wprintf( L"PostalCodes failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_Profile( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"Profile: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"Profile property not found.\n" ) ;
    }
    else {
        wprintf( L"Profile failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_RequireUniquePassword( &bBool ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"RequireUniquePassword: %ws\n", ( bBool ? L"TRUE" : L"FALSE" ) ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"RequireUniquePassword property not found.\n" ) ;
    }
    else {
        wprintf( L"RequireUniquePassword failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_SeeAlso( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var), 1, &lBound ) ) &&
             SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var), 1, &uBound ) ) ) {
            VARIANT elem ;
            while ( lBound <= uBound ) {
                hr = SafeArrayGetElement( V_ARRAY(&var), &lBound, &elem ) ;
                if ( SUCCEEDED( hr ) ) {
                    wprintf( L"SeeAlso: %ws\n", elem.bstrVal ) ;
                    VariantClear( &elem ) ;
                }
                ++lBound ;
            }
        }
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"SeeAlso property not found.\n" ) ;
    }
    else {
        wprintf( L"SeeAlso failed with hr: 0x%08x\n", hr ) ;
    }
     
    hr = pUser->get_TelephoneHome( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        // Note:  This property method can return either a VARIANT with
        // type VT_BSTR and a single string, or a VARIANT with type
        // VT_ARRAY | VT_BSTR and multiple strings, depending on whether
        // the target directory's property is single- or multi-valued.
        // Generic code should probably handle both cases.                                                                        
        if ( VT_BSTR == var.vt ) {        
            wprintf( L"TelephoneHome: %ws\n", var.bstrVal ) ;        
        }
        else if ( ( VT_ARRAY | VT_BSTR ) == var.vt ) {
            VARIANT elem ;
            VariantInit( &elem ) ;
            if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var), 1, &lBound ) ) &&
                 SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var), 1, &uBound ) ) ) {
                while ( lBound <= uBound ) {
                    if ( SUCCEEDED( SafeArrayGetElement( V_ARRAY(&var), &lBound, &elem ) ) ) {
                        wprintf( L"TelephoneHome: %ws\n", elem.bstrVal ) ;
                        VariantClear( &elem ) ;
                    }
                    ++lBound ;
                }
            }
        }
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"TelephoneHome property not found.\n" ) ;
    }
    else {
        wprintf( L"TelephoneHome failed with hr: 0x%08x\n", hr ) ;
    }
       
    hr = pUser->get_TelephoneMobile( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        // Note:  This property method can return either a VARIANT with
        // type VT_BSTR and a single string, or a VARIANT with type
        // VT_ARRAY | VT_BSTR and multiple strings, depending on whether
        // the target directory's property is single- or multi-valued.
        // Generic code should probably handle both cases.                                                                        
        if ( VT_BSTR == var.vt ) {        
            wprintf( L"TelephoneMobile: %ws\n", var.bstrVal ) ;        
        }
        else if ( ( VT_ARRAY | VT_BSTR ) == var.vt ) {
            VARIANT elem ;
            VariantInit( &elem ) ;
            if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var), 1, &lBound ) ) &&
                 SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var), 1, &uBound ) ) ) {
                while ( lBound <= uBound ) {
                    if ( SUCCEEDED( SafeArrayGetElement( V_ARRAY(&var), &lBound, &elem ) ) ) {
                        wprintf( L"TelephoneMobile: %ws\n", elem.bstrVal ) ;
                        VariantClear( &elem ) ;
                    }
                    ++lBound ;
                }
            }
        }
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"TelephoneMobile property not found.\n" ) ;
    }
    else {
        wprintf( L"TelephoneMobile failed with hr: 0x%08x\n", hr ) ;
    }
        
    hr = pUser->get_TelephoneNumber( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        // Note:  This property method can return either a VARIANT with
        // type VT_BSTR and a single string, or a VARIANT with type
        // VT_ARRAY | VT_BSTR and multiple strings, depending on whether
        // the target directory's property is single- or multi-valued.
        // Generic code should probably handle both cases.                                                                        
        if ( VT_BSTR == var.vt ) {        
            wprintf( L"TelephoneNumber: %ws\n", var.bstrVal ) ;        
        }
        else if ( ( VT_ARRAY | VT_BSTR ) == var.vt ) {
            VARIANT elem ;
            VariantInit( &elem ) ;
            if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var), 1, &lBound ) ) &&
                 SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var), 1, &uBound ) ) ) {
                while ( lBound <= uBound ) {
                    if ( SUCCEEDED( SafeArrayGetElement( V_ARRAY(&var), &lBound, &elem ) ) ) {
                        wprintf( L"TelephoneNumber: %ws\n", elem.bstrVal ) ;
                        VariantClear( &elem ) ;
                    }
                    ++lBound ;
                }
            }
        }
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"TelephoneNumber property not found.\n" ) ;
    }
    else {
        wprintf( L"TelephoneNumber failed with hr: 0x%08x\n", hr ) ;
    }
       
    hr = pUser->get_TelephonePager( &var ) ;
    if ( SUCCEEDED( hr ) ) {
        // Note:  This property method can return either a VARIANT with
        // type VT_BSTR and a single string, or a VARIANT with type
        // VT_ARRAY | VT_BSTR and multiple strings, depending on whether
        // the target directory's property is single- or multi-valued.
        // Generic code should probably handle both cases.                                                                        
        if ( VT_BSTR == var.vt ) {        
            wprintf( L"TelephonePager: %ws\n", var.bstrVal ) ;        
        }
        else if ( ( VT_ARRAY | VT_BSTR ) == var.vt ) {
            VARIANT elem ;
            VariantInit( &elem ) ;
            if ( SUCCEEDED( SafeArrayGetLBound( V_ARRAY(&var), 1, &lBound ) ) &&
                 SUCCEEDED( SafeArrayGetUBound( V_ARRAY(&var), 1, &uBound ) ) ) {
                while ( lBound <= uBound ) {
                    if ( SUCCEEDED( SafeArrayGetElement( V_ARRAY(&var), &lBound, &elem ) ) ) {
                        wprintf( L"TelephonePager: %ws\n", elem.bstrVal ) ;
                        VariantClear( &elem ) ;
                    }
                    ++lBound ;
                }
            }
        }
        VariantClear( &var ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"TelephonePager property not found.\n" ) ;
    }
    else {
        wprintf( L"TelephonePager failed with hr: 0x%08x\n", hr ) ;
    }

    hr = pUser->get_Title( &bstr ) ;
    if ( SUCCEEDED( hr ) ) {
        wprintf( L"Title: %ws\n", bstr ) ;
        SysFreeString( bstr ) ;
    }
    else if ( E_ADS_PROPERTY_NOT_FOUND == hr ) {
        wprintf( L"Title property not found.\n" ) ;
    }
    else {
        wprintf( L"Title failed with hr: 0x%08x\n", hr ) ;
    }

    return( S_OK ) ;
}

HRESULT FindUserByName( IDirectorySearch *pSearchBase,
                        LPWSTR szFindUser,
                        IADs **ppUser )
{
    HRESULT hr ;
    wchar_t pszSearchFilter[MAX_PATH] ;
    wchar_t pszADsPath[MAX_PATH] ;
    ADS_SEARCHPREF_INFO searchPrefs ;
    ADS_SEARCH_COLUMN col ;
    ADS_SEARCH_HANDLE hSearch ;
    LPWSTR pszAttribute[1] = { L"ADsPath"} ;   
	*ppUser = NULL;

    if ( NULL == pSearchBase || NULL == szFindUser || NULL == ppUser ) {
        return( E_INVALIDARG ) ;
    }

    // build our search filter.
    if ( 0 > _snwprintf_s( pszSearchFilter, sizeof(pszSearchFilter)/sizeof(pszSearchFilter[0]),MAX_PATH, L"(&(objectCategory=person)(objectClass=user)(cn=%ws))", szFindUser ) ) {
        // uh oh, username is too long.  just abort.
        return( E_INVALIDARG ) ;
    }
    ZeroMemory(&searchPrefs, sizeof( searchPrefs ) ) ;
    searchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE ;
    searchPrefs.vValue.dwType = ADSTYPE_INTEGER ;
    searchPrefs.vValue.Integer = ADS_SCOPE_SUBTREE ;
    hr = pSearchBase->SetSearchPreference( &searchPrefs, 1 ) ;
    if ( FAILED( hr ) ) {
        return( hr ) ;
    }
    hr = pSearchBase->ExecuteSearch( pszSearchFilter, pszAttribute, 1, &hSearch ) ;
    if ( SUCCEEDED( hr ) ) {
        if ( S_ADS_NOMORE_ROWS != pSearchBase->GetNextRow( hSearch ) ) {       
            hr = pSearchBase->GetColumn( hSearch, pszAttribute[0], &col ) ;
            if ( SUCCEEDED( hr ) ) {
                wcsncpy_s( pszADsPath, sizeof(pszADsPath)/sizeof(pszADsPath[0]),col.pADsValues->CaseIgnoreString, MAX_PATH ) ;
                pszADsPath[MAX_PATH-1] = 0 ;
                hr = ADsOpenObject( pszADsPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IADs, (void**)ppUser ) ;
                if ( SUCCEEDED( hr ) ) {
                    wprintf( L"Found User.\n" ) ;
                    wprintf( L"%ws: %ws\n", pszAttribute[0], pszADsPath ) ;
                }
                pSearchBase->FreeColumn( &col ) ;
            }            
        }
        pSearchBase->CloseSearchHandle( hSearch ) ;
    }

	if ( NULL == *ppUser )
		hr = E_FAIL;

    return( hr ) ;

}



