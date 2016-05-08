/* **************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzHelper.h

Abstract:

    Declaration of the CAzHelper class

 History:

 ***************************************************************************/
#pragma once
#include "stdafx.h"
#include "azapplication.h"
#include "azoperation.h"
#include "aztask.h"
#include "azrole.h"
#include "azAppGroup.h"

template <class NativeInterfaceType>
class CAzHelper
{
public:
    CAzHelper(void);

    ~CAzHelper(void);

     static HRESULT CreateTasks(CComPtr<NativeInterfaceType> &,
            CComPtr<NativeInterfaceType> &);

     static HRESULT CreateAppGroups(CComPtr<NativeInterfaceType> &,
            CComPtr<NativeInterfaceType> &);

     static HRESULT CreateRoles(CComPtr<NativeInterfaceType> &,
            CComPtr<NativeInterfaceType> &);

private:
    static HRESULT CreateTasks(CComPtr<NativeInterfaceType> &,
            CComPtr<NativeInterfaceType> &,bool);

    static HRESULT CreateAppGroups(CComPtr<NativeInterfaceType> &,
            CComPtr<NativeInterfaceType> &,bool);

};


template <typename IAzNewNative, typename CAzNewClass,
        typename IAzParentNative,typename IAzCollection>
HRESULT __cdecl CreateChildren(IAzParentNative *psourceNative,IAzParentNative *pdestNative,
                      HRESULT (__stdcall IAzParentNative:: *getCollectionMethod)(IAzCollection **),
                      HRESULT (__stdcall IAzParentNative:: *createMethod)(BSTR, VARIANT,IAzNewNative **) 
                       ){
    CComPtr<IAzCollection> spAzColl;
    long lCount=0;
    HRESULT hr=(psourceNative->*getCollectionMethod)(&spAzColl);
    //CHECK_HR("Getting Operations for Application");
    CComVariant cVappl;	
    hr=spAzColl->get_Count(&lCount);
    CHECK_HR("Get Operations Count")
    if (lCount==0)	
        return S_OK;
    for (long i=1;i<=lCount;i++) {
        CComPtr<IAzNewNative> spOp,spNewOp;
        hr=spAzColl->get_Item(i,&cVappl);
        CHECK_HR("Get Operation Item");
        CComPtr<IDispatch> spDispatchtmp(cVappl.pdispVal);
        cVappl.Clear();
        hr = spDispatchtmp.QueryInterface(&spOp);		
        CHECK_HR("Getting Operation Interface");
        CAzNewClass oldOp=CAzNewClass(spOp,false);
        hr=(pdestNative->*createMethod)(oldOp.getName(),CComVariant(),&spNewOp);
        CHECK_HR("Creating new Operation in Destination Store");
        CAzNewClass newOp=CAzNewClass(spNewOp,true);
        hr=newOp.Copy(oldOp);
        CHECK_HR("Copy Operation Properties");	
    }

    return S_OK;
}
