/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzGlobalOptions.h

Abstract:

    Declaration of the CAzGlobalOptions class


 History:

****************************************************************************/
#pragma once
#include <atlbase.h>
#include <vector>
#include "AzLogging.h"

class CAzGlobalOptions
{
public:
    CAzGlobalOptions(void);

    ~CAzGlobalOptions(void);

    static CComBSTR m_bstrSourceStoreName;

    static CComBSTR m_bstrDestStoreName;

    static bool m_bOverWrite;

    static bool m_bSpecificApp;

    static bool m_bIgnoreMembers;

    static bool m_bVerbose;

    static bool m_bIgnorePolicyAdmins;

    static bool m_bVersionTwo;

    static vector<CComBSTR> m_bstrAppNames;

    static void setDefaults() {

        m_bOverWrite = false;

        m_bSpecificApp = false;

        m_bIgnoreMembers = false;

        m_bIgnorePolicyAdmins = false;

        m_bVerbose = false;

        m_bVersionTwo = false;

        CAzLogging::Initialize(CAzLogging::LOG_ERROR);

    }


    static const _TCHAR LOGFILETAG[];

    static const _TCHAR APPNAMETAG[];

    static const _TCHAR OVERWRITETAG[];

    static const _TCHAR IGNOREMEMBERSTAG[];

    static const _TCHAR IGNOREPOLICYADMINSTAG[];

    static const _TCHAR HELPTAG[];

    static const _TCHAR VERBOSETAG[];

    static const int LOGFILETAG_LEN=2;

    static const int APPNAMETAG_LEN=2;

    static const int OVERWRITETAG_LEN=2;

    static const int IGNOREMEMBERSTAG_LEN=3;

    static const int IGNOREPOLICYADMINSTAG_LEN=3;

    static const int VERBOSETAG_LEN=2;

    static const int HELPTAG_LEN=2;
};

