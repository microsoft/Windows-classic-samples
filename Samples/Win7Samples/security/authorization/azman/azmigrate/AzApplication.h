/* **************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzApplication.h

Abstract:

    Declaration of the CAzApplication class

 History:

 ***************************************************************************/

#pragma once

#include "stdafx.h"
#include "AzBase.h"
#include "AzRole.h"
#include "AzOperation.h"
#include "AzTask.h"
#include "AzScope.h"
#include "AzAppGroup.h"
#include "AzHelper.h"


class CAzApplication:public CAzBase<IAzApplication>
{
    friend class CAzApplication;

public:

    CAzApplication(void);

    CAzApplication(IAzApplication *pNative,bool pIsNewApp=false);

    ~CAzApplication(void);

    HRESULT Copy(CAzApplication &);

protected:
    HRESULT CreateChildItems(CAzApplication &);

    HRESULT CreateOperations(CAzApplication &);

    HRESULT CreateScopes(CAzApplication &);

    static const unsigned int m_props[];

    static const unsigned char m_uchNumberOfProps;


};
