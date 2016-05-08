/*++
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 Copyright (c) Microsoft Corporation. All rights reserved.

Abstract:

	This sample demonstrates how PLA API can programmatically collect performance data. 

Environment:

    User mode

--*/


#include <windows.h>
#include <stdio.h>
#include <ole2.h>
#include <pla.h>

#define CHECK_HR(v) if (FAILED(v)) { goto Exit; }
#define CHECK_MEMORY(p) if (NULL == (p)) { hr = E_OUTOFMEMORY; goto Exit; }
#define RELEASE(p) if (NULL != (p)) { (p)->Release(); (p) = NULL; }
#define FREE_SAFEARRAY(s) if (NULL != (s)) { SafeArrayDestroy(s); (s) = NULL; }
#define FREE_BSTR(bstr) { SysFreeString(bstr); (bstr) = NULL; }

PCWSTR TypeNames[] = { L"Performance Counters", L"Trace", L"Configuration", L"Alert" };
PCWSTR StatusNames[] = { L"Stopped", L"Running", L"Compiling", L"Pending" };

HRESULT
CreateArray(
    SAFEARRAY **OutArray,
    ULONG     Count,
    ...
    )
/*++

Routine Description:

    This function creates a safearray of BSTRs with the given strings.

Arguments:

    OutArray - Pointer that will point to the 

    Count - Number of strings that will be the array.

    ... - Strings to be added to the array.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT   hr;
    ULONG     i;
    PCWSTR    String;
    va_list   va_marker;    
    SAFEARRAY *Array = NULL;
    BSTR      *Data  = NULL;

    Array = SafeArrayCreateVector(VT_BSTR, 0, Count);
    CHECK_MEMORY(Array);

    hr = SafeArrayAccessData(Array, (PVOID*)&Data);
    CHECK_HR(hr);

    va_start(va_marker, Count);

    for (i = 0; i < Count; i++) {
        String = va_arg(va_marker, PCWSTR);

        Data[i] = SysAllocString(String);
        CHECK_MEMORY(Data[i]);
    }

    va_end(va_marker);

    *OutArray = Array;

Exit:
    if (NULL != Data) {
        SafeArrayUnaccessData(Array);
    }

    if (FAILED(hr) && NULL != Array) {
        SafeArrayDestroy(Array);
    }

    return hr;
}

HRESULT
PrintDataCollectors(
    IDataCollectorSet *CollectorSet
    )
/*++

Routine Description:

    Prints all the Data Collectors (DC) on screen that belong
    to the given data collector set.

Arguments:

    CollectorSet - Data Collector Set (DCS) that will be printed.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT                  hr;
    LONG                     Count;
    DataCollectorType        Type;
    BSTR                     Name        = NULL;
    IDataCollectorCollection *Collectors = NULL;
    IDataCollector           *Collector  = NULL;
    VARIANT                  Index;

    hr = CollectorSet->get_DataCollectors(&Collectors);
    CHECK_HR(hr);

    hr = Collectors->get_Count(&Count);
    CHECK_HR(hr);

    Index.vt = VT_I4;
    for (Index.lVal = 0; Index.lVal < Count; Index.lVal++) {
        RELEASE(Collector);
        hr = Collectors->get_Item(Index, &Collector);
        CHECK_HR(hr);

        //
        // Get DC's name and type, and print them
        //
        FREE_BSTR(Name);
        hr = Collector->get_Name(&Name);
        CHECK_HR(hr);

        hr = Collector->get_DataCollectorType(&Type);
        CHECK_HR(hr);

        wprintf(L"  DC name: %s (%s)\n", Name, TypeNames[Type]);
    }

Exit:
    FREE_BSTR(Name);
    RELEASE(Collector);
    RELEASE(Collectors);

    return hr;
}

HRESULT
CreatePerformanceCounterCollector(
    IDataCollectorCollection *Collectors
    )
/*++

Routine Description:

    Creates a performance counter data collector, sets some of its properties
    and adds it to data collector collection provided.

Arguments:

    Collectors - Data collector collection where a new DC will be added.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT                          hr;
    SAFEARRAY                        *Counters  = NULL;
    IDataCollector                   *Collector = NULL;
    IPerformanceCounterDataCollector *PerfCollector = NULL;

    hr = Collectors->CreateDataCollector(plaPerformanceCounter, &Collector);
    CHECK_HR(hr);

    hr = Collector->QueryInterface(IID_IPerformanceCounterDataCollector, (VOID**)&PerfCollector);
    CHECK_HR(hr);

    //
    // Add two counters to the list to be collected
    //
    hr = CreateArray(&Counters,
                     2,
                     L"\\Processor(_Total)\\% Processor Time",
                     L"\\Processor(_Total)\\% User Time");
    CHECK_HR(hr);

    hr = PerfCollector->put_PerformanceCounters(Counters);
    CHECK_HR(hr);

    //
    // Add the collector to the list of collectors
    //
    hr = Collectors->Add(PerfCollector);
    CHECK_HR(hr);

Exit:

    FREE_SAFEARRAY(Counters);
    
    RELEASE(Collector);
    RELEASE(PerfCollector);

    return hr;
}

HRESULT
CreateTraceCollector(
    IDataCollectorCollection *Collectors
    )
/*++

Routine Description:

    Creates a trace data collector, sets some of its properties
    and adds it to data collector collection provided.

Arguments:

    Collectors - Data collector collection where a new DC will be added.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT             hr;
    IDataCollector      *Collector = NULL;
    ITraceDataCollector *TraceCollector = NULL;

    hr = Collectors->CreateDataCollector(plaTrace, &Collector);
    CHECK_HR(hr);

    hr = Collector->QueryInterface(IID_ITraceDataCollector, (VOID**)&TraceCollector);
    CHECK_HR(hr);

    hr = Collectors->Add(TraceCollector);
    CHECK_HR(hr);

Exit:

    RELEASE(Collector);
    RELEASE(TraceCollector);

    return hr;
}

HRESULT
CreateConfigurationCollector(
    IDataCollectorCollection *Collectors
    )
/*++

Routine Description:

    Creates a configuration data collector, sets some of its properties
    and adds it to data collector collection provided.

Arguments:

    Collectors - Data collector collection where a new DC will be added.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT                     hr;
    SAFEARRAY                   *Keys      = NULL;
    IDataCollector              *Collector = NULL;
    IConfigurationDataCollector *ConfigCollector = NULL;

    hr = Collectors->CreateDataCollector(plaConfiguration, &Collector);
    CHECK_HR(hr);

    hr = Collector->QueryInterface(IID_IConfigurationDataCollector, (VOID**)&ConfigCollector);
    CHECK_HR(hr);

    //
    // We want to save networking information plus a couple of reg keys
    //
    hr = ConfigCollector->put_QueryNetworkAdapters(VARIANT_TRUE);
    CHECK_HR(hr);

    hr = CreateArray(&Keys,
                     2,
                     L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\CurrentBuildNumber",
                     L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\RegisteredOwner");
    CHECK_HR(hr);

    hr = ConfigCollector->put_RegistryKeys(Keys);
    CHECK_HR(hr);

    //
    // Add the collector to the list of collectors
    //
    hr = Collectors->Add(ConfigCollector);
    CHECK_HR(hr);

Exit:

    FREE_SAFEARRAY(Keys);

    RELEASE(Collector);
    RELEASE(ConfigCollector);

    return hr;
}

HRESULT
CreateAlertCollector(
    IDataCollectorCollection *Collectors
    )
/*++

Routine Description:

    Creates an alert data collector, sets some of its properties
    and adds it to data collector collection provided.

Arguments:

    Collectors - Data collector collection where a new DC will be added.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT             hr;
    SAFEARRAY           *Thresholds = NULL;
    IDataCollector      *Collector  = NULL;
    IAlertDataCollector *AlertCollector  = NULL;

    hr = Collectors->CreateDataCollector(plaAlert, &Collector);
    CHECK_HR(hr);

    hr = Collector->QueryInterface(IID_IAlertDataCollector, (VOID**)&AlertCollector);
    CHECK_HR(hr);

    //
    // We want to write an event to the Event Log if CPU usage goes above 90%,
    // with a sample interval of 5 seconds
    //
    hr = AlertCollector->put_EventLog(VARIANT_TRUE);
    CHECK_HR(hr);

    hr = AlertCollector->put_SampleInterval(5);
    CHECK_HR(hr);

    hr = CreateArray(&Thresholds,
                     1,
                     L"\\Processor(_Total)\\% Processor Time>90");
    CHECK_HR(hr);

    hr = AlertCollector->put_AlertThresholds(Thresholds);
    CHECK_HR(hr);

    //
    // Add the collector to the list of collectors
    //
    hr = Collectors->Add(AlertCollector);
    CHECK_HR(hr);

Exit:

    FREE_SAFEARRAY(Thresholds);

    RELEASE(Collector);
    RELEASE(AlertCollector);

    return hr;
}

HRESULT
CreateCollectorSet(
    BSTR Name
    )
/*++

Routine Description:

    Creates a data collector set, adds some data collectors to it and
    saves it with the given name.

Arguments:

    Name - Name to save data collector set as.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT                  hr;
    IDataCollectorSet        *CollectorSet = NULL;
    IDataCollectorCollection *Collectors   = NULL;
    IValueMap                *Validation   = NULL;

    hr = CoCreateInstance(CLSID_DataCollectorSet,
                          NULL,
                          CLSCTX_SERVER,
                          IID_IDataCollectorSet,
                          (PVOID*)&CollectorSet);
    CHECK_HR(hr);

    //
    // Put in a subdirectory named after the DCS name
    //
    hr = CollectorSet->put_Subdirectory(Name);
    CHECK_HR(hr);

    //
    // Create data collectors
    //
    hr = CollectorSet->get_DataCollectors(&Collectors);
    CHECK_HR(hr);

    hr = CreatePerformanceCounterCollector(Collectors);
    CHECK_HR(hr);

    hr = CreateTraceCollector(Collectors);
    CHECK_HR(hr);

    hr = CreateAlertCollector(Collectors);
    CHECK_HR(hr);

    hr = CreateConfigurationCollector(Collectors);
    CHECK_HR(hr);

    //
    // Save DCS
    //
    hr = CollectorSet->Commit(Name, NULL, plaCreateNew, &Validation);
    CHECK_HR(hr);

Exit:

    RELEASE(Validation);
    RELEASE(Collectors);
    RELEASE(CollectorSet);

    return hr;
}

HRESULT
DeleteCollectorSet(
    BSTR Name
    )
/*++

Routine Description:

    Deletes the data collector set with the given name.

Arguments:

    Name - Name of the data collector set to be deleted.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT           hr;
    IDataCollectorSet *CollectorSet = NULL;

    hr = CoCreateInstance(CLSID_DataCollectorSet,
                          NULL,
                          CLSCTX_SERVER,
                          IID_IDataCollectorSet,
                          (PVOID*)&CollectorSet);
    CHECK_HR(hr);

    hr = CollectorSet->Query(Name, NULL);
    CHECK_HR(hr);

    hr = CollectorSet->Delete();
    CHECK_HR(hr);

Exit:
    
    RELEASE(CollectorSet);

    return hr;
}

HRESULT
StartCollectorSet(
    BSTR Name
    )
/*++

Routine Description:

    Starts the data collector set with the given name.

Arguments:

    Name - Name of the data collector set to be started.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT           hr;
    IDataCollectorSet *CollectorSet = NULL;

    hr = CoCreateInstance(CLSID_DataCollectorSet,
                          NULL,
                          CLSCTX_SERVER,
                          IID_IDataCollectorSet,
                          (PVOID*)&CollectorSet);
    CHECK_HR(hr);

    hr = CollectorSet->Query(Name, NULL);
    CHECK_HR(hr);

    hr = CollectorSet->Start( VARIANT_TRUE );
    CHECK_HR(hr);

Exit:
    
    RELEASE(CollectorSet);

    return hr;
}

HRESULT
StopCollectorSet(
    BSTR Name
    )
/*++

Routine Description:

    Stops the data collector set with the given name.

Arguments:

    Name - Name of the data collector set to be stopped.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT           hr;
    IDataCollectorSet *CollectorSet = NULL;

    hr = CoCreateInstance(CLSID_DataCollectorSet,
                          NULL,
                          CLSCTX_SERVER,
                          IID_IDataCollectorSet,
                          (PVOID*)&CollectorSet);
    CHECK_HR(hr);

    hr = CollectorSet->Query(Name, NULL);
    CHECK_HR(hr);

    hr = CollectorSet->Stop( VARIANT_TRUE );
    CHECK_HR(hr);

Exit:
    
    RELEASE(CollectorSet);

    return hr;
}

HRESULT
QueryCollectorSet(
    BSTR Name
    )
/*++

Routine Description:

    Queries the given data collector set name and prints its name
    and list of data collectors.

Arguments:

    Name - Name of the data collector set to be queried.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT                hr;
    DataCollectorSetStatus Status;
    BSTR                   Name2         = NULL;
    IDataCollectorSet      *CollectorSet = NULL;

    hr = CoCreateInstance(CLSID_DataCollectorSet,
                          NULL,
                          CLSCTX_SERVER,
                          IID_IDataCollectorSet,
                          (PVOID*)&CollectorSet);
    CHECK_HR(hr);

    hr = CollectorSet->Query(Name, NULL);
    CHECK_HR(hr);

    hr = CollectorSet->get_Name(&Name2);
    CHECK_HR(hr);

    hr = CollectorSet->get_Status(&Status);
    CHECK_HR(hr);

    wprintf(L"Name: %s\nStatus: %s\n", Name2, StatusNames[Status]);
    
    hr = PrintDataCollectors(CollectorSet);
    CHECK_HR(hr);

Exit:

    FREE_BSTR(Name2);
    RELEASE(CollectorSet);

    return hr;
}

HRESULT
ListCollectorSets()
/*++

Routine Description:

    Lists all data collector sets and their status.

Return Value:

    Standard HRESULT.

--*/
{
    HRESULT                     hr;
    DataCollectorSetStatus      Status;
    LONG                        Count;
    BSTR                        Name           = NULL;
    BSTR                        Filter         = NULL;
    IDataCollectorSetCollection *CollectorSets = NULL;
    IDataCollectorSet           *CollectorSet  = NULL;
    VARIANT                     Index;

    hr = CoCreateInstance(CLSID_DataCollectorSetCollection,
                          NULL,
                          CLSCTX_SERVER,
                          IID_IDataCollectorSetCollection,
                          (PVOID*)&CollectorSets);
    CHECK_HR(hr);

    //
    // Get data collector sets and iterate through them
    //
    Filter = SysAllocString(L"service\\*");
    CHECK_MEMORY(Filter);

    hr = CollectorSets->GetDataCollectorSets(NULL, Filter);
    CHECK_HR(hr);

    hr = CollectorSets->get_Count(&Count);
    CHECK_HR(hr);

    Index.vt = VT_I4;
    for (Index.lVal = 0; Index.lVal < Count; Index.lVal++) {
        RELEASE(CollectorSet);
        hr = CollectorSets->get_Item(Index, &CollectorSet);
        CHECK_HR(hr);

        FREE_BSTR(Name);
        hr = CollectorSet->get_Name(&Name);
        CHECK_HR(hr);

        hr = CollectorSet->get_Status(&Status);
        CHECK_HR(hr);

        wprintf(L"[%7s] %s\n", StatusNames[Status], Name);
    }

Exit:

    FREE_BSTR(Name);
    FREE_BSTR(Filter);

    RELEASE(CollectorSet);
    RELEASE(CollectorSets);

    return hr;
}

int __cdecl wmain(int argc, WCHAR **argv)
{
    HRESULT hr;
    BSTR    Name = NULL;

    //
    // Check the argument count and initialize COM
    //
    if (3 != argc && 1 != argc) {
        fwprintf(stderr, 
                 L"Usage: %s [<create|delete|start|stop|query> <name>]\n",
                 argv[0]);
        return -1;
    }

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        fwprintf(stderr, L"CoInitializeEx failed: 0x%x\n", hr);
        return hr;
    }

    if (1 == argc) {
        //
        // No arguments, just list the DCSs
        //
        hr = ListCollectorSets();
        CHECK_HR(hr);
    } else {
        //
        // Create BSTR name and call worker function based on the arguments
        //
        Name = SysAllocString(argv[2]);
        CHECK_MEMORY(Name);

        if (0 == _wcsicmp(L"create", argv[1])) {
            hr = CreateCollectorSet(Name);
        } else if (0 == _wcsicmp(L"delete", argv[1])) {
            hr = DeleteCollectorSet(Name);
        } else if (0 == _wcsicmp(L"start", argv[1])) {
            hr = StartCollectorSet(Name);
        } else if (0 == _wcsicmp(L"stop", argv[1])) {
            hr = StopCollectorSet(Name);
        } else if (0 == _wcsicmp(L"query", argv[1])) {
            hr = QueryCollectorSet(Name);
        }
        CHECK_HR(hr);
    }

Exit:

    SysFreeString(Name);

    CoUninitialize();

    if (FAILED(hr)) {
        fwprintf(stderr, L"\nError: 0x%x.\n", hr);
    } else {
        wprintf(L"\nSuccess.\n");
    }

    return hr;
}
