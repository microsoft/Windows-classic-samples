/* **************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzAppGroup.h

Abstract:

    Declaration of the CAzAppGroup class

 History:

 ***************************************************************************/
#pragma once
#include "stdafx.h"
#include "AzBase.h"

class CAzAppGroup:public CAzBase<IAzApplicationGroup>
{

    friend class CAzRole;

public:
    CAzAppGroup(void);

    ~CAzAppGroup(void);

    CAzAppGroup(IAzApplicationGroup  *pNative,bool pisNew);

    HRESULT Copy(CAzAppGroup &);

    HRESULT CopyLinks(CAzAppGroup &);
protected:
    HRESULT CopyVersion2Constructs(CAzAppGroup &srcAppGroup);
};
