/*++

Copyright (c) 2008  Microsoft Corporation

Title:

    Express Writer sample application

Abstract:

    Header file for helper functions

--*/

#pragma once


bool LoadMetadata(
    __in        IVssExpressWriter               *pExpressWriter,
    __in        PCWSTR                          wszParameter,
    __out       PWSTR                           *pwszData
);

bool SaveToFile(
    __in        IVssCreateExpressWriterMetadata *pMetadata,
    __in        PCWSTR                          wszFileName
);

bool ConstructWriterDefinition(
    __in        IVssCreateExpressWriterMetadata *pMetadata
);


