/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzRole.h

Abstract:

    Declaration of the CAzRole class


 History:

****************************************************************************/
#pragma once
#include "stdafx.h"
#include "AzBase.h"
class CAzRole:public CAzBase<IAzRole>
{
    friend class CAzRole;
public:
    CAzRole(void);

    ~CAzRole(void);

    CAzRole(IAzRole *pNative,bool pisNew);

    HRESULT Copy(CAzRole &);

private:
    HRESULT CopyUserData(CAzRole &);

};
