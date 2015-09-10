
#Find Classes 
Get-CimClass -Namespace root/StandardCimv2/sample
Get-CimClass -Namespace root/StandardCimv2/sample -ClassName MSFT*

#Enumerate instances
Get-CimInstance -ClassName MSFT_WindowsProcess -Namespace root/StandardCimv2/sample
#Enumerate instances with keys only 
Get-CimInstance -ClassName MSFT_WindowsProcess -Namespace root/StandardCimv2/sample -KeyOnly

#Query Instances
Get-CimInstance -query 'Select * from MSFT_WindowsProcess Where name like "svc%"' -Namespace root/StandardCimv2/sample  
#Query Instances with filter
Get-CimInstance -ClassName MSFT_WindowsProcess -Namespace root/StandardCimv2/sample -Filter  'name like "s%"' | Select Name

#Get Instance
$a,$b = Get-CimInstance -ClassName MSFT_WindowsProcess -Namespace root/StandardCimv2/sample
Get-CimInstance -InputObject $a

#Modify Instance
$a |  Select Name, Caption, Priority
Set-CimInstance -InputObject $a -Property @{Priority=128}
Get-CimInstance $a | Select Name, Priority

#Setting other properties should fail because process provider does not support modifying properties other than Priority
Set-CimInstance -InputObject $a -Property @{Caption="abcd"}

#Create Instance
#Using CimClass to perform better client side validation
$c = Get-CimClass -Namespace root/StandardCimv2/sample -ClassName MSFT_WindowsProcess
New-CimInstance -CimClass $c -Property @{CommandLine="notepad.exe"}

#Using className and namespace, validation will be done on the provider side
New-CimInstance -ClassName MSFT_WindowsProcess -namespace root/StandardCimv2/sample  -Property @{CommandLine="notepad.exe"}

#Invoking method
$c.CimClassMethods
Invoke-CimMethod -ClassName MSFT_WindowsProcess -namespace root/StandardCimv2/sample -MethodName Create -Arguments @{CommandLine="calc.exe"}

Invoke-CimMethod -InputObject $a -MethodName SetPriority -Arguments @{Priority=32}


#Deleting instance
calc
$a = Get-CimInstance -query 'Select * from MSFT_WindowsProcess Where name like "calc.exe"' -Namespace root/StandardCimv2/sample  
$a|Remove-CimInstance


##Working with associations

#Look for association classes in the namespace
Get-CimClass -Namespace root/StandardCimv2/sample -QualifierName Association

$a = Get-CimInstance -ClassName MSFT_WindowsService -Namespace root/StandardCimv2/sample -Filter  'name like "winrm%"' 
Get-CimAssociatedInstance -input $a 

#Getting associated instances of a specific type
Get-CimAssociatedInstance -input $a -ResultClassName MSFT_WindowsProcess

#Getting  instances through a specific association
Get-CimAssociatedInstance -input $a -Association MSFT_WindowsServiceProcess

#Getting services associated with process
$b = Get-CimInstance -ClassName MSFT_WindowsProcess -Namespace root/StandardCimv2/sample -Filter  'name like "svc%"'  
Get-CimAssociatedInstance -InputObject $b[0] | select name
