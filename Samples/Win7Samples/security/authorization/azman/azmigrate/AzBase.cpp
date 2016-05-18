/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzBase.cpp

Abstract:

    Routines containing some common methods for all the other classes


 History:

****************************************************************************/


#include "AzBase.h"

template <class  IAzNative>
CAzBase<IAzNative>::CAzBase(void)
{
}
template <class  IAzNative>
/*++

Routine description:

    Constructors

--*/
CAzBase<IAzNative>::~CAzBase(void)
{
}

template <class  IAzNative>
CAzBase<IAzNative>::CAzBase(IAzNative *pNative,bool bIsNew){

    CAzLogging::Entering(_TEXT("Constructor"));

    pNative->AddRef();

    m_native.Attach(pNative);

    pNative->get_Name(&m_name);

    m_isNew=bIsNew;

    CAzLogging::Exiting(_TEXT("Constructor"));
}
/*++

Routine description:

    This method is a common method which takes a VARIANT
    which contains a SAFEARRAY of BSTR which are then set
    into *this* object`s Authorization Manager Native interface
    by calling the target method

Arguments: cVVar - Variant which contains the SAFEARRAY
           targetMethod - TargetMethod which is invoked for Setting
                          each element contained in the SAFEARRAY

Return Value:

    Returns success, appropriate failure value of the procedures done within 
    this method

--*/
template <class  IAzNative>
HRESULT CAzBase<IAzNative>::InitializeUsingSafeArray(VARIANT &cVVar, 
                                                     HRESULT (__stdcall IAzNative::*targetMethod)(BSTR, VARIANT)) {
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

            CAzLogging::Log(hr,_TEXT("Calling Set Method in InitializeUsingSafeArray"),COLE2T(getName()));			

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


// Adding explicit template instantiation to prevent C++ linkage errors
// which are caused by seperate compilation units.
template class CAzBase<IAzRole>;

template class CAzBase<IAzApplication>;

template class CAzBase<IAzOperation>;

template class CAzBase<IAzTask>;

template class CAzBase<IAzScope>;

template class CAzBase<IAzApplicationGroup>;