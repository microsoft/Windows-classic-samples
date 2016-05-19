/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzScope.cpp

Abstract:

    Routines performing the migration for the CAzScope objects

 History:

****************************************************************************/
#include "AzScope.h"


CAzScope::CAzScope(void):CAzBase<IAzScope>()
{
}

CAzScope::~CAzScope(void)
{
}
CAzScope::CAzScope(IAzScope *pNative,bool pisNew):CAzBase<IAzScope>(pNative,pisNew){
}

/*++

Routine description:

    This method copies properties from the source scope to *this* 
    roscope

Arguments: srcScope - Source scope

Return Value:

    Returns success, appropriate failure value 

--*/

HRESULT CAzScope::Copy(CAzScope &srcScope) {

    CAzLogging::Entering(_TEXT("CAzScope::Copy"));

    CComBSTR bstrData;

    CComVariant cVVar;

    HRESULT hr=srcScope.m_native->get_Description(&bstrData);

    CAzLogging::Log(hr,_TEXT("Getting Description for scope"),COLE2T(getName()));

    if (SUCCEEDED(hr)) {

        hr=m_native->put_Description(bstrData);

        CAzLogging::Log(hr,_TEXT("Setting Description for scope"),COLE2T(getName()));

        bstrData.Empty();

    }

    hr=srcScope.m_native->get_ApplicationData(&bstrData);

    CAzLogging::Log(hr,_TEXT("Getting ApplicationData for scope"),COLE2T(getName()));


    if (SUCCEEDED(hr)) {

        hr=m_native->put_ApplicationData(bstrData);

        CAzLogging::Log(hr,_TEXT("Setting ApplicationData for scope"),COLE2T(getName()));

    }

    if (!CAzGlobalOptions::m_bIgnorePolicyAdmins) {

        hr=srcScope.m_native->get_PolicyAdministrators(&cVVar);

        CAzLogging::Log(hr,_TEXT("Getting PolicyAdministrators for scope"),COLE2T(getName()));

        if (SUCCEEDED(hr)) {

            hr=InitializeUsingSafeArray(cVVar,&IAzScope::AddPolicyAdministrator );

            CAzLogging::Log(hr,_TEXT("Setting PolicyAdministrators for scope"),COLE2T(getName()));

            cVVar.Clear();
        }

        hr=srcScope.m_native->get_PolicyReaders(&cVVar);

        CAzLogging::Log(hr,_TEXT("Getting PolicyReaders for scope"),COLE2T(getName()));			

        if (SUCCEEDED(hr)) {

            hr=InitializeUsingSafeArray(cVVar,&IAzScope::AddPolicyReader );

            CAzLogging::Log(hr,_TEXT("Setting PolicyReaders for scope"),COLE2T(getName()));

            cVVar.Clear();
        }

    }

    hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting property changes for scope"),COLE2T(getName()));

    if (FAILED(hr))
        goto lError1;

    hr=CAzHelper<IAzScope>::CreateAppGroups(srcScope.m_native,m_native);

    CAzLogging::Log(hr,_TEXT("Creating App Groups for scope"),COLE2T(getName()));

    hr=CAzHelper<IAzScope>::CreateTasks(srcScope.m_native,m_native);

    CAzLogging::Log(hr,_TEXT("Creating Tasks for scope"),COLE2T(getName()));

    hr=CAzHelper<IAzScope>::CreateRoles(srcScope.m_native,m_native);

    CAzLogging::Log(hr,_TEXT("Creating Roles for scope"),COLE2T(getName()));

    hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting child object addition changes for scope"),COLE2T(getName()));

lError1:
    CAzLogging::Exiting(_TEXT("CAzScope::Copy"));

    return hr;
}

