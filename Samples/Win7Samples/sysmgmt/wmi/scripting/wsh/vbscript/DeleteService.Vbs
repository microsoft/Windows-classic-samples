' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Delete a service (VBScript)
'
' This script demonstrates how to delete a specific service from instances of
' Win32_Service.
'
' NOTE:  This script only applies to NT-based systems since Win9x does support services
'
' NOTE:  Deleting a service is a permanent change to the system and this script should
'        be used with caution.  To help avoid users from accidently deleting a useful
'        service, the script has been created with the string "MyService" as the name of
'        the service to be deleted.  Change this to the name of the service you wish to 
'        delete but be sure you wish to permanently remove the service before using this 
'        script.
'
'
'
'***************************************************************************

Set ServiceSet = GetObject("winmgmts:").ExecQuery("select * from Win32_Service where Name='MyService'")

for each Service in ServiceSet
	RetVal = Service.Delete()
	if RetVal = 0 then 
		WScript.Echo "Service deleted" 
	else  
		WScript.Echo "Delete failed: " & RetVal
 	end if
next
