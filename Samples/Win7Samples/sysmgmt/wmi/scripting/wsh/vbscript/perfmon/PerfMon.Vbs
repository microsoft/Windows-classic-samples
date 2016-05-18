' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'
' This sample illustrates how to retrieve perfmon counter values from WMI.  Note that the accompanying MOF
' (perfmon.mof) must be compiled and loaded for this sample to run correctly.
'

set processes = GetObject("winmgmts:root/perfmonScriptExample").InstancesOf ("NTProcesses")

WScript.Echo "Displaying Process Counters"
WScript.Echo "==========================="
WScript.Echo

for each process in processes
	WScript.Echo "  " & process.Process & ": #Threads=" & process.Threads & " Working Set=" & process.WorkingSet
	WScript.Echo
next


WScript.Echo "Displaying Memory Counters"
WScript.Echo "=========================="
WScript.Echo

set memorySet = GetObject("winmgmts:root/perfmonScriptExample").InstancesOf ("NTMemory")

for each memoryItem in memorySet
	WScript.Echo "  " & memoryItem.Memory & ": Committed Bytes=" & memoryItem.CommittedBytes
next