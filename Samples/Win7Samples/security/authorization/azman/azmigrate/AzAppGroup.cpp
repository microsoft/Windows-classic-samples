/* **************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzAppGroup.cpp

Abstract:

    Routines performing the migration for the AppGroup object


 History:

 ***************************************************************************/
#include "AzAppGroup.h"


CAzAppGroup::CAzAppGroup(void):CAzBase<IAzApplicationGroup>()
{
}

CAzAppGroup::~CAzAppGroup(void)
{
}

CAzAppGroup::CAzAppGroup(IAzApplicationGroup *pNative,bool pisNew):CAzBase<IAzApplicationGroup>(pNative,pisNew){
}


/*++

Routine description:

    This method copies properties from the source app group to *this* app
    Group which are specific to version 1.2

Arguments: Source App Group

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/

HRESULT CAzAppGroup::CopyVersion2Constructs(CAzAppGroup &srcAppGroup) {

    static unsigned int rgProperties[]={AZ_PROP_GROUP_BIZRULE,AZ_PROP_GROUP_BIZRULE_LANGUAGE,AZ_PROP_GROUP_BIZRULE_IMPORTED_PATH};

    CComVariant cVVar;

    HRESULT hr=S_OK;

    if (!CAzGlobalOptions::m_bVersionTwo)
        goto lDone;

    for (long i=0;i<3;i++) {

        hr=srcAppGroup.m_native->GetProperty(rgProperties[i],CComVariant(), &cVVar);

        CAzLogging::Log(hr,_TEXT("Getting IAzApplicationGroup Property ID:"),COLE2T(srcAppGroup.getName()),rgProperties[i]);

        if (SUCCEEDED(hr) && (SysStringByteLen(cVVar.bstrVal))!=0) {

            hr=m_native->SetProperty(rgProperties[i],cVVar,CComVariant());

            CAzLogging::Log(hr,_TEXT("Setting IAzApplicationGroup Property ID:"),COLE2T(getName()),rgProperties[i]);

            cVVar.Clear();

        }

    }
lDone:
    return hr;
}

/*++

Routine description:

    This method copies properties from the source app group to *this* app
    Group

Arguments: Source App Group

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/

HRESULT CAzAppGroup::Copy(CAzAppGroup &srcAppGroup) {

    CAzLogging::Entering(_TEXT("Copy"));

    CComBSTR bstrData;

    CComVariant cVVar;

    LONG lType;


    HRESULT hr=srcAppGroup.m_native->get_Description(&bstrData);

    CAzLogging::Log(hr,_TEXT("Getting Description for App Group"),COLE2T(srcAppGroup.getName()));

    if (SUCCEEDED(hr)) {

        hr=m_native->put_Description(bstrData);

        CAzLogging::Log(hr,_TEXT("Setting Description for App Group"),COLE2T(getName()));

        bstrData.Empty();
    }

    hr=srcAppGroup.m_native->get_Type(&lType);

    CAzLogging::Log(hr,_TEXT("Getting Type for App Group"),COLE2T(srcAppGroup.getName()));

    if (SUCCEEDED(hr)) {

        hr=m_native->put_Type(lType);

        CAzLogging::Log(hr,_TEXT("Setting Type for App Group"),COLE2T(getName()));

    }


    hr=srcAppGroup.m_native->get_LdapQuery(&bstrData);

    CAzLogging::Log(hr,_TEXT("Getting LDAPQuery for App Group"),COLE2T(srcAppGroup.getName()));

    // THe length check is reqd below as adding an empty string
    // LDAP Query returns an error

    if (SUCCEEDED(hr) && bstrData.Length()!=0) {

        hr=m_native->put_LdapQuery(bstrData);

        CAzLogging::Log(hr,_TEXT("Setting LDAPQuery for App Group"),COLE2T(getName()));

        bstrData.Empty();
    }

    if (CAzGlobalOptions::m_bVersionTwo) {

            hr = CopyVersion2Constructs(srcAppGroup);

            CAzLogging::Log(hr,_TEXT("Copying bizrule properties for App Group"),COLE2T(srcAppGroup.getName()));
    }

    if (!CAzGlobalOptions::m_bIgnoreMembers) {

        hr=srcAppGroup.m_native->get_Members(&cVVar);

        CAzLogging::Log(hr,_TEXT("Getting Members for App Group"),COLE2T(srcAppGroup.getName()));

        if (SUCCEEDED(hr)) {

            hr=InitializeUsingSafeArray(cVVar,&IAzApplicationGroup::AddMember);

            CAzLogging::Log(hr,_TEXT("Setting Members for App Group"),COLE2T(getName()));

            cVVar.Clear();
        }

        hr=srcAppGroup.m_native->get_NonMembers(&cVVar);

        CAzLogging::Log(hr,_TEXT("Getting Non Members for App Group"),COLE2T(srcAppGroup.getName()));

        if (SUCCEEDED(hr)) {

            hr=InitializeUsingSafeArray(cVVar,&IAzApplicationGroup::AddNonMember );

            CAzLogging::Log(hr,_TEXT("Setting Non Members for App Group"),COLE2T(getName()));

            cVVar.Clear();
        }
    }

    hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting App Group"),COLE2T(getName()));

    CAzLogging::Exiting(_TEXT("Copy"));

    return hr;
}

/*++

Routine description:

    This method copies links found in the source app group to *this* app
    Group

Arguments: Source App Group

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/

HRESULT CAzAppGroup::CopyLinks(CAzAppGroup &srcAppGroup) {

    CAzLogging::Entering(_TEXT("CopyLinks"));

    CComVariant cVVar;

    HRESULT hr;

    hr=srcAppGroup.m_native->get_AppMembers(&cVVar);

    CAzLogging::Log(hr,_TEXT("Getting App Members for App Group"),COLE2T(srcAppGroup.getName()));

    if (SUCCEEDED(hr)) {

        hr=InitializeUsingSafeArray(cVVar,&IAzApplicationGroup::AddAppMember );

        CAzLogging::Log(hr,_TEXT("Setting App Members for App Group"),COLE2T(getName()));

        cVVar.Clear();
    }

    hr=srcAppGroup.m_native->get_AppNonMembers(&cVVar);

    CAzLogging::Log(hr,_TEXT("Getting App Non Members for App Group"),COLE2T(srcAppGroup.getName()));

    if (SUCCEEDED(hr)) {

        hr=InitializeUsingSafeArray(cVVar,&IAzApplicationGroup::AddAppNonMember );

        CAzLogging::Log(hr,_TEXT("Setting App Non Members for App Group"),COLE2T(getName()));

        cVVar.Clear();
    }

    hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting for App Group"),COLE2T(getName()));

    CAzLogging::Exiting(_TEXT("CopyLinks"));

    return hr;
}
