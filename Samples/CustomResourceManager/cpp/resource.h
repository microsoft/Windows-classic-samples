//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include <windows.h>
#include <list>

enum ResourceType
{
    FORUM,
    SECTION,
    TOPIC
};

#define NUMBER_OF_RESOURCES 10

enum ResourceIndices
{
    NONEXISTENT_OBJECT = -1,
    CONTOSO_FORUMS,
    SPORTS,
    FAVORITE_TEAM,
    UPCOMING_EVENTS,
    MOVIES,
    NEW_RELEASES,
    CLASSICS,
    HOBBIES,
    LEARNING_TO_COOK,
    SNOWBOARDING,
};

// This class represents a FORUM, SECTION, or TOPIC.
class Resource
{

public:

    Resource(_In_ PWSTR name, _In_ ResourceType type, _In_ PWSTR sd, _In_ int parentIndex);

    // The destructor will free the security descriptor if necessary
    ~Resource();
    
    // Adds to childIndices
    void AddChild(int index);

    // Returns true if this is a FORUM or SECTION
    bool IsContainer();

    // Frees the security descriptor string
    void FreeSD();

    // All of these functions are basic accessors/mutators
    void SetSD(_In_ PWSTR newSD);
    PWSTR GetSD();
    PWSTR GetName();
    ResourceType GetType();
    int GetParentIndex();
    std::list<int> GetChildIndices();

private:
    // The name of this resource, e.g. "Sports"
    PWSTR name;

    // Type of this resource (FORUM, SECTION, TOPIC)
    ResourceType type;

    // The security descriptor for this resource.
    PWSTR sd;

    // The index into CSecInfo's m_resources of the parent resource
    int parentIndex;

    // Similarly to the parent index, this keeps track of all children
    std::list<int> childIndices;
};

typedef Resource* PRESOURCE;

#endif //_RESOURCE_H_
