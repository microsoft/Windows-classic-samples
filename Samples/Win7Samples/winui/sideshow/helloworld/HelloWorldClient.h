// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CHelloWorldClient
{
private:
    ISideShowSession            *m_pSession;
    ISideShowContentManager     *m_pContentMgr;
    ISideShowContent            *m_pContent;

public:
    CHelloWorldClient();
    virtual ~CHelloWorldClient();

    void Register();
    void Unregister();

    void AddContent();
    void RemoveAllContent();
};
