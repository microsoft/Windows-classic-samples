// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "stdafx.h"
#include "Tasks_h.h"
#include "resource.h"

class ATL_NO_VTABLE CTasksPropertyPage :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CTasksPropertyPage, &CLSID_TasksPropertyPage>,
    public IPropertyPageImpl<CTasksPropertyPage>,
    public CDialogImpl<CTasksPropertyPage>
{
public:
    CTasksPropertyPage();

    enum {IDD = IDD_TASKSPROPPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_TASKSAPPPROPPAGE)

BEGIN_COM_MAP(CTasksPropertyPage) 
    COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CTasksPropertyPage)
    CHAIN_MSG_MAP(IPropertyPageImpl<CTasksPropertyPage>)
END_MSG_MAP()

    STDMETHOD(Show)(UINT nCmdShow);
    STDMETHOD(Apply)();
};

OBJECT_ENTRY_AUTO(__uuidof(TasksPropertyPage), CTasksPropertyPage)
