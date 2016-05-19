
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright 1993 - 2000 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/

/******************************************************************************\
*
*                                  ENUMPRT.H
*
\******************************************************************************/

/******************************************************************************\
*                             SYMBOLIC CONSTANTS
\******************************************************************************/

#define MAX_ATTRIBUTES     4   // # of elements in gaAttributes
#define MAX_PRIORITIES     4   // # of elements in gaPriorities
#define MAX_STATUS         3   // # of elements in gaStatus
#define MAX_FIELDS         15  // # of elements in gaFields
#define MAX_ORIENTATION    2   // # of elements in gaOrientation
#define MAX_PAPERSIZE      41  // # of elements in gaPaperSize
#define MAX_DEFAULTSOURCE  16  // # of elements in gaDefaultSource
#define MAX_PRINTQUALITY   4   // # of elements in gaPrintQuality
#define MAX_COLOR          2   // # of elements in gaColor
#define MAX_DUPLEX         3   // # of elements in gaDuplex

#define ERR_MOD_NAME       IDS_ERR_ENUMPRT

#define BLANKS             "\t\t\t"

/******************************************************************************\
*                                  TYPEDEFS
\******************************************************************************/

typedef struct tagENUMPRTLOOKUP
{
  DWORD   dwValue;
  LPCSTR  szValue;

} ENUMPRTLOOKUP;



/******************************************************************************\
*                              GLOBAL VARIABLES
\******************************************************************************/

HWND  ghwndEnumPrt;
DWORD gdwEnumFlags;
char  gszEnumName[BUFSIZE];

LPSTR gaEnumPrt[] =

      { "PRINTER_INFO_1"  ,
        "  pDescription:      \t%s",
        "  pName:             \t%s",
        "  pComment:          \t%s",
        "PRINTER_INFO_2"  ,
        "  pServerName:       \t%s",  // 5
        "  pPrinterName:      \t%s",
        "  pShareName:        \t%s",
        "  pPortName:         \t%s",
        "  pDriverName:       \t%s",
        "  pComment:          \t%s",  // 10
        "  pLocation:         \t%s",
        "  pDevMode           \t%s",
        "     dmDeviceName:     \t%s",
        "     dmSpecVersion:    \t%d",
        "     dmDriverVersion:  \t%d",  // 15
        "     dmSize:           \t%d",
        "     dmDriverExtra:    \t%d",
        "     dmFields:         \t",
        "     dmOrientation:    \t",
        "     dmPaperSize:      \t",  // 20
        "     dmPaperLength:    \t%d",
        "     dmPaperWidth:     \t%d",
        "     dmScale:          \t%d",
        "     dmCopies:         \t%d",
        "     dmDefaultSource:  \t",  // 25
        "     dmPrintQuality:   \t%s",
        "     dmColor:          \t",
        "     dmDuplex:         \t",
        "     dmYResolution:    \t%d",
        "     dmTTOption:       \t%d",  // 30
        "     dmCollate:        \t&d",
        "     dmFormName:       \t%s",
        "  pSepFile:          \t%s",
        "  pPrintProcessor:   \t%s",
        "  pDatatype:         \t%s",  // 35
        "  pParameters:       \t%s",
        "  Attributes:        \t",
        "  Priority:          \t%s",
        "  Priority:          \t%ld",
        "  DefaultPriority:   \t%ld",  // 40
        "  StartTime:         \t%ld",
        "  UntilTime:         \t%ld",
        "  Status:            \t",
        "  cJobs:             \t%ld",
        "  AveragePPM:        \t%ld"    };

ENUMPRTLOOKUP gaAttributes[] =

    { { PRINTER_ATTRIBUTE_QUEUED       , "PRINTER_ATTRIBUTE_QUEUED  " },
      { PRINTER_ATTRIBUTE_DIRECT       , "PRINTER_ATTRIBUTE_DIRECT  " },
      { PRINTER_ATTRIBUTE_DEFAULT      , "PRINTER_ATTRIBUTE_DEFAULT " },
      { PRINTER_ATTRIBUTE_SHARED       , "PRINTER_ATTRIBUTE_SHARED  " } };

ENUMPRTLOOKUP gaPriorities[] =

    { { NO_PRIORITY                    , "NO_PRIORITY  " },
      { MAX_PRIORITY                   , "MAX_PRIORITY " },
      { MIN_PRIORITY                   , "MIN_PRIORITY " },
      { DEF_PRIORITY                   , "DEF_PRIORITY " } };

ENUMPRTLOOKUP gaStatus[] =

    { { PRINTER_STATUS_PAUSED          , "PRINTER_STATUS_PAUSED "          },
      { PRINTER_STATUS_ERROR           , "PRINTER_STATUS_ERROR  "          },
      { PRINTER_STATUS_PENDING_DELETION, "PRINTER_STATUS_PENDING_DELETION "},
      { 0                              , ""                                }};

ENUMPRTLOOKUP gaFields[] =

    { { DM_ORIENTATION  , "DM_ORIENTATION  " },
      { DM_PAPERSIZE    , "DM_PAPERSIZE    " },
      { DM_PAPERLENGTH  , "DM_PAPERLENGTH  " },
      { DM_PAPERWIDTH   , "DM_PAPERWIDTH   " },
      { DM_SCALE        , "DM_SCALE        " },
      { DM_COPIES       , "DM_COPIES       " },
      { DM_DEFAULTSOURCE, "DM_DEFAULTSOURCE" },
      { DM_PRINTQUALITY , "DM_PRINTQUALITY " },
      { DM_COLOR        , "DM_COLOR        " },
      { DM_DUPLEX       , "DM_DUPLEX       " },
      { DM_YRESOLUTION  , "DM_YRESOLUTION  " },
      { DM_TTOPTION     , "DM_TTOPTION     " },
      { DM_COLLATE      , "DM_COLLATE      " },
      { DM_FORMNAME     , "DM_FORMNAME     " },
      { 0               , "0               " } };

ENUMPRTLOOKUP gaOrientation[] =

    { { DMORIENT_PORTRAIT , "DMORIENT_PORTRAIT " },
      { DMORIENT_LANDSCAPE, "DMORIENT_LANDSCAPE" } };

ENUMPRTLOOKUP gaPaperSize[] =

    { { DMPAPER_FIRST             , "DMPAPER_FIRST             " },
      { DMPAPER_LETTER            , "DMPAPER_LETTER            " },
      { DMPAPER_LETTERSMALL       , "DMPAPER_LETTERSMALL       " },
      { DMPAPER_TABLOID           , "DMPAPER_TABLOID           " },
      { DMPAPER_LEDGER            , "DMPAPER_LEDGER            " },
      { DMPAPER_LEGAL             , "DMPAPER_LEGAL             " },
      { DMPAPER_STATEMENT         , "DMPAPER_STATEMENT         " },
      { DMPAPER_EXECUTIVE         , "DMPAPER_EXECUTIVE         " },
      { DMPAPER_A3                , "DMPAPER_A3                " },
      { DMPAPER_A4                , "DMPAPER_A4                " },
      { DMPAPER_A4SMALL           , "DMPAPER_A4SMALL           " },
      { DMPAPER_A5                , "DMPAPER_A5                " },
      { DMPAPER_B4                , "DMPAPER_B4                " },
      { DMPAPER_B5                , "DMPAPER_B5                " },
      { DMPAPER_FOLIO             , "DMPAPER_FOLIO             " },
      { DMPAPER_QUARTO            , "DMPAPER_QUARTO            " },
      { DMPAPER_10X14             , "DMPAPER_10X14             " },
      { DMPAPER_11X17             , "DMPAPER_11X17             " },
      { DMPAPER_NOTE              , "DMPAPER_NOTE              " },
      { DMPAPER_ENV_9             , "DMPAPER_ENV_9             " },
      { DMPAPER_ENV_10            , "DMPAPER_ENV_10            " },
      { DMPAPER_ENV_11            , "DMPAPER_ENV_11            " },
      { DMPAPER_ENV_12            , "DMPAPER_ENV_12            " },
      { DMPAPER_ENV_14            , "DMPAPER_ENV_14            " },
      { DMPAPER_CSHEET            , "DMPAPER_CSHEET            " },
      { DMPAPER_DSHEET            , "DMPAPER_DSHEET            " },
      { DMPAPER_ESHEET            , "DMPAPER_ESHEET            " },
      { DMPAPER_ENV_DL            , "DMPAPER_ENV_DL            " },
      { DMPAPER_ENV_C5            , "DMPAPER_ENV_C5            " },
      { DMPAPER_ENV_C3            , "DMPAPER_ENV_C3            " },
      { DMPAPER_ENV_C4            , "DMPAPER_ENV_C4            " },
      { DMPAPER_ENV_C6            , "DMPAPER_ENV_C6            " },
      { DMPAPER_ENV_C65           , "DMPAPER_ENV_C65           " },
      { DMPAPER_ENV_B4            , "DMPAPER_ENV_B4            " },
      { DMPAPER_ENV_B5            , "DMPAPER_ENV_B5            " },
      { DMPAPER_ENV_B6            , "DMPAPER_ENV_B6            " },
      { DMPAPER_ENV_ITALY         , "DMPAPER_ENV_ITALY         " },
      { DMPAPER_ENV_MONARCH       , "DMPAPER_ENV_MONARCH       " },
      { DMPAPER_ENV_PERSONAL      , "DMPAPER_ENV_PERSONAL      " },
      { DMPAPER_FANFOLD_STD_GERMAN, "DMPAPER_FANFOLD_STD_GERMAN" },
      { DMPAPER_FANFOLD_LGL_GERMAN, "DMPAPER_FANFOLD_LGL_GERMAN" } };

ENUMPRTLOOKUP gaDefaultSource[] =

    { { DMBIN_FIRST        , "DMBIN_FIRST        " },
      { DMBIN_UPPER        , "DMBIN_UPPER        " },
      { DMBIN_ONLYONE      , "DMBIN_ONLYONE      " },
      { DMBIN_LOWER        , "DMBIN_LOWER        " },
      { DMBIN_MIDDLE       , "DMBIN_MIDDLE       " },
      { DMBIN_MANUAL       , "DMBIN_MANUAL       " },
      { DMBIN_ENVELOPE     , "DMBIN_ENVELOPE     " },
      { DMBIN_ENVMANUAL    , "DMBIN_ENVMANUAL    " },
      { DMBIN_AUTO         , "DMBIN_AUTO         " },
      { DMBIN_TRACTOR      , "DMBIN_TRACTOR      " },
      { DMBIN_SMALLFMT     , "DMBIN_SMALLFMT     " },
      { DMBIN_LARGEFMT     , "DMBIN_LARGEFMT     " },
      { DMBIN_LARGECAPACITY, "DMBIN_LARGECAPACITY" },
      { DMBIN_CASSETTE     , "DMBIN_CASSETTE     " },
      { DMBIN_LAST         , "DMBIN_LAST         " },
      { DMBIN_USER         , "DMBIN_USER         " } };

ENUMPRTLOOKUP gaPrintQuality[] =

    { { (DWORD)DMRES_DRAFT , "DMRES_DRAFT " },
      { (DWORD)DMRES_LOW   , "DMRES_LOW   " },
      { (DWORD)DMRES_MEDIUM, "DMRES_MEDIUM" },
      { (DWORD)DMRES_HIGH  , "DMRES_HIGH  " } };

ENUMPRTLOOKUP gaColor[] =

    { { DMCOLOR_MONOCHROME, "DMCOLOR_MONOCHROME" },
      { DMCOLOR_COLOR     , "DMCOLOR_COLOR     " } };

ENUMPRTLOOKUP gaDuplex[] =

    { { DMDUP_SIMPLEX   , "DMDUP_SIMPLEX   " },
      { DMDUP_VERTICAL  , "DMDUP_VERTICAL  " },
      { DMDUP_HORIZONTAL, "DMDUP_HORIZONTAL" } };

char *gaDriverInfo[] =

    {   "DRIVER_INFO_1",
        "  pName\t\t%s",

        "DRIVER_INFO_2",
        "  cVersion\t%ld",
        "  pName\t\t%s",
        "  pEnvironment\t%s",
        "  pDriverPath\t%s",
        "  pDataFile\t%s",
        "  pConfigFile\t%s" };



/******************************************************************************\
*                              EXTERNAL VARIABLES
\******************************************************************************/

extern HWND ghwndMain;



/******************************************************************************\
*                                  MACROS
\******************************************************************************/

#define outstr(fmt,arg)  { _sntprintf_s (buf,_countof(buf),_TRUNCATE,fmt,arg?arg:"(null)"); \
                           SendDlgItemMessage (hwnd, DID_LISTBOX,         \
                                               LB_INSERTSTRING, (UINT)-1, \
                                               (LONG_PTR) buf); }

#define outstr2(buf)     SendDlgItemMessage (hwnd, DID_LISTBOX,         \
                                             LB_INSERTSTRING, (UINT)-1, \
                                             (LONG_PTR) buf);

#define outstr3()        SendDlgItemMessage (hwnd, DID_LISTBOX,         \
                                             LB_INSERTSTRING,           \
                                             (UINT)-1, (LONG_PTR) buf);

#define outnum(fmt,arg)  { _sntprintf_s (buf,_countof(buf),_TRUNCATE,fmt,arg); \
                           SendDlgItemMessage (hwnd, DID_LISTBOX,         \
                                               LB_INSERTSTRING, (UINT)-1, \
                                               (LONG_PTR) buf); }



/******************************************************************************\
*                            FUNCTION PROTOTYPES
\******************************************************************************/

LRESULT CALLBACK EnumPrintersOptionsDlgProc (HWND, UINT, WPARAM, LPARAM);

BOOL DisplayEnumPrintersInfo   (HWND);
void SetEnumPrintersDlgFields  (HWND, DWORD, LPPRINTER_INFO_1,
                                LPPRINTER_INFO_2);
void ComplexEnumPrintersLine   (HWND, char *, ENUMPRTLOOKUP *, int, DWORD);
BOOL DisplayPrinterDriversInfo (HWND);
