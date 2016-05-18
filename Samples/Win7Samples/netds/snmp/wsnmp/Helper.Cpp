/*++

Copyright (c) Microsoft Corporation

Module Name: helper.cpp

Abstract:

    helper functions used by wsnmputil.cpp.

--*/ 


#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>
#include  <winsock2.h>
#include  <winsnmp.h>
#include  <snmp.h>
#include  <mgmtapi.h>
#include  <ws2tcpip.h>
#include  <strsafe.h>
#include  "wsnmputil.h"



//
// abstract: print the usage for wsnmputil
// input  :  none
// output :  none
//
void    Usage( )
{  
    printf("\nusage:  wsnmputil [-v1|-v2] [get|getnext|walk|getbulk|subtree] agent community [non_repeaters max_repetitions] oid [oid ...]\n");
    printf("Examples:\n");
    printf("  wsnmputil trap\n");
    printf("  wsnmputil -v1 get localhost public 1.3.6.1.2.1.1.1.0\n");
    printf("  wsnmputil -v1 getnext localhost public 1.3.6.1.2.1.1.1.0\n");
    printf("  wsnmputil -v1 subtree localhost public 1.3.6.1.2.1.1\n");
    printf("  wsnmputil -v1 set toaster public 1.3.6.1.4.1.12.2.5.0   2\n");
    printf("  wsnmputil -v2 walk localhost public 1\n");
    printf("  wsnmputil -v2 getbulk toaster public 1  2   1.3.6.1.2.1.1.2.0  1.3.6.1.2.1.4.22.1.2  1.3.6.1.2.1.4.22.1.4\n");
}  // end of Usage



//
// abstract: Print the debug message.
// input:    variable
// output:   none
//
void    PrintDbgMessage( LPSTR szFormat, ... )
{  
    va_list argList;
    char    szBuf[ 1024 ];

    va_start ( argList, szFormat );
    StringCchVPrintf ( szBuf, 1024, szFormat, argList );
    va_end     ( argList );
    
    printf     ( szBuf );
}  //end of PrintDbgMessage



//
// abstract: convert agentAddress ( "a.b.c.d" format or "text.xyz.combination or "a:b:c:d" format)
// input  : text string from the command line
// output : TRUE if name is resolved to IP address, FALSE otherwise. 
//
BOOL ConvertTexttoIp( char *agentAddress )
{  

    struct hostent *phost = NULL;
    struct addrinfo hints;
    struct addrinfo *res;
    BOOL bRet = FALSE;

    if ( agentAddress == NULL )
    {
        return ( FALSE );
    }
    ZeroMemory(&hints,sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    if(!getaddrinfo(agentAddress,NULL,&hints,&res))
    {
        if(res->ai_family == AF_INET || res->ai_family == AF_INET6)
        {
            if(!getnameinfo(res->ai_addr,res->ai_addrlen,(char*)(gVars.agentAddr),MAX_ADDR_LEN,0,0,NI_NUMERICHOST))
            {
               bRet = TRUE;
            }
        }
    }
    if(res)
    {
        freeaddrinfo(res);
        res = 0;
    }
    return bRet;
} // end of ConvertTexttoIp



//
// abstract: Parse the command line parameters and fill up the pSession structure.
// input parameters:
// argc   number of command line parameters
// argv   list of all the command line parameters.
//
// output parameters:
// TRUE   Command line parsing is successful.
// FALSE  Command line parsing failed, print the usage.
//
BOOL    ParseCommandLine( int argc, char **argv )
{  

    char        localOid[] = "1.3.6.1.2.1";
    
    // should we wait for traps?
    if ( (argc < 5) && (argc != 2) )
    {  
        Usage( );
        return ( FALSE );
    } 
    else if ( (_stricmp( argv[1], "trap")) && (argc < 5) )
    {  
        Usage( );
        return ( FALSE );
    } 

    // init the class
    gVars.oidCount = 0;

    // update the pointers.
    ++argv; 
    --argc;

    if ( !_stricmp( *argv, "trap" ) )
    {  
        gVars.operation = TRAP;        // wait for traps.
        return ( TRUE );
    } 
    else if ( !_stricmp(*argv, "-v2") )
    {  
        gVars.version = TRUE;   // use v2c mode.
    } 
    else if ( !_stricmp(*argv, "-v1") )
    {  
        gVars.version = FALSE;   // use v1 mode.
    } 

    // update the pointers.
    ++argv;
    --argc;

    // check the operation specified.
    if ( !_stricmp( *argv, "get" ) )
    {  
        gVars.operation = GET;        // do a get.
    } 
    else if ( !_stricmp( *argv, "getnext" ) )
    {  
        gVars.operation = GET_NEXT;    // do get next.
    } 
    else if ( !_stricmp( *argv, "walk" ) )
    {          
        gVars.operation = WALK;        // do a walk.
    } 
    else if ( !_stricmp( *argv, "set" ) )
    {  
        gVars.operation = SET;        // do a set.
    } 
    else if ( !_stricmp( *argv, "getbulk" ) )
    {  
        gVars.operation = GET_BULK;    // do get bulk operation.
    } 
    else if ( !_stricmp( *argv, "subtree" ) )
    {          
        gVars.operation = SUB_TREE;    // go through the sub tree.
    } 
    else 
    {  
        Usage( );
        return ( FALSE );
    }     

    // Get agent address...
    argv++;
    argc--;

    gVars.pAgentStrAddr = (LPSTR)SnmpUtilMemAlloc( (int) (strlen(*argv) + 1) );
    if ( gVars.pAgentStrAddr == NULL )
    {  
        PrintDbgMessage( "wsnmputil: Memory allocation failed ..\n" );
        return ( FALSE );
    } 

    StringCchCopyA( gVars.pAgentStrAddr, strlen(*argv) + 1, *argv );
    
    // convert the string to IP address.
    if ( ConvertTexttoIp(*argv) == FALSE )
    {  
        PrintDbgMessage( "Invalid SNMP agent ..\n" );
        return ( FALSE );
    } 

    // Get agent community...
    argv++;
    argc--;

    gVars.pAgentCommunity = (LPSTR)SnmpUtilMemAlloc( (int) (strlen( *argv ) + 1 ));
    if ( gVars.pAgentCommunity == NULL )
    {  
        PrintDbgMessage( "wsnmputil: Memory allocation failed ..\n" );
        return ( FALSE );
    } 

    StringCchCopyA( gVars.pAgentCommunity, strlen( *argv ) + 1, *argv );

    //   If it is a GetBulkRequest
    //   Get non_repeaters field and max_repetitions field for
    //   SNMP_PDU_GETBULK PDU.

    if (gVars.operation == GET_BULK)
    {  
        argv++;
        argc--;
        if (argc)
            gVars.non_repeaters = atol(*argv);
        else
            return FALSE;
        argv++;
        argc--;
        if (argc)
            gVars.max_repetitions = atol(*argv);
        else
            return FALSE;
    }

    // ensure that the command line arguments are less than CMD_LINE_ARGS
    while( ( --argc ) && ( gVars.oidCount < CMD_LINE_ARGS ) )
    {  
        argv++;

        if ( ( ( gVars.operation == WALK  ) || ( gVars.operation == SUB_TREE ) ) 
             && ( strlen( *argv ) <= 3 ) )
        {  
            
            if ( ( *argv[0] ) == '.' ) 
            {  
                // trying to preserve old snmputil behavior.
                // if the user wants to browse a tree starting from .1
                // or if the user wants to browse the tree from .2

                gVars.pszOid[ gVars.oidCount ] = (LPSTR)SnmpUtilMemAlloc( (int)(strlen( localOid )) + (int) (strlen( *argv ) + 1 ));
                if ( gVars.pszOid[ gVars.oidCount ] == NULL )
                {  
                    PrintDbgMessage( "wsnmputil: Memory allocation failed ..\n" );
                    return ( FALSE );
                } 
                StringCchCopyA( gVars.pszOid[ gVars.oidCount ], (strlen( localOid ) +  strlen( *argv ) + 1), localOid );
                StringCchCatA( gVars.pszOid[ gVars.oidCount ], (strlen( localOid ) +  strlen( *argv ) + 1), *argv );
            } 
            else if ( isdigit( *argv[0] ) != 0 ) 
            {  
                // if the user wants to browse the tree from 1 or 2

                gVars.pszOid[ gVars.oidCount ] = (LPSTR)SnmpUtilMemAlloc( (int)(strlen( localOid )) + (int) (strlen( *argv ) + 2 ));
                if ( gVars.pszOid[ gVars.oidCount ] == NULL )
                {  
                    PrintDbgMessage( "wsnmputil: Memory allocation failed ..\n" );
                    return ( FALSE );
                } 
                StringCchCopyA( gVars.pszOid[ gVars.oidCount ], strlen( localOid ) + strlen( *argv ) + 2, localOid );
                StringCchCatA( gVars.pszOid[ gVars.oidCount ], strlen( localOid ) + strlen( *argv ) + 2, "." );
                StringCchCatA( gVars.pszOid[ gVars.oidCount ], strlen( localOid ) + strlen( *argv ) + 2, *argv );
            } 
            else 
            {  
                gVars.pszOid[ gVars.oidCount ] = (LPSTR)SnmpUtilMemAlloc( (int)(strlen( *argv ) + 1 ));
                if ( gVars.pszOid[ gVars.oidCount ] == NULL )
                {  
                    PrintDbgMessage( "wsnmputil: Memory allocation failed ..\n" );
                    return ( FALSE );
                } 
                StringCchCopyA( gVars.pszOid[ gVars.oidCount ], strlen( *argv ) + 1,  *argv );
            } 

            ++gVars.oidCount;
            // do a walk or SUB_TREE on one oid.
            break;
        } 
        else if ( gVars.operation == SET )
        {  
            if ( gVars.pszOid[ gVars.oidCount ] == NULL )
            {  
                gVars.pszOid[ gVars.oidCount ] = ( LPSTR )SnmpUtilMemAlloc( (int) (strlen( ( *argv ) ) + 1 ));
                if ( gVars.pszOid[ gVars.oidCount ] == NULL )
                {  
                    PrintDbgMessage( "wsnmputil: Memory allocation failed ..\n" );
                    return ( FALSE );
                } 
                StringCchCopyA( gVars.pszOid[ gVars.oidCount ], strlen( *argv ) + 1, *argv );
            } 
            else
            {  
                gVars.pSetValue = ( LPSTR )SnmpUtilMemAlloc( (int)(strlen( *argv ) + 1 ));
                if ( gVars.pszOid[ gVars.oidCount ] == NULL )
                {  
                    PrintDbgMessage( "wsnmputil: Memory allocation failed ..\n" );
                    return ( FALSE );
                } 
                StringCchCopyA( gVars.pSetValue, strlen( *argv ) + 1, *argv );
                break; // we only do a SET for 1 pair of (name, value) varbinding
            } 
            
        } 
        else 
        {  
            gVars.pszOid[ gVars.oidCount ] = (LPSTR)SnmpUtilMemAlloc( (int)(strlen( *argv ) + 1 ));
            if ( gVars.pszOid[ gVars.oidCount ] == NULL )
            {  
                PrintDbgMessage( "wsnmputil: Memory allocation failed ..\n" );
                return ( FALSE );
            } 
            StringCchCopyA( gVars.pszOid[ gVars.oidCount ], strlen( *argv ) + 1, *argv );
            ++gVars.oidCount;
        } 

    } 

    if (gVars.operation == GET_BULK)
    {  
        //check if user has entered valid arguments for GET_BULK operation
        if (gVars.non_repeaters < 0 || gVars.max_repetitions < 0 )
            return (FALSE);
        if (gVars.oidCount < gVars.non_repeaters)
            return (FALSE);
    } 

    return ( TRUE );
}  // end of ParseCommandLine



//
// abstract: convert the passes string to smiVALUE
// input  :  a pointer to smiValue
// output :  none
// 
void ConvertStringToSmiValue( smiVALUE *pValue )
{  

    int     i;
    DWORD   ipAddr;
    BYTE    *pBytePtr;

    switch( pValue->syntax )
    {  

        case SNMP_SYNTAX_INT:
            pValue->value.sNumber = atol( gVars.pSetValue );
            break;

        case SNMP_SYNTAX_UINT32:
        case SNMP_SYNTAX_CNTR32:
        case SNMP_SYNTAX_GAUGE32:
        case SNMP_SYNTAX_TIMETICKS:
            pValue->value.uNumber = atol( gVars.pSetValue );
            break;
 
        case SNMP_SYNTAX_CNTR64:
            pValue->value.hNumber.hipart = ( smiUINT32 )( _atoi64( gVars.pSetValue ) >> 32);
            pValue->value.hNumber.lopart = (smiUINT32)( _atoi64( gVars.pSetValue ) & 0xFFFFFFFF);
            break;

        case SNMP_SYNTAX_OCTETS:
        case SNMP_SYNTAX_BITS:
        case SNMP_SYNTAX_OPAQUE:
            pValue->value.string.len = (int) strlen( gVars.pSetValue );
            pValue->value.string.ptr = ( smiBYTE * )SnmpUtilMemAlloc( pValue->value.string.len * sizeof( smiBYTE ) );

            if ( pValue->value.string.ptr )
            {  

                memcpy( pValue->value.string.ptr, 
                        gVars.pSetValue,
                        pValue->value.string.len * sizeof( smiBYTE ) 
                      ); 
            } 
            break;
        case SNMP_SYNTAX_NSAPADDR:
            break;
        case SNMP_SYNTAX_IPADDR:

            pValue->value.string.len = 4;        // len of IPAddress field.
            pValue->value.string.ptr = ( smiBYTE * )SnmpUtilMemAlloc( pValue->value.string.len * sizeof( smiBYTE ) );

            ipAddr = inet_addr( gVars.pSetValue );
            
            pBytePtr = ( BYTE * )&ipAddr;
            
            for( i = 0; i < 4; i++ )
            {  
                pValue->value.string.ptr[ i ] = *pBytePtr++;
            } 

            break;
        case SNMP_SYNTAX_NULL:
        case SNMP_SYNTAX_NOSUCHOBJECT:
        case SNMP_SYNTAX_NOSUCHINSTANCE:
        case SNMP_SYNTAX_ENDOFMIBVIEW:
            pValue->value.empty = 0;
            break;

        default:
            break;                    
    } 

}  //end of ConvertStringToSmiValue



//
// abstract: print a smiValue
// input  :  a pointer to smiValue
// output :  none
// 
void PrintOidValue( smiVALUE *pValue )
{  
    
    if ( pValue == NULL )
        return;


    switch( pValue->syntax )
    {  
        case SNMP_SYNTAX_INT32:
            printf( "INTEGER: %ld \n", pValue->value.sNumber );
            break;
        case SNMP_SYNTAX_UINT32:
            printf( "UNSIGNED INTEGER: %ld \n", pValue->value.uNumber );
            break;
        case SNMP_SYNTAX_CNTR32:
            printf( "COUNTER: %ld \n", pValue->value.uNumber );
            break;
        case SNMP_SYNTAX_GAUGE32:
            printf( "GAUGE: %ld \n", pValue->value.uNumber );
            break;
        case SNMP_SYNTAX_TIMETICKS:
            printf( "TIMETICKS: %ld \n", pValue->value.uNumber );
            break;
        case SNMP_SYNTAX_CNTR64:
            printf( "COUNTER64: %ld%ld \n", pValue->value.hNumber.hipart, 
                                        pValue->value.hNumber.lopart );
            break;
        case SNMP_SYNTAX_OCTETS:
        {  
            UINT  J;
            BOOL  IsDisplayString = TRUE;
            LPSTR StringFormat;
            for ( J=0; J < pValue->value.string.len && IsDisplayString; J++ )
            {  
                IsDisplayString = isprint( pValue->value.string.ptr[J] );
            } 
            StringFormat = IsDisplayString ? "%c" : "<0x%02x>" ;

            printf( "OCTET STRING - " );
            for ( J=0; J < pValue->value.string.len; J++ )
            {  
                printf( StringFormat, pValue->value.string.ptr[J] );
            } 
            putchar( '\n' );
        } 
            break;

        case SNMP_SYNTAX_BITS:
        {      
            UINT J;
            printf( "Bits - " );
            for ( J=0; J < pValue->value.string.len; J++ )
            {  
                printf( "0x%x ", pValue->value.string.ptr[J] );
            } 
            putchar( '\n' );
        } 
            break;

        case SNMP_SYNTAX_OPAQUE:
        {  
            UINT J;
            printf( "Opaque - " );
            for ( J=0; J < pValue->value.string.len; J++ )
            {  
                printf( "0x%x ", pValue->value.string.ptr[J] );
            } 
            putchar( '\n' );
        } 
            break;

        case SNMP_SYNTAX_IPADDR:  
        {  
            printf( "IpAddress - " );
            printf( "%d.%d.%d.%d ",
                  pValue->value.string.ptr[0] ,
                  pValue->value.string.ptr[1] ,
                  pValue->value.string.ptr[2] ,
                  pValue->value.string.ptr[3] );
            putchar( '\n' );
        } 
            break;

        case SNMP_SYNTAX_NSAPADDR:
            break;

        case SNMP_SYNTAX_OID:
        {  
            UINT J;
            printf( "OBJECT IDENTIFIER - " );
            for ( J=0; J < pValue->value.oid.len; J++ )
            {  
                printf( ".%d", pValue->value.oid.ptr[J] );
            } 
            putchar( '\n' );
        } 
            break;

        case SNMP_SYNTAX_NULL:
            printf( "NULL - NULL \n" );
            break;

        case SNMP_SYNTAX_NOSUCHOBJECT:
            printf( "No such object ..\n" );
            break;

        case SNMP_SYNTAX_NOSUCHINSTANCE:
            printf( "No such instance ..\n" );
            break;

        case SNMP_SYNTAX_ENDOFMIBVIEW:
            printf( " End of MibView ..\n " );
            break;

        default:
            printf( "Invalid Type ..\n" );
            break;
    } 
    
}  //end of PrintOidValue



// 
// abstarct: print the varBinds in pSession->hVbl
// input:    a pointer to manager session
// output:   none
//
BOOL    PrintVarBind( PSNMP_MGR_SESSION pSession )
{  
    char                 szBuf[ MAX_BUFFER_LEN ];
    char                 szSecString[ MAX_BUFFER_LEN ];
    int                     oidCount;
    int                     i;
    smiINT               result;
    AsnObjectIdentifier  Oid;      //unfortunate that there is no better way
    LPSTR                string = NULL;


    oidCount = SnmpCountVbl( pSession->hVbl );


    // using gVars.oid for walk operation.
    // doesnt hurt when using get, get next operations as well.

    for( i = 1; i <= oidCount; i++ )
    {  

        if (i > 1) //free the last gVars.oid before getting a new one
        {  
            SnmpFreeDescriptor ( SNMP_SYNTAX_OID, (smiLPOPAQUE)&gVars.oid);
            if  ( gVars.value.syntax == SNMP_SYNTAX_OCTETS )    
            {  
                SnmpFreeDescriptor ( SNMP_SYNTAX_OCTETS, (smiLPOPAQUE)&gVars.value.value.string);
            } 
            if  ( gVars.value.syntax == SNMP_SYNTAX_OID )  
            {  
                SnmpFreeDescriptor ( SNMP_SYNTAX_OID, (smiLPOPAQUE)&gVars.value.value.oid);
            } 
        } 

        SnmpGetVb( pSession->hVbl, i, &gVars.oid, &gVars.value );

        // a special case for SUB_TREE walk.
        if ( gVars.operation == SUB_TREE )
        {
            SnmpOidCompare( &gVars.startOid, &gVars.oid, ( gVars.startOid.len ), &result );
            if ( result != 0 )
                    return FALSE;            
        }
        // a special case for WALK operation.
        if (gVars.operation == WALK && gVars.value.syntax == SNMP_SYNTAX_ENDOFMIBVIEW)
        {
            return FALSE;
        }


        if ( gVars.doSet == FALSE )
        {  
            printf( "OID :" );
            memset( szBuf, 0, MAX_BUFFER_LEN );
            SnmpOidToStr( &gVars.oid, MAX_BUFFER_LEN, szBuf );
            printf( szBuf );
            printf ( "\n\r" );

            // The old mgmtapi requires a leading dot....
            StringCchCopyA( szSecString, MAX_BUFFER_LEN, "." );
            StringCchCatA( szSecString, MAX_BUFFER_LEN, szBuf );


            if ( SnmpMgrStrToOid( szSecString, &Oid ) )
            {  
                SnmpMgrOidToStr( &Oid, &string );
            } 

            if ( string )
            {  
                printf( "%s \n", string );
                SnmpUtilMemFree( string );
                SnmpUtilOidFree ( &Oid); 
            } 

            PrintOidValue( &gVars.value );
            printf( "\n\r\n\r" );
        } 
        
    } 

    return TRUE;
                             
}  //PrintVarBind




//
// abstract : parse and print v2 trap in v1 form.
//            assuming that the trap is sent by a v1 agent
//
// input:     PSNMP_MGR_SESSION pointer to the session
// output:    none
//
void    ParseAndPrintv2Trap( PSNMP_MGR_SESSION pSession )
{  
    smiOID      sysUpTime, 
                snmpTrapOid,
                snmpTrapEnterprise,
                snmpTraps,
                snmpTrapEnterpriseOID,
                tmpOid;

    smiINT      result;
    smiVALUE    value;

    DWORD       oidCount;
    DWORD       index;
    int         genTrap;
    char        szBuf[ MAX_BUFFER_LEN ];
    char        szSecString[ MAX_BUFFER_LEN ];
    AsnObjectIdentifier  Oid;      //unfortunate that there is no better way
    LPSTR       string = NULL;
    BOOL        entOid = FALSE;


    if ( pSession == NULL )
        return;

    // convert to oids.

    if ( SNMP_FAILURE( SnmpStrToOid( "1.3.6.1.2.1.1.3", &sysUpTime ) ) )
        return;
    if ( SNMP_FAILURE( SnmpStrToOid( "1.3.6.1.6.3.1.1.4", &snmpTrapOid ) ) )
        return;
    if ( SNMP_FAILURE( SnmpStrToOid( "1.3.6.1.6.3.1.1.4", &snmpTrapEnterprise ) ) )
        return;
    if ( SNMP_FAILURE( SnmpStrToOid( "1.3.6.1.6.3.1.1.5", &snmpTraps ) ) )
        return;
    if ( SNMP_FAILURE( SnmpStrToOid( "1.3.6.1.6.3.1.1.4.3.0", &snmpTrapEnterpriseOID ) ) )
        return;

    oidCount = SnmpCountVbl ( pSession->hVbl );

    for( index = 1; index <= oidCount; index++ )
    {  
        // print out varbind (name, value) pair in format of:
        // Name in numerical form
        // Name in string form
        // value
        // generic trap name (if this varbind is a generic trap)     
        SnmpGetVb( pSession->hVbl, index, &tmpOid, &value );
        printf( "OID :" );
        memset( szBuf, 0, MAX_BUFFER_LEN );
        SnmpOidToStr( &tmpOid, MAX_BUFFER_LEN, szBuf );
        printf( szBuf );
        printf ( "\n\r" );

        if (index == oidCount)
        {
            // Check if the last variable binding equals to "snmpTrapEnterpriseOID.0"
            SnmpOidCompare( &snmpTrapEnterpriseOID, &tmpOid, snmpTrapEnterpriseOID.len, &result );
            entOid = (result == 0);
        } 

        // The old mgmtapi requires a leading dot....
        StringCchCopyA( szSecString, MAX_BUFFER_LEN, "." );
        StringCchCatA( szSecString, MAX_BUFFER_LEN, szBuf );


        if ( SnmpMgrStrToOid( szSecString, &Oid ) )
        {  
            SnmpMgrOidToStr( &Oid, &string );
        } 

        if ( string )
        {  
            printf( "OID string: %s \n", string );
            SnmpUtilMemFree( string );
            SnmpUtilOidFree ( &Oid); 
        } 
        else
        {  
            printf( "OID string: <Can't convert oid to string form>\n");
        }     
        SnmpFreeDescriptor ( SNMP_SYNTAX_OID, (smiLPOPAQUE)&tmpOid);

        PrintOidValue( &value );

        // process generic traps only ..
        if ( value.syntax == SNMP_SYNTAX_OID )
        {
            tmpOid = value.value.oid; // structure copy ..
            
            SnmpOidCompare( &snmpTraps, &tmpOid, snmpTraps.len, &result );

            if ( !result )
            {  
                if ( tmpOid.len == ( snmpTraps.len + 1 ) )
                {
                    genTrap = tmpOid.ptr[ snmpTraps.len ];

                    switch( genTrap )
                        {
                        case 1:
                            PrintDbgMessage( "ColdStart \n" );
                            break;
                        case 2:
                            PrintDbgMessage( "WarmStart \n" );
                            break;
                        case 3:
                            PrintDbgMessage( "LinkDown \n" );
                            break;
                        case 4:
                            PrintDbgMessage( "LinkUp \n" );
                            break;
                        case 5:
                            PrintDbgMessage( "Authentication Failure \n" );
                            break;
                        case 6:
                            PrintDbgMessage( "EGP Neighbor Loss \n" );
                            break;
                        case 7:
                            PrintDbgMessage( "enterprise specific \n" );
                            break;
                        default:
                            break;
                        }
                }
            }
        }

        printf( "\n\r\n\r" );

        if ( entOid )
        {  
            PrintDbgMessage( "Enterprise specific oid from v1 trap \n" );
            PrintDbgMessage( "\n\r\n\r" );
        } 

        if  ( value.syntax == SNMP_SYNTAX_OCTETS )    
        {  
            SnmpFreeDescriptor ( SNMP_SYNTAX_OCTETS, (smiLPOPAQUE)&value.value.string);
        } 
        if  ( value.syntax == SNMP_SYNTAX_OID )  
        {  
            SnmpFreeDescriptor ( SNMP_SYNTAX_OID, (smiLPOPAQUE)&value.value.oid);
        } 
    } 
        
        
        SnmpFreeDescriptor ( SNMP_SYNTAX_OID, (smiLPOPAQUE)&sysUpTime);
        SnmpFreeDescriptor ( SNMP_SYNTAX_OID, (smiLPOPAQUE)&snmpTrapOid);
        SnmpFreeDescriptor ( SNMP_SYNTAX_OID, (smiLPOPAQUE)&snmpTrapEnterprise);
        SnmpFreeDescriptor ( SNMP_SYNTAX_OID, (smiLPOPAQUE)&snmpTraps);
        SnmpFreeDescriptor ( SNMP_SYNTAX_OID, (smiLPOPAQUE)&snmpTrapEnterpriseOID);

}  // end of ParseAndPrintv2Trap