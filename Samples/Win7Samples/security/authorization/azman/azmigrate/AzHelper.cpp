/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzHelper.cpp

Abstract:

    Templatized helper routines used to aggregate common methods
    to createTasks, createRoles, createAppGroups used commonly for 
    both CAzScopes and CAzApplications

 History:

 ***************************************************************************/
#include "AzHelper.h"


template <class NativeInterfaceType>
CAzHelper<NativeInterfaceType>::CAzHelper(void)
{
}

template <class NativeInterfaceType>
CAzHelper<NativeInterfaceType>::~CAzHelper(void)
{
}

/*++

Routine description:

    This method copies tasks from the source native AzMan interface to 
    the destination native AzMan interface

Arguments: pNativeSource - Source native interface
           pNativeNew - destination native interface

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

Description:
    2 Pass method used. In the first pass, create all the task objects;
    In the second pass, create the links between the task objects.

--*/
template <class NativeInterfaceType>
HRESULT CAzHelper<NativeInterfaceType>::CreateTasks(CComPtr<NativeInterfaceType> &pNativeSource,
                                                    CComPtr<NativeInterfaceType> &pNativeNew) {
// Copy all tasks objects; donot create any links between then
    HRESULT hr=CreateTasks(pNativeSource,pNativeNew,false);

//Create links between the tasks now.
    if (SUCCEEDED(hr))
        hr=CreateTasks(pNativeSource,pNativeNew,true);

    return hr;
}

/*++

Routine description:

    This method copies appgroups from the source native AzMan interface to 
    the destination native AzMan interface

Arguments: pNativeSource - Source native interface
           pNativeNew    - Destination native interface

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

Description:
    2 Pass method used. In the first pass, create all the appgroup objects;
    In the second pass, create the links between the appgroup objects.

--*/

template <class NativeInterfaceType>
HRESULT CAzHelper<NativeInterfaceType>::CreateAppGroups(CComPtr<NativeInterfaceType> &pNativeSource,
                                                        CComPtr<NativeInterfaceType> &pNativeNew) {

    HRESULT hr=CreateAppGroups(pNativeSource,pNativeNew,false);

    if (SUCCEEDED(hr))
        hr=CreateAppGroups(pNativeSource,pNativeNew,true);

    return hr;
}

/*++

Routine description:

    This method copies tasks from the source native AzMan interface to 
    the destination native AzMan interface.
    If createlinks == true, then donot create tasks, only create links
    else create the new task objects

Arguments: pNativeSource - Source native interface, 
           pNativeNew -  destination native interface, 
           bCreateLinks - createLinks

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method
--*/

template <class NativeInterfaceType>
HRESULT CAzHelper<NativeInterfaceType>::CreateTasks(CComPtr<NativeInterfaceType> &pNativeSource,
                                                    CComPtr<NativeInterfaceType> &pNativeNew,
                                                    bool bCreateLinks) {

    CAzLogging::Entering(_TEXT("CreateTasks"));

    CComPtr<IAzTasks> spAzTasks;

    long lCount=0;

    CComVariant cVappl;	

    HRESULT hr=pNativeSource->get_Tasks(&spAzTasks);

    CAzLogging::Log(hr,_TEXT("Getting Tasks"));

    if (FAILED(hr))
        goto lError1;

    hr=spAzTasks->get_Count(&lCount);

    CAzLogging::Log(hr,_TEXT("Getting Task Object count"));

    if (FAILED(hr))
        goto lError1;

    if (lCount==0)
        goto lError1;

    for (long i = 1 ; i <= lCount ; i++) {

        CComPtr<IAzTask> spOldTask,spNewTask;

        hr=spAzTasks->get_Item(i,&cVappl);

        CAzLogging::Log(hr,_TEXT("Getting Task Item"));

        if (FAILED(hr))
            goto lError1;

        CComPtr<IDispatch> spDispatchtmp(cVappl.pdispVal);

        cVappl.Clear();

        hr = spDispatchtmp.QueryInterface(&spOldTask);

        if (FAILED(hr))
            goto lError1;

        CAzTask oldTask=CAzTask(spOldTask,false);

        if (!bCreateLinks) {

            hr=pNativeNew->CreateTask(oldTask.getName(),CComVariant(),&spNewTask);

            CAzLogging::Log(hr,_TEXT("Creating New Task Object"));			

            if (FAILED(hr))
                goto lError1;

        } else {

                hr=pNativeNew->OpenTask(oldTask.getName(),CComVariant(),&spNewTask);

                CAzLogging::Log(hr,_TEXT("Opening New Task Object to create links"));			

                if (FAILED(hr))
                    goto lError1;        

        }

        CAzTask newTask=CAzTask(spNewTask,!bCreateLinks);

        hr=bCreateLinks ? newTask.CopyLinks(oldTask) : newTask.Copy(oldTask);

        CAzLogging::Log(hr,_TEXT("Copying Task properties"));

        if (FAILED(hr))
            goto lError1;

    }
lError1:
    CAzLogging::Exiting(_TEXT("CreateTasks"));

    return hr;
}

/*++

Routine description:

    This method copies roles from the source native AzMan interface to 
    the destination native AzMan interface

Arguments: pNativeSource - Source native interface, 
           pNativeNew -  destination native interface, 

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method

--*/

template <class NativeInterfaceType>
HRESULT CAzHelper<NativeInterfaceType>::CreateRoles(CComPtr<NativeInterfaceType> &pNativeSource,
                                                    CComPtr<NativeInterfaceType> &pNativeNew) {
    CAzLogging::Entering(_TEXT("CreateRoles"));

    CComPtr<IAzRoles> spAzRoles;

    long lCount=0;

    CComVariant cVappl;

    HRESULT hr=pNativeSource->get_Roles(&spAzRoles);

    CAzLogging::Log(hr,_TEXT("Getting Roles"));

    if (FAILED(hr))
        goto lError1;

    hr=spAzRoles->get_Count(&lCount);

    CAzLogging::Log(hr,_TEXT("Getting Role Object count"));

    if (FAILED(hr))
        goto lError1;

    if (lCount==0)
        goto lError1;

    for (long i = 1;i <= lCount ; i++) {

        CComPtr<IAzRole> spRole,spNewRole;

        hr=spAzRoles->get_Item(i,&cVappl);

        CAzLogging::Log(hr,_TEXT("Getting Role Item"));

        if (FAILED(hr))
            goto lError1;

        CComPtr<IDispatch> spDispatchtmp(cVappl.pdispVal);

        cVappl.Clear();

        hr = spDispatchtmp.QueryInterface(&spRole);

        if (FAILED(hr))
            goto lError1;

        CAzRole role=CAzRole(spRole,false);

        hr=pNativeNew->CreateRole(role.getName(),CComVariant(),&spNewRole);

        CAzLogging::Log(hr,_TEXT("Creating New Role Object"));

        if (FAILED(hr))
            goto lError1;

        CAzRole newRole=CAzRole(spNewRole,true);

        hr=newRole.Copy(role);

        CAzLogging::Log(hr,_TEXT("Copying Role properties"));

        if (FAILED(hr))
            goto lError1;
    }
lError1:
        CAzLogging::Exiting(_TEXT("CreateRoles"));

        return hr;
}

/*++

Routine description:

    This method copies appgroups from the source native AzMan interface to 
    the destination native AzMan interface.
    If createlinks == true, then donot create appgroups, only create links
    else create the new appgroup objects

Arguments: pNativeSource - Source native interface, 
           pNativeNew -  destination native interface, 
           bCreateLinks - createLinks

Return Value:

    Returns success, appropriate failure value of the get/set methods done within 
    this method
--*/


template <class NativeInterfaceType>
HRESULT CAzHelper<NativeInterfaceType>::CreateAppGroups(CComPtr<NativeInterfaceType> &pNativeSource,
                                                        CComPtr<NativeInterfaceType> &pNativeNew,
                                                        bool bCreateLinks) {

    CAzLogging::Entering(_TEXT("CreateAppGroups"));

    CComPtr<IAzApplicationGroups> spAzAppGroups;

    long lCount=0;

    CComVariant cVappl;	

    HRESULT hr=pNativeSource->get_ApplicationGroups(&spAzAppGroups);

    CAzLogging::Log(hr,_TEXT("Getting App Groups"));

    if (FAILED(hr))
        goto lError1;

    hr=spAzAppGroups->get_Count(&lCount);

    CAzLogging::Log(hr,_TEXT("Getting App Group Object count"));

    if (FAILED(hr))
        goto lError1;

    if (lCount==0)
        goto lError1;

    for (long i = 1 ; i <= lCount ; i++) {

        CComPtr<IAzApplicationGroup> spSrcAppGroup,spNewAppGroup;

        hr=spAzAppGroups->get_Item(i,&cVappl);

        CAzLogging::Log(hr,_TEXT("Getting App Group Item"));

        if (FAILED(hr))
            goto lError1;

        CComPtr<IDispatch> spDispatchtmp(cVappl.pdispVal);

        cVappl.Clear();

        hr = spDispatchtmp.QueryInterface(&spSrcAppGroup);

        CAzLogging::Log(hr,_TEXT("Querying AppGroup interface"));

        if (FAILED(hr))
            goto lError1;

        CAzAppGroup oldAppGroup=CAzAppGroup(spSrcAppGroup,false);

        if (!bCreateLinks) {

            hr=pNativeNew->CreateApplicationGroup(oldAppGroup.getName(),CComVariant(),&spNewAppGroup);

            CAzLogging::Log(hr,_TEXT("Creating New App Group Object"));

            if (FAILED(hr))
                goto lError1;

        } else {

                hr=pNativeNew->OpenApplicationGroup(oldAppGroup.getName(),CComVariant(),&spNewAppGroup);

                CAzLogging::Log(hr,_TEXT("Opening New App Group Object to create links"));

                if (FAILED(hr))
                    goto lError1;

        }

        CAzAppGroup newAppGroup=CAzAppGroup(spNewAppGroup,!bCreateLinks);

        hr=bCreateLinks ? newAppGroup.CopyLinks(oldAppGroup) : newAppGroup.Copy(oldAppGroup);

        CAzLogging::Log(hr,_TEXT("Copying App Group properties"));

        if (FAILED(hr))
            goto lError1;

    }
lError1:
    CAzLogging::Exiting(_TEXT("CreateAppGroups"));

    return hr;

}


template class CAzHelper<IAzScope>;

template class CAzHelper<IAzApplication>;


