/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzMStore.h

Abstract:

    Declaration of the CAzMStore class


 History:

****************************************************************************/

#pragma once
#include "stdafx.h"
#include "AzApplication.h"

class CAzMStore
{
    friend class CAzMStore;
private:

    std::vector<CAzApplication *> m_apps;

    bool m_isNew;

    CComBSTR m_storeName;

    CComPtr<IAzAuthorizationStore> m_native;

public:

    CAzMStore(CComBSTR &pStoreName);

    HRESULT InitializeStore(bool pbcreateStore,bool overWritten=false);

    HRESULT OverWriteStore();

    HRESULT OpenApp(CComBSTR &pAppName);

    HRESULT OpenApps(vector<CComBSTR> &pAppNames);

    HRESULT OpenAllApps();

    HRESULT Copy(CAzMStore&);

    ~CAzMStore(void);

    const std::vector<CAzApplication*>& getApps() const;

protected:

    HRESULT CreateAppGroups(CAzMStore& ,bool);

    HRESULT CopyStoreProperties(CAzMStore&);

    HRESULT InitializeUsingSafeArray(VARIANT &var, 
                                    HRESULT (__stdcall IAzAuthorizationStore:: *targetMethod)(BSTR, VARIANT));

    static const unsigned int m_props[];

    static const unsigned char m_uchNumberOfProps;

};
