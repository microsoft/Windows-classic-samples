//***************************************************************************

//

//  WBEMSEC.H

//

//  Purpose: Provides prototypes for some security helper functions.

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
//***************************************************************************

#ifndef _WBEMSEC_H_
#define _WBEMSEC_H_

HRESULT InitializeSecurity(DWORD dwAuthLevel, DWORD dwImpLevel);
SCODE GetAuthImp(IUnknown * pFrom, DWORD * pdwAuthLevel, DWORD * pdwImpLevel);
HRESULT SetInterfaceSecurity(IUnknown * pInterface, LPWSTR pDomain, LPWSTR pUser, LPWSTR pPassword, DWORD dwAuthLevel, DWORD dwImpLevel);

#endif // _WBEMSEC_H_
