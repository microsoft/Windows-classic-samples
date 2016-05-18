VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "adoupdat_tm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Implements ITestModule

Const casecount = 5
'Dim g_caseobj As Variant
Dim g_provobj As ModuleBase.IProviderInfo
Dim g_tm As adoupdat.adoupdat_tm
Dim g_errorobj As ModuleBase.IError
Dim tracemod As Boolean
Dim cases(casecount) As Variant
Dim numcases As Integer

Private Function ITestModule_GetDescription() As String
    ITestModule_GetDescription = "ADO Provider Tests - Updatable"
End Function
Private Function ITestModule_GetName() As String
    If tracemod Then
        g_errorobj.Transmit "Inside: ITestModule_GetName" + Chr(13)
    End If
    ITestModule_GetName = "ADO Updatable Conformance Tests"
End Function
Private Function ITestModule_GetOwnerName() As String
    ITestModule_GetOwnerName = "WayneL"
End Function
Private Function ITestModule_GetVersion() As Long
    ITestModule_GetVersion = 1   ' version number
End Function
Private Function ITestModule_GetCLSID() As String
    ITestModule_GetCLSID = "adoupdat.adoupdat_tm"
End Function
Private Function ITestModule_Init() As Long
    Set g_tm = Me
    ITestModule_Init = 1
End Function
Private Function ITestModule_GetCaseCount() As Long
    ITestModule_GetCaseCount = casecount
End Function
Private Function ITestModule_GetCase(ByVal lIndex As Long) As ModuleBase.ITestCases
    If tracemod Then
        g_errorobj.Transmit "Inside: ITestModule_GetCase(" + CStr(lIndex) + ")" + Chr(13)
    End If
    numcases = numcases + 1
    Select Case lIndex
        Case 0
             Set cases(numcases - 1) = CreateObject("adoupdat.rscancelupdate")
       Case 1
            Set cases(numcases - 1) = CreateObject("adoupdat.rsaddnew")
        Case 2
            Set cases(numcases - 1) = CreateObject("adoupdat.rsdelete")
        Case 3
            Set cases(numcases - 1) = CreateObject("adoupdat.rsupdate")
        Case 4
            Set cases(numcases - 1) = CreateObject("adoupdat.rseditmode")
   End Select
    cases(numcases - 1).SetCaseError g_errorobj
    cases(numcases - 1).SetCaseProvider g_provobj
    Set Locprov = g_provobj
    Set ITestModule_GetCase = cases(numcases - 1)
End Function
Private Function ITestModule_GetErrorInterface() As ModuleBase.IError
    Set ITestModule_GetErrorInterface = g_errorobj
End Function
Private Function ITestModule_GetProviderInterface() As ModuleBase.IProviderInfo
    Set ITestModule_GetProviderInterface = g_provobj
End Function
Private Sub ITestModule_SetErrorInterface(ByVal pError As ModuleBase.IError)
    Set g_errorobj = pError
    If tracemod Then
        g_errorobj.Transmit "Inside: ITestModule_SetErrorInterface" + Chr(13)
    End If
End Sub
Private Sub ITestModule_SetMallocSpyCallback(pbVoodoo As Byte)
    tracemod = False
    numcases = 0
    'MsgBox ("ITestModule_SetMallocSpyCallback")
End Sub
Private Sub ITestModule_SetProviderInterface(ByVal pProvInfo As ModuleBase.IProviderInfo)
On Error GoTo ixx
    Set g_provobj = pProvInfo
    For i = 0 To numcases - 1
        cases(i).SetCaseProvider g_provobj
    Next i
Exit Sub
ixx:
MsgBox Err.Description
End Sub
Private Function ITestModule_Terminate() As Boolean
    ITestModule_Terminate = True
End Function
