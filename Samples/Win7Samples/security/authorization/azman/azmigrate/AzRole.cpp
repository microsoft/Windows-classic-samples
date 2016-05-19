/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzRole.cpp

Abstract:

    Routines performing the migration for the AzRole object

 History:

****************************************************************************/
#include "AzRole.h"

CAzRole::CAzRole(void):CAzBase<IAzRole>()
{
}

CAzRole::~CAzRole(void)
{
}

CAzRole::CAzRole(IAzRole *pNative,bool pisNew):CAzBase<IAzRole>(pNative,pisNew){
}

/*++

Routine description:

    This method copies properties from the source role to *this* 
    role

Arguments: srcRole - Source role

Return Value:

    Returns success, appropriate failure value 

--*/

HRESULT CAzRole::CopyUserData(CAzRole &srcRole){

    CAzLogging::Entering(_TEXT("CAzRole::CopyUserData"));

    HRESULT hr;

    CComVariant var;

    hr=srcRole.m_native->get_Operations(&var);

    CAzLogging::Log(hr,_TEXT("Getting operations for role"),COLE2T(getName()));			

    if (SUCCEEDED(hr)) {

        hr=InitializeUsingSafeArray(var,&IAzRole::AddOperation);

        CAzLogging::Log(hr,_TEXT("Adding Operations"),COLE2T(getName()));

        var.Clear();
    }

    hr=srcRole.m_native->get_AppMembers(&var);

    CAzLogging::Log(hr,_TEXT("Getting app members for role"),COLE2T(getName()));

    if (SUCCEEDED(hr)) {

        hr=InitializeUsingSafeArray(var,&IAzRole::AddAppMember);

        CAzLogging::Log(hr,_TEXT("Setting app members for role"),COLE2T(getName()));

        var.Clear();

    }

    if (!CAzGlobalOptions::m_bIgnoreMembers) {

        hr=srcRole.m_native->get_Members(&var);

        CAzLogging::Log(hr,_TEXT("Getting Members for role"),COLE2T(getName()));

        if (SUCCEEDED(hr)) {

            hr=InitializeUsingSafeArray(var,&IAzRole::AddMember);

            CAzLogging::Log(hr,_TEXT("Setting Members for role"),COLE2T(getName()));

            var.Clear();

        }
    }

    hr=srcRole.m_native->get_Tasks(&var);

    CAzLogging::Log(hr,_TEXT("Getting Tasks for role"),COLE2T(getName()));

    if (SUCCEEDED(hr)) {

        hr=InitializeUsingSafeArray(var,&IAzRole::AddTask);

        CAzLogging::Log(hr,_TEXT("Setting Tasks for role"),COLE2T(getName()));

        }

    CAzLogging::Exiting(_TEXT("CAzRole::CopyUserData"));

    return hr;
}

/*++

Routine description:

    This method copies properties from the source role to *this* 
    role

Arguments: srcRole - Source role

Return Value:

    Returns success, appropriate failure value 

--*/

HRESULT CAzRole::Copy(CAzRole &srcRole) {

    CAzLogging::Entering(_TEXT("CAzRole::Copy"));

    CComBSTR bstrData;

    HRESULT hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting for role"),COLE2T(getName()));

    if (FAILED(hr))
        goto lError1;

    hr=srcRole.m_native->get_Description(&bstrData);

    CAzLogging::Log(hr,_TEXT("Getting Description for role"),COLE2T(getName()));

    if (SUCCEEDED(hr)) {

        hr=m_native->put_Description(bstrData);

        CAzLogging::Log(hr,_TEXT("Setting Description for role"),COLE2T(getName()));

        bstrData.Empty();
    }

    hr=srcRole.m_native->get_ApplicationData(&bstrData);

    CAzLogging::Log(hr,_TEXT("Getting ApplicationData for role"),COLE2T(getName()));

    if (SUCCEEDED(hr)) {

        hr=m_native->put_ApplicationData(bstrData);

        CAzLogging::Log(hr,_TEXT("Setting ApplicationData for role"),COLE2T(getName()));

    }

    hr=CopyUserData(srcRole);

    CAzLogging::Log(hr,_TEXT("CopyUserData for role"),COLE2T(getName()));

    hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting for role"),COLE2T(getName()));

lError1:
    CAzLogging::Exiting(_TEXT("CAzRole::Copy"));

    return hr;
}
