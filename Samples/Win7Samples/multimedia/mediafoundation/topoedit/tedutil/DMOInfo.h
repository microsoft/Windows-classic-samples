// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

class DMOInfo 
{
public:
    static DMOInfo* GetSingleton();

    CAtlStringW GetCLSIDName(const CLSID& clsid);
    CLSID GetCLSIDFromName(const CAtlStringW& strName);
    
protected:
    DMOInfo();
    
private:
    CAtlArray<CLSID> m_clsids;
    CAtlArray<CAtlStringW> m_names;
};
