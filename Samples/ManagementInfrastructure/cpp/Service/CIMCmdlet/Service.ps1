
#Find Classes 
Get-CimClass -Namespace root/StandardCimv2/sample
Get-CimClass -Namespace root/StandardCimv2/sample -ClassName *Service


#Enumerate instances
Get-CimInstance -ClassName MSFT_WindowsService -Namespace root/StandardCimv2/sample

#Enumerate instances with keys only 
Get-CimInstance -ClassName MSFT_WindowsService -Namespace root/StandardCimv2/sample -KeyOnly

#Query Instances
Get-CimInstance -query 'Select * from MSFT_WindowsService Where name like "win%"' -Namespace root/StandardCimv2/sample  

#Query Instances with filter
Get-CimInstance -ClassName MSFT_WindowsService -Namespace root/StandardCimv2/sample -Filter 'name like "win%"' | Select Name, Status

#Get all Instances of MSFT_WindowsService
$a,$b = Get-CimInstance -ClassName MSFT_WindowsService -Namespace root/StandardCimv2/sample
#Get MSFT_WindowsService instance with specific key property
Get-CimInstance -InputObject $a

#Get cimclass
$a = Get-CimInstance -ClassName MSFT_WindowsService -Namespace root/StandardCimv2/sample -Filter 'name like "plug%"' 
$a.CimClass.CimClassMethods

#Invoking StartService method
$a | Select Status, Started
Invoke-CimMethod -InputObject $a -MethodName StartService -verbose
Get-CimInstance $a | Select Status, Started

#Invoking StopService method
Invoke-CimMethod -InputObject $a -MethodName StopService -verbose
Get-CimInstance $a | Select Status, Started

#Invoking GetWindowsServices method of class msft_windowsservicemanager
$x=Invoke-CimMethod -MethodName GetWindowsServices -ClassName msft_windowsservicemanager -Arguments @{Status=[uint32]1} -Namespace root/standardcimv2/sample
$x[0].ItemValue

#Service sample provider does not support Delete/Create/Modify operation for MSFT_WindowsService class