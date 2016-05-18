/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzGlobalOptions.cpp

Abstract:

    All the global options used for as command line options


 History:

****************************************************************************/
#include "AzGlobalOptions.h"

bool CAzGlobalOptions::m_bOverWrite;

bool CAzGlobalOptions::m_bSpecificApp;

bool CAzGlobalOptions::m_bIgnoreMembers;

bool CAzGlobalOptions::m_bIgnorePolicyAdmins;

bool CAzGlobalOptions::m_bVerbose;

bool CAzGlobalOptions::m_bVersionTwo;

vector<CComBSTR>  CAzGlobalOptions::m_bstrAppNames;

CComBSTR  CAzGlobalOptions::m_bstrSourceStoreName;

CComBSTR  CAzGlobalOptions::m_bstrDestStoreName;


// All of the below are searched for in a case-insensitive manner
const _TCHAR CAzGlobalOptions::LOGFILETAG[]=_TEXT("/l");

const _TCHAR CAzGlobalOptions::APPNAMETAG[]=_TEXT("/a");

const _TCHAR CAzGlobalOptions::OVERWRITETAG[]=_TEXT("/o");

const _TCHAR CAzGlobalOptions::IGNOREMEMBERSTAG[]=_TEXT("/im");

const _TCHAR CAzGlobalOptions::IGNOREPOLICYADMINSTAG[]=_TEXT("/ip");

const _TCHAR CAzGlobalOptions::VERBOSETAG[]=_TEXT("/v");

const _TCHAR CAzGlobalOptions::HELPTAG[]=_TEXT("/?");

