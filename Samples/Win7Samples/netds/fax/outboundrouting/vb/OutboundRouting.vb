'==========================================================================
'
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
' PARTICULAR PURPOSE.
'
' Copyright (c) Microsoft Corporation. All rights reserved.
'
'--------------------------------------------------------------------------

Imports System
Imports System.Globalization
Imports System.Security.Permissions
Imports Microsoft.VisualBasic
Imports Microsoft.VisualBasic.ApplicationServices
Imports FAXCOMEXLib

<Assembly: System.Reflection.AssemblyKeyFile("key.snk")> 
<Assembly: CLSCompliant(True)> 
Namespace Microsoft.Samples.Fax.ServerConfig.VB

    Module ServerConfig
        '+---------------------------------------------------------------------------
        '
        '  function:   GiveUsage
        '
        '  Synopsis:   prints the usage of the application
        '
        '  Arguments:  void
        '
        '  Returns:    void
        '
        '  Modifies:
        '
        '----------------------------------------------------------------------------
        Sub GiveUsage()
            System.Console.WriteLine("Usage : " + System.Diagnostics.Process.GetCurrentProcess().ProcessName)
            System.Console.WriteLine(" /s Fax Server Name ")
            System.Console.WriteLine(" /o <listGroups/ListRules/RemoveGroup/AddGroup/RemoveRule/AddRule> ")
            System.Console.WriteLine(" /i index of the group or rule to be removed. valid values 0 to n ")
            System.Console.WriteLine(" /n name of the new routing rule or group  ")
            System.Console.WriteLine(" /c country code for the new routing rule")
            System.Console.WriteLine(" /a area code for the new routing rule ")
            System.Console.WriteLine(" /d device id for the routing rule and list of device ids separated by  for routing group ")
            System.Console.WriteLine(" /b 1 to use the device id for the new routing rule else 0 ")
            System.Console.WriteLine("Usage : " + System.Diagnostics.Process.GetCurrentProcess().ProcessName + " /? -- help message")
        End Sub
        '+---------------------------------------------------------------------------
        '
        '  function:   IsOSVersionCompatible
        '
        '  Synopsis:   finds whether the target OS supports this functionality.
        '
        '  Arguments:  [iVersion] - Minimum Version of the OS required for the Sample to run.
        '
        '  Returns:    bool - true if the Sample can run on this OS
        '
        '----------------------------------------------------------------------------
        Function IsOSVersionCompatible(ByVal iVersion As Integer) As Boolean
            Dim os As OperatingSystem
            Dim osVersion As Version

            os = Environment.OSVersion
            osVersion = os.Version
            If (osVersion.Major >= iVersion) Then
                Return True
            Else
                Return False
            End If
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   listDeviceIds
        '
        '  Synopsis:   lists the set of devices on for a routing group
        '
        '  Arguments:  [IFaxDeviceIds] - FaxDevIds object pointing to the list of device ids are part of the routing group
        '                
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function listDeviceIds(ByVal objFaxDevIds As IFaxDeviceIds) As Boolean
            Dim lDevId As Long
            Dim count As Long
            Dim bRetVal As Boolean
            Dim i As Integer

            lDevId = 0
            count = 0
            bRetVal = False

            'check for null
            If (TypeOf objFaxDevIds Is FAXCOMEXLib.IFaxDeviceIds) Then
                count = objFaxDevIds.Count
                i = 0
                While (i < count)
                    lDevId = objFaxDevIds(i + 1)
                    System.Console.Write("Device No: ")
                    System.Console.Write(i)
                    System.Console.Write(" Device Id= ")
                    System.Console.Write(lDevId)
                    System.Console.WriteLine()
                    i = i + 1
                End While
                bRetVal = True

                Return bRetVal
            End If
            System.Console.WriteLine("listDevices: Parameter passed is null")
            Return False
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   listGroups
        '
        '  Synopsis:   list of Routing Groups on the Server
        '
        '  Arguments:  [objFaxOutRoutGrps] - FaxOutboundRoutingGroups object pointing to the Routing Groups of the current server
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function listGroups(ByVal objFaxOutRoutGrps As IFaxOutboundRoutingGroups) As Boolean

            Dim bRetVal As Boolean
            Dim lCount As Long
            Dim objFaxOutRoutGrp As IFaxOutboundRoutingGroup
            Dim strGrpName As String
            Dim i As Integer

            bRetVal = True
            lCount = 0
            objFaxOutRoutGrp = Nothing
            strGrpName = Nothing

            'check for null
                     If (TypeOf objFaxOutRoutGrps Is FAXCOMEXLib.IFaxOutboundRoutingGroups) Then   
                System.Console.WriteLine(" Listing Routing Group details....")
                lCount = objFaxOutRoutGrps.Count
                i = 0
                'enumerate
                While (i < lCount)
                    Dim objFaxDevIds As IFaxDeviceIds
                    objFaxDevIds = Nothing
                    Dim enumStatus As FAX_GROUP_STATUS_ENUM
                    objFaxOutRoutGrp = objFaxOutRoutGrps(i + 1)
                    objFaxDevIds = objFaxOutRoutGrp.DeviceIds
                    strGrpName = objFaxOutRoutGrp.Name
                    enumStatus = objFaxOutRoutGrp.Status

                    'print all the details
                    System.Console.WriteLine(" ===================================================")
                    System.Console.Write("Group No: ")
                    System.Console.Write(i + 1)

                    System.Console.Write(" Group Name: ")
                    System.Console.WriteLine(strGrpName)

                    If (enumStatus = FAX_GROUP_STATUS_ENUM.fgsALL_DEV_VALID) Then
                        System.Console.WriteLine("Status : All the devices in the routing group are valid and available for sending outgoing faxes. ")
                    End If
                    If (enumStatus = FAX_GROUP_STATUS_ENUM.fgsEMPTY) Then
                        System.Console.WriteLine("Status : The routing group does not contain any devices. ")
                    End If
                    If (enumStatus = FAX_GROUP_STATUS_ENUM.fgsALL_DEV_NOT_VALID) Then
                        System.Console.WriteLine("Status : The routing group does not contain any available devices for sending faxes. (Devices can be unavailable when they are offline and when they do not exist.) ")
                    End If
                    If (enumStatus = FAX_GROUP_STATUS_ENUM.fgsSOME_DEV_NOT_VALID) Then
                        System.Console.WriteLine("Status : The routing group contains some devices that are unavailable for sending faxes. (Devices can be unavailable when they are offline and when they do not exist.) ")
                    End If

                    If (listDeviceIds(objFaxDevIds) = False) Then
                        'we dont want to log any error here as the error will be logged in the function itself
                        bRetVal = False
                    End If
                    i = i + 1
                End While

                Return bRetVal
            End If
            System.Console.WriteLine("listGroups: Parameter passed is null")
            Return False
        End Function

        '+---------------------------------------------------------------------------
        '
        '  function:   removeGroup
        '
        '  Synopsis:   removes a routing group from FaxOutboundRoutingGroups based on index
        '
        '  Arguments:  [objFaxOutRoutGrps] - FaxOutboundRoutingGroups object pointing to the Routing Groups of the current server
        '                [index] - index of the group to be removed
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function removeGroup(ByVal objFaxOutRoutGrps As IFaxOutboundRoutingGroups, ByVal index As Integer) As Boolean
            Dim bRetVal As Boolean
            Dim lCount As Long
            lCount = 0
            bRetVal = False

            'check for null
            If (TypeOf objFaxOutRoutGrps Is FAXCOMEXLib.IFaxOutboundRoutingGroups) Then
                'get count of groups
                lCount = objFaxOutRoutGrps.Count

                'invalid index
                If (index > lCount Or index <= 0) Then
                    System.Console.Write("removeGroup: Invalid index value. It can be from 1 to ")
                    System.Console.WriteLine(lCount)
                    bRetVal = False
                    GoTo ExitFun
                End If

                'remove group
                objFaxOutRoutGrps.Remove(index)
                System.Console.WriteLine("Group removed successfully. ")
                bRetVal = True
ExitFun:
                Return bRetVal
            End If
            System.Console.WriteLine("removeGroup: Parameter passed is null")
            Return False
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   SplitDevIds
        '
        '  Synopsis:   Splits the sting of "" separated Device Ids in to String array
        '
        '  Arguments:   [inputDevIdList] - The list of device ids in string format separated by semicolon
        '               [numDevIds] -    The number of device ids 
        '               [bRetVal] - Array of strings each containing a single device id
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function SplitDevIds(ByVal inputDevIdList As String, ByRef numDevIds As Integer, ByRef bRetVal As Boolean) As String()
            bRetVal = False
            If (String.IsNullOrEmpty(inputDevIdList)) Then
                Return Nothing
            End If
            Dim strDelimiter As String
            Dim delimiter As Char()
            Dim devIdStrArray As String()

            strDelimiter = ";"
            delimiter = strDelimiter.ToCharArray()
            devIdStrArray = inputDevIdList.Split(delimiter)
            bRetVal = True
            numDevIds = devIdStrArray.Length
            Return devIdStrArray
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   addGroup
        '
        '  Synopsis:   adds a routing group to FaxOutboundRoutingGroups
        '
        '  Arguments:  [objFaxOutRoutGrps] - FaxOutboundRoutingGroups object pointing to the Routing Groups of the current server
        '                [strGrpName] - Routing Group Name
        '                [strDevIds] - device ids for the new group
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function addGroup(ByVal objFaxOutRoutGrps As IFaxOutboundRoutingGroups, ByVal strGrpName As String, ByVal strDevIds As String) As Boolean
            Dim bRetVal As Boolean
            Dim bRet As Boolean
            Dim iDevCount As Integer
            Dim objFaxOutRoutGrp As IFaxOutboundRoutingGroup
            Dim objFaxDevIds As IFaxDeviceIds
            Dim devArr As String()

            bRetVal = False
            bRet = False
            iDevCount = 0
            objFaxOutRoutGrp = Nothing
            objFaxDevIds = Nothing

            'check for NULL
            If (TypeOf objFaxOutRoutGrps Is FAXCOMEXLib.IFaxOutboundRoutingGroups) And (String.IsNullOrEmpty(strGrpName) = False) And (String.IsNullOrEmpty(strDevIds) = False) Then

                objFaxOutRoutGrp = objFaxOutRoutGrps.Add(strGrpName)
                devArr = SplitDevIds(strDevIds, iDevCount, bRet)
                objFaxDevIds = objFaxOutRoutGrp.DeviceIds
                Dim i As Integer
                i = 0
                While (i < iDevCount)
                    Dim iVal As Integer
                    iVal = Int32.Parse(devArr.GetValue(i), CultureInfo.CurrentCulture.NumberFormat)
                    objFaxDevIds.Add(iVal)
                    i = i + 1
                End While
                System.Console.WriteLine("Group added successfully. ")
                bRetVal = True
                Return bRetVal
            End If
            System.Console.WriteLine("addGroup: Parameter passed is null")
            Return False
        End Function

        '+---------------------------------------------------------------------------
        '
        '  function:   listRules
        '
        '  Synopsis:   list of Routing Rules on the Server
        '
        '  Arguments:  [objFaxOutRoutRules] - FaxOutboundRoutingRules object pointing to the Routing Rules of the current server
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function listRules(ByVal objFaxOutRoutRules As IFaxOutboundRoutingRules) As Boolean
            Dim bRetVal As Boolean
            Dim lCount As Long
            Dim objFaxOutRoutRule As IFaxOutboundRoutingRule
            bRetVal = True
            lCount = 0
            objFaxOutRoutRule = Nothing

            'check for null
            If (TypeOf objFaxOutRoutRules Is FAXCOMEXLib.IFaxOutboundRoutingRules) Then
                System.Console.WriteLine(" Listing Routing Rule details.... ")
                lCount = objFaxOutRoutRules.Count
                Dim i As Integer
                i = 0
                While (i < lCount)
                    Dim lDeviceId As Long
                    Dim lAreaCode As Long
                    Dim lCountryCode As Long
                    Dim bUseDevice As Boolean
                    Dim enumStatus As FAX_RULE_STATUS_ENUM
                    Dim strGrpName As String

                    lDeviceId = 0
                    lAreaCode = 0
                    lCountryCode = 0
                    bUseDevice = False
                    strGrpName = Nothing

                    objFaxOutRoutRule = objFaxOutRoutRules(i + 1)
                    lAreaCode = objFaxOutRoutRule.AreaCode
                    lCountryCode = objFaxOutRoutRule.CountryCode
                    lDeviceId = objFaxOutRoutRule.DeviceId
                    strGrpName = objFaxOutRoutRule.GroupName
                    bUseDevice = objFaxOutRoutRule.UseDevice
                    enumStatus = objFaxOutRoutRule.Status

                    System.Console.WriteLine(" ===================================================")
                    System.Console.Write("Rule No: ")
                    System.Console.Write(i + 1)
                    System.Console.Write("Group Name = ")
                    System.Console.WriteLine(strGrpName)

                    If (lAreaCode = FAX_ROUTING_RULE_CODE_ENUM.frrcANY_CODE) Then
                        System.Console.WriteLine("Area Code: The outbound routing rule applies to all area codes. ")
                    Else
                        System.Console.Write("Area Code: ")
                        System.Console.WriteLine(lAreaCode)
                    End If

                    If (lCountryCode = FAX_ROUTING_RULE_CODE_ENUM.frrcANY_CODE) Then
                        System.Console.WriteLine("Country Code: The outbound routing rule applies to all area codes. ")
                    Else
                        System.Console.Write("Country Code: ")
                        System.Console.WriteLine(lCountryCode)
                    End If

                    System.Console.Write("Associated Device Id: = ")
                    System.Console.WriteLine(lDeviceId)

                    If (bUseDevice = True) Then
                        System.Console.WriteLine("Applies to single device ")
                    Else
                        System.Console.WriteLine("Applies to group of devices. ")
                    End If

                    If (enumStatus = FAX_RULE_STATUS_ENUM.frsVALID) Then
                        System.Console.WriteLine("Status : The routing rule is valid and can be applied to outbound faxes. ")
                    End If
                    If (enumStatus = FAX_RULE_STATUS_ENUM.frsEMPTY_GROUP) Then
                        System.Console.WriteLine("Status : The routing rule cannot be applied because the rule uses an outbound routing group for its destination and the group is empty. ")
                    End If
                    If (enumStatus = FAX_RULE_STATUS_ENUM.frsALL_GROUP_DEV_NOT_VALID) Then
                        System.Console.WriteLine("Status : The routing rule cannot be applied because the rule uses an existing outbound routing group for its destination and the group does not contain devices that are valid for sending faxes. ")
                    End If
                    If (enumStatus = FAX_RULE_STATUS_ENUM.frsSOME_GROUP_DEV_NOT_VALID) Then
                        System.Console.WriteLine("Status : The routing rule uses an existing outbound routing group for its destination but the group contains devices that are not valid for sending faxes. ")
                    End If
                    If (enumStatus = FAX_RULE_STATUS_ENUM.frsBAD_DEVICE) Then
                        System.Console.WriteLine("Status : The routing rule cannot be applied because the rule uses a single device for its destination and that device is not valid for sending faxes. ")
                    End If
                    i = i + 1
                End While
ExitFun:
                Return bRetVal
            End If

            System.Console.WriteLine("listRules: Parameter passed is null")
            Return False
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   removeRule
        '
        '  Synopsis:   removes a routing rule from FaxOutboundRoutingRules based on index
        '
        '  Arguments:  [objFaxOutRoutRules] - FaxOutboundRoutingRules object pointing to the Routing Rules of the current server
        '                [index] - index of the group to be removed
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function removeRule(ByVal objFaxOutRoutRules As IFaxOutboundRoutingRules, ByVal index As Integer) As Boolean
            Dim bRetVal As Boolean
            Dim lCount As Long
            bRetVal = False
            lCount = 0
            'check for null
            If (TypeOf objFaxOutRoutRules Is FAXCOMEXLib.IFaxOutboundRoutingRules) Then
                'get the count
                lCount = objFaxOutRoutRules.Count
                'check if valid
                If (index > lCount Or index <= 0) Then
                    System.Console.Write("removeRule: Invalid index value. It can be from 1 to ")
                    System.Console.WriteLine(lCount)
                    bRetVal = False
                    GoTo ExitFun
                End If

                'remove rule
                objFaxOutRoutRules.Remove(index)
                System.Console.WriteLine("Rule removed successfully. ")
                bRetVal = True
ExitFun:
                Return bRetVal
            End If
            System.Console.WriteLine("removeRule: Parameter passed is null")
            Return False
        End Function
        '+---------------------------------------------------------------------------
        '
        '  function:   addRule
        '
        '  Synopsis:   adds a routing rule to FaxOutboundRoutingRules
        '
        '  Arguments:  [objFaxOutRoutRules] - FaxOutboundRoutingRules object pointing to the Routing Rules of the current server
        '                [strGrpName] - Routing Group Name
        '                [strDevId] - device ids for the new group
        '
        '  Returns:    bool: true if passed successfully
        '
        '----------------------------------------------------------------------------
        Function addRule(ByVal objFaxOutRoutRules As IFaxOutboundRoutingRules, ByVal strGrpName As String, ByVal strDevId As String, byval strCountryCode as String, byval strAreaCode as String, byval bUseDevice as Boolean) As Boolean
            Dim bRetVal As Boolean
            Dim iDevId As Integer
            Dim objFaxOutRoutRule As IFaxOutboundRoutingRule
            bRetVal = False
            iDevId = -1
            objFaxOutRoutRule = Nothing

            'check for null
            If ((String.IsNullOrEmpty(strGrpName) = True) And (bUseDevice = False)) Or (String.IsNullOrEmpty(strCountryCode) = True) Or (String.IsNullOrEmpty(strAreaCode) = True) Then
                System.Console.WriteLine("addRule: Parameter passed is null")
                Return False
            End If

            'check for null
            If (TypeOf objFaxOutRoutRules Is FAXCOMEXLib.IFaxOutboundRoutingRules) Then
                If (String.IsNullOrEmpty(strDevId) = False) Then
                    iDevId = Int32.Parse(strDevId, CultureInfo.CurrentCulture.NumberFormat)
                End If
                'Set Area Code and Country Code for all codes
                objFaxOutRoutRule = objFaxOutRoutRules.Add(Int32.Parse(strCountryCode, CultureInfo.CurrentCulture.NumberFormat), Int32.Parse(strAreaCode, CultureInfo.CurrentCulture.NumberFormat), bUseDevice, strGrpName, iDevId)
                System.Console.WriteLine("Rule added successfully. ")
                bRetVal = True
                Return bRetVal
            End If
            System.Console.WriteLine("addRule: Parameter passed is null")
            Return False

        End Function

        Sub Main()
            Dim objFaxServer As FAXCOMEXLib.FaxServer
            Dim objFaxOutRoutGrps As FAXCOMEXLib.IFaxOutboundRoutingGroups
            Dim objFaxOutRoutRules As FAXCOMEXLib.IFaxOutboundRoutingRules
            Dim strServerName As String
            Dim strName As String
            Dim strIds As String
            Dim strAreaCode As String
            Dim strCountryCode As String
            Dim strUseDevice As String
            Dim strIndex As String
            Dim strOption As String
            Dim bConnected As Boolean
            Dim bRetVal As Boolean
            Dim count As Integer
            Dim args As String
            Dim bState As Boolean

            Dim iVista As Integer
            Dim bVersion As Boolean

            iVista = 6
            bVersion = IsOSVersionCompatible(iVista)

            If (bVersion = False) Then
                System.Console.WriteLine("OS Version does not support this feature")
                bRetVal = False
                Return
            End If

            strServerName = Nothing
            strName = String.Empty
            strIds = String.Empty
            strCountryCode = String.Empty
            strAreaCode = String.Empty
            strIndex = String.Empty
            strUseDevice = String.Empty
            strOption = Nothing
            objFaxServer = Nothing
            bRetVal = True
            bState = True
            bConnected = False
            count = 0
            args = Nothing

            Try
                If ((My.Application.CommandLineArgs.Count = 0)) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If


                ' check for commandline switches
                Do Until count = My.Application.CommandLineArgs.Count - 1
                    If count >= My.Application.CommandLineArgs.Count - 1 Then
                        Exit Do
                    End If
                    args = My.Application.CommandLineArgs.Item(count)
                    If ((String.Compare(args.Substring(0, 1), "/", True, CultureInfo.CurrentCulture) = 0) Or (String.Compare(args.Substring(0, 1), "-", True, CultureInfo.CurrentCulture) = 0)) Then
                        Select Case (((args.ToLower(CultureInfo.CurrentCulture).Substring(1, 1))))
                            Case "s"
                                If (String.IsNullOrEmpty(strServerName) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strServerName = My.Application.CommandLineArgs.Item(count + 1)
                            Case "o"
                                If (String.IsNullOrEmpty(strOption) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strOption = My.Application.CommandLineArgs.Item(count + 1)
                            Case "i"
                                If (String.IsNullOrEmpty(strIndex) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strIndex = My.Application.CommandLineArgs.Item(count + 1)
                            Case "i"
                                If (String.IsNullOrEmpty(strIndex) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strIndex = My.Application.CommandLineArgs.Item(count + 1)
                            Case "n"
                                If (String.IsNullOrEmpty(strName) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strName = My.Application.CommandLineArgs.Item(count + 1)
                            Case "d"
                                If (String.IsNullOrEmpty(strIds) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strIds = My.Application.CommandLineArgs.Item(count + 1)
                            Case "a"
                                If (String.IsNullOrEmpty(strAreaCode) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strAreaCode = My.Application.CommandLineArgs.Item(count + 1)
                            Case "c"
                                If (String.IsNullOrEmpty(strCountryCode) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strCountryCode = My.Application.CommandLineArgs.Item(count + 1)
                            Case "b"
                                If (String.IsNullOrEmpty(strUseDevice) = False) Then
                                    GiveUsage()
                                    bRetVal = False
                                    GoTo ExitFun
                                End If
                                strUseDevice = My.Application.CommandLineArgs.Item(count + 1)
                            Case "?"
                                GiveUsage()
                                bRetVal = False
                                GoTo ExitFun
                            Case Else
                        End Select
                    End If
                    count = count + 1
                Loop

                If (String.IsNullOrEmpty(strOption) = True) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If

                If (((String.IsNullOrEmpty(strName) = True) Or (String.IsNullOrEmpty(strIds) = True)) And (String.Compare("addgroup", strOption, True, CultureInfo.CurrentCulture) = 0)) Then
                    System.Console.WriteLine("Missing args111.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If

                'if addrule and strUseDev is not set
                If (((String.IsNullOrEmpty(strUseDevice) = True) Or ((String.Compare("1", strUseDevice, True, CultureInfo.CurrentCulture) <> 0) And (String.Compare("0", strUseDevice, True, CultureInfo.CurrentCulture) <> 0))) And (String.Compare("addrule", strOption, True, CultureInfo.CurrentCulture) = 0)) Then
                    System.Console.WriteLine("Set /b tag to 0 or 1.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If

                If ((String.IsNullOrEmpty(strName) = True) And ((String.Compare("removegroup", strOption, True, CultureInfo.CurrentCulture) = 0) And (String.Compare("removerule", strOption, True, CultureInfo.CurrentCulture) = 0))) Then
                    System.Console.WriteLine("Missing args.")
                    GiveUsage()
                    bRetVal = False
                    GoTo ExitFun
                End If

                'if UseDev = 1 then set lptstrIds
                'if UseDev = 0 then set lptstrName
                If (String.IsNullOrEmpty(strUseDevice) = False) Then
                    If (((String.Compare(strUseDevice, "0", True, CultureInfo.CurrentCulture) = 0) And (String.IsNullOrEmpty(strName) = True)) Or (((String.Compare(strUseDevice, "1", True, CultureInfo.CurrentCulture) = 0) And String.IsNullOrEmpty(strIds) = True) Or (String.IsNullOrEmpty(strAreaCode) = True) Or ((String.IsNullOrEmpty(strCountryCode) = True)) And (String.Compare("addrule", strOption, True, CultureInfo.CurrentCulture) <> 0))) Then
                        System.Console.WriteLine("Missing args.")
                        GiveUsage()
                        bRetVal = False
                        GoTo ExitFun
                    End If
                End If

                objFaxServer = New FAXCOMEXLib.FaxServer()
                'Connect to Fax Server
                objFaxServer.Connect(strServerName)
                bConnected = True

                'check API version
                If (objFaxServer.APIVersion < FAXCOMEXLib.FAX_SERVER_APIVERSION_ENUM.fsAPI_VERSION_3) Then
                    bRetVal = False
                    System.Console.WriteLine("OS Version does not support this feature")
                    GoTo ExitFun
                End If

                objFaxOutRoutGrps = objFaxServer.OutboundRouting.GetGroups()
                objFaxOutRoutRules = objFaxServer.OutboundRouting.GetRules()


                'list groups
                If (String.Compare("listgroups", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (listGroups(objFaxOutRoutGrps) = False) Then
                        bRetVal = False
                    End If
                End If
                'list rules
                If (String.Compare("listrules", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then

                    If (listRules(objFaxOutRoutRules) = False) Then
                        bRetVal = False
                    End If
                End If
                'remove group
                If (String.Compare("removegroup", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (removeGroup(objFaxOutRoutGrps, Int32.Parse(strIndex, CultureInfo.CurrentCulture.NumberFormat)) = False) Then
                        bRetVal = False
                    End If
                End If
                'remove rule
                If (String.Compare("removerule", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then

                    If (removeRule(objFaxOutRoutRules, Int32.Parse(strIndex, CultureInfo.CurrentCulture.NumberFormat)) = False) Then
                        bRetVal = False
                    End If
                End If

                'add group
                If (String.Compare("addgroup", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then
                    If (addGroup(objFaxOutRoutGrps, strName, strIds) = False) Then
                        bRetVal = False
                    End If
                End If
                'add rule
                If (String.Compare("addrule", strOption.ToLower(CultureInfo.CurrentCulture), True, CultureInfo.CurrentCulture) = 0) Then

                    Dim bUseDevice As Boolean
                    bUseDevice = False
                    If (String.Compare("0", strUseDevice, True, CultureInfo.CurrentCulture) = 0) Then

                        bUseDevice = False

                    Else
                        bUseDevice = True
                    End If
                    If (addRule(objFaxOutRoutRules, strName, strIds, strCountryCode, strAreaCode, bUseDevice) = False) Then
                        bRetVal = False
                    End If
                End If

            Catch excep As Exception
                System.Console.WriteLine("Exception:" + excep.Message)
            End Try
ExitFun:
            If (bConnected) Then
                objFaxServer.Disconnect()
            End If
            If (bRetVal = False) Then
                System.Console.WriteLine("Function Failed")
            End If
        End Sub
    End Module
End Namespace

