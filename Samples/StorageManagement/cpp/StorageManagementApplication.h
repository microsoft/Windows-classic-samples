//THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//
//Copyright (C) Microsoft Corporation. All rights reserved 

#pragma once

#include "MI.h"
#include <stdio.h>
#include <windows.h>
#include <tchar.h>


/******************************************************************************
 * Constants
 *****************************************************************************/
#define STORAGESUBSYSTEM_CLASSNAME L"MSFT_StorageSubSystem"
#define PHYSICALDISK_CLASSNAME L"MSFT_PhysicalDisk"
#define STORAGEPOOL_CLASSNAME L"MSFT_StoragePool"
#define VIRTUALDISK_CLASSNAME L"MSFT_VirtualDisk"
#define PARTITION_CLASSNAME L"MSFT_Partition"
#define DISK_CLASSNAME L"MSFT_Disk"
#define VOLUME_CLASSNAME L"MSFT_Volume"
#define NAMESPACE L"root\\microsoft\\windows\\storage"

//
// Maximum number of drives that may be used in creating s storagePool
// we use 3 in this sample
//
#define MAX_DRIVES 3


/******************************************************************************
 * Property is used to help create instance properties
 *****************************************************************************/
struct Property
{
    const MI_Char * name;
    const MI_Value * value;
    MI_Type type;
    BOOL isKey;

    Property() : name(NULL), value(NULL), isKey(true)
    {
    }
};

/******************************************************************************
 * StorageSpace and Pool SAMPLE Functions:
 *****************************************************************************/
MI_Result GetStorageSubsystem(
    MI_Session * session,
	MI_Value * storageSubsystemName,
    MI_Instance ** outInstance);

MI_Result CreateStoragePool(
    MI_Application * application,
    MI_Session * session,
    MI_Instance * inboundInstance,
	MI_Value * poolFriendlyName,
    MI_Instance ** outInstance);

MI_Result CreateVirtualDisk(
    MI_Application * application,
    MI_Session * session,
    MI_Instance * inboundInstance,
	MI_Value * virtualDiskFriendlyName,
	MI_Value * virtualDiskSize,
    MI_Instance ** outInstance);

MI_Result InitializeDisk(
    MI_Application * application,
    MI_Session * session,
	MI_Value * partitionType,
    MI_Instance * inboundInstance);

MI_Result CreatePartitionOnDisk(
    MI_Application * application,
    MI_Session * session,
    MI_Instance * inboundInstance,
    MI_Instance ** outInstance);

MI_Result FormatVolume(
    MI_Application * application,
    MI_Session * session,
	MI_Value * filesystemType,
    MI_Instance * inboundInstance);

MI_Result FindTheInstanceOfClassWithProperty(
    const MI_Char * className,
    MI_Session * session,
    const MI_Char * propName,
    const MI_Value * PropValue,
    MI_Instance ** pInstance
    );

MI_Result GetAssociatorInstances(
    MI_Session * session,
    MI_Instance * inboundInstance,
    MI_Instance ** associatedInstances,
    _In_z_ MI_Char * associatedClassName,
    _In_z_ MI_Char* associationClass,
    _In_z_ MI_Char* role,
    _In_z_ MI_Char* resultRole);

MI_Result ConstructNewInstance(
    MI_Application * application,
    const MI_Char * className,
    const MI_ClassDecl * classRTTI,
    Property * properties,
    int numOfProperties,
    MI_Instance ** instance );

MI_Result GetCreatedInstance(
    const MI_Instance * instance,
    const MI_Char * propName,
    MI_Instance ** createdInstance);


/******************************************************************************
 * Helper Structures, classes, and functions
 *****************************************************************************/

void PrintFailMessage(const MI_Char * lpMessage, MI_Result errorCode);

void PrintPropValue(
    const MI_Instance * instance,
    const MI_Char * propName);

bool IsTheRightInstance(
    const MI_Instance * instance,
    const MI_Char * propName,
    const MI_Value * propValue);

MI_Char * GetStringPropValue(
    const MI_Instance * instance,
    const MI_Char * propName);

MI_Uint32  GetIntPropValue(
    const MI_Instance * instance,
    const MI_Char * propName);

MI_Result  GetPropValue(
    const MI_Instance * instance,
    const MI_Char * propName,
    MI_Value * value);

class ApplicationPtr
{
public:
    MI_Application application;
    ApplicationPtr();
    ~ApplicationPtr();
private:
    const ApplicationPtr & operator = (const ApplicationPtr &);
    ApplicationPtr(const ApplicationPtr &);
};

class SessionPtr
{
public:
    MI_Session session;
    SessionPtr();
    ~SessionPtr();
private:
    const SessionPtr & operator = (const SessionPtr &);
    SessionPtr(const SessionPtr &);
};

class OperationPtr
{
public:
    MI_Operation operation;
    OperationPtr();
    ~OperationPtr();
private:
    const OperationPtr & operator = (const OperationPtr &);
    OperationPtr(const OperationPtr &);
};

MI_Result InitializeMI(ApplicationPtr *pApplication, SessionPtr *pSession);
