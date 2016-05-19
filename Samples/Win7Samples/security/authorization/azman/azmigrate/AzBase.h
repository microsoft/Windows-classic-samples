/* **************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzBase.h

Abstract:

    Declaration of the CAzBase class

 History:

 ***************************************************************************/
#pragma once

#include "stdafx.h"

template <class IAzNative> 
class CAzBase
{
public:
    CAzBase(void);

    virtual ~CAzBase(void)=0;

    CAzBase(IAzNative *pNative,bool pisNew);

    const CComBSTR& getName() const {

        return m_name;

}
protected:
    HRESULT InitializeUsingSafeArray(VARIANT &var, 
                                    HRESULT (__stdcall IAzNative:: *targetMethod)(BSTR, VARIANT));

    CComPtr<IAzNative> m_native;	

    CComBSTR m_name;

    bool m_isNew;
};
