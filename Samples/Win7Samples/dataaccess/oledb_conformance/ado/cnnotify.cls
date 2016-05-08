VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "cnnotify"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Implements ITestCases

Dim WithEvents CEO As Connection
Attribute CEO.VB_VarHelpID = -1
Dim codelib As New adolvl2.Common
Dim g_caseerrorobj As ModuleBase.IError
Dim g_caseprovobj As ModuleBase.IProviderInfo
Dim xModInfo As New ModInfo
Dim pifObj As ParseInitFile
Dim col As Column
Dim tracecase As Boolean
Dim provstr As String
Dim curlocstr As String
Dim rsstr As String
Dim connstr As String
Dim eventfired As String

Private Function ITestCases_ExecuteVariation(ByVal lIndex As Long) As ModuleBase.tagVARIATION_STATUS
    ' call variation indicated by lIndex
    ITestCases_ExecuteVariation = eVariationStatusFailed
    If tracecase Then
        g_caseerrorobj.Transmit "Inside: ITestCases_ExecuteVariation(" + CStr(lIndex) + ")" + Chr(10)
    End If
    Select Case lIndex
        Case 0
            ITestCases_ExecuteVariation = validtest()
        Case 1
            ITestCases_ExecuteVariation = invalidtest()
    End Select
End Function
Private Function ITestCases_GetDescription() As String
    ' eventually get the description from the registry
    ITestCases_GetDescription = "Connection Notification Tests"
End Function
Private Function ITestCases_GetIndexOfVariationWithID(ByVal lID As Long) As Long
    If tracecase Then
        g_caseerrorobj.Transmit "Inside: ITestCases_GetIndexOfVariationWithID(" + CStr(lID) + ")" + Chr(10)
    End If
    ITestCases_GetIndexOfVariationWithID = lID + 1
End Function
Private Function ITestCases_GetName() As String
    If tracecase Then
        g_caseerrorobj.Transmit "Inside: ITestCases_GetName" + Chr(10)
    End If
    ITestCases_GetName = "cn - Notification"
End Function
Private Function ITestCases_GetOwningITestModule() As ModuleBase.ITestModule
    Set ITestCases_GetOwningITestModule = g_tm
End Function
Private Function ITestCases_GetProviderInterface() As ModuleBase.IProviderInfo
    Set ITestCases_GetProviderInterface = g_caseprovobj
End Function
Private Function ITestCases_GetVariationCount() As Long
    ITestCases_GetVariationCount = 1
End Function
Private Function ITestCases_GetVariationDesc(ByVal lIndex As Long) As String
    If tracecase Then
        g_caseerrorobj.Transmit "Inside: ITestCases_GetVariationDesc(" + CStr(lIndex) + ")" + Chr(10)
    End If
    Select Case lIndex
        Case 0
            ITestCases_GetVariationDesc = "Fire Connection Notifications"
        Case 1
            ITestCases_GetVariationDesc = ""
    End Select
End Function
Private Function ITestCases_GetVariationID(ByVal lIndex As Long) As Long
    If tracecase Then
        g_caseerrorobj.Transmit "Inside: ITestCases_GetVariationID(" + CStr(lIndex) + ")" + Chr(10)
    End If
    Select Case lIndex
        Case 0
            ITestCases_GetVariationID = 1
        Case 1
            ITestCases_GetVariationID = 2
    End Select
End Function
Private Function ITestCases_Init() As Long
Const SELECT_ALLFROMTBL = 2
Dim inistr As String
    
    ITestCases_Init = 0
    
    xModInfo.InitString = g_caseprovobj.GetInitString
    fResult = xModInfo.Init() 'Initialize CModuleInfo::Init()
    fResult = xModInfo.ParseInitString
    
    retcode = xModInfo.GetInitStringValue("FILE", inistr)
    If inistr = "" Then
        ' we don't have an ini file, we require one to run
        g_caseerrorobj.Transmit "The ADO tests require an ini file to run."
    Else
        Set pifObj = xModInfo.ParseObject

        ' build connection string and initialize pifObj
        connstr = codelib.GetConnStr(xModInfo, g_caseprovobj)
        rsstr = pifObj.GetQuery(SELECT_ALLFROMTBL)
        retcode = xModInfo.GetInitStringValue("CURSORLOC", curlocstr)
        If UCase(Trim(curlocstr)) = "CLIENT" Then
            curlocstr = "3"
        Else
            curlocstr = "2"
        End If
        If (connstr = "" Or rsstr = "") Then
            ' we don't have enough info to run
            g_caseerrorobj.Transmit "The ADO tests require a valid ini FILE and a DATASOURCE/LOCATION,USERID, and PASSWORD."
        Else
            ITestCases_Init = 1
        End If
    End If
    
End Function

Private Sub ITestCases_SyncProviderInterface()

End Sub

Private Function ITestCases_Terminate() As Boolean
    Set xModInfo = Nothing
    Set pifObj = Nothing
    ITestCases_Terminate = True
End Function
Public Sub SetCaseError(lError As ModuleBase.IError)
    Set g_caseerrorobj = lError
    tracecase = False
End Sub
Public Sub SetCaseProvider(lprov As ModuleBase.IProviderInfo)
    Set g_caseprovobj = lprov
End Sub

Public Function validtest() As ModuleBase.tagVARIATION_STATUS
    Dim connection1 As New ADODB.Connection
    Dim recset1 As New ADODB.Recordset
    On Error GoTo ErrorHandler
    
    g_caseerrorobj.SetErrorLevel (HR_STRICT)
    g_ExpError = 0

    If tracecase Then
        g_caseerrorobj.Transmit ("inside validtest" + Chr(10))
    End If

    bTestPassed = True
    
    ' open connection
    connection1.ConnectionString = connstr
    connection1.CursorLocation = CInt(curlocstr)
    
    'Set event object to connection
    Set CEO = connection1
    
    ' Fire open event
    eventfired = ""
    connection1.Open
    If eventfired <> "WillConnect:ConnectComplete:" Then
        g_caseerrorobj.Transmit "Incorrect Event sequence!" + Chr(10)
        g_caseerrorobj.Transmit "   Expected: WillConnect:ConnectComplete:" + Chr(10)
        g_caseerrorobj.Transmit "   Actual: " + eventfired + Chr(10)
        bTestPassed = False
    End If
    
    ' Fire execute event
    eventfired = ""
    Set recset1 = connection1.Execute(rsstr)
    If eventfired <> "WillExecute:ExecuteComplete:" Then
        g_caseerrorobj.Transmit "Incorrect Event sequence!" + Chr(10)
        g_caseerrorobj.Transmit "   Expected: WillExecute:ExecuteComplete:" + Chr(10)
        g_caseerrorobj.Transmit "   Actual: " + eventfired + Chr(10)
        bTestPassed = False
    End If

    ' Fire Close event
    eventfired = ""
    connection1.Close
    If eventfired <> "Disconnect:" Then
        g_caseerrorobj.Transmit "Incorrect Event sequence!" + Chr(10)
        g_caseerrorobj.Transmit "   Expected: Disconnect:" + Chr(10)
        g_caseerrorobj.Transmit "   Actual: " + eventfired + Chr(10)
        bTestPassed = False
    End If
        
    ' Output Test pass/fail
    If (bTestPassed = False) Then
        validtest = eVariationStatusFailed
    Else
        validtest = eVariationStatusPassed
    End If
    
    Exit Function
ErrorHandler:
    ' Output error message
    bTestPassed = codelib.ErrorHandler(g_caseerrorobj, ITestCases_GetName(), g_ExpError)
    Resume Next

End Function
Public Function invalidtest() As ModuleBase.tagVARIATION_STATUS
    Dim command1 As New ADODB.Command
    On Error GoTo ErrorHandler

    bad_timeout = -999
    
    g_caseerrorobj.SetErrorLevel (HR_STRICT)
    g_ExpError = adErrObjectClosed
    
    If tracecase Then
        g_caseerrorobj.Transmit ("inside invalidtest" + Chr(10))
    End If

    bTestPassed = True
    
    ' Set expected error
    g_ExpError = adErrInvalidArgument

    command1.CommandTimeout = bad_timeout
    
    If hiterror = False Then
        bTestPassed = False
        g_caseerrorobj.Transmit ("No error raised for expected error:" + CStr(g_ExpError) + Chr(10))
    End If
   
    ' Output Test pass/fail
    If (bTestPassed = False) Then
        invalidtest = eVariationStatusFailed
    Else
        invalidtest = eVariationStatusPassed
    End If

    Exit Function
ErrorHandler:
    ' Output error message
    bTestPassed = codelib.ErrorHandler(g_caseerrorobj, ITestCases_GetName(), g_ExpError)
    hiterror = True
    Resume Next
End Function
'**********************************************************************
'                       EVENT NOTIFICATION SUBS
'**********************************************************************
Private Sub CEO_WillConnect(ConnectionString As String, UserID As String, Password As String, Options As Long, adStatus As ADODB.EventStatusEnum, ByVal pConnection As ADODB.Connection)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
g_caseerrorobj.Transmit "-----> WillConnect fired!" + Chr(10)
g_caseerrorobj.Transmit "       Status = " + CStr(adStatus) + Chr(10)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
eventfired = eventfired + "WillConnect:"
End Sub
Private Sub CEO_ConnectComplete(ByVal pError As ADODB.Error, adStatus As ADODB.EventStatusEnum, ByVal pConnection As ADODB.Connection)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
g_caseerrorobj.Transmit "-----> ConnectComplete fired!" + Chr(10)
g_caseerrorobj.Transmit "       Status = " + CStr(adStatus) + Chr(10)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
eventfired = eventfired + "ConnectComplete:"
End Sub
Private Sub CEO_InfoMessage(ByVal pError As ADODB.Error, adStatus As ADODB.EventStatusEnum, ByVal pConnection As ADODB.Connection)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
g_caseerrorobj.Transmit "-----> InfoMessage fired!" + Chr(10)
g_caseerrorobj.Transmit "       Status = " + CStr(adStatus) + Chr(10)
g_caseerrorobj.Transmit "       Message = " + pError.Description + Chr(10)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
'eventfired = eventfired + "InfoMessage:"
End Sub
Private Sub CEO_Disconnect(adStatus As ADODB.EventStatusEnum, ByVal pConnection As ADODB.Connection)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
g_caseerrorobj.Transmit "-----> Disconnect fired!" + Chr(10)
g_caseerrorobj.Transmit "       Status = " + CStr(adStatus) + Chr(10)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
eventfired = eventfired + "Disconnect:"
End Sub
Private Sub CEO_BeginTransComplete(ByVal TransactionLevel As Long, ByVal pError As ADODB.Error, adStatus As ADODB.EventStatusEnum, ByVal pConnection As ADODB.Connection)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
g_caseerrorobj.Transmit "-----> BeginTransComplete fired!" + Chr(10)
g_caseerrorobj.Transmit "       Status = " + CStr(adStatus) + Chr(10)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
eventfired = eventfired + "BeginTransComplete:"
End Sub
Private Sub CEO_CommitTransComplete(ByVal pError As ADODB.Error, adStatus As ADODB.EventStatusEnum, ByVal pConnection As ADODB.Connection)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
g_caseerrorobj.Transmit "-----> CommitTransComplete fired!" + Chr(10)
g_caseerrorobj.Transmit "       Status = " + CStr(adStatus) + Chr(10)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
eventfired = eventfired + "CommitTransComplete:"
End Sub
Private Sub CEO_RollbackTransComplete(ByVal pError As ADODB.Error, adStatus As ADODB.EventStatusEnum, ByVal pConnection As ADODB.Connection)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
g_caseerrorobj.Transmit "-----> RollbackTransComplete fired!" + Chr(10)
g_caseerrorobj.Transmit "       Status = " + CStr(adStatus) + Chr(10)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
eventfired = eventfired + "RollbackTransComplete:"
End Sub
Private Sub CEO_WillExecute(Source As String, CursorType As ADODB.CursorTypeEnum, locktype As ADODB.LockTypeEnum, Options As Long, adStatus As ADODB.EventStatusEnum, ByVal pCommand As ADODB.Command, ByVal pRecordset As ADODB.Recordset, ByVal pConnection As ADODB.Connection)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
g_caseerrorobj.Transmit "-----> WillExecute fired!" + Chr(10)
g_caseerrorobj.Transmit "       Status = " + CStr(adStatus) + Chr(10)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
eventfired = eventfired + "WillExecute:"
End Sub
Private Sub CEO_ExecuteComplete(ByVal RecordsAffected As Long, ByVal pError As ADODB.Error, adStatus As ADODB.EventStatusEnum, ByVal pCommand As ADODB.Command, ByVal pRecordset As ADODB.Recordset, ByVal pConnection As ADODB.Connection)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
g_caseerrorobj.Transmit "-----> ExecuteComplete fired!" + Chr(10)
g_caseerrorobj.Transmit "       Status = " + CStr(adStatus) + Chr(10)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
eventfired = eventfired + "ExecuteComplete:"
End Sub
Private Sub CEO_WillMove(adReason As ADODB.EventReasonEnum, adStatus As ADODB.EventStatusEnum, ByVal pRecordset As ADODB.Recordset)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
g_caseerrorobj.Transmit "-----> WillMove fired!" + Chr(10)
g_caseerrorobj.Transmit "       Status = " + CStr(adStatus) + Chr(10)
g_caseerrorobj.Transmit "       Reason = " + CStr(adReason) + Chr(10)
g_caseerrorobj.Transmit "----------------------------------" + Chr(10)
eventfired = eventfired + "WillMove:"
End Sub





