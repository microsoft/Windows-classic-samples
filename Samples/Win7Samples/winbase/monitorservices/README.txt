Instructions for running the sample

1. Build the sample to generate the service executable. (MonSvc.exe) 
2. Create a folder %ProgramFiles%\MonSvc and copy the service executable there. 
3. Now install the service on the system as MonSvc. (See MSDN for guidelines on how to install a service). 
4. Under the parameters key of this service (HKLM\System\CurrentControlSet\Services\MonSvc\Parameters\) create a REG_SZ value called "SvcName" and set it to the name of the service you want to monitor. 
5. Start the MonSvc service. 
6. This service will now monitor the start and stop of the service you specified and log this information to %ProgramFiles%\MonSvc\SvcMonitor.log
