VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "adolvl0_tm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Implements ITestModule

Const casecount = 31

'Dim g_caseobj As Variant
Dim g_provobj As ModuleBase.IProviderInfo
Dim g_tm As adolvl0.adolvl0_tm
Dim g_errorobj As ModuleBase.IError
Dim tracemod As Boolean
Dim cases(casecount) As Variant
Dim numcases As Integer

Public Function ITestModule_GetDescription() As String
    ITestModule_GetDescription = "ADO Provider Tests - Level 0"
End Function
Public Function ITestModule_GetName() As String
    If tracemod Then
        g_errorobj.Transmit "Inside: ITestModule_GetName" + Chr(10)
    End If
    ITestModule_GetName = "ADO Level 0 Conformance Tests"
End Function
Public Function ITestModule_GetOwnerName() As String
    ITestModule_GetOwnerName = "WayneL"
End Function
Public Function ITestModule_GetVersion() As Long
    ITestModule_GetVersion = 1   ' version number
End Function
Public Function ITestModule_GetCLSID() As String
    ITestModule_GetCLSID = "adolvl0.adolvl0_tm"
End Function
Public Function ITestModule_Init() As Long
    Set g_tm = Me
    ITestModule_Init = 1
End Function
Public Function ITestModule_GetCaseCount() As Long
    ITestModule_GetCaseCount = casecount
End Function
Public Function ITestModule_GetCase(ByVal lIndex As Long) As ModuleBase.ITestCases
    If tracemod Then
        g_errorobj.Transmit "Inside: ITestModule_GetCase(" + CStr(lIndex) + ")" + Chr(10)
    End If
    numcases = numcases + 1
    Select Case lIndex
        Case 0
            Set cases(numcases - 1) = CreateObject("adolvl0.cnclose")
        Case 1
            Set cases(numcases - 1) = CreateObject("adolvl0.cndefdat")
        Case 2
            Set cases(numcases - 1) = CreateObject("adolvl0.cnexec")
        Case 3
            Set cases(numcases - 1) = CreateObject("adolvl0.cnmode")
        Case 4
            Set cases(numcases - 1) = CreateObject("adolvl0.cnopen")
        Case 5
            Set cases(numcases - 1) = CreateObject("adolvl0.cnprop")
        Case 6
            Set cases(numcases - 1) = CreateObject("adolvl0.cnprovider")
        Case 7
            Set cases(numcases - 1) = CreateObject("adolvl0.cnstring")
        Case 8
            Set cases(numcases - 1) = CreateObject("adolvl0.cntimeout")
        Case 9
            Set cases(numcases - 1) = CreateObject("adolvl0.fldactualsize")
        Case 10
            Set cases(numcases - 1) = CreateObject("adolvl0.fldattributes")
        Case 11
            Set cases(numcases - 1) = CreateObject("adolvl0.flddefinedsize")
        Case 12
            Set cases(numcases - 1) = CreateObject("adolvl0.fldname")
        Case 13
            Set cases(numcases - 1) = CreateObject("adolvl0.fldoriginalvalue")
        Case 14
            Set cases(numcases - 1) = CreateObject("adolvl0.fldprecision")
        Case 15
            Set cases(numcases - 1) = CreateObject("adolvl0.fldtype")
        Case 16
            Set cases(numcases - 1) = CreateObject("adolvl0.fldvalue")
        Case 17
            Set cases(numcases - 1) = CreateObject("adolvl0.rsactivecn")
        Case 18
            Set cases(numcases - 1) = CreateObject("adolvl0.rsclose")
        Case 19
            Set cases(numcases - 1) = CreateObject("adolvl0.rsmove")
        Case 20
            Set cases(numcases - 1) = CreateObject("adolvl0.rsmovefirst")
        Case 21
            Set cases(numcases - 1) = CreateObject("adolvl0.rsmovenext")
        Case 22
            Set cases(numcases - 1) = CreateObject("adolvl0.rsmoveprev")
        Case 23
            Set cases(numcases - 1) = CreateObject("adolvl0.rsopen")
        Case 24
            Set cases(numcases - 1) = CreateObject("adolvl0.rssource")
        Case 25
            Set cases(numcases - 1) = CreateObject("adolvl0.rssupports")
        Case 26
            Set cases(numcases - 1) = CreateObject("adolvl0.rsbof")
        Case 27
            Set cases(numcases - 1) = CreateObject("adolvl0.rseof")
        Case 28
            Set cases(numcases - 1) = CreateObject("adolvl0.rscachesize")
        Case 29
            Set cases(numcases - 1) = CreateObject("adolvl0.rspagesize")
        Case 30
            Set cases(numcases - 1) = CreateObject("adolvl0.rsrequery")
  End Select
    cases(numcases - 1).SetCaseError g_errorobj
    cases(numcases - 1).SetCaseProvider g_provobj
    Set Locprov = g_provobj
    Set ITestModule_GetCase = cases(numcases - 1)
End Function
Public Function ITestModule_GetErrorInterface() As ModuleBase.IError
    Set ITestModule_GetErrorInterface = g_errorobj
End Function
Public Function ITestModule_GetProviderInterface() As ModuleBase.IProviderInfo
    Set ITestModule_GetProviderInterface = g_provobj
End Function
Public Sub ITestModule_SetErrorInterface(ByVal pError As ModuleBase.IError)
    Set g_errorobj = pError
    If tracemod Then
        g_errorobj.Transmit "Inside: ITestModule_SetErrorInterface" + Chr(10)
    End If
End Sub
Public Sub ITestModule_SetMallocSpyCallback(pbVoodoo As Byte)
    tracemod = False
    numcases = 0
    'MsgBox ("ITestModule_SetMallocSpyCallback")
End Sub
Public Sub ITestModule_SetProviderInterface(ByVal pProvInfo As ModuleBase.IProviderInfo)
On Error GoTo ixx
    Set g_provobj = pProvInfo
    For i = 0 To numcases - 1
        cases(i).SetCaseProvider g_provobj
    Next i
Exit Sub
ixx:
MsgBox Err.Description
End Sub
Public Function ITestModule_Terminate() As Boolean
    ITestModule_Terminate = True
End Function
