//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
//
// This file contains the definitions of the Resource class.
//
#include "resource.h"

Resource::Resource(_In_ PWSTR name, _In_ ResourceType type, _In_ PWSTR sd, _In_ int parentIndex) :
    name(name),
    type(type),
    sd(sd),
    parentIndex(parentIndex)
{
}

void Resource::SetSD(_In_ PWSTR newSD)
{
    sd = newSD;
}
PWSTR Resource::GetSD()
{
    return sd;
}
bool Resource::IsContainer()
{
    // We could also implement this by checking to see if childIndices
    // is non-empty.
    return type == FORUM || type == SECTION;
}
PWSTR Resource::GetName()
{
    return name;
}
ResourceType Resource::GetType()
{
    return type;
}

int Resource::GetParentIndex()
{
    return parentIndex;
}

void Resource::FreeSD()
{
    LocalFree(sd);
    sd = nullptr;
}

Resource::~Resource()
{
    FreeSD();
}

void Resource::AddChild(int index)
{
    childIndices.push_back(index);
}

std::list<int> Resource::GetChildIndices()
{
    return childIndices;
}
