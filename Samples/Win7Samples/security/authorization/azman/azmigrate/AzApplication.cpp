/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzApplication.cpp

Abstract:

    Routines performing the migration for the CAzApplication object

 History:

****************************************************************************/
#include "AzApplication.h"


CAzApplication::CAzApplication(void):CAzBase<IAzApplication>()
{
}
CAzApplication::CAzApplication(IAzApplication *pApp,bool pIsNewApp) :CAzBase<IAzApplication>(pApp,pIsNewApp){   
}


/*++

Routine description:

    This method copies properties from the source application to *this*
    Application

Arguments: pApp - Source Application

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/
HRESULT CAzApplication::Copy(CAzApplication &pApp) {

    CAzLogging::Entering(_TEXT("Copy"));

    if (!m_isNew)

        return E_FAIL;

    CComVariant cVVar;

    HRESULT hr;

    for (long i=0;i<m_uchNumberOfProps;i++) {

        hr=pApp.m_native->GetProperty(m_props[i],CComVariant(), &cVVar);

        CAzLogging::Log(hr,_TEXT("Getting IAzApplication Property ID:"),COLE2T(getName()),m_props[i]);

        if (SUCCEEDED(hr)) {

            hr=m_native->SetProperty(m_props[i],cVVar,CComVariant());

            CAzLogging::Log(hr,_TEXT("Setting IAzApplication Property ID:"),COLE2T(getName()),m_props[i]);

            cVVar.Clear();

        }

    }
    
    if (!CAzGlobalOptions::m_bIgnorePolicyAdmins) {

        hr=pApp.m_native->get_DelegatedPolicyUsers(&cVVar);

        CAzLogging::Log(hr,_TEXT("Getting IAzApplication Delegated Policy Users"),COLE2T(getName()));			

        if (SUCCEEDED(hr)) {

            hr=InitializeUsingSafeArray(cVVar,&IAzApplication::AddDelegatedPolicyUser);

            CAzLogging::Log(hr,_TEXT("Setting IAzApplication Delegated Policy Users"),COLE2T(getName()));

            cVVar.Clear();
        } 

        hr=pApp.m_native->get_PolicyAdministrators(&cVVar);

        CAzLogging::Log(hr,_TEXT("Getting IAzApplication Policy Admins"),COLE2T(getName()));

        if (SUCCEEDED(hr)) {

            hr=InitializeUsingSafeArray(cVVar,&IAzApplication::AddPolicyAdministrator);

            CAzLogging::Log(hr,_TEXT("Setting IAzApplication Policy Admins"),COLE2T(getName()));

            cVVar.Clear();
        }

        hr=pApp.m_native->get_PolicyReaders(&cVVar);

        CAzLogging::Log(hr,_TEXT("Getting IAzApplication Policy Readers"),COLE2T(getName()));

        if (SUCCEEDED(hr)) {

            hr=InitializeUsingSafeArray(cVVar,&IAzApplication::AddPolicyReader);

            CAzLogging::Log(hr,_TEXT("Setting IAzApplication Delegated Policy Readers"),COLE2T(getName()));

            cVVar.Clear();
        } 

    }
    hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting for IAzApplication"),COLE2T(getName()));

    hr=CreateChildItems(pApp);

    CAzLogging::Log(hr,_TEXT("Creating Child Objects for IAzApplication"),COLE2T(getName()));

    CAzLogging::Exiting(_TEXT("Copy"));
    return hr;
}
/*++

Routine description:

    This method creates child objects(like Roles, Scopes etc)
    found in the source application into *this*
    Application

Arguments: pApp - Source Application

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/

HRESULT CAzApplication::CreateChildItems(CAzApplication &pApp){

    /*Below order has to be the same

     First create operations, then app groups, then tasks, then roles

    */

    CAzLogging::Entering(_TEXT("CAzApplication::CreateChildItems"));

    HRESULT hr=CreateOperations(pApp);

    CAzLogging::Log(hr,_TEXT("Creating Operations for IAzApplication"),COLE2T(getName()));			

    hr=CAzHelper<IAzApplication>::CreateAppGroups(pApp.m_native,m_native);

    CAzLogging::Log(hr,_TEXT("Creating Application Groups for IAzApplication"),COLE2T(getName()));	

    hr=CAzHelper<IAzApplication>::CreateTasks(pApp.m_native,m_native);

    CAzLogging::Log(hr,_TEXT("Creating Tasks for IAzApplication"),COLE2T(getName()));

    hr=CAzHelper<IAzApplication>::CreateRoles(pApp.m_native,m_native);

    CAzLogging::Log(hr,_TEXT("Creating Roles for IAzApplication"),COLE2T(getName()));

    hr=CreateScopes(pApp);

    CAzLogging::Log(hr,_TEXT("Creating Scopes for IAzApplication"),COLE2T(getName()));

    hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting child object addition for IAzApplication"),COLE2T(getName()));

    CAzLogging::Exiting(_TEXT("CAzApplication::CreateChildItems"));

    return hr;
}

/*++

Routine description:

    This method creates  the operation objects
    found in the source application into *this*
    Application

Arguments: pApp - Source Application

Return Value:

    Returns success, appropriate failure value 

--*/

HRESULT CAzApplication::CreateOperations(CAzApplication &pApp){

    CAzLogging::Entering(_TEXT("CAzApplication::CreateOperations"));

    CComPtr<IAzOperations> spAzOpes;

    long lCount=0;

    CComVariant cVappl;

    HRESULT hr=pApp.m_native->get_Operations(&spAzOpes);

    CAzLogging::Log(hr,_TEXT("Getting operations for IAzApplication"),COLE2T(getName()));			

    if (FAILED(hr))
        goto lError1;

    hr=spAzOpes->get_Count(&lCount);

    CAzLogging::Log(hr,_TEXT("Getting operation count for IAzApplication from IAzOperations"),COLE2T(getName()));			

    if (FAILED(hr))
        goto lError1;

    if (lCount==0)	
        goto lError1;
    for (long i = 1 ; i <= lCount ; i++) {

        CComPtr<IAzOperation> spOp,spNewOp;

        hr=spAzOpes->get_Item(i,&cVappl);

        CAzLogging::Log(hr,_TEXT("Getting operation object"),COLE2T(getName()));

        if (FAILED(hr))
            goto lError1;

        CComPtr<IDispatch> spDispatchtmp(cVappl.pdispVal);

        cVappl.Clear();

        hr = spDispatchtmp.QueryInterface(&spOp);		

        if (FAILED(hr))
            goto lError1;

        CAzOperation oldOp=CAzOperation(spOp,false);

        hr=m_native->CreateOperation(oldOp.getName(),CComVariant(),&spNewOp);

        CAzLogging::Log(hr,_TEXT("Creating Operation for IAzApplication"),COLE2T(getName()));

        if (FAILED(hr))
            goto lError1;

        CAzOperation newOp=CAzOperation(spNewOp,true);

        hr=newOp.Copy(oldOp);

        CAzLogging::Log(hr,_TEXT("Copying IAzOperation properties for IAzApplication"),COLE2T(getName()));

        if (FAILED(hr))
            goto lError1;
    }
lError1:
    CAzLogging::Exiting(_TEXT("CAzApplication::CreateOperations"));

    return hr;
}

/*++

Routine description:

    This method creates  the scope objects
    found in the source application into *this*
    Application

Arguments: pApp - Source Application

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/

HRESULT CAzApplication::CreateScopes(CAzApplication &pApp){

    CAzLogging::Entering(_TEXT("CAzApplication::CreateScopes"));

    CComPtr<IAzScopes> spAzScopes;

    CComVariant cVappl;

    long lCount=0;

    HRESULT hr=pApp.m_native->get_Scopes(&spAzScopes);

    CAzLogging::Log(hr,_TEXT("Getting scopes for IAzApplication"),COLE2T(getName()));

    if (FAILED(hr))
        goto lError1;

    hr=spAzScopes->get_Count(&lCount);

    CAzLogging::Log(hr,_TEXT("Getting operation count for IAzApplication from IAzOperations"),COLE2T(getName()));

    if (FAILED(hr))
        goto lError1;

    if (lCount==0)	
        goto lError1;
    for (long i = 1 ; i <= lCount ; i++) {

        CComPtr<IAzScope> spSrcScope,spNewScope;

        hr = spAzScopes->get_Item(i,&cVappl);

        CAzLogging::Log(hr,_TEXT("Getting scope object"),COLE2T(getName()));

        if (FAILED(hr))
            goto lError1;

        CComPtr<IDispatch> spDispatchtmp(cVappl.pdispVal);

        cVappl.Clear();

        hr = spDispatchtmp.QueryInterface(&spSrcScope);		

        if (FAILED(hr))
            goto lError1;

        CAzScope srcScope=CAzScope(spSrcScope,false);

        hr=m_native->CreateScope(srcScope.getName(),CComVariant(),&spNewScope);

        CAzLogging::Log(hr,_TEXT("Creating scope for IAzApplication"),COLE2T(getName()));

        if (FAILED(hr))
            goto lError1;

        CAzScope newScope=CAzScope(spNewScope,true);

        hr = newScope.Copy(srcScope);

        CAzLogging::Log(hr,_TEXT("Copying IAzScope properties for IAzApplication"),COLE2T(getName()));

        if (FAILED(hr))
            goto lError1;
    }
lError1:
    CAzLogging::Exiting(_TEXT("CAzApplication::CreateScopes"));

    return hr;
}





CAzApplication::~CAzApplication(void)
{
}
// All the properties which are "Settable" for IAzApplication
const unsigned int CAzApplication::m_props[]={
    AZ_PROP_APPLICATION_DATA,
    AZ_PROP_APPLICATION_AUTHZ_INTERFACE_CLSID,
    AZ_PROP_APPLICATION_VERSION,
    AZ_PROP_APPLY_STORE_SACL,
    AZ_PROP_DESCRIPTION,
    AZ_PROP_GENERATE_AUDITS,
    AZ_PROP_NAME
};
const unsigned char CAzApplication::m_uchNumberOfProps=7;
