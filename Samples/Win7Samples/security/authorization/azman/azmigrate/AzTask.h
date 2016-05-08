/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzTask.h

Abstract:

    Declaration of the CAzTask class


 History:

****************************************************************************/
#pragma once
#include "azbase.h"


class CAzTask :
    public CAzBase<IAzTask>
{
        friend class CAzTask;

public:
    CAzTask(void);

    ~CAzTask(void);

    CAzTask(IAzTask *pNative,bool pisNew);

    HRESULT Copy(CAzTask &);

    HRESULT CopyLinks(CAzTask &);

protected:
    HRESULT CopyVersion2Constructs(CAzTask &);

private:

    static const unsigned int m_props[];

    static const unsigned char m_numberOfProps;

};
