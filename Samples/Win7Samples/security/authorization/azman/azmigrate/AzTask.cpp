/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzTask.cpp

Abstract:

    Routines performing the migration for the CAzTask object

 History:

****************************************************************************/


#include "AzTask.h"

CAzTask::CAzTask(void):CAzBase<IAzTask>()
{
}

CAzTask::~CAzTask(void)
{
}
CAzTask::CAzTask(IAzTask *pNative,bool pisNew):CAzBase<IAzTask>(pNative,pisNew){
}

/*++

Routine description:

    This method copies links from the source task to *this* 
    task

Arguments: srcTask - Source Task

Return Value:

    Returns success, appropriate failure value 

--*/

HRESULT CAzTask::CopyLinks(CAzTask &srcTask) {

    CAzLogging::Entering(_TEXT("CopyLinks"));

    CComVariant cVVar;

    HRESULT hr;

    hr=srcTask.m_native->get_Tasks(&cVVar);

    CAzLogging::Log(hr,_TEXT("Getting Tasks for Task"),COLE2T(getName()));

    if (SUCCEEDED(hr)) {
        hr=InitializeUsingSafeArray(cVVar,&IAzTask::AddTask);

        CAzLogging::Log(hr,_TEXT("Setting Task links for role"),COLE2T(getName()));

    }

    hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting Task links"),COLE2T(getName()));

    CAzLogging::Exiting(_TEXT("CopyLinks"));

    return hr;
}

/*++

Routine description:

    This method copies properties from the source task to *this* 
    task

Arguments: srcTask - Source Task

Return Value:

    Returns success, appropriate failure value 

--*/

HRESULT CAzTask::Copy(CAzTask &srcTask) {

    CAzLogging::Exiting(_TEXT("Copy"));

    CComVariant cVVar;

    HRESULT hr;

    CComBSTR bstrData;

    for (long i=0 ; i < m_numberOfProps ; i++) {

        hr=srcTask.m_native->GetProperty(m_props[i],CComVariant(), &cVVar);

        CAzLogging::Log(hr,_TEXT("Getting IAzTask Property ID:"),COLE2T(srcTask.getName()),m_props[i]);			

        if (SUCCEEDED(hr)) {

            hr=m_native->SetProperty(m_props[i],cVVar,CComVariant());

            CAzLogging::Log(hr,_TEXT("Setting IAzTask Property ID:"),COLE2T(getName()),m_props[i]);

            cVVar.Clear();
        }
    }	

    hr=srcTask.m_native->get_BizRuleImportedPath(&bstrData);

    CAzLogging::Log(hr,_TEXT("Getting BizRuleImportedPath for Task"),COLE2T(getName()));

    if (SUCCEEDED(hr)) {

        hr=m_native->put_BizRuleImportedPath(bstrData);

        CAzLogging::Log(hr,_TEXT("Setting BizRuleImportedPath for App Group"),COLE2T(getName()));

        bstrData.Empty();
    }

    hr=srcTask.m_native->get_Operations(&cVVar);

    CAzLogging::Log(hr,_TEXT("Getting Operations for Task"),COLE2T(getName()));

    if (FAILED(hr))
        goto lError1;

    hr=InitializeUsingSafeArray(cVVar,&IAzTask::AddOperation);

    CAzLogging::Log(hr,_TEXT("Setting Operations for Task"),COLE2T(getName()));

    if (FAILED(hr))
        goto lError1;

    cVVar.Clear();

    if (CAzGlobalOptions::m_bVersionTwo) {

            hr = CopyVersion2Constructs(srcTask);

            CAzLogging::Log(hr,_TEXT("Copying bizrule properties for Task"),COLE2T(srcTask.getName()));
    }

    hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting Task "),COLE2T(getName()));

lError1:
    CAzLogging::Exiting(_TEXT("Copy"));

    return hr;
}



/*++

Routine description:

    This method copies properties from the source task to *this* task
    which are specific to version 1.2

Arguments: Source Task

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/

HRESULT CAzTask::CopyVersion2Constructs(CAzTask &srcTask) {

    static unsigned int rgProperties[]={AZ_PROP_TASK_BIZRULE,AZ_PROP_TASK_BIZRULE_LANGUAGE,AZ_PROP_TASK_BIZRULE_IMPORTED_PATH};

    CComVariant cVVar;

    HRESULT hr=S_OK;

    if (!CAzGlobalOptions::m_bVersionTwo)
        goto lDone;

    for (long i=0;i<3;i++) {

        hr=srcTask.m_native->GetProperty(rgProperties[i],CComVariant(), &cVVar);

        CAzLogging::Log(hr,_TEXT("Getting IAzTask Property ID:"),COLE2T(srcTask.getName()),rgProperties[i]);

        if (SUCCEEDED(hr) && (SysStringByteLen(cVVar.bstrVal))!=0) {

            hr=m_native->SetProperty(rgProperties[i],cVVar,CComVariant());

            CAzLogging::Log(hr,_TEXT("Setting IAzTask Property ID:"),COLE2T(getName()),rgProperties[i]);

            cVVar.Clear();

        }

    }
lDone:
    return hr;
}


const unsigned char CAzTask::m_numberOfProps=5;

// All the properties which are "Settable" for IAzTask
const unsigned int CAzTask::m_props[]={
        AZ_PROP_APPLICATION_DATA,        
        AZ_PROP_TASK_BIZRULE_LANGUAGE,
        AZ_PROP_TASK_BIZRULE,
        AZ_PROP_DESCRIPTION,
        AZ_PROP_TASK_IS_ROLE_DEFINITION
    };