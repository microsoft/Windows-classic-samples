' ----------------------------------------------------------------------------
' 
' This file is part of the Microsoft COM+ Samples.
' 
' Copyright (C) 1995-2000 Microsoft Corporation. All rights reserved.
' 
' This source code is intended only as a supplement to Microsoft
' Development Tools and/or on-line documentation. See these other
' materials for detailed information regarding Microsoft code samples.
' 
' THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
' KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
' IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
' PARTICULAR PURPOSE.
' 
' ----------------------------------------------------------------------------

'****************************************************************************************************
'	Filename:		COMAdmin.vbs
'
'	Description:	VB Script that captures most of the functionality 
'					of the COM Explorer in a command line utility.
'	COMAdmin
'	[/add] [/delete] [/get] [/set]
'	[/server "servername"] [/localhost*]
'	[/application "applicationname"]
'	[/component "componentname"]
'	[/interface "interfacename"]
'	[/method "methodname"]
'	[/role "rolename"]
'	[/user "username"]
'
'	[/property "PropName_1;PropValue_1" "PropName_2;PropValue_2" . . . "PropName_n;PropValue_n"
'
'	[/applicationfile "applicationfile" "installpath"]
'	[/componentfile "componentfile" "type"] [/componentprogid "componentprogid"]
'****************************************************************************************************

'Get arguments
Dim objArgs
Set objArgs = WScript.Arguments

'Initialize local vars
Dim nextArg
Dim strTarget
Dim strTarget2
Dim strCommand
Dim strServerName
Dim applicationName
Dim componentName
Dim roleName
Dim userName
Dim interfaceName
Dim methodName
Dim applicationFile
Dim installPath
Dim componentFile
Dim componentFileType
Dim componentProgid
Dim propertyCheck
Dim PropArrayUBound
Dim PropPtr

nextArg = 0
strTarget = ""
strTarget2 = ""
strCommand = ""
strServerName = ""
applicationName = ""
componentName = ""
roleName = ""
userName = ""
interfaceName = ""
methodName = ""
applicationFile = ""
installPath = ""
componentFile = ""
componentFileType = 0
componentProgid = ""
propertyCheck = ""
PropArrayUBound = 0
PropPtr = 0

'Eliminate '/'from arguments and convert them to lowercase
Dim strArgs()
Dim caseArgs()
Dim origArgs()
ReDim strArgs(2 * objArgs.Count)
ReDim caseArgs(2 * objArgs.Count)
ReDim origArgs(2 * objArgs.Count)

'multiple properties
Dim strPropNames()
Dim strPropValues()

For i = 0 To objArgs.Count - 1
    If Left(objArgs(i), 1) = "/" Then
        caseArgs(i) = Right(objArgs(i), Len(objArgs(i)) - 1)
        strArgs(i) = LCase(caseArgs(i))
        origArgs(i) = objArgs(i)
    Else
        caseArgs(i) = objArgs(i)
        strArgs(i) = LCase(objArgs(i))
        origArgs(i) = objArgs(i)
    End If
Next

For i = objArgs.Count To (2 * objArgs.Count - 1)
    caseArgs(i) = "/"
    strArgs(i) = "/"
    origArgs(i) = "/"
Next


'Parse arguments
'===============

'Parse "/?"
If strArgs(nextArg) = "?" Then
    WScript.Echo "COMAdmin" + chr(13) + "[/add] [/delete] [/get] [/set]" + chr(13) + "[/server 'servername'] [/localhost]" + chr(13) + "[/application 'applicationname']" + chr(13) + "[/component 'componentname']" + chr(13) + "[/interface 'interfacename']" + chr(13) + "[/method 'methodname']" + chr(13) + "[/role 'rolename']" + chr(13) + "[/user 'username']" + chr(13) + "[/applicationfile 'applicationfile''installpath']" + chr(13) + "[/componentfile 'componentfile''type'] [/componentprogid 'componentprogid']" + chr(13) + "[/property 'PropName_1;PropValue_1' 'PropName_2;PropValue_2' . . . 'PropName_n;PropValue_n'"
    WScript.Quit (0)
End If

'Parse add/delete/get/set
If strArgs(nextArg) = "add" Or strArgs(nextArg) = "delete" Or strArgs(nextArg) = "get" Or strArgs(nextArg) = "set" Then
    strCommand = strArgs(nextArg)
    nextArg = nextArg + 1
Else
    WScript.Echo "Error:  must specify add, delete, set or get"
    WScript.Quit (-1)
End If


'Parse server or local machine
If strArgs(nextArg) = "server" Then
    If (Left(origArgs(nextArg + 1), 1)) <> "/" Then
        strServerName = caseArgs(nextArg + 1)
        nextArg = nextArg + 2
    Else
        WScript.Echo "Error:  unspecified server name"
        WScript.Quit (-1)
    End If
Else
    strServerName = "localhost"
    If strArgs(nextArg) = "localhost" Then
        nextArg = nextArg + 1
    End If
End If


'Parse application name
If strArgs(nextArg) = "application" Then
    strTarget = "application"
    If (Left(origArgs(nextArg + 1), 1)) <> "/" Then
        applicationName = caseArgs(nextArg + 1)
        nextArg = nextArg + 2
    Else
        strTarget2 = "application"
        nextArg = nextArg + 1
        If strCommand <> "get" Then
            WScript.Echo "Error:  unspecified application name"
            WScript.Quit (-1)
        End If
    End If
End If


'Parse component name
If strArgs(nextArg) = "component" Then
    strTarget = "component"
    If (Left(origArgs(nextArg + 1), 1)) <> "/" Then
        componentName = caseArgs(nextArg + 1)
        nextArg = nextArg + 2
    Else
        strTarget2 = "component"
        nextArg = nextArg + 1
        If strCommand <> "get" Then
            WScript.Echo "Error:  unspecified component name"
            WScript.Quit (-1)
        End If
    End If
End If


'Parse interface name
If strArgs(nextArg) = "interface" Then    
    'Sanity checks
    If componentName = "" Then
        WScript.Echo "Error:  interface cannot be specified without a component"
        WScript.Quit (-1)
    End If
    
    If (Left(origArgs(nextArg + 1), 1)) <> "/" Then
        interfaceName = caseArgs(nextArg + 1)
        nextArg = nextArg + 2
    Else
        strTarget2 = "interface"
        nextArg = nextArg + 1
        If strCommand <> "get" Then
            WScript.Echo "Error:  unspecified interface"
            WScript.Quit (-1)
        End If
    End If
End If


'Parse method name
If strArgs(nextArg) = "method" Then
	'Sanity checks
	If componentName = "" Then
		WScript.Echo "Error:  method cannot be specified without a component"
		WScript.Quit (-1)
	End If
	
	If interfaceName = "" Then
		WScript.Echo "Error:  interface cannot be specified without an interface"
		WScript.Quit (-1)
	End If
	
    If (Left(origArgs(nextArg + 1), 1)) <> "/" Then
        methodName = caseArgs(nextArg + 1)
        nextArg = nextArg + 2
    Else
        strTarget2 = "method"
        nextArg = nextArg + 1
        If strCommand <> "get" Then
            WScript.Echo "Error:  unspecified method"
            WScript.Quit (-1)
        End If
    End If	
End If


'Parse role name
If strArgs(nextArg) = "role" Then
    If (Left(origArgs(nextArg + 1), 1)) <> "/" Then
        roleName = caseArgs(nextArg + 1)
        nextArg = nextArg + 2
    Else
        strTarget2 = "role"
        nextArg = nextArg + 1
        If strCommand <> "get" Then
            WScript.Echo "Error:  unspecified role name"
            WScript.Quit (-1)
        End If
    End If
Else
    'Sanity check
    If strCommand = "add" And componentName <> "" Then
        WScript.Echo "Error:  cannot set component name when adding component"
        WScript.Quit (-1)
    End If

    If (strCommand = "add" Or strCommand = "delete") And interfaceName <> "" Then
        WScript.Echo "Error:  cannot add or delete an interface"
        WScript.Quit (-1)
    End If
End If


'Parse user name
If strArgs(nextArg) = "user" Then
    'Sanity check
    If roleName = "" Then
        WScript.Echo "Error:  user cannot be specified without a role"
        WScript.Quit (-1)
    End If
    
    If componentName <> "" Then
        WScript.Echo "Error:  cannot specify users when accessing component membership roles"
        WScript.Quit (-1)
    End If
    
    If (Left(origArgs(nextArg + 1), 1)) <> "/" Then
        userName = caseArgs(nextArg + 1)
        nextArg = nextArg + 2
    Else
        strTarget2 = "user"
        nextArg = nextArg + 1
        If strCommand <> "get" Then
            WScript.Echo "Error:  unspecified user"
            WScript.Quit (-1)
        End If
    End If
End If


'Parse property name and new value
If strArgs(nextArg) = "property" Then
    'Sanity checks
    If strCommand = "add" Or strCommand = "delete" Then
        WScript.Echo "Error:  cannot add or delete a property"
        WScript.Quit (-1)
    End If

    If strTarget2 <> "" Then
        WScript.Echo "Error:  unspecified " & strTarget2
        WScript.Quit (-1)
    End If

	'Work variables declaration and initialization to handle mulitple properties
	Dim DelimiterPos
	DelimiterPos = 0

	'Move to the next command line element
	nextArg = nextArg + 1

	'Verify we have defined atleast one property name;value pair, if not bail
	if Left(origArgs(nextArg), 1) = "/" then
		if ("set" = strComand) then
			WScript.Echo "Error:  unspecified property PropName;PropValue pair"
		else
			WScript.Echo "Error:  unspecified property name"
		end if
		WScript.Quit (-1)
	end if

	'Loop through and fill the property arrays (name and value)
	Do while (Left(origArgs(nextArg), 1) <> "/")
		'Right size the arrays
		Redim Preserve PropNames(PropArrayUBound)
		Redim Preserve PropValues(PropArrayUBound)
		'Determine if where, if any, the delimiter exists
		DelimiterPos = Instr(strArgs(nextArg), chr(59))
		'Continue depending whether a delimiter was found
		if (0 = DelimiterPos) then
			if ("get" = strCommand) then
				'Property names only ok only when executing the get command
				propertyCheck = "VALID"
				PropNames(PropArrayUBound) = Trim(caseArgs(nextArg))
				PropValues(PropArrayUBound) = ""
			else
				PropNames(PropArrayUBound) = Trim(caseArgs(nextArg))
				nextArg = nextArg + 1
				if Left(origArgs(nextArg), 1) = "/" Then
					WScript.Echo "Error:  you must specify a property value with a set command"
					WScript.Quit (-1)
				End If
				PropValues(PropArrayUBound) = Trim(caseArgs(nextArg))
			end if
		else
			if ("set" = strCommand) then
				'Property names and values ok only when execute set command
				propertyCheck = "VALID"
				PropNames(PropArrayUBound) = Mid(caseArgs(nextArg), 1, DelimiterPos - 1)
				'Handle blank properties to be specified
				if (DelimiterPos = len(caseArgs(nextArg))) then
					PropValues(PropArrayUBound) = ""
				else
					PropValues(PropArrayUBound) = Mid(caseArgs(nextArg), DelimiterPos + 1)
				end if
			else
				WScript.Echo "Error:  cannot specify property values with a get command"
				WScript.Quit (-1)
			end if
		end if
		'Increment the array upper bounds
		PropArrayUBound = PropArrayUBound + 1
		'Move the the next command line argument
		nextArg = nextArg + 1
	Loop
Else
    If strCommand = "set" Or strCommand = "get" And strTarget2 = "" Then
        WScript.Echo "Error:  no property specified"
        WScript.Quit (-1)
    End If
End If


'Parse application file
If strArgs(nextArg) = "applicationfile" Then
    'Sanity check
    If applicationName = "" Then
        WScript.Echo "Warning:  unspecified application name.  If application exists it will not be deleted"
    End If
    strTarget = "applicationfile"
    If (Left(origArgs(nextArg + 1), 1)) <> "/" Then
        applicationFile = caseArgs(nextArg + 1)
        nextArg = nextArg + 2
    Else
        WScript.Echo "Error:  unspecified applicationfile name"
        WScript.Quit (-1)
    End If
    
    If (Left(origArgs(nextArg), 1)) <> "/" Then
        installPath = caseArgs(nextArg)
        nextArg = nextArg + 1
    Else
        WScript.Echo "Warning:  unspecified application installation path.  Using default"
    End If
End If


'Parse component file
If strArgs(nextArg) = "componentfile" Then
    'Sanity checks
    If applicationFile <> "" Then
        WScript.Echo "Error:  cannot install more than one object"
        WScript.Quit (-1)
    End If

    If componentName <> "" Then
        WScript.Echo "Error: cannot specify component name"
        WScript.Quit (-1)
    End If

    If applicationName = "" Then
        WScript.Echo "Error:  cannot add component without target application"
        WScript.Quit (-1)
    End If

    strTarget = "componentfile"
    If (Left(origArgs(nextArg + 1), 1)) <> "/" Then
        componentFile = caseArgs(nextArg + 1)
        nextArg = nextArg + 2
    Else
        WScript.Echo "Error:  unspecified componentfile name"
        WScript.Quit (-1)
    End If
    
    If (Left(origArgs(nextArg), 1)) <> "/" Then
        componentFileType = CInt(caseArgs(nextArg))
        nextArg = nextArg + 1
    Else
        WScript.Echo "Error:  unspecified componentfile type"
        WScript.Quit (-1)
    End If
    
    'Sanity check
    If componentFileType < 1 And componentFileType > 3 Then
        WScript.Echo "Error:  invalid componentfile type"
        WScript.Quit (-1)
    End If
End If


'Parse component progid
If strArgs(nextArg) = "componentprogid" Then
    'Sanity checks
    If applicationFile <> "" Or componentFile <> "" Then
        WScript.Echo "Error:  cannot install more than one object"
        WScript.Quit (-1)
    End If

    If componentName <> "" Then
        WScript.Echo "Error:  cannot set name of component"
        WScript.Quit (-1)
    End If

    If applicationName = "" Then
        WScript.Echo "Error:  cannot add component without target application"
        WScript.Quit (-1)
    End If

    strTarget = "componentprogid"
    If (Left(origArgs(nextArg + 1), 1)) <> "/" Then
        componentProgid = caseArgs(nextArg + 1)
        nextArg = nextArg + 2
    Else
        WScript.Echo "Error:  unspecified component progID"
        WScript.Quit (-1)
    End If
End If


'Final sanity checks
'=============

'Can't get/set properties without a target
If strCommand = "get" Or strCommand = "set" Then
    If strTarget <> "application" And strTarget <> "component" Then
        WScript.Echo "Error:  cannot find target for property"
        WScript.Quit (-1)
    End If
End If

If strTarget2 = "component" And applicationName = "" Then
    WScript.Echo "Error:  unspecified application name"
    WScript.Quit (-1)
End If

'Check for extraneous commands
If strArgs(nextArg) <> "/" Then
    WScript.Echo "Error:  unknown argument '" & strArgs(nextArg) & "'"
    WScript.Quit (-1)
End If


'Get down to business
'====================

'First, we create the catalog object
Dim catalog
Set catalog = CreateObject("COMAdmin.COMAdminCatalog.1")

'Then we get the applications collection
Dim applications
If strServerName <> "localhost" Then
    Dim root
    Set root = catalog.Connect(strServerName)
    Set applications = root.GetCollection("Applications", "")
Else
    Set applications = catalog.GetCollection("Applications")
End If

applications.Populate
numApplications = applications.Count

If applicationName <> "" Then
    Dim newApplication
    numApplications = applications.Count
    For i = numApplications - 1 To 0 Step -1
        If applications.Item(i).Value("Name") = applicationName Then
            Set newApplication = applications.Item(i)
        End If
    Next
End If

'Operate on a application if a application is the target
If strTarget = "application" Or strTarget = "applicationfile" Then    
    'User operations
    If userName <> "" Or strTarget2 = "user" Then
        Set rolesInPack = applications.GetCollection("Roles", newApplication.Value("ID"))
        rolesInPack.Populate

        numRoles = rolesInPack.Count

        Dim userCollection
        Dim numUsers

        For i = numRoles - 1 To 0 Step -1
            If rolesInPack.Item(i).Value("Name") = roleName Then
                Set userCollection = rolesInPack.GetCollection("UsersInRole", rolesInPack.Item(i).Key)
                userCollection.Populate
                numUsers = userCollection.Count
                
                For j = numUsers - 1 To 0 Step -1
                    If strTarget2 = "user" Then
                        WScript.Echo userCollection.Item(j).Name
                    Else
                        If userCollection.Item(j).Name = userName Then
                            If strCommand = "add" Or strCommand = "delete" Then
                                'Delete user, if user exists
                                If userCollection.Item(j).Name = userName Then
                                    userCollection.Remove (j)
									userCollection.SaveChanges
                                End If	
                            Else	
								if ("set" = strCommand) then
									PropPtr = 0
									for PropPtr = Lbound(PropNames) to Ubound(PropNames)
										userCollection.Item(j).Value(PropNames(PropPtr)) = PropValues(PropPtr)
									next
									userCollection.SaveChanges
								else
									WScript.Echo userCollection.term(j).Value(PropNames(PropPtr))
								end if	
                            End If
                        End If
                    End If
                Next
            End If
        Next
	
        'Add new user, if necessary
        If strCommand = "add" Then
            Dim newUser
            Set newUser = userCollection.Add
            newUser.Value("User") = userName
            userCollection.SaveChanges
        End If
        WScript.Quit (0)
    End If
    
	'Role operations
	If (roleName <> "" And userName = "") Or (strTarget2 = "role") Then
		'Get Application
		Dim rolesInPack
		Set rolesInPack = applications.GetCollection("Roles", newApplication.Value("ID"))
		rolesInPack.Populate
		numRoles = rolesInPack.Count
		For i = numRoles - 1 To 0 Step -1
			If strTarget2 = "role" Then
				WScript.Echo rolesInPack.Item(i).Value("Name")
			Else
				If rolesInPack.Item(i).Value("Name") = roleName Then
					If strCommand = "add" Or strCommand = "delete" Then
						'Delete role
						rolesInPack.Remove (i)
						rolesInPack.SaveChanges
					Else
						if ("set" = strCommand) then
							PropPtr = 0
							For PropPtr = Lbound(PropNames) to Ubound(PropNames)
								rolesInPack.Item(i).Value(PropNames(PropPtr)) = PropValues(PropPtr)
							Next
							rolesInPack.SaveChanges
						else
							WScript.Echo rolesInPack.Item(i).Value(PropNames(PropPtr))
						end if
					End If
				End If
			End If
		Next
		'Add new role, if necessary
		If strCommand = "add" Then
			Dim newRole
			Set newRole = rolesInPack.Add
			newRole.Value("Name") = roleName
			rolesInPack.SaveChanges
		End If
		WScript.Quit (0)
	End If

    'Application operations
    If strCommand = "add" Or strCommand = "delete" Then
        'Remove all applications that go by the same name as the given application
        If applicationName <> "" Then
            numApplications = applications.Count
            For i = numApplications - 1 To 0 Step -1
                If applications.Item(i).Value("Name") = applicationName Then
                    applications.Remove (i)
                    applications.SaveChanges
                End If
            Next
        End If
        If strCommand = "add" Then
            If strTarget = "application" Then
                'Add empty application
                Set newApplication = applications.Add
                newApplication.Value("Name") = applicationName
                applications.SaveChanges
            Else
                'Add application from application file
				catalog.InstallApplication applicationFile, installPath, mtsInstallUsers
            End If
        End If
    Else
        'Operate upon desired property
        flag = False
        numApplications = applications.Count
        For i = numApplications - 1 To 0 Step -1
            If strTarget2 = "application" Then
                flag = True
                WScript.Echo applications.Item(i).Value("Name")
            Else
                If applications.Item(i).Value("Name") = applicationName Then
                    flag = True
					PropPtr = 0
					if ("set" = strCommand) Then
						for PropPtr = Lbound(PropNames) to Ubound(PropNames)						
							applications.Item(i).Value(PropNames(PropPtr)) = PropValues(PropPtr)
						Next
						applications.SaveChanges
					else 'get
						for PropPtr = Lbound(PropNames) to Ubound(PropNames)						
							WScript.Echo applications.Item(i).Value(PropNames(PropPtr))
						Next
					End if
                End If
            End If
        Next
        
        If flag = False Then
            WScript.Echo "Error:  unknown property"
            WScript.Quit (-1)
        End If
    End If
    WScript.Quit (0)
End If


'Operate on a component if a component is the target
If strTarget = "component" Or strTarget = "componentfile" Or strTarget = "componentprogid" Then
    
    'Get components
    Dim components
    Dim numApplications
    numApplications = applications.Count
    For i = numApplications - 1 To 0 Step -1
        If applications.Item(i).Value("Name") = applicationName Then
            Set components = applications.GetCollection("Components", applications.Item(i).Value("ID"))
            components.Populate
        End If
    Next
    
    'User and Role operations
    Dim numComponents
    numComponents = components.Count

    If interfaceName <> "" Or roleName <> "" Or (strTarget2 <> "component" And strTarget2 <> "") Then
        
        'Component role membership operations
        If interfaceName = "" And strTarget2 <> "interface" Then
            Dim rolesInComponent
            For i = numComponents - 1 To 0 Step -1
                If components.Item(i).Name = componentName Then
                    Set rolesInComponent = components.GetCollection("RolesForComponent", components.Item(i).Key)
                    rolesInComponent.Populate
                End If
            Next
            Dim numRoles
            numRoles = rolesInComponent.Count
            For i = numRoles - 1 To 0 Step -1
                If strTarget2 = "role" Then
                    WScript.Echo rolesInComponent.Item(i).Name
                Else
                    If rolesInComponent.Item(i).Value("Name") = roleName Then
                        If strCommand = "add" Or strCommand = "delete" Then
                             rolesInComponent.Remove (i)
                             rolesInComponent.SaveChanges
                        Else
                            If strCommand = "set" Then
                                WScript "Error:  Cannot set properties for component role membership"
                                WScript.Quit (-1)
                            Else
								PropPtr = 0
								For PropPtr = Lbound(PropNames) to UBound(PropNames)
									WScript.Echo rolesInComponent.Item(i).Value(PropNames(PropPtr))
								Next
                            End If
                        End If
                    End If
                End If
            Next
            
            If strCommand = "add" Then
                Dim newRole2
                Set newRole2 = rolesInComponent.Add
  	 	        newRole2.Value("Name") = roleName
                rolesInComponent.SaveChanges
            End If

            WScript.Quit (0)
            
        Else 'Method Or Interface operations
			
			Dim interfaces
			Dim numInterfaces

			For i = numComponents - 1 To 0 Step -1
				If components.Item(i).Name = componentName Then
					Set interfaces = components.GetCollection("InterfacesForComponent", components.Item(i).key)
					interfaces.Populate
				End If
			Next

			numInterfaces = interfaces.Count

			If methodName <> "" Or strTarget2 = "method" Then 'Is it method or what?

				Dim methods
				Dim numMethods

				For i = numInterfaces - 1 To 0 Step -1
					If interfaces.Item(i).Name = interfaceName Then
						Set methods = interfaces.GetCollection("MethodsForInterface", interfaces.Item(i).key)
						methods.Populate
					End If
				Next

				numMethods = methods.Count

				'Get roles in method
				Dim rolesMethod

				For i = numMethods - 1 To 0 Step -1
					If strTarget2 = "method" Then
						WScript.Echo methods.Item(i).Value("Name")
					Else
						If methods.Item(i).Value("Name") = methodName Then
							If propertyCheck <> "" And roleName = "" And strTarget2 = "" Then
								PropPtr = 0
								if ("set" = strCommand) then
									For PropPtr = LBound(PropNames) to UBound(PropNames)
										methods.Item(i).Value(PropNames(PropPtr)) = PropValues(PropPtr)
									Next
									methods.SaveChanges
								else  'get
									For PropPtr = LBound(PropNames) to UBound(PropNames)
										WScript.Echo methods.Item(i).Value(PropNames(PropPtr))
									Next
									WScript.Quit (0)
								End If
							End If
							Set rolesMethod = methods.GetCollection("RolesForMethod", methods.Item(i).Key)
							rolesMethod.Populate
						End If
					End If
				Next

				If strTarget2 = "method" Then
					WScript.Quit (0)
				End If

				numRoles = rolesMethod.Count

				For i = numRoles - 1 To 0 Step -1
					If strTarget2 = "role" Then
						WScript.Echo rolesMethod.Item(i).Value("Name")
					Else
						If rolesMethod.Item(i).Value("Name") = roleName Then
							If strCommand = "add" Or strCommand = "delete" Then
								rolesMethod.Remove (i)
								rolesMethod.SaveChanges
							Else
								PropPtr = 0
								if ("set" = strCommand) then
									For PropPtr = LBound(PropNames) to UBound(PropNames)
										rolesMethod.Item(i).Value(PropNames(PropPtr)) = PropValues(PropPtr)
									Next
									rolesMethod.SaveChanges
								else
									For PropPtr = LBound(PropNames) to UBound(PropNames)
										WScript.Echo rolesMethod.Item(i).Value(PropNames(PropPtr))
									Next
								End if
							End If
						End If
					End If
				Next
            
				If strCommand = "add" Then
					Dim newRole4
					Set newRole4 = rolesMethod.Add
					newRole4.Value("Name") = roleName
					rolesMethod.SaveChanges
				End If

			Else 'Interface...

				'Get roles in interface
				Dim rolesInterface

				For i = numInterfaces - 1 To 0 Step -1
					If strTarget2 = "interface" Then
						WScript.Echo interfaces.Item(i).Value("Name")
					Else
						If interfaces.Item(i).Value("Name") = interfaceName Then
							If propertyCheck <> "" And roleName = "" And strTarget2 = "" Then
								PropPtr = 0
								if ("set" = strCommand) then
									For PropPtr = LBound(PropNames) to UBound(PropNames)
										interfaces.Item(i).Value(PropNames(PropPtr)) = PropValues(PropPtr)
									Next
									interfaces.SaveChanges
								else
									For PropPtr = LBound(PropNames) to UBound(PropNames)
										WScript.Echo interfaces.Item(i).Value(PropNames(PropPtr))
									Next
									WScript.Quit(0)
								End If
							End if
							Set rolesInterface = interfaces.GetCollection("RolesForInterface", interfaces.Item(i).Key)
							rolesInterface.Populate
						End If
					End If
				Next
            
				If strTarget2 = "interface" Then
					WScript.Quit (0)
				End If

				numRoles = rolesInterface.Count

				For i = numRoles - 1 To 0 Step -1
					If strTarget2 = "role" Then
						WScript.Echo rolesInterface.Item(i).Value("Name")
					Else
						If rolesInterface.Item(i).Value("Name") = roleName Then
							If strCommand = "add" Or strCommand = "delete" Then
								rolesInterface.Remove (i)
								rolesInterface.SaveChanges
							Else
								PropPtr = 0
								if ("set" = strCommand) then
									For PropPtr = LBound(PropNames) to UBound(PropNames)
										rolesInterface.Item(i).Value(PropNames(PropPtr)) = PropValues(PropPtr)
									Next 
									rolesInterface.SaveChanges
								else
									For PropPtr = LBound(PropNames) to UBound(PropNames)
										WScript.Echo rolesInterface.Item(i).Value(PropNames(PropPtr))
									Next 
								end if
							End If
						End If
					End If
				Next
            
				If strCommand = "add" Then
					Dim newRole3
					Set newRole3 = rolesInterface.Add
					newRole3.Value("Name") = roleName
					rolesInterface.SaveChanges
				End If
			End If
        End If
        WScript.Quit (0)
    End If

    'Component operations
    If strCommand = "add" Or strCommand = "delete" Then
        
        'Remove all components that go by the same name as the given component
        For i = numComponents - 1 To 0 Step -1
            If components.Item(i).Name = componentName Or (components.Item(i).Value ("ProgID") = componentProgid And componentProgid <> "") Then
                components.Remove (i)
                components.SaveChanges
            End If
        Next
    End If

    If strCommand = "add" Then
		If strTarget = "componentfile" Then
            'Add component from file
            Select Case componentFileType
            Case 1 'Regular dll
                catalog.InstallComponent newApplication.Value("ID"), componentFile, "", ""
           Case 2 'tlb
                catalog.InstallComponent newApplication.Value("ID"), "", componentFile, ""
            Case 3 'proxy/stub dll
                catalog.InstallComponent newApplication.Value("ID"), "", "", componentFile
           End Select
        Else
            catalog.ImportComponent newApplication.Value("ID"), componentProgid
       End If
    Else
        If strCommand = "get" Or strCommand = "set" Then
            'Operate upon desired property
            flag = False
            For i = numComponents - 1 To 0 Step -1
                If strTarget2 = "component" Then
                    WScript.Echo components.Item(i).Name
                    flag = True
                Else
                    If components.Item(i).Name = componentName Then
                        flag = True
						PropPtr = 0
						if ("set" = strCommand) then
							For PropPtr = LBound(PropNames) to UBound(PropNames)
								components.Item(i).Value(PropNames(PropPtr)) = PropValues(PropPtr)
							Next 
							components.SaveChanges
						else  ' get
							For PropPtr = LBound(PropNames) to UBound(PropNames)
								WScript.Echo components.Item(i).Value(PropNames(PropPtr))
							Next 
							WScript.Quit(0)
						end if
                    End If
                End If
            Next

            If flag = False Then
                WScript.Echo "Error:  unknown property"
                WScript.Quit (-1)
            End If
        End If
    End If
    WScript.Quit (0)
End If

WScript.Echo "Command failed."
