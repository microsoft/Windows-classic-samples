// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


// FSRM_Get_Enum_Properties.cpp : Console application to get/enum FSRM properties
//

#include "stdafx.h"
#include "FSRM_Get_Enum_Properties.h"
#include "fsrm.h"
#include "fsrmenums.h"
#include "fsrmpipeline.h"
#include "fsrmtlb.h"
#include "comutil.h"
#include <crtdbg.h>
#include <winerror.h>
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <iostream>
#include <vector>
using namespace std;


/*++

    Routine GetValues

Description:

    This routine is called to convert a SAFEARRAY of strings to a vector of strings
    
Arguments:

    ValueList - The SAFEARRAY of strings to be converted
    Values    - The out parameter containing the vector of strings

Return value:

    HRESULT

Notes:
    

--*/
HRESULT GetValues(  
    SAFEARRAY           *const  ValueList,
    vector <wstring>            &Values )
{
    HRESULT hr = S_OK;
    long lowerBound = 0;
       long upperBound = 0;

    //Go through SAFEARRAY and populate the vector

       //Get the lower bound of the safearray
    hr = SafeArrayGetLBound (ValueList, 1, &lowerBound);
    if(FAILED(hr))
    {
        goto exit;
    }

       //Get the upper bound of the safearray
    hr = SafeArrayGetUBound (ValueList, 1, &upperBound);
    if(FAILED(hr))
    {
        goto exit;
    }

       //Loop through the safe array and populate the vector
    for (long i = lowerBound; i <= upperBound; i++) 
    {
        //Convert each element into a wstring and save it to the vector
        _variant_t var;
        VariantInit(&var);
        hr = SafeArrayGetElement(ValueList, &i, &var);
        if(FAILED(hr))
        {
            goto exit;
        }
        wstring value = OLE2CW(var.bstrVal);
        Values.push_back(value);
    }

exit:
    
    return hr;
}

/*++

    Routine DisplayPropertyDefinition

Description:

    This routine prints a property definition along with its possible values
    
Arguments:

    PropertyDefinition - The FSRM property definition to be printed

Return value:

    HRESULT

Notes:
    

--*/

HRESULT DisplayPropertyDefinition(
    const CComPtr<IFsrmPropertyDefinition>  & PropertyDefinition
    )
{
    CComBSTR        description;
    CComBSTR        name;
    SAFEARRAY *     list;
    vector<wstring> possibleValues;
    vector<wstring> valueDescriptions;

    FsrmPropertyDefinitionType type;
    HRESULT hr=S_OK;

    // get the members of the PropertyDefinition
    
    hr = PropertyDefinition->get_Description(&description);
    if(FAILED(hr)){
        goto exit;
    }
    hr = PropertyDefinition->get_Name(&name);
    if(FAILED(hr)){
        goto exit;
    }
    hr = PropertyDefinition->get_Type(&type);
    if(FAILED(hr)){
        goto exit;
    }

    hr = PropertyDefinition->get_PossibleValues(&list);
    if(FAILED(hr)){
        goto exit;
    }
    hr = GetValues(list,possibleValues);
    if(FAILED(hr)){
        goto exit;
    }

    hr = PropertyDefinition->get_ValueDescriptions(&list);
    if(FAILED(hr)){
        goto exit;
    }
    hr = GetValues(list,valueDescriptions);
    if(FAILED(hr)){
        goto exit;
    }


    //getting all the values will have succeeded
    //print all values

    printf("\tName:\t\t%ws\n",OLE2CW(name.m_str));
    printf("\tDefinition:\t%ws\n",OLE2CW(description.m_str));
    printf("\tType:\t\t");
    switch(type){
        case FsrmPropertyDefinitionType_Unknown:
            printf("Unknown\n");
            break;

        case FsrmPropertyDefinitionType_OrderedList:
            printf("Ordered List\n");
            //Print the possible values and their descriptions
            for(unsigned int i=0;i<possibleValues.size() && valueDescriptions.size(); ++i){
                printf("\t\t%d Value:\t %ws\n",i+1,possibleValues[i].c_str());
                printf("\t\t%d Description:\t %ws\n",i+1,valueDescriptions[i].c_str());
            }
            break;

        case FsrmPropertyDefinitionType_MultiChoiceList:
            printf("Multichoice List\n");
            //Print the possible values and their descriptions
            for(unsigned int i=0;i<possibleValues.size() && valueDescriptions.size(); ++i){
                printf("\t\t%d Value:\t %ws\n",i+1,possibleValues[i].c_str());
                printf("\t\t%d Description:\t %ws\n",i+1,valueDescriptions[i].c_str());
            }
            break;

        case FsrmPropertyDefinitionType_Int:
            printf("Int\n");
            break;

        case FsrmPropertyDefinitionType_Bool:
            printf("Bool\n");
            break;

        case FsrmPropertyDefinitionType_Date:
            printf("Date\n");
            break;

        case FsrmPropertyDefinitionType_MultiString:
            printf("Multistring\n");
            break;

        case FsrmPropertyDefinitionType_String:
            printf("String\n");
            break;

        default:
            printf("Error type %d not defined\n",type);

    }

exit:

    return hr;

}

/*++

    Routine EnumerateProperties

Description:

    This routine enumerates the properties defined in FSRM
    
Arguments:

    FsrmManager - The FSRM classification manager object

Return value:

    HRESULT

Notes:
    

--*/

HRESULT EnumerateProperties(
    IFsrmClassificationManager* FsrmManager)
{
    HRESULT hr = S_OK;
    CComPtr<IFsrmCollection> fsrmCollection;
    long propertyCount = 0;
    
    // Get the collection of property definition
    hr = FsrmManager->EnumPropertyDefinitions(  FsrmEnumOptions_None,
                                                &fsrmCollection);
    if ( FAILED(hr)) {
         
        goto exit;
    }

    // loop over all the properties
    // printing each one

    hr = fsrmCollection->get_Count(&propertyCount);
    if (FAILED(hr)) {

        goto exit;
    }       
    for(long i = 0; i < propertyCount; ++i ) {

        // get the variant and convert it into a IFsrmPropertyDefinition
        CComPtr<IFsrmPropertyDefinition> propertyDefinition;
        _variant_t var;
        
        hr = fsrmCollection->get_Item(i+1, &var);

        if (FAILED(hr)) {

            goto exit;
        }

        hr = var.pdispVal->QueryInterface(  __uuidof(IFsrmPropertyDefinition),
                                            reinterpret_cast<void**>(&propertyDefinition));
        if (FAILED(hr)){

            goto exit;
        }

        printf("Property %d\n",1+i);
        DisplayPropertyDefinition(propertyDefinition);
        printf("\n");
    
    }

exit:

    return hr;
}

/*++

    Routine DisplayProperty

Description:

    This routine prints the FSRM property to the screen
    
Arguments:

    Property - The FSRM property object

Return value:

    HRESULT

Notes:
    

--*/

HRESULT DisplayProperty(
    const CComPtr<IFsrmProperty>  &Property)
{
    CComBSTR        description;
    CComBSTR        name;
    SAFEARRAY *     list;
    CComBSTR        value;
    vector<wstring> sources;

    long propertyFlag;
    HRESULT hr = S_OK;

    //Get the property's value
    hr = Property->get_Value(&value);
    if(FAILED(hr)){
        goto exit;
    }

    //Get the property's name
    hr = Property->get_Name(&name);
    if(FAILED(hr)){
        goto exit;
    }

    //Get the property flags    
    hr = Property->get_PropertyFlags(&propertyFlag);
    if(FAILED(hr)){
        goto exit;
    }

    //Get the sources that set the property
    hr = Property->get_Sources(&list);
    if(FAILED(hr)){
        goto exit;
    }

    //Convert the SAFEARRAY to a wstring vector
    hr = GetValues(list,sources);
    if(FAILED(hr)){
        goto exit;
    }
    
    //print the information
    
    printf("\tName:\t\t%ws\n",OLE2CW(name.m_str));
    printf("\tValue:\t\t%ws\n",OLE2CW(value.m_str));
    printf("\tProperty Flags:\t");

    if ( propertyFlag & FsrmPropertyFlags_Orphaned ) {
        printf("Orphaned ");
    }
    if ( propertyFlag & FsrmPropertyFlags_RetrievedFromCache ) {
        printf("RetrievedFromCache ");
    }
    if ( propertyFlag & FsrmPropertyFlags_RetrievedFromStorage ) {
        printf("RetrievedFromStorage ");
    }
    if ( propertyFlag & FsrmPropertyFlags_SetByClassifier ) {
        printf("SetByClassifier ");
    }
    if ( propertyFlag & FsrmPropertyFlags_Deleted ) {
        printf("Deleted ");
    }
    if ( propertyFlag & FsrmPropertyFlags_Reclassified ) {
        printf("Reclassified ");
    }
    if ( propertyFlag & FsrmPropertyFlags_AggregationFailed ) {
        printf("AggregationFailed");
    }
    printf("\n");

    printf("\tSources:\n");
    for(unsigned int i=0;i<sources.size(); ++i){
        printf("\t\t%d - %ws\n",i+1,sources[i].c_str());
    }

exit:

    return hr;

}

/*++

    Routine EnumerateFileProperties

Description:

    This routine enumerates and prints the FSRM properties on a file
    
Arguments:

    FilePath         - The file whose properties will be enumerated
    FsrmManager - The FSRM classification manager object used to lookup the file's properties

Return value:

    HRESULT

Notes:
    

--*/

HRESULT EnumerateFileProperties(
    CComBSTR                    &FilePath, 
    IFsrmClassificationManager  *FsrmManager)
{
    HRESULT hr = S_OK;
    CComPtr<IFsrmCollection> fsrmCollection;
    long propertyCount = 0;

    //Get the list of properties for a file
    hr = FsrmManager->EnumFileProperties(   FilePath, 
                                            FsrmGetFilePropertyOptions_None, 
                                            &fsrmCollection);
                                            
    //If failed, get the error info and print the message
    if ( FAILED(hr)) {

            CComPtr<IErrorInfo> spErrorInfo = NULL;
            CComBSTR spErrorMessage;

            if(SUCCEEDED(GetErrorInfo(0, &spErrorInfo)))
            {
                if (spErrorInfo)
                {
                    if (SUCCEEDED(spErrorInfo->GetDescription(&spErrorMessage)))
                    {
                        printf(
                            "The following error(s) occurred while enumerating properties:\n%ws\n",
                            !spErrorMessage ? L"No additional error information found" : spErrorMessage
                            );
                    }
                }
            }

        goto exit;
    }
    
    //Loop over all the properties and print each property


    //Get the count of properties on the file
    hr = fsrmCollection->get_Count(&propertyCount);
    if (FAILED(hr)) {

        goto exit;
    }
    for(long i = 0; i < propertyCount; ++i ) {

        //Get the variant and convert it into a IFsrmProperty

        
        CComPtr<IFsrmProperty> property_;
        _variant_t var;

        //Get the next property item
        hr = fsrmCollection->get_Item(i+1, &var);

        if (FAILED(hr)) {

            goto exit;
        }

        //Get the IFsrmProperty object on the property
        hr = var.pdispVal->QueryInterface(  __uuidof(IFsrmProperty),
                                            reinterpret_cast<void**>(&property_));
        if (FAILED(hr)){

            goto exit;
        }

        printf("Property %d\n",1+i);

        //Call the display property method to print the property
        DisplayProperty(property_);
        printf("\n");
    
    }

exit:

    return hr;
}

/*++

    Routine GetFileProperty

Description:

    This routine queries and prints a requested property from a file
    
Arguments:

    FilePath          - The file whose properties will be enumerated
    PropertyName - The FSRM property to retrieve from the list of properties set on the file
    FsrmManager  - The FSRM classification manager object used to lookup the file's properties

Return value:

    HRESULT

Notes:
    

--*/

HRESULT GetFileProperty(
    CComBSTR                    &FilePath,
    CComBSTR                    &PropertyName, 
    IFsrmClassificationManager  *FsrmManager)
{
    HRESULT hr = S_OK;
    CComPtr<IFsrmProperty> property_;
    long propertyCount = 0;

    //Call to get the requested property on the file
    hr = FsrmManager->GetFileProperty(  FilePath, 
                                        PropertyName, 
                                        FsrmGetFilePropertyOptions_None, 
                                        &property_);

    //If succeeded to get property, print it                                        
    if ( SUCCEEDED(hr)) {

        DisplayProperty(property_);

        }
    //If failed to get property, get and print the error
    else {     
    
        CComPtr<IErrorInfo> spErrorInfo = NULL;
        CComBSTR spErrorMessage;

        //Get the error that occured whle getting the file property
        if(SUCCEEDED(GetErrorInfo(0, &spErrorInfo)))
        {
            if (spErrorInfo)
            {
                //Get the error's description
                if (SUCCEEDED(spErrorInfo->GetDescription(&spErrorMessage)))
                {
                    printf(
                        "The following error(s) occurred while getting file properties:\n%ws\n",
                        !spErrorMessage ? L"No additional error information found" : spErrorMessage
                        );
                }
            }
        }
    }

    return hr;
}

/*++

    Routine WCharCaseInsensitiveCompare

Description:

    This routine compares to wchar_t values
    
Arguments:

    a   -   A wchar_t to compare
    b   -   A wchar_t to compare
    
Return value:

    bool
    
Notes:
    

--*/
inline bool WCharCaseInsensitiveCompare(wchar_t a, wchar_t b)
{
    return(towupper(a)==toupper(b));
}


/*++

    Routine StringCaseInsensitiveEqual

Description:

    This routine performs a case insensitive comparison on two wstring objects
    
Arguments:

    a   -   A wstring to compare
    b   -   A wstring to compare
    
Return value:

    bool
    
Notes:
    

--*/

inline bool const StringCaseInsensitiveEqual(std::wstring& a, const std::wstring& b)
{
    return (a.size() == b.size() && 
        equal(a.begin(),a.end(),b.begin(),WCharCaseInsensitiveCompare));    
}

/*++

    Routine DisplayUsage

Description:

    This routine prints the usage of this program
    
Arguments:

    void
    
Return value:

    void
    
Notes:
    

--*/
static void DisplayUsage()
{
    printf( "Sample Usage:\n" );
    printf( "\t FSRM_Get_Enum_Properties.exe -f c:\\foo\\cat.txt -p SomePropertyName -EnumerateProperties -GetFileProperty -EnumerateFileProperties\n" );
    printf( "\n" );
    printf( "You must specify atleast one of the following\n" );
    printf( "\t -EnumerateProperties, -EnumerateFileProperties, -GetFileProperty\n" );
    printf( "\n" );
    printf( "If specifying -EnumerateFileProperties\n" );
    printf( "\t The following must also be specified: \n" );
    printf( "\t\t -f <FilePath>\n" );
    printf( "If specifying -GetFileProperty\n" );
    printf( "\t The following must also be specified: \n" );
    printf( "\t\t -f <FilePath>\n" );
    printf( "\t\t -p <PropertyName>\n" );
    printf( "\n" );
}


/*++

    Routine _tmain

Description:

    main program routine
    
Arguments:

    argc - argument count
    argv - arguments
    
Return value:

    int
    
Notes:
    

--*/
int _tmain(int argc, _TCHAR* argv[])
{

    HRESULT hr = S_OK; 
    CComBSTR *filePath = NULL ;
    CComBSTR *propertyName = NULL;

    bool getFileProperty = false;
    bool enumerateFileProperties = false;
    bool enumerateProperties = false;

    //Parse command line arguments 

    for(int i=1;i<argc;i++){
        std::wstring input = std::wstring(argv[i]);

        //Get the file path 
        if(StringCaseInsensitiveEqual(input,L"-f") && i+1<argc){
            filePath = new CComBSTR(argv[i+1]);
            i++;
            continue;
        }

        //Get the property name to retrieve
        if(StringCaseInsensitiveEqual(input,L"-p") && i+1<argc){
            propertyName = new CComBSTR(argv[i+1]);
            i++;
            continue;
        }

        //Program must run GetFileProperty
        if(StringCaseInsensitiveEqual(input,L"-GetFileProperty")){
            getFileProperty = true;
        }

        //Program must EnumerateFileProperties
        if(StringCaseInsensitiveEqual(input,L"-EnumerateFileProperties")){
            enumerateFileProperties = true;
        }

        //Program must Enumerate FSRM properties
        if(StringCaseInsensitiveEqual(input,L"-EnumerateProperties")){
            enumerateProperties = true;
        }
    }

    if (!(getFileProperty || enumerateProperties || enumerateFileProperties )){

        DisplayUsage();
        return -1;
    }
    if (enumerateFileProperties && filePath == NULL) {

        DisplayUsage();
        return -1;
    }
    if (getFileProperty && (filePath == NULL || propertyName == NULL)) {

        DisplayUsage();
        return -1;
    }


    //Initialize COM
    hr = CoInitialize(0);


    if (SUCCEEDED(hr)) {
        {
            
            CComPtr<IFsrmClassificationManager> fsrmManager;

            //Create an instance of the FSRM Classification Manager
            hr = CoCreateInstance(
                __uuidof(FsrmClassificationManager), 
                NULL, 
                CLSCTX_SERVER,
                __uuidof(IFsrmClassificationManager),
                (void**) &fsrmManager);

            if ( SUCCEEDED(hr)) {

                if (enumerateProperties) {
                    printf( "Enumerating Properties\n" );

                    hr = EnumerateProperties(fsrmManager);

                    if(FAILED(hr)){
                        printf("Failed to enumerate properties with error 0x%x\n",hr);
                    }
                }
                if (enumerateFileProperties) {
                    printf( "Enumerating File Properties - %ws\n",(LPWSTR)*filePath );

                    hr = EnumerateFileProperties(*filePath,fsrmManager);

                    if(FAILED(hr)){
                        printf("Failed to get file properties of %ws with error 0x%x\n", 
                                (LPWSTR)*filePath, 
                                hr );
                    }
                }
                if (getFileProperty) {
                    printf( "Getting the %ws Property from file %ws\n",
                                (LPWSTR)*propertyName,
                                (LPWSTR)*filePath );

                    hr = GetFileProperty(*filePath,*propertyName,fsrmManager);

                    if(FAILED(hr)){
                        printf("Failed to get file property %ws of file %ws with error 0x%x\n",
                                (LPWSTR)*propertyName,
                                (LPWSTR)*filePath, 
                                hr );
                    }
                }

            }
        }
        //Uninitialize COM
        CoUninitialize();
    }

    delete(filePath);
    filePath = NULL;

    delete(propertyName);
    propertyName = NULL;

    return 0;
}
