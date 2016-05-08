/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzScope.h

Abstract:

    Declaration of the CAzScope class


 History:

****************************************************************************/
#pragma once
#include "azbase.h"
#include "AzTask.h"
#include "AzRole.h"
#include "AzAppGroup.h"
#include "AzHelper.h"

class CAzScope :
    public CAzBase<IAzScope>
{

    friend class CAzScope;

public:

    CAzScope(void);

    ~CAzScope(void);

    CAzScope(IAzScope *pNative,bool pisNew);

    HRESULT Copy(CAzScope &);

};
