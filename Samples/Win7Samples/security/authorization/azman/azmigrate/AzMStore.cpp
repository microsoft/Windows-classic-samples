/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzMStore.cpp

Abstract:

    Routines performing the migration for the AzMStore object


 History:

****************************************************************************/
#include "AzMStore.h"

CAzMStore::CAzMStore(CComBSTR &pStoreName):m_isNew(false)
{
    CAzLogging::Entering(_TEXT("CAzMStore::CAzMStore"));

    m_storeName.Attach(pStoreName.Copy());

    CAzLogging::Exiting(_TEXT("CAzMStore::CAzMStore"));
}

CAzMStore::~CAzMStore(void)
{
    CAzLogging::Entering(_TEXT("CAzMStore::~CAzMStore"));

    for(std::vector<CAzApplication*>::iterator iter1 = m_apps.begin();iter1 != m_apps.end() ;iter1++ ) {

        CAzApplication *ptr=(*iter1);

        delete ptr;
    }

    CAzLogging::Exiting(_TEXT("CAzMStore::~CAzMStore"));
}

/*++

Routine description:

    This method overwrites the destination store

Arguments: None

Return Value:

    Returns success, appropriate failure value of the native interface methods 

--*/
HRESULT CAzMStore::OverWriteStore() {

    CAzLogging::Entering(_TEXT("CAzMStore::OverWriteStore"));    

    HRESULT hr=InitializeStore(false,true);

    CAzLogging::Log(hr,_TEXT("Initializing existing destination store for deletion"),COLE2T(m_storeName));

    if (FAILED(hr))
        goto lError1;

    if (SUCCEEDED(hr)) {

        hr=m_native->Delete(CComVariant());

        CAzLogging::Log(hr,_TEXT("Deleting existing store"),COLE2T(m_storeName));

        if (FAILED(hr))
            goto lError1;

        if (SUCCEEDED(hr)) {

            hr=InitializeStore(true,true);
        }

        CAzLogging::Log(hr,_TEXT("Initializing new destination store"),COLE2T(m_storeName));

    }
lError1:
    CAzLogging::Exiting(_TEXT("CAzMStore::OverWriteStore"));        

    return hr;
}

/*++

Routine description:

    This method initializes the store

Arguments: pbcreateStore - Flag to create Store, or manage store
           pbOverWritten - Flag if current store initialize called from OverWriteStore()

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/
HRESULT CAzMStore::InitializeStore(bool pbcreateStore,bool pbOverWritten){

    CAzLogging::Entering(_TEXT("CAzMStore::InitializeStore"));

    HRESULT hr;

    CComVariant cvVar;

    CComPtr<IAzAuthorizationStore3> newStore;

    long lnewVersionFLag=0;

    //Donot create another instance if pbOverWritten==true as instance
    //already created 

    if (!pbOverWritten) {

        hr=m_native.CoCreateInstance(__uuidof(AzAuthorizationStore));

        CAzLogging::Log(hr,_TEXT("Creating instance of AzMan interface"),COLE2T(m_storeName));

        if (FAILED(hr))
            goto lError1;

    }

    hr=m_native.QueryInterface(&newStore);

    if (SUCCEEDED(hr)) {
        CAzGlobalOptions::m_bVersionTwo = true;
    }

    long flags=0;

    if (pbcreateStore==true) {

       flags = AZ_AZSTORE_FLAG_CREATE;

       if (CAzGlobalOptions::m_bVersionTwo) 
           flags |= AZ_AZSTORE_NT6_FUNCTION_LEVEL;

    } else
      flags = AZ_AZSTORE_FLAG_MANAGE_STORE_ONLY;


//    long flags=((pbcreateStore==true) ? AZ_AZSTORE_FLAG_CREATE : AZ_AZSTORE_FLAG_MANAGE_STORE_ONLY);

    hr = m_native->Initialize(flags, m_storeName, cvVar);

    if (!(CAzGlobalOptions::m_bOverWrite && pbOverWritten == false))

        CAzLogging::Log(hr,_TEXT("Initialize AzMan Authorization Store"),COLE2T(m_storeName));

    m_isNew=pbcreateStore;

    if (SUCCEEDED(hr) && pbcreateStore) {

        m_native->Submit(0,CComVariant());

        CAzLogging::Log(hr,_TEXT("Submitting changes on AzMan Authornization Store"),COLE2T(m_storeName));			
    } 

lError1:

    CAzLogging::Exiting(_TEXT("CAzMStore::InitializeStore"));

    return hr;

}



HRESULT CAzMStore::OpenApps(vector<CComBSTR> &pAppNames) {

    CAzLogging::Entering(_TEXT("CAzMStore::OpenApp"));

    CComPtr<IAzApplication> spApp;

    HRESULT hr=S_OK;

    for (vector<CComBSTR>::iterator itr=pAppNames.begin() ; itr !=pAppNames.end() ; itr++) {

        hr=m_native->OpenApplication(*itr,CComVariant(),&spApp);

        CAzLogging::Log(hr,_TEXT("Opening Application"),COLE2T(*itr));

        if (SUCCEEDED(hr)) {

            CAzApplication *app=new CAzApplication(spApp,false);

            if (!app) {
                hr = E_FAIL;

                CAzLogging::Log(hr,_TEXT("Creating Application"),COLE2T(m_storeName));

                goto lError1;

            }

            m_apps.push_back(app);
        }
       

    }

lError1:

    CAzLogging::Exiting(_TEXT("CAzMStore::OpenApp"));

    return hr;

}



/*++

Routine description:

    This method Opens the specified application and adds it to the internal vector 
    of applications

Arguments: pAppName - Name of the app that needs to be opened

Return Value:

    Returns success, appropriate failure value 

--*/


HRESULT CAzMStore::OpenApp(CComBSTR &pAppName) {

    CAzLogging::Entering(_TEXT("CAzMStore::OpenApp"));

    CComPtr<IAzApplication> spApp;

    if (m_isNew)
        return E_FAIL;

    HRESULT hr=m_native->OpenApplication(pAppName,CComVariant(),&spApp);

    CAzLogging::Log(hr,_TEXT("Opening Application"),COLE2T(pAppName));

    if (SUCCEEDED(hr)) {

        CAzApplication *app=new CAzApplication(spApp,false);

        if (!app) {
            hr = E_FAIL;

            CAzLogging::Log(hr,_TEXT("Creating Application"),COLE2T(m_storeName));

            goto lError1;
        }

        m_apps.push_back(app);

    }

lError1:

    CAzLogging::Exiting(_TEXT("CAzMStore::OpenApp"));

    return hr;
}

/*++

Routine description:

    This method Opens all applications in the specified store

Arguments: NONE

Return Value:

    Returns success, appropriate failure value 

--*/


HRESULT CAzMStore::OpenAllApps() {

    CAzLogging::Entering(_TEXT("CAzMStore::OpenAllApps"));

    if (m_isNew)
        return E_FAIL;

    CComPtr<IAzApplications> spAzApplications;

    long lCount=0;

    CAzApplication *app;

    CComVariant cVappl;	

    HRESULT hr=m_native->get_Applications(&spAzApplications);

    CAzLogging::Log(hr,_TEXT("Getting all applications"),COLE2T(m_storeName));

    if (FAILED(hr))
        goto lError1;

    hr=spAzApplications->get_Count(&lCount);

    CAzLogging::Log(hr,_TEXT("Getting application count"),COLE2T(m_storeName));	

    if (FAILED(hr))
        goto lError1;

    if (lCount==0)	
        goto lError1;

    for (long i = 1 ; i <= lCount ; i++) {

        CComPtr<IAzApplication> spApp;

        hr=spAzApplications->get_Item(i,&cVappl);

        CAzLogging::Log(hr,_TEXT("Getting application item"),COLE2T(m_storeName));	

        if (FAILED(hr))
            goto lError1;

        CComPtr<IDispatch> spDispatchtmp(cVappl.pdispVal);

        cVappl.Clear();

        hr = spDispatchtmp.QueryInterface(&spApp);

        if (FAILED(hr))
            goto lError1;

        app=new CAzApplication(spApp);

       if (!app) {

            hr=E_FAIL;

            CAzLogging::Log(hr,_TEXT("Creating Application"),COLE2T(m_storeName));

            goto lError1;
        }

        m_apps.push_back(app);

    }

lError1:

    CAzLogging::Exiting(_TEXT("CAzMStore::OpenAllApps"));

    return hr;
}

/*++

Routine description:

    This method copies all store properties from source store to *this* store

Arguments: sourceStore - Name of the source store 

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/


HRESULT CAzMStore::CopyStoreProperties(CAzMStore &sourceStore) {

    CAzLogging::Entering(_TEXT("CopyStoreProperties"));

    HRESULT hr=S_OK;

    CComVariant cVVar;

    for (long i=0;i<m_uchNumberOfProps;i++) {

        hr=sourceStore.m_native->GetProperty(m_props[i],CComVariant(), &cVVar);

        CAzLogging::Log(hr,_TEXT("Getting IAzAuthorizationStore Property ID:"),COLE2T(sourceStore.m_storeName),m_props[i]);

        if (SUCCEEDED(hr)) {

            hr=m_native->SetProperty(m_props[i],cVVar,CComVariant());

            CAzLogging::Log(hr,_TEXT("Setting IAzAuthorizationStore Property ID:"),COLE2T(m_storeName),m_props[i]);

            cVVar.Clear();

        }

    }

    if (!CAzGlobalOptions::m_bIgnorePolicyAdmins) {

        hr=sourceStore.m_native->get_DelegatedPolicyUsers(&cVVar);

        CAzLogging::Log(hr,_TEXT("Getting IAzAuthorizationStore Delegated Policy Users"),COLE2T(m_storeName));

        if (SUCCEEDED(hr)) {

            hr=InitializeUsingSafeArray(cVVar,&IAzAuthorizationStore::AddDelegatedPolicyUser);

            CAzLogging::Log(hr,_TEXT("Setting IAzAuthorizationStore Delegated Policy Users"),COLE2T(m_storeName));

            cVVar.Clear();

        } 

        hr=sourceStore.m_native->get_PolicyAdministrators(&cVVar);

        CAzLogging::Log(hr,_TEXT("Getting IAzAuthorizationStore Policy Admins"),COLE2T(m_storeName));

        if (SUCCEEDED(hr)) {

            hr=InitializeUsingSafeArray(cVVar,&IAzAuthorizationStore::AddPolicyAdministrator);

            CAzLogging::Log(hr,_TEXT("Setting IAzAuthorizationStore Policy Admins"),COLE2T(m_storeName));

            cVVar.Clear();
        }

        hr=sourceStore.m_native->get_PolicyReaders(&cVVar);

        CAzLogging::Log(hr,_TEXT("Getting IAzAuthorizationStore Policy Readers"),COLE2T(m_storeName));

        if (SUCCEEDED(hr)) {

            hr=InitializeUsingSafeArray(cVVar,&IAzAuthorizationStore::AddPolicyReader);

            CAzLogging::Log(hr,_TEXT("Setting IAzAuthorizationStore Delegated Policy Readers"),COLE2T(m_storeName));

            cVVar.Clear();

            } 
        
    hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting for IAzAuthorizationStore"),COLE2T(m_storeName));

    }

    CAzLogging::Exiting(_TEXT("CopyStoreProperties"));

    return hr;

}


/*++

Routine description:

    This method copies all app groups from source store to *this* store
    It also copies all applications in the vector in the source store
    to *this* store

Arguments: sourceStore - Name of the source store 

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/



HRESULT CAzMStore::Copy(CAzMStore& sourceStore){

    CAzLogging::Entering(_TEXT("CAzMStore::Copy"));

    CAzApplication *newApp;

    std::vector<CAzApplication*> sourceApps;

    HRESULT hr;

    if (!m_isNew)
        return E_FAIL;

    hr = CopyStoreProperties(sourceStore);

    CAzLogging::Log(hr,_TEXT("Copying Store Properties "),COLE2T(m_storeName));

    hr=CreateAppGroups(sourceStore,false);

    CAzLogging::Log(hr,_TEXT("Creating AppGroups under Store"),COLE2T(m_storeName));

    if (FAILED(hr))
        goto lError1;

    hr=CreateAppGroups(sourceStore,true);

    CAzLogging::Log(hr,_TEXT("Creating AppGroup Links under Store"),COLE2T(m_storeName));	

    if (FAILED(hr))
        goto lError1;

    sourceApps=sourceStore.getApps();

    for(std::vector<CAzApplication*>::iterator iter1 = sourceApps.begin();iter1 != sourceApps.end() ;iter1++ ) {

        CComPtr<IAzApplication> app;

        hr=m_native->CreateApplication((*iter1)->getName(),CComVariant(),&app);

        CAzLogging::Log(hr,_TEXT("Creating Application under Store"),COLE2T(m_storeName));	

        newApp=new CAzApplication(app,true);

        if (!newApp) {

            hr=E_FAIL;

            CAzLogging::Log(hr,_TEXT("Creating Application"),COLE2T(m_storeName));

            goto lError1;
        }

        hr=newApp->Copy(*(*iter1));

        if (FAILED(hr))
            goto lError1;

        if (SUCCEEDED(hr)) {

            m_apps.push_back(newApp);

            m_native->CloseApplication((*iter1)->getName(),0);

        }
 }
    goto lDone;

lError1:
    CAzLogging::MIGRATE_SUCCESS=false;

lDone:

    CAzLogging::Exiting(_TEXT("CAzMStore::Copy"));

    return hr;
    
}

const std::vector<CAzApplication *>& CAzMStore::getApps() const {
    return m_apps;
}


/*++

Routine description:

    This method creates all app groups from source store into *this* store

Arguments: sourceStore - Name of the source store 
           bCreateLinks - If links between app groups need to be created or not

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/


HRESULT CAzMStore::CreateAppGroups(CAzMStore& sourceStore,bool bCreateLinks){

    CAzLogging::Entering(_TEXT("CAzMStore::CreateAppGroups"));

    CComPtr<IAzApplicationGroups> spAzAppGroups;

    long lCount=0;

    CComVariant cVappl;	

    HRESULT hr=sourceStore.m_native->get_ApplicationGroups(&spAzAppGroups);

    CAzLogging::Log(hr,_TEXT("Getting ApplicationGroups for Store"),COLE2T(m_storeName));	

    if (FAILED(hr))
        goto lError1;	

    hr=spAzAppGroups->get_Count(&lCount);

    CAzLogging::Log(hr,_TEXT("Getting ApplicationGroups Count"),COLE2T(m_storeName));	

    if (FAILED(hr))
        goto lError1;

    if (lCount==0)	
        goto lError1;

    for (long i = 1 ; i <= lCount ; i++) {

        CComPtr<IAzApplicationGroup> spSrcAppGroup,spNewAppGroup;

        hr=spAzAppGroups->get_Item(i,&cVappl);

        CAzLogging::Log(hr,_TEXT("Getting ApplicationGroup Item"),COLE2T(m_storeName));	

        if (FAILED(hr))
            goto lError1;

        CComPtr<IDispatch> spDispatchtmp(cVappl.pdispVal);

        cVappl.Clear();

        hr = spDispatchtmp.QueryInterface(&spSrcAppGroup);

        if (FAILED(hr))
            goto lError1;

        CAzAppGroup oldAppGroup = CAzAppGroup(spSrcAppGroup,false);

        if (!bCreateLinks) {

            hr=m_native->CreateApplicationGroup(oldAppGroup.getName(),CComVariant(),&spNewAppGroup);

            CAzLogging::Log(hr,_TEXT("Creating new application group for store"),COLE2T(m_storeName));	

            if (FAILED(hr))
                goto lError1;

        } else {

                hr=m_native->OpenApplicationGroup(oldAppGroup.getName(),CComVariant(),&spNewAppGroup);

                CAzLogging::Log(hr,_TEXT("Opening New App Group Object to create links"));			

                if (FAILED(hr))
                    goto lError1;
        }

        CAzAppGroup newAppGroup = CAzAppGroup(spNewAppGroup,!bCreateLinks);

        hr=bCreateLinks ? newAppGroup.CopyLinks(oldAppGroup) : newAppGroup.Copy(oldAppGroup);

        CAzLogging::Log(hr,_TEXT("Setting Application group properties"),COLE2T(m_storeName));	

        if (FAILED(hr))
            goto lError1;
    }

lError1:
    return hr;

    CAzLogging::Exiting(_TEXT("CAzMStore::CreateAppGroups"));
}


/*++

Routine description:

    This method is a utility method which takes a VARIANT
    which contains a SAFEARRAY of BSTR which are then set
    into *this* object`s Authorization Stores Native interface
    by calling the target method

Arguments: cVVar - Variant which contains the SAFEARRAY
           targetMethod - TargetMethod which is invoked for Setting
                          each element contained in the SAFEARRAY

Return Value:

    Returns success, appropriate failure value of the procedures done within 
    this method

--*/

HRESULT CAzMStore::InitializeUsingSafeArray(VARIANT &cVVar, 
                                                     HRESULT (__stdcall IAzAuthorizationStore::*targetMethod)(BSTR, VARIANT)) {
    CAzLogging::Entering(_TEXT("InitializeUsingSafeArray"));

    SAFEARRAY *sa = V_ARRAY( &cVVar ); 

    LONG lstart, lend;

    LONG idx = -1;

    HRESULT hr;


    // Get the lower and upper bound
    hr = SafeArrayGetLBound( sa, 1, &lstart );

    if(FAILED(hr)) 
        goto lEnd;

    hr = SafeArrayGetUBound( sa, 1, &lend );

    if(FAILED(hr)) 
        goto lEnd;

    if (0==(lend-lstart+1))
        goto lEnd;

    CComVariant *rgcVVar;

    hr = SafeArrayAccessData(sa,(void HUGEP* FAR*)&rgcVVar);

    if(SUCCEEDED(hr))
    {
        for(idx = lstart ; idx <= lend ; idx++)
        {
            CComBSTR s(rgcVVar[idx].bstrVal);

            hr=(m_native->*targetMethod)(s,CComVariant());

            CAzLogging::Log(hr,_TEXT("Calling Set Method in InitializeUsingSafeArray"),COLE2T(m_storeName));

        }

        if(FAILED(hr)) 
           goto lError1;
    }   
    else
        goto lEnd;

lError1:
    hr = SafeArrayUnaccessData(sa);
    
lEnd:
    CAzLogging::Exiting(_TEXT("InitializeUsingSafeArray"));

    return hr;
}


// All the properties which are "Settable" for IAzApplication
const unsigned int CAzMStore::m_props[]={
    AZ_PROP_AZSTORE_DOMAIN_TIMEOUT,
    AZ_PROP_AZSTORE_MAX_SCRIPT_ENGINES,
    AZ_PROP_AZSTORE_SCRIPT_ENGINE_TIMEOUT,
    AZ_PROP_APPLICATION_DATA,
    AZ_PROP_APPLY_STORE_SACL,
    AZ_PROP_DESCRIPTION,
    AZ_PROP_GENERATE_AUDITS
};
const unsigned char CAzMStore::m_uchNumberOfProps=7;
