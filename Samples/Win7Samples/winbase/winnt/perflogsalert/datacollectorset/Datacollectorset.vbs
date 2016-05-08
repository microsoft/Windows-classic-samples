TypeNames = Array("Performance Counters", "Trace", "Configuration", "Alert")
StatusNames = Array("Stopped  ", "Running  ", "Compiling", "Pending  ")

'/*++
'
'Routine Description:
'
'    Prints all the Data Collectors (DC) on screen that belong
'    to the given data collector set.
'
'Arguments:
'
'    CollectorSet - Data Collector Set (DCS) that will be printed.
'
'--*/
Sub PrintDataCollectors(CollectorSet) 
    For Each Collector in CollectorSet.DataCollectors
        WScript.Echo "  DC Name: " & Collector.Name & " (" & TypeNames(Collector.DataCollectorType) & ")"
    Next
End Sub

'/*++
'
'Routine Description:
'
'    Creates a performance counter data collector, sets some of its properties
'    and adds it to data collector collection provided.
'
'Arguments:
'
'    Collectors - Data collector collection where a new DC will be added.
'
'--*/
Sub CreatePerformanceCounterCollector(Collectors)
    Set Collector = Collectors.CreateDataCollector(0)

    Collector.PerformanceCounters = Array("\Processor(_Total)\% Processor Time", "\Processor(_Total)\% User Time")

    Collectors.Add Collector
End Sub

'/*++
'
'Routine Description:
'
'    Creates a trace data collector, sets some of its properties
'    and adds it to data collector collection provided.
'
'Arguments:
'
'    Collectors - Data collector collection where a new DC will be added.
'
'--*/
Sub CreateTraceCollector(Collectors)
    Set Collector = Collectors.CreateDataCollector(1)

    Collectors.Add Collector
End Sub

'/*++
'
'Routine Description:
'
'    Creates a configuration data collector, sets some of its properties
'    and adds it to data collector collection provided.
'
'Arguments:
'
'    Collectors - Data collector collection where a new DC will be added.
'
'--*/
Sub CreateConfigurationCollector(Collectors)
    Set Collector = Collectors.CreateDataCollector(2)

    Collector.QueryNetworkAdapters = true
    Collector.RegistryKeys = Array("HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\CurrentBuildNumber", "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\RegisteredOwner")

    Collectors.Add Collector
End Sub

'/*++
'
'Routine Description:
'
'    Creates an alert data collector, sets some of its properties
'    and adds it to data collector collection provided.
'
'Arguments:
'
'    Collectors - Data collector collection where a new DC will be added.
'
'--*/
Sub CreateAlertCollector(Collectors)
    Set Collector = Collectors.CreateDataCollector(3)

    Collector.EventLog = true
    Collector.SampleInterval = 5
    Collector.AlertThresholds = Array("\Processor(_Total)\% Processor Time>90")

    Collectors.Add Collector
End Sub


'/*++
'
'Routine Description:
'
'    Creates a data collector set, adds some data collectors to it and
'    saves it with the given name.
'
'Arguments:
'
'    Name - Name to save data collector set as.
'
'--*/
Sub CreateCollectorSet(Name)
    Set CollectorSet = CreateObject("PLA.DataCollectorSet")

    '
    ' Put in a subdirectory named after the DCS name
    '
    CollectorSet.Subdirectory = Name

    '
    ' Create data collectors
    '
    CreatePerformanceCounterCollector CollectorSet.DataCollectors
    CreateTraceCollector CollectorSet.DataCollectors
    CreateAlertCollector CollectorSet.DataCollectors
    CreateConfigurationCollector CollectorSet.DataCollectors

    '
    ' Save DCS
    '
    CollectorSet.Commit Name, "", 0
End SUb

'/*++
'
'Routine Description:
'
'    Deletes the data collector set with the given name.
'
'Arguments:
'
'    Name - Name of the data collector set to be deleted.
'
'--*/
Sub DeleteCollectorSet(Name)
    Set CollectorSet = CreateObject("PLA.DataCollectorSet")

    CollectorSet.Query Name, ""
    CollectorSet.Delete
End Sub

'/*++
'
'Routine Description:
'
'    Starts the data collector set with the given name.
'
'Arguments:
'
'    Name - Name of the data collector set to be started.
'
'--*/
Sub StartCollectorSet(Name)
    Set CollectorSet = CreateObject("PLA.DataCollectorSet")

    CollectorSet.Query Name, ""
    CollectorSet.Start true
End Sub

'/*++
'
'Routine Description:
'
'    Stops the data collector set with the given name.
'
'Arguments:
'
'    Name - Name of the data collector set to be stopped.
'
'--*/
Sub StopCollectorSet(Name)
    Set CollectorSet = CreateObject("PLA.DataCollectorSet")

    CollectorSet.Query Name, ""
    CollectorSet.Stop true
End Sub


'/*++
'
'Routine Description:
'
'    Queries the given data collector set name and prints its name
'    and list of data collectors.
'
'Arguments:
'
'    Name - Name of the data collector set to be queried.
'
'--*/
Sub QueryCollectorSet(Name)
    Set CollectorSet = CreateObject("PLA.DataCollectorSet")

    CollectorSet.Query Name, ""

    WScript.Echo "Name: " & CollectorSet.Name
    WScript.Echo "Status: " & StatusNames(CollectorSet.Status)

    PrintDataCollectors CollectorSet
End Sub


'/*++
'
'Routine Description:
'
'    Lists all data collector sets and their status.
'
'--*/
Sub ListCollectorSets
    Set CollectorSets = CreateObject("PLA.DataCollectorSetCollection")

    CollectorSets.GetDataCollectorSets "", "service\*"

    For Each CollectorSet in CollectorSets
        WScript.Echo "[" & StatusNames(CollectorSet.Status) & "] " & Collectorset.Name
    Next
End Sub

' Check the number of arguments is correct
if WScript.Arguments.Count <> 0 and WScript.Arguments.Count <> 2 Then
    WScript.Echo "Usage: sample.vbs [<create|delete|start|stop|query> <name>]"
    WScript.Quit
end if

' Display the DCSs and exit if no arguments were passed
if WScript.Arguments.Count = 0 Then
    ListCollectorSets
    WScript.Quit
end if

'Determine the verb
Select Case lcase(WScript.Arguments(0))
    case "create" CreateCollectorSet WScript.Arguments(1)
    case "delete" DeleteCollectorSet WScript.Arguments(1)
    case "start" StartCollectorSet WScript.Arguments(1)
    case "stop" StopCollectorSet WScript.Arguments(1)
    case "query" QueryCollectorSet WScript.Arguments(1)
    case else WScript.Echo "Invalid verb"
End Select
