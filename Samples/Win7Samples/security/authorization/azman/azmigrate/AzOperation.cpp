/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzOperation.cpp

Abstract:

    Routines performing the migration for the CAzOperation object

 History:

****************************************************************************/
#include "AzOperation.h"

CAzOperation::CAzOperation(void):CAzBase<IAzOperation>()
{
}

CAzOperation::~CAzOperation(void)
{
}
CAzOperation::CAzOperation(IAzOperation *pNative,bool pisNew):CAzBase<IAzOperation>(pNative,pisNew){
}

/*++

Routine description:

    This method copies properties from the source operation to *this* 
    operation

Arguments: srcOp - Source Operation

Return Value:

    Returns success, appropriate failure value 

--*/

HRESULT CAzOperation::Copy(CAzOperation &srcOp) {

    CAzLogging::Entering(_TEXT("Copy"));

    CComBSTR data;

    LONG lopID;

    HRESULT hr;

    hr=srcOp.m_native->get_ApplicationData(&data);

    CAzLogging::Log(hr,_TEXT("Getting Application Data for Operation"),COLE2T(getName()));

    hr=m_native->put_ApplicationData(data);

    CAzLogging::Log(hr,_TEXT("Setting Application Data for Operation"),COLE2T(getName()));

    data.Empty();

    hr=srcOp.m_native->get_OperationID(&lopID);

    CAzLogging::Log(hr,_TEXT("Getting OperationID for Operation"),COLE2T(getName()));

    hr=m_native->put_OperationID(lopID);

    CAzLogging::Log(hr,_TEXT("Setting Operation ID for Operation"),COLE2T(getName()));

    hr=srcOp.m_native->get_Description(&data);

    CAzLogging::Log(hr,_TEXT("Getting Description for Operation"),COLE2T(getName()));

    hr=m_native->put_Description(data);

    CAzLogging::Log(hr,_TEXT("Setting Description for Operation"),COLE2T(getName()));

    hr=m_native->Submit(0,CComVariant());

    CAzLogging::Log(hr,_TEXT("Submitting changes for Operation"),COLE2T(getName()));

    CAzLogging::Exiting(_TEXT("Copy"));

    return hr;
}
