//****************************************************************
// 
// header file for wsnmputil.cpp utility.
//
//****************************************************************
#ifndef _WSNMP_UTIL_
#define _WSNMP_UTIL_ 1


//**************************************************************************
//
//  definitions.
//
//**************************************************************************

#define        WM_SNMP_INCOMING    (WM_USER + 1)
#define        WM_SNMP_DONE        (WM_USER + 2)
#define        NOTIFICATION_CLASS  "SNMPUTIL NOTIFICATION CLASS"
#define        SNMP_FAILURE(s)     ((s) == SNMPAPI_FAILURE)
#define        BUFFER_LEN          255
#define        MAXENTITYSTRLEN     128
#define        IPADDRLEN           4
#define        MINVARBINDLEN       2
#define        SYSUPTIMEINDEX      0
#define        SNMPTRAPOIDINDEX    1
#define        DEFAULT_ADDRESS_IP  "127.0.0.1"
#define        CMD_LINE_ARGS       255      // maximum command line arguments
#define        MAX_GETBLK_OIDS     10
#define        MAX_BUFFER_LEN      2048

#define        MAX_ADDR_LEN        128
// defintions that define the operation we are about to do.

#define        GET                 1
#define        GET_NEXT            2
#define        WALK                3
#define        SET                 4
#define        TRAP                5
#define        GET_BULK            6
#define        SUB_TREE            7


//**************************************************************************
//
//  Structure definitions.
//
//**************************************************************************

typedef struct _SNMP_MGR_SESSION
{

 HSNMP_SESSION     hSnmpSession;     // handle to winsnmp session
 HSNMP_ENTITY      hAgentEntity;     // handle to agent entity
 HSNMP_ENTITY      hManagerEntity;   // handle to manager entity
 HSNMP_CONTEXT     hViewContext;     // handle to view context
 HSNMP_PDU         hPdu;             // handle to snmp pdu
 HSNMP_VBL         hVbl;             // handle to var bind list
 HWND              hWnd;             // handle to window

 smiINT32          nPduType;         // current pdu type
 smiINT32          nRequestId;       // current request id
 smiINT32          nError;           // last system error
 smiINT32          nErrorStatus;     // error status
 smiINT32          nErrorIndex;      // error index
 
} SNMP_MGR_SESSION, *PSNMP_MGR_SESSION;


//****************************************************************
//
// global vars.
//
//****************************************************************

class GlobalVars
{
public:
    GlobalVars( ) 
    { 
        nTimeOut        = 6000;
        nRetries        = 3;
        nRequestId      = 1;
        oidCount        = 0;
        pAgentStrAddr   = NULL;
        pAgentCommunity = NULL;
        pSetValue       = NULL;
        fDone           = FALSE;           // default
        version         = FALSE;           // Use v1 by default;
        doSet           = FALSE;           // dont do set by default.
    }

    ~GlobalVars( ) { }
    HINSTANCE    g_hInst;                     // module handle.
    smiINT32     oidCount;                    // number of Oids.
    smiINT32     operation;                   // define the operation
    smiINT32     nTimeOut;                    // timeout in milli-seconds
    smiINT32     nRetries;                    // number of retries
    smiINT32     nRequestId;                  // request id.
    smiVALUE     value;                       // encapsulate in global vars
    smiOID       startOid;                    // starting OID.
    smiOID       oid;                         // oid from the command line.
    smiINT       non_repeaters;               // non_repeaters field of SNMP_PDU_GETBULK
    smiINT       max_repetitions;             // max_repetitions field of SNMP_PDU_GETBULK
    BOOL         doSet;                       // should we do a set?
    BOOL         fDone;                       // A bool var 
    BOOL         version;                     // if TRUE -> v2C else use v1.
    CHAR         *pAgentStrAddr;              // pointer to agent string
    CHAR         *pSetValue;                  // pointer to the SET value in ASCII
    CHAR         *pAgentCommunity;            // pointer to agent community string
    CHAR         *pszOid[ CMD_LINE_ARGS ];    // pointer to OID string
    CHAR		 agentAddr[MAX_ADDR_LEN];

};

extern GlobalVars    gVars;

//****************************************************************
//
// function prototypes. 
//
//****************************************************************

//
// following functions are defined in wsnmputil.cpp
//
BOOL    OpenWinSNMPSession( PSNMP_MGR_SESSION pSession );
BOOL    CloseWinSNMPSession( PSNMP_MGR_SESSION pSession );
BOOL    CreateNotificationWindow( PSNMP_MGR_SESSION pSession );
BOOL    DestroyNotificationWindow( PSNMP_MGR_SESSION pSession );
LRESULT CALLBACK    NotificationWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
BOOL    ProcessNotification( PSNMP_MGR_SESSION pSession );
BOOL    WaitForTraps( PSNMP_MGR_SESSION pSession );
BOOL    ProcessAgentResponse( PSNMP_MGR_SESSION pSession );

BOOL    CreateVbl( PSNMP_MGR_SESSION pSession, smiOID *pOid, smiVALUE * pValue  );
void    FreeVblandPdu( PSNMP_MGR_SESSION pSession );
BOOL    CreatePduSendRequest( PSNMP_MGR_SESSION pSession, smiVALUE *pValue );

//
// following functions are defined in helper.cpp
//
void    Usage( );
void    PrintDbgMessage( LPSTR, ... );
BOOL    ConvertTexttoIp( char *agentAddress );
BOOL    ParseCommandLine( int argc, char **argv );
void    ConvertStringToSmiValue( smiVALUE *pValue );
void    PrintOidValue( smiVALUE *value );
BOOL    PrintVarBind( PSNMP_MGR_SESSION pSession );
void    ParseAndPrintv2Trap( PSNMP_MGR_SESSION pSession );


#endif

