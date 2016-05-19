/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzOperations.h

Abstract:

    Declaration of the CAzOperation class


 History:

****************************************************************************/
#pragma once
#include "azbase.h"

class CAzOperation :
    public CAzBase<IAzOperation>
{
    friend class CAzOperation;

public:
    CAzOperation(void);

    ~CAzOperation(void);

    CAzOperation(IAzOperation *pNative,bool pisNew);

    HRESULT Copy(CAzOperation &);
};
