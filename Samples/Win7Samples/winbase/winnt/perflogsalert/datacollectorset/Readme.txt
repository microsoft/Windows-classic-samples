What is it?
==========
Before Vista, a user had to use tools (e.g., Perfmon, Logman) in order to collect performance data. 
Vista introduces a new API, called Performance Logs & Alerts (PLA), which allows you to programmatically collect
performance data. This sample demonstrates how PLA API can help you log performance data. PLA supports collection 
of the following types of performance data.

	- Counter data
	- Trace data
	- Alert
	- Configuration data

This particular sample code collects the following data:

	- Counter data: \\Processor(_Total)\\% Processor Time, \\Processor(_Total)\\% User Time

	- Trace data: a trace session is created but no trace provider data is added into it.

	- Alert: If CPU usage goes above 90% (sample interval 5 seconds), it writes an event to the event log

	- Configuration data: it collects networking interface information and the following registry key values:
		"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\CurrentBuildNumber",
		"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\RegisteredOwner"


Files
=====
Readme.txt:		This file

Makefile:		Make file

Datacollectorset.cpp:	A sample code for PLA data collection. 

Datacollectorset.sln:	Visual studio solution file

Datacollectorset.vcproj:Visual studio project file.


How to Build
============

Under the same directory where the sample code is present, 

	type "nmake"


How to Run
==========


Datacollectset [<create|delete|start|stop|query> <data collector name>]

create:		Create a new data collector set. The created data collect set contains a counter, a trace, an alert, 
		and a configuration data collector.
Delete:		Delete the specified data collector set
Start:		Start the specified data collector set
Stop:		Stop the specified data collector set
Query:		Query a list of current data collector sets (running, stopped)

For example,

	Datacollectset create my_collector_set  	:Create a data collector set, called "my_collector_set"
	Datacollectset start my_collector_set		:Start collection. Log files are created under
							%systemdrive%\Perflogs\my_collector_set\

