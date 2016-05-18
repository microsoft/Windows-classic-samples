/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright 1995 - 2000 Microsoft Corporation

Module Name:

    dialer.h

Abstract:

    Header file for dialer

Author:

    Dan Knudson (DanKn)        05-Apr-1995

Revision History:

    Jeremy Horwitz (t-jereh)   30-May-1995

--*/


#define TAPI_VERSION_1_0 0x00010003
#define TAPI_VERSION_1_4 0x00010004
#define TAPI_VERSION_2_0 0x00020000
#define TAPI_CURRENT_VERSION TAPI_VERSION_2_0

#include <windows.h>
#include "tapi.h"
#include "resource.h"

#define MENU_CHOICE         1 // for Connect Using dialog...
#define INVALID_LINE        2 // if INVALID_LINE, turn off CANCEL
                              // button and add extra text...

#define MAXNUMLENGTH    64
#define MAXBUFSIZE      256
#define NSPEEDDIALS     8 // Dialer supports 8 configurable speed dial entries.
#define NLASTDIALED     20 // Dialer keeps track of the 20 last dialed numbers.

#define ERR_NONE        0
#define ERR_NOVOICELINE 1
#define ERR_LINECLOSE   2
#define ERR_NOLINES     3
#define	ERR_911WARN		4
#define ERR_NEWDEFAULT	5


