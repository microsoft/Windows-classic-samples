//THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//
//Copyright (C) Microsoft Corporation. All rights reserved 

#pragma once
#include "StorageManagementApplication.h"

/******************************************************************************
 * GetStorageSubsystem() returns an instance of StorageSubsystem
 *
 *****************************************************************************/
MI_Result GetStorageSubsystem(
                MI_Session * session,
                MI_Value * storageSubsystemName,
                MI_Instance ** outInstance )
{
    MI_Result result = MI_RESULT_FAILED;

    //
    //Get the first Instance of StorageSubystem
    //
    result =  FindTheInstanceOfClassWithProperty(
            STORAGESUBSYSTEM_CLASSNAME,
            session,
            MI_T("FriendlyName"),
            storageSubsystemName,
            outInstance
            );
    if (result != MI_RESULT_OK)
    {
        PrintFailMessage(L"Get an Instance of StorageSubSystem", result);
        return result;
    }

    return result;
}

/******************************************************************************
 * CreateStoragePool() creates and returns an instance of StoragePool
 *      given an instance of subsystem
 *      PhysicalDisks are obtained through Associator instances
 *****************************************************************************/
MI_Result CreateStoragePool(
                MI_Application * application,
                MI_Session * session,
                MI_Instance * inboundInstance,
                MI_Value * poolFriendlyName,
                MI_Instance ** outInstance)
{
    OperationPtr operation;
    MI_Instance * primordialPoolInstance = NULL;
    MI_Instance * inboundProperties = NULL;
    const MI_Instance * resultInstance;
    const MI_Instance * errorDetails;
    const MI_Char * errorString;
    MI_Result result = MI_RESULT_FAILED;
    int i = 0;
    MI_Boolean moreResults = MI_TRUE;  
    const MI_Char *errorMessage;  
    const MI_Instance *completionDetails;  

    MI_Instance *AssociatedPhysicalDisk[MAX_DRIVES];

    //initialize the pointer array:
    for (i = 0; i < MAX_DRIVES; i++)
    {
        AssociatedPhysicalDisk[i] = NULL;
    }

    //
    //Get the Associator instances of Storage Subsystem  --> Physical Disk
    //
    MI_Value miValueTrue;
    miValueTrue.boolean = MI_TRUE;

    MI_Session_AssociatorInstances(session, 
                                   0, 
                                   NULL, 
                                   NAMESPACE, 
                                   inboundInstance, 
                                   MI_T("MSFT_StorageSubsystemToPhysicalDisk"), 
                                   PHYSICALDISK_CLASSNAME, 
                                   MI_T("StorageSubsystem"), 
                                   MI_T("PhysicalDisk"), 
                                   MI_FALSE, 
                                   NULL, 
                                   &operation.operation);  

    i = 0;

    while (moreResults)
    {
        MI_Operation_GetInstance(&operation.operation, 
                                    &resultInstance, 
                                    &moreResults, 
                                    &result, 
                                    &errorMessage, 
                                    &completionDetails);  
        if (result != MI_RESULT_OK)
        {
            PrintFailMessage(L"Get Associated PhysicalDisks", result);
            return result;
        }

        if (resultInstance == NULL)
        {
            result = MI_RESULT_NOT_FOUND;
            PrintFailMessage(L"No PhysicalDisks found", result);
            return result;
        }
  
        if (IsTheRightInstance(resultInstance, MI_T("CanPool"), &miValueTrue) && i < MAX_DRIVES)
        {
            result = MI_Instance_Clone(resultInstance, &primordialPoolInstance);
            if (result != MI_RESULT_OK)
            {
                PrintFailMessage(L"PhysicalDisk instance", result);
                return result;
            }

            AssociatedPhysicalDisk[i++] = primordialPoolInstance;
        }
    }

    MI_Operation_Close(&operation.operation);

    //
    // Build the parameters used in Invoking CreateStoragePool:
    //
    MI_Result instanceCreationResult = MI_Application_NewInstance(
                                                        application,
                                                        MI_T("__PARAMETERS"), //className
                                                        NULL,                 //classRTTI,
                                                        &inboundProperties);
    if (instanceCreationResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"create Instance", instanceCreationResult);
        return instanceCreationResult;
    }
      
    //
    //StoragePool FriendlyName:
    //
    MI_Result addElementResult = MI_Instance_AddElement(
                                                    inboundProperties,
                                                    MI_T("FriendlyName"),
                                                    poolFriendlyName,
                                                    MI_STRING,
                                                    0);
    if(addElementResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"Add StoragePool FriendlyName parameter", addElementResult);
        return addElementResult;
    }
                     
    //
    // Add PhysicalDisk instance to the parameter list
    //
    MI_Value  physicalDiskInstances;

    physicalDiskInstances.instancea.data = AssociatedPhysicalDisk;
    physicalDiskInstances.instancea.size = i;

    addElementResult = MI_Instance_AddElement(
                                            inboundProperties,
                                            MI_T("PhysicalDisks"),
                                            &physicalDiskInstances,
                                            MI_INSTANCEA,
                                            0);
    if(addElementResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"Add PhysicalDisks parameter", addElementResult);
        return addElementResult;
    }

    MI_Value  resiliencySettingName;
    
    //
    // In this example,  we use the ResiliencySetting  "Simple"
    //	you can use other names/types in your own code
    //  
    //  When managing your own proprietary storage, you can use your own name too.
    //
    resiliencySettingName.string = MI_T("Simple");

    addElementResult = MI_Instance_AddElement(
        inboundProperties,
        MI_T("ResiliencySettingNameDefault"),
        &resiliencySettingName,
        MI_STRING,
        0);

    if(addElementResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"Add ResiliencySettingNameDefault parameter", addElementResult);
        return addElementResult;
    }
       
    wprintf(L"Invoking CreateStoragePool");


    //
    // Invoke "CreateStoragePool"
    //
    MI_Session_Invoke(
        session,
        0,          //flags
        NULL,       //options
        NAMESPACE,
        STORAGESUBSYSTEM_CLASSNAME,
        MI_T("CreateStoragePool"),
        inboundInstance,
        inboundProperties,
        NULL,
        &operation.operation);

    //
    // Get Results
    //
    MI_Operation_GetInstance(
        &operation.operation,
        &resultInstance,
        NULL,
        &result,
        &errorString,
        &errorDetails);

    if(result != MI_RESULT_OK )
    {
        PrintFailMessage(L"Invoke CreateStoragePool ", result);
        if (errorString)
            wprintf(L"\tError String <%s>\n", errorString);

        return result;
    }
    else
    {
        printf("Invoke CreateStoragePool Successful\n");
        PrintPropValue(resultInstance, L"ReturnValue");

        //
        //Get the Created instance --> storagePool
        //
        MI_Instance * createdInstance = NULL;

        result = GetCreatedInstance(
                        resultInstance,
                        MI_T("CreatedStoragePool"),
                        &createdInstance);
        
        if(result != MI_RESULT_OK )
        {
                PrintFailMessage(L" Get the created StoragePool instance! ", result);
                   
                return result;
        }

        //
        // Get the newly created instance by Enumeration:
        //
        result =  FindTheInstanceOfClassWithProperty(
                            STORAGEPOOL_CLASSNAME,
                            session,
                            MI_T("FriendlyName"),
                            poolFriendlyName,
                            outInstance
                            );

        if(result != MI_RESULT_OK )
        {
            PrintFailMessage(L" find the newly created StoragePool instance! ", result);
            
            return result;
        }
        else
        {
            printf("Creating StoragePool Successfully\n");
        }
    }

    //
    //Clean Instances:
    //
    if(NULL!= inboundProperties)
    {
        MI_Instance_Delete(inboundProperties);
    }

    if(NULL!= primordialPoolInstance)
    {
        MI_Instance_Delete(primordialPoolInstance);
    }

    return result;
}


/******************************************************************************
 * CreateVirtualDisk   returns an instance of Disk
 *****************************************************************************/
MI_Result CreateVirtualDisk(
                    MI_Application * application,
                    MI_Session * session,
                    MI_Instance * inboundInstance,
                    MI_Value * virtualDiskFriendlyName,
                    MI_Value * virtualDiskSize, 
                    MI_Instance ** outInstance)

{
    OperationPtr operation;
    MI_Instance * inboundProperties = NULL;
    MI_Instance * createdInstance = NULL;

    const MI_Instance * resultInstance;
    const MI_Instance * errorDetails;
    const MI_Char * errorString;
    MI_Result result = MI_RESULT_FAILED;


    //
    // Build the parameters used in Invoking CreateVirtualDisk:
    //
    MI_Result instanceCreationResult = MI_Application_NewInstance(
                                                        application,
                                                        MI_T("__PARAMETERS"), //className
                                                        NULL,                 //classRTTI,
                                                        &inboundProperties);
    if (instanceCreationResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"create Instance", instanceCreationResult);
        return instanceCreationResult;
    }
          
    // 1. VirtualDisk(lun) Friendly name    
    MI_Result addElementResult = MI_Instance_AddElement(
                                                    inboundProperties,
                                                    MI_T("FriendlyName"),
                                                    virtualDiskFriendlyName,
                                                    MI_STRING,
                                                    0);
    
    if(addElementResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"Add VirtualDisk Friendlyname parameter ", addElementResult);
        return addElementResult;
    }

    
    // 2. VirtualDisk(lun) Size	 
    addElementResult = MI_Instance_AddElement(
                                            inboundProperties,
                                            MI_T("Size"),
                                            virtualDiskSize,
                                            MI_UINT64,
                                            0);
    
    if(addElementResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"Add Size parameter", addElementResult);
        return addElementResult;
    }

    // 3. VirtualDisk(lun) Size
    MI_Value  storageAttributesName;
    storageAttributesName.string = MI_T("Simple");   
    
    addElementResult = MI_Instance_AddElement(
                                            inboundProperties,
                                            MI_T("ResiliencySettingName"),
                                            &storageAttributesName,
                                            MI_STRING,
                                            0);
    
    if(addElementResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"Add ResiliencySettingName parameter ", addElementResult);
        return addElementResult;
    }

    //
    // Invoke "CreateVirtualDisk" method
    //
    wprintf(L"Invoking  CreateVirtualDisk \n");

    MI_Session_Invoke(
                    session,
                    0,          //flags
                    NULL,       //options
                    NAMESPACE,
                    STORAGEPOOL_CLASSNAME,
                    MI_T("CreateVirtualDisk"),
                    inboundInstance,
                    inboundProperties,
                    NULL,
                    &operation.operation);

    //
    // Get Results
    //
    MI_Operation_GetInstance(
                    &operation.operation,
                    &resultInstance,
                    NULL,
                    &result,
                    &errorString,
                    &errorDetails);

    if(result != MI_RESULT_OK )
    {
        PrintFailMessage(L"Invoke CreateVirtualDisk !\n", result);
        if (errorString)
            wprintf(L"\tError String <%s>\n", errorString);

        return result;
    }
    else
    {
        //
        // Get the Created instance --> VirtualDisk
        //
        result = GetCreatedInstance(
                        resultInstance,
                        MI_T("CreatedVirtualDisk"),
                        &createdInstance);
        
        if(result != MI_RESULT_OK )
        {
                PrintFailMessage(L" get the created VirtualDisk instance!\n", result);
                   
                return result;
        }

        //
        // Get the VirtualDisk Instance:
        //
        result =  FindTheInstanceOfClassWithProperty(
                                VIRTUALDISK_CLASSNAME,
                                session,
                                MI_T("FriendlyName"),
                                virtualDiskFriendlyName,
                                outInstance
                                );

        if(result != MI_RESULT_OK )
        {
            PrintFailMessage(L"find the newly created VirtualDisk instance !", result);

            return result;
        }
        else
        {
            printf("Invoking CreateVirtualDisk Successfully\n");
        }

    }

    //
    //Clean Instances:
    //
    if(NULL!= inboundProperties)
    {
        MI_Instance_Delete(inboundProperties);
    }

    return result;
}



/******************************************************************************
 * GetAssociatorInstances   returns associated instances of the inboundInstance
 *****************************************************************************/
MI_Result GetAssociatorInstances(
                    MI_Session * session,
                    MI_Instance * inboundInstance,
                    MI_Instance ** associatedInstances,
                    _In_z_ MI_Char * associatedClassName,
                    _In_z_ MI_Char* associationClass,
                    _In_z_ MI_Char* role,
                    _In_z_ MI_Char* resultRole)

{
    OperationPtr operation;
    const MI_Instance * resultInstance;
    MI_Result result = MI_RESULT_FAILED;
    MI_Boolean moreResults = MI_TRUE;  
    bool firstInstance = true;

    //
    //Get the Associator instances of PhysicalDisks  
    //
    MI_Session_AssociatorInstances(session, 
                                0, 
                                NULL, 
                                NAMESPACE, 
                                inboundInstance, 
                                associationClass,
                                associatedClassName, 
                                role, 
                                resultRole, 
                                MI_TRUE,
                                NULL, 
                                &operation.operation);  
      
    while (moreResults)  
    {  
        MI_Operation_GetInstance(&operation.operation, 
                                    &resultInstance, 
                                    &moreResults, 
                                    &result, 
                                    NULL, 
                                    NULL);
        if (result != MI_RESULT_OK )
        {
            PrintFailMessage(L" MI_Operation_GetInstance() ", result);
            return result;
        }

        //
        //resultInstance could be NULL
        //
        if(resultInstance == NULL)
        {
            printf("MI_Operation_GetInstance() failed, resultInstance is NULL !");

            result = MI_RESULT_FAILED;

            return result;
        }

        if (firstInstance)
        {
            result = MI_Instance_Clone(resultInstance, associatedInstances);

            if (result != MI_RESULT_OK)
            {
                PrintFailMessage(L"Clone associated instance ", result);
                return result;
            }

            firstInstance = false;
        }
    }

    return result;
}


/******************************************************************************
 * InitializeDisk() - Initializes the given disk
 *****************************************************************************/
MI_Result InitializeDisk(
        MI_Application * application,
        MI_Session * session,
        MI_Value * partitionType,
        MI_Instance * inboundInstance)
{
    OperationPtr operation;
    MI_Instance * inboundProperties = NULL;
    const MI_Instance * resultInstance;
    const MI_Instance * errorDetails;
    const MI_Char * errorString;
    MI_Result result = MI_RESULT_FAILED;

    //
    // Build the parameters used in Invoking Initialize():
    //
    MI_Result instanceCreationResult = MI_Application_NewInstance(
                                                        application,
                                                        MI_T("__PARAMETERS"), //className
                                                        NULL,                 //classRTTI,
                                                        &inboundProperties);
    if (instanceCreationResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"create Instance", instanceCreationResult);
        return instanceCreationResult;
    }
            
    // Partition type 
    //
    MI_Result addElementResult = MI_Instance_AddElement(
                                                    inboundProperties,
                                                    MI_T("PartitionStyle"),
                                                    partitionType,
                                                    MI_UINT16,
                                                    0);
    
    if(addElementResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"Add Element", addElementResult);
        return addElementResult;
    }

    wprintf(L"Invoking Initialize \n");


    //
    // Invoke "Initialize" Method
    //
    MI_Session_Invoke(
                    session,
                    0,          
                    NULL,       
                    NAMESPACE,
                    DISK_CLASSNAME,
                    MI_T("Initialize"),
                    inboundInstance,
                    inboundProperties,
                    NULL,
                    &operation.operation);

    //
    // Get Results
    //
    MI_Operation_GetInstance(
                    &operation.operation,
                    &resultInstance,
                    NULL,
                    &result,
                    &errorString,
                    &errorDetails);

    if(result != MI_RESULT_OK )
    {
        PrintFailMessage(L"Invoking Initialize", result);
        if (errorString)
            wprintf(L"Error String <%s>\n", errorString);
    }
    else
    {
        printf("Invoke InitializeDisk Successful\n");
    }

    
    //
    //Clean Instances:
    //
    if(NULL!= inboundProperties)
    {
        MI_Instance_Delete(inboundProperties);
    }

    return result;
}

/******************************************************************************
 * FormatVolume() - formats the given volume
 *****************************************************************************/
MI_Result FormatVolume(
        MI_Application * application,
        MI_Session * session,
        MI_Value  * filesystemType,
        MI_Instance * inboundInstance)
{
    OperationPtr operation;
    MI_Instance * inboundProperties = NULL;
    const MI_Instance * resultInstance;
    const MI_Instance * errorDetails;
    const MI_Char * errorString;
    MI_Result result = MI_RESULT_FAILED;

    const MI_Char * volume_FileSystem = MI_T("FileSystem");

    //
    // Build the parameters used in Invoking Format():
    //
    MI_Result instanceCreationResult = MI_Application_NewInstance(
                                                        application,
                                                        MI_T("__PARAMETERS"), //className
                                                        NULL,            //classRTTI,
                                                        &inboundProperties);
    if (instanceCreationResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"create Instance", instanceCreationResult);
        return instanceCreationResult;
    }

    //      
    // filesystem type
    //
    MI_Result addElementResult = MI_Instance_AddElement(
                                                    inboundProperties,
                                                    volume_FileSystem,
                                                    filesystemType,
                                                    MI_STRING,
                                                    0);
    
    if(addElementResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"Add Element", addElementResult);
        return addElementResult;
    }

    wprintf(L"Invoking Format \n");

    //
    // Invoke "Format" method
    //
    MI_Session_Invoke(
        session,
        0,           
        NULL,        
        NAMESPACE,
        VOLUME_CLASSNAME,
        MI_T("Format"),
        inboundInstance,
        inboundProperties,
        NULL,
        &operation.operation);

    //
    // Get Results
    //
    MI_Operation_GetInstance(
        &operation.operation,
        &resultInstance,
        NULL,
        &result,
        &errorString,
        &errorDetails);

    if(result != MI_RESULT_OK )
    {
        PrintFailMessage(L"Invoke Format", result);
        if (errorString)
            wprintf(L"Error String <%s>\n", errorString);
    }
    else
    {
        printf("Invoke Format Successful\n");
    }

    //
    // Cleaning
    //
    if(NULL!= inboundProperties)
    {
        MI_Instance_Delete(inboundProperties);
    }

    return result;
}


MI_Result CreatePartitionOnDisk(
                MI_Application * application,
                MI_Session * session,
                MI_Instance * inboundInstance,
                MI_Instance ** outInstance)
{
    OperationPtr operation;
    MI_Instance * inboundProperties = NULL;
    const MI_Instance * resultInstance;
    const MI_Instance * errorDetails;
    const MI_Char * errorString;
    MI_Result result = MI_RESULT_FAILED;


    //
    // Creating Partition
    //
    //
    // Build the parameters used in Invoke:
    //
    MI_Result instanceCreationResult = MI_Application_NewInstance(
                                                        application,
                                                        MI_T("__PARAMETERS"), //className
                                                        NULL,            //classRTTI,
                                                        &inboundProperties);
    if (instanceCreationResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"create Instance", instanceCreationResult);
        return instanceCreationResult;
    }

    MI_Value useMaxSize;
    useMaxSize.boolean = MI_TRUE;
    MI_Result addElementResult = MI_Instance_AddElement(
                                                    inboundProperties,
                                                    MI_T("UseMaximumSize"),
                                                    &useMaxSize,
                                                    MI_BOOLEAN,
                                                    0);
    
    if(addElementResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"Add CreatedPartition parameter", addElementResult);
        return addElementResult;
    }

    wprintf(L"Invoking CreatePartition \n");

    //
    // Invoke "CreatePartition" method
    //
    MI_Session_Invoke(
                    session,
                    0,         
                    NULL,       
                    NAMESPACE,
                    DISK_CLASSNAME,
                    MI_T("CreatePartition"),
                    inboundInstance,
                    inboundProperties,
                    NULL,
                    &operation.operation);

    //
    // Get Results
    //
    MI_Operation_GetInstance(
                    &operation.operation,
                    &resultInstance,
                    NULL,
                    &result,
                    &errorString,
                    &errorDetails);

    if(result != MI_RESULT_OK )
    {
        PrintFailMessage(L"Invoke CreatePartition", result);
        if (errorString)
            wprintf(L"Error String <%s>\n", errorString);

        return result;
    }
    else
    {
        //
        // Get the Created instance --> Partition
        //
        MI_Instance * createdInstance = NULL;

        result = GetCreatedInstance(
                        resultInstance,
                        MI_T("CreatedPartition"),
                        &createdInstance);
        
        if(result != MI_RESULT_OK )
        {
                PrintFailMessage(L" get the created Partition instance!\n", result);
               
                return result;
        }
        else
        {
            //
            // Get the Partition Instance through DiskId:
            //
            MI_Value miValue;

            miValue.string  = GetStringPropValue(createdInstance, L"DiskId");

            result =  FindTheInstanceOfClassWithProperty(
                                    PARTITION_CLASSNAME,
                                    session,
                                    MI_T("DiskId"),
                                    &miValue,
                                    outInstance
                                    );

            if(result != MI_RESULT_OK )
            {
                PrintFailMessage(L" Find the Created Partition instance !", result);

                return result;
            }
            else
            {
                wprintf(L"DiskId = \"%s\" \n", miValue.string);
            }
        }
    }

    //
    //Cleaning
    //
    if(NULL!= inboundProperties)
    {
        MI_Instance_Delete(inboundProperties);
    }

    return result;
}



/******************************************************************************
 * Enumerate "className" Instances Synchronously 
 * In this example, the found instance's property with be compare with
 * the property provided, the matched property will be returned
 *****************************************************************************/
MI_Result FindTheInstanceOfClassWithProperty(
    const MI_Char * className,
    MI_Session * session,
    const MI_Char * propName,
    const MI_Value * PropValue,
    MI_Instance ** pInstance
    )
{
    OperationPtr operation;
    MI_OperationOptions *options = NULL;
    MI_Uint32 flags = 0;
    const MI_Instance *resultInstance = NULL;
    const MI_Char *errorMessage;
    const MI_Instance *completionDetails;
    MI_Boolean moreResults = MI_TRUE;
    MI_Result result = MI_RESULT_FAILED;
    BOOLEAN bFound = FALSE;

    MI_Session_EnumerateInstances(
        session,
        flags,
        options,
        NAMESPACE,
        className,
        MI_FALSE,
        NULL,
        &operation.operation);

    while (moreResults)
    {
        MI_Operation_GetInstance(
            &operation.operation,
            &resultInstance,
            &moreResults,
            &result,
            &errorMessage,
            &completionDetails);

        if (result != MI_RESULT_OK)
        {
            PrintFailMessage(L"Get Instance.", result);
            if (errorMessage)
                wprintf(L"Error String <%s>\n", errorMessage);
        }
        else
        {

            //
            //resultInstance could be NULL, we need to check it.
            //
            if (resultInstance != NULL && IsTheRightInstance(resultInstance, propName,  PropValue) && !bFound)
            {
                result = MI_Instance_Clone(resultInstance, pInstance);
                //
                // Clone the instance
                //
                if( result != MI_RESULT_OK)
                {
                    PrintFailMessage(L"Clone Instance failed in FindTheInstanceOfClassWithProperty()!", result);
                }

                bFound = TRUE;
            }

        }
    }

    if(result == MI_RESULT_OK && !bFound)
    {
        result = MI_RESULT_NOT_FOUND;
    }

    return result;
}


/******************************************************************************
 * StorageSpaceAndPool SAMPLE Function:
 * Create a new instance with given properties
 *****************************************************************************/
MI_Result ConstructNewInstance(
    MI_Application * application,
    const MI_Char * className,
    const MI_ClassDecl * classRTTI,
    Property * properties,
    int numOfProperties,
    MI_Instance ** instance )
{
    MI_Result instanceCreationResult = MI_Application_NewInstance(
                                                            application,
                                                            className,
                                                            classRTTI,
                                                            instance);

    if (instanceCreationResult != MI_RESULT_OK)
    {
        PrintFailMessage(L"create Instance", instanceCreationResult);
        return instanceCreationResult;
    }

    if (properties!= NULL)
    {
        for (int i = 0; i < numOfProperties; i++)
        {
            MI_Uint32 flags = 0;
            if(properties[i].isKey)
            {
                flags = MI_FLAG_KEY;
            }
            MI_Result addElementResult = MI_Instance_AddElement(
                *instance,
                properties[i].name,
                properties[i].value,
                properties[i].type,
                flags);
            if(addElementResult != MI_RESULT_OK)
            {
                PrintFailMessage(L"Add Element", addElementResult);
                return addElementResult;
            }
        }
    }

    return MI_RESULT_OK;
}

/************************************************************************************************
 * StorageSpaceAndPool SAMPLE Function:
 * it does the necessary Initialization of the Management Infrastructure to use WMIv2 Client APIs
 *
 ************************************************************************************************/
MI_Result InitializeMI(ApplicationPtr *pApplication, SessionPtr *pSession)
{
    MI_Result result = MI_RESULT_FAILED;
    
    result = MI_Application_Initialize(0,
        NULL,
        NULL,
        &pApplication->application);

    if (result != MI_RESULT_OK)
    {
        PrintFailMessage(L"MI_Application_Initialize", result);

        return result;
    }

    result = MI_Application_NewSession(
        &pApplication->application,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &pSession->session);

    if (result != MI_RESULT_OK)
    {
        PrintFailMessage(L"MI_Application_NewSession", result);

        return result;
    }

    return result;
}


ApplicationPtr::ApplicationPtr()
{
    memset(&application, 0, sizeof(MI_Application));
}

ApplicationPtr::~ApplicationPtr()
{
    if (application.ft != NULL)
    {
        MI_Application_Close(&application);
    }
}

SessionPtr::SessionPtr()
{
    memset(&session, 0, sizeof(MI_Session));
}

SessionPtr::~SessionPtr()
{
    if (session.ft != NULL)
    {
        MI_Session_Close(&session, NULL, NULL);
    }
}

OperationPtr::OperationPtr()
{
    memset(&operation, 0, sizeof(MI_Operation));
}

OperationPtr::~OperationPtr()
{
    if (operation.ft != NULL)
    {
        MI_Operation_Close(&operation);
    }
}


/******************************************************************************
 * Help Functions that print out instances message etc.
 *
 * They are only needed in coding/debugging, not necessarily in the sample codes
 *****************************************************************************/

void PrintFailMessage(const MI_Char * lpMessage, MI_Result errorCode)
{
    wprintf(L"Failed to %s. Error Code is '%d'.\n\n", lpMessage, errorCode);
}

void PrintValue(
    const MI_Char * propName,
    MI_Type type,
    const MI_Value * value,
    MI_Uint32 flags,
    MI_Uint32 index)
{

    wprintf(L"\n\t--Begin Property {%s}--\n", propName);
    if (type == MI_STRING)
    {
        wprintf(L"\t%s = '%s'; flag =%u; index= %u.\r\n",
            propName,
            value->string,
            flags,
            index);
    }
    else if (type == MI_UINT8)
    {
        wprintf(L"\t%s = '%u'; flag =%u; index= %u.\r\n",
            propName,
            value->uint8,
            flags,
            index);
    }
    else if (type == MI_UINT16)
    {
        wprintf(L"\t%s = '%u'; flag =%u; index= %u.\r\n",
            propName,
            value->uint16,
            flags,
            index);
    }
    else if (type == MI_CHAR16)
    {
        wprintf(L"\t%s = '%c'; flag =%u; index= %u.\r\n",
            propName,
            value->char16,
            flags,
            index);
    }
    else if (type == MI_BOOLEAN)
    {
        wprintf(L"\t%s = '%u'; flag =%u; index= %u.\r\n",
            propName,
            value->boolean,
            flags,
            index);
    }
    else if (type == MI_REFERENCE)
    {
        MI_Instance * pInstance = value->reference;
        wprintf(L"\t");

        wprintf(L"\t%s = Instance of Class:'%s'; flag =%u; index= %u.\r\n",
            propName,
            pInstance->classDecl->name,
            flags,
            index);
    }
    else if (type == MI_REFERENCEA)
    {
        wprintf(L"\t");
        MI_ReferenceA refA = value->referencea;
        for (MI_Uint32 i = 0; i < refA.size; i++)
        {
            wprintf(L"\t%s = Instance of Class:'%s'; flag =%u; index= %u.\r\n",
                propName,
                refA.data[i]->classDecl->name,
                flags,
                index);
        }
    }
    else if (type == MI_INSTANCE)
    {
        MI_Instance * pInstance = value->instance;
        wprintf(L"\t");
        wprintf(L"\t%s = Instance of Class:'%s'; flag =%u; index= %u.\r\n",
            propName,
            pInstance->classDecl->name,
            flags,
            index);
    }
    else
    {
        wprintf(L"\t%s = '%u'; flag =%u; index= %u.\r\n",
            propName,
            value->uint32,
            flags,
            index);
    }
    wprintf(L"\t--End Property {%s}--\n", propName);
}


void PrintPropValue(
    const MI_Instance * instance,
    const MI_Char * propName)
{
    MI_Value value;
    MI_Type type;
    MI_Uint32 flags;
    MI_Uint32 index;
    MI_Result result = MI_RESULT_FAILED;
    
    result = instance->ft->GetElement(
                                instance,
                                propName, 
                                &value,
                                &type,
                                &flags,
                                &index);

    if (result == MI_RESULT_OK)
    {
        PrintValue(propName, type, &value, flags, index);
    }
    else
    {
 
        wprintf(L"Property {%s} can not be obtained!.\n", propName);;
    }
    wprintf(L"\n");
}

MI_Result GetCreatedInstanceOrig(
    const MI_Instance * instance,
    const MI_Char * propName,
    MI_Instance ** createdInstance)
{
    MI_Value value;
    MI_Type type;
    MI_Uint32 flags;
    MI_Uint32 index;
    MI_Result result = MI_RESULT_FAILED;
    
    result = instance->ft->GetElement(
                                instance,
                                propName, 
                                &value,
                                &type,
                                &flags,
                                &index);

    if (result == MI_RESULT_OK)
    {
        PrintValue(propName, type, &value, flags, index);

        if (type == MI_INSTANCE)
        {
            //
            // Clone the instance
            //
            result = MI_Instance_Clone(value.instance, createdInstance);
            if( result != MI_RESULT_OK)
            {
                *createdInstance = NULL;
                PrintFailMessage(L"Clone the Created Instance failed!", result);
            }
            else
            {
                wprintf(L"\t");
                wprintf(L"\t%s = Created Instance of Class:'%s'; flag =%u; index= %u.\r\n",
                    propName,
                    (*createdInstance)->classDecl->name,
                    flags,
                    index);
            }
        }
    }
    else
    {
 
        wprintf(L"Instance {%s} can not be obtained!.\n", propName);;
    }
    wprintf(L"\n");

    return result;
}


MI_Result GetCreatedInstance(
    const MI_Instance * instance,
    const MI_Char * propName,
    MI_Instance ** createdInstance)
{
    MI_Value value;
    MI_Type type;
    MI_Uint32 flags;
    MI_Uint32 index;
    MI_Result result = MI_RESULT_FAILED;
    
    result = instance->ft->GetElement(
                                instance,
                                propName, 
                                &value,
                                &type,
                                &flags,
                                &index);

    if (result == MI_RESULT_OK)
    {
        PrintValue(propName, type, &value, flags, index);

        if (type == MI_INSTANCE)
        {
            *createdInstance = value.instance;

            wprintf(L"\t");
            wprintf(L"\t%s = Created Instance of Class:'%s'; flag =%u; index= %u.\r\n",
                propName,
                (*createdInstance)->classDecl->name,
                flags,
                index);
        }
    }
    else
    {
 
        wprintf(L"Instance {%s} can not be obtained!.\n", propName);;
    }
    wprintf(L"\n");

    return result;
}



bool IsTheRightInstance(
    const MI_Instance * instance,
    const MI_Char * propName,
    const MI_Value * PropValue
    )
{
    MI_Value value;
    MI_Type type;
    MI_Uint32 flags;
    MI_Uint32 index;
    MI_Result result = MI_RESULT_FAILED;
    bool returnValue = false;

    
    result = instance->ft->GetElement(
                                instance,
                                propName, 
                                &value,
                                &type,
                                &flags,
                                &index);

    if (result == MI_RESULT_OK)
    {
        if (type == MI_STRING)
        {
            wprintf(L"\t The value of property %s = %s\r\n",
                propName,
                value.string
                );
            
            if( value.string == NULL)
            {
                wprintf(L"\t Property <%s> is NULL \n", propName);
                return false;
            }

            if(!_wcsicmp(PropValue->string,  value.string))
            {
                returnValue = true;
            }
        } 
        else if (type == MI_UINT32)
        {
            wprintf(L"\t The value of property %s = %u\r\n",
                propName,
                value.uint32
                );
            
            if(PropValue->uint32 == value.uint32)
            {
                returnValue = true;
            }
        }
        else if (type == MI_CHAR16)
        {
            wprintf(L"\t The value of property %s = %c\r\n",
                propName,
                value.char16
                );
            
            if(PropValue->char16 == value.char16)
            {
                returnValue = true;
            }
        }
        else if (type == MI_BOOLEAN)
        {
            wprintf(L"\t The value of property %s = %s\r\n",
                propName,
                value.boolean ? L"TRUE" : L"FALSE"
                );

            if (PropValue->boolean == value.boolean)
            {
                returnValue = true;
            }
        }
    }
    else
    {
        wprintf(L"Property {%s} can not be obtained!.\n", propName);
    }

    return returnValue;
}

MI_Char * GetStringPropValue(
    const MI_Instance * instance,
    const MI_Char * propName)
{
    MI_Value value;
    MI_Type type;
    MI_Uint32 flags;
    MI_Uint32 index;
    MI_Result result = MI_RESULT_FAILED;
    
    result = instance->ft->GetElement(
                                instance,
                                propName, 
                                &value,
                                &type,
                                &flags,
                                &index);

    if (result == MI_RESULT_OK)
    {
        PrintValue(propName, type, &value, flags, index);

        return value.string;
    }
    else
    {
        wprintf(L"Property {%s} can not be obtained!.\n", propName);

        return NULL;
    }
}

MI_Uint32  GetIntPropValue(
    const MI_Instance * instance,
    const MI_Char * propName)
{
    MI_Value value;
    MI_Type type;
    MI_Uint32 flags;
    MI_Uint32 index;
    MI_Result result = MI_RESULT_FAILED;
    
    result = instance->ft->GetElement(
                                instance,
                                propName, 
                                &value,
                                &type,
                                &flags,
                                &index);

    if (result == MI_RESULT_OK)
    {
        PrintValue(propName, type, &value, flags, index);

        return value.uint32;
    }
    else
    {
 
        wprintf(L"Property {%s} can not be obtained!.\n", propName);

        return NULL;
    }
}

MI_Result  GetPropValue(
    const MI_Instance * instance,
    const MI_Char * propName,
    MI_Value * value)
{
    MI_Type type;
    MI_Uint32 flags;
    MI_Uint32 index;
    MI_Result result = MI_RESULT_FAILED;
    
    result = instance->ft->GetElement(
                                instance,
                                propName, 
                                value,
                                &type,
                                &flags,
                                &index);

    if (result == MI_RESULT_OK)
    {
        PrintValue(propName, type, value, flags, index);

    }
    else
    {
        wprintf(L"Property {%s} can not be obtained!.\n", propName);
    }

    return result;
}

/******************************************************************************
 * Main Function
 *****************************************************************************/
int __cdecl _tmain(_In_ int argc, _In_reads_(argc) _TCHAR* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    ApplicationPtr application;
    OperationPtr operation;
    SessionPtr session;    
    MI_Result result = MI_RESULT_FAILED;
    MI_Value  storageSubsystemName;


    MI_Instance * instanceOfStorageSubsystem = NULL;
    MI_Instance * instanceOfStoragePool = NULL;
    MI_Instance * instanceOfVirtualDisk = NULL;
    MI_Instance * instanceOfDisk = NULL;
    MI_Instance * instanceOfPartition = NULL;
    MI_Instance * instanceOfVolume = NULL;

    //
    //Parse the command line first: we read in the Subsystem Friendlyname.
    //
    if(argc != 2)
    {
        printf("Usage: StorageManagementApplication.exe  yourSubsystemFriendlyName \n");

        return 0;
    }
    else
    {
        storageSubsystemName.string = argv[1];

        wprintf( L"The given StorageSubsystem friendlyName is <%s> \n", storageSubsystemName.string);
    }


    //
    //Necessary Initialization of Management Infrastructure
    //
    result = InitializeMI(&application, &session);
    if (result != MI_RESULT_OK)
    {
        //
        //Initialization failed:
        //
        PrintFailMessage(L"MI initialization", result);
        return result;
    }
    

    /******************************************************************************
    * End-to-End operations:   Start from StorageSubsystem  to a Formatted volume 
    *
    *****************************************************************************/

    //
    // Get the StorageSubsystem specified:
    //
    result = GetStorageSubsystem(
                        &session.session,
                        &storageSubsystemName,
                        &instanceOfStorageSubsystem);
    
    if (result != MI_RESULT_OK)
    {
        PrintFailMessage(L"GetStorageSubsystem()", result);
        goto EXIT;
    }

    //
    // Create StoragePool with the FriendlyName specified:
    //
    MI_Value storagePoolFriendlyName;
    storagePoolFriendlyName.string = MI_T("TestStoragePool01");
    
    result = CreateStoragePool(
                        &application.application,
                        &session.session,
                        instanceOfStorageSubsystem,
                        &storagePoolFriendlyName,
                        &instanceOfStoragePool);

    if (result != MI_RESULT_OK)
    {
        PrintFailMessage(L"CreateStoragePool()", result);
        goto EXIT;
    }

    //
    // Create VirtualDisk:
    //
    MI_Value  virtualDiskFriendlyName;
    MI_Value  virtualDiskSize;

    virtualDiskFriendlyName.string = MI_T("TestVirtualDisk01");

    virtualDiskSize.uint64 = 1024*1024*1024; //1GB 


    result =  CreateVirtualDisk(
                        &application.application,
                        &session.session,
                        instanceOfStoragePool,
                        &virtualDiskFriendlyName,
                        &virtualDiskSize,
                        &instanceOfVirtualDisk);
    
    if (result != MI_RESULT_OK)
    {
        PrintFailMessage(L"CreateVirtualDisk()", result);
        goto EXIT;
    }


    //
    // Get the Instance of Disk through VirtualDisk/Disk association 
    //
    result = GetAssociatorInstances(
                        &session.session,
                        instanceOfVirtualDisk,
                        &instanceOfDisk,
                        DISK_CLASSNAME,
                        MI_T("MSFT_VirtualDiskToDisk"),
                        MI_T("VirtualDisk"),
                        MI_T("Disk")
                        );
    
    if (result != MI_RESULT_OK)
    {
        PrintFailMessage(L"GetAssociatorInstances()", result);
        
        goto EXIT;

    }

    //
    // Initialize the Disk with the partition type specified:
    //
    MI_Value  partitionType;

    partitionType.uint16 = 1;   //MBR

    result =  InitializeDisk(
                        &application.application,
                        &session.session,
                        &partitionType,
                        instanceOfDisk);

    if (result != MI_RESULT_OK)
    {
        PrintFailMessage(L"Initialize the Disk", result);
        goto EXIT;
    }

    //
    // Create Partition
    //
    result = CreatePartitionOnDisk(
                        &application.application,
                        &session.session,
                        instanceOfDisk,
                        &instanceOfPartition);

    if (result != MI_RESULT_OK)
    {
        PrintFailMessage(L"Create partition on the Disk", result);
        goto EXIT;
    }

    //
    // Get the Instance of Volume through Partition association
    //
    result = GetAssociatorInstances(
                        &session.session,
                        instanceOfPartition,
                        &instanceOfVolume,
                        VOLUME_CLASSNAME,
                        MI_T("MSFT_PartitionToVolume"),
                        MI_T("Partition"),
                        MI_T("Volume"));
    
    if (result != MI_RESULT_OK)
    {
        PrintFailMessage(L"GetAssociatorInstances() -- Volume", result);
        
        goto EXIT;
    }

    //
    // Format the Volume:
    //
    MI_Value  filesystemType;

    filesystemType.string = MI_T("NTFS"); 

    result = FormatVolume(
                    &application.application,
                    &session.session,
                    &filesystemType,
                    instanceOfVolume);
    
    if (result != MI_RESULT_OK)
    {
        PrintFailMessage(L"Format the volume", result);
        return result;
    }
    else
    {
        printf("The Volume has been Formated Successfully\n");
    }

    //
    //Cleaning
    //
EXIT:

    if(NULL!= instanceOfStorageSubsystem)
    {
        MI_Instance_Delete(instanceOfStorageSubsystem);
    }

    if(NULL!= instanceOfStoragePool)
    {
        MI_Instance_Delete(instanceOfStoragePool);
    }

    if(NULL!= instanceOfVirtualDisk)
    {
        MI_Instance_Delete(instanceOfVirtualDisk);
    }

    if(NULL!= instanceOfDisk)
    {
        MI_Instance_Delete(instanceOfDisk);
    }

    if(NULL!= instanceOfPartition)
    {
        MI_Instance_Delete(instanceOfPartition);
    }

    if(NULL!= instanceOfVolume)
    {
        MI_Instance_Delete(instanceOfVolume);
    }

    return 0;
}
