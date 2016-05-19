'*****************************************************************************
'
' Microsoft Windows Media
' Copyright (C) Microsoft Corporation. All rights reserved.
'
' FileName: VBDBLoggingPlugin.vb
'
' Abstract:            
'
'*****************************************************************************

Imports Microsoft.WindowsMediaServices.Interop
Imports System.Data.SqlClient
Imports System.Text
Imports System.Runtime.InteropServices
Imports Microsoft.Win32

Public Class VBDBBasePlugin
    Implements IWMSBasicPlugin

    Public m_strConnectString As String
    Public m_strCreateTable As String
    Private m_UserDatabase As SqlConnection
    Private m_NonQueryCommand As SqlCommand

    Public Overloads Sub InitializePlugin( _
        ByVal ServerContext As IWMSContext, _
        ByVal NamedValues As WMSNamedValues, _
        ByVal ClassFactory As IWMSClassObject) _
        Implements IWMSBasicPlugin.InitializePlugin

        m_UserDatabase = New SqlConnection(m_strConnectString)
    End Sub

    Public Overloads Sub ShutdownPlugin() _
        Implements IWMSBasicPlugin.ShutdownPlugin

    End Sub

    Public Overloads Sub EnablePlugin( _
        ByRef Flags As Integer, _
        ByRef HeartbeatPeriod As Integer) _
        Implements IWMSBasicPlugin.EnablePlugin

        m_UserDatabase.Open()
        m_NonQueryCommand = New SqlCommand("", m_UserDatabase)
        Dim iAffectedRows As Integer = 0
        Try
            ExecuteNonQueryCommand(m_strCreateTable, iAffectedRows)

        Catch ESql As SqlException
            ' only catch sql exception with error number &H9A9
            ' this error happens when the table already exists
            ' and the plug-in tries to create it again
            If ESql.Number <> &HA9A Then
                Throw ESql
            End If
        End Try
    End Sub

    Public Overloads Sub DisablePlugin() _
        Implements IWMSBasicPlugin.DisablePlugin

        m_UserDatabase.Close()
    End Sub

    Public Overloads Function GetCustomAdminInterface() As Object _
        Implements IWMSBasicPlugin.GetCustomAdminInterface

        GetCustomAdminInterface = Nothing
    End Function

    Public Overloads Sub OnHeartBeat() _
        Implements IWMSBasicPlugin.OnHeartbeat

    End Sub

    Public Function ExecuteNonQueryCommand( _
        ByRef Cmd As String, _
        ByRef iAffectedRows As Integer) _
        As Boolean

        Dim ret As Boolean = False
        m_NonQueryCommand.CommandText = Cmd
        iAffectedRows = m_NonQueryCommand.ExecuteNonQuery()
        ret = True
        Return ret
    End Function

End Class

<GuidAttribute(VBDBLoggingPlugin.ClassId)> _
Public Class VBDBLoggingPlugin
    Inherits VBDBBasePlugin
    Implements IWMSEventNotificationPlugin

    Public Const ClassId As String = "72CE1F3D-DCA1-4AA7-8C65-356CD0B07D46"
    Public Const strSubKey1 As String = "SOFTWARE\\Microsoft\\Windows Media\\Server\\RegisteredPlugins\\Event Notification and Authorization\\{" + VBDBLoggingPlugin.ClassId + "}"
    Public Const strSubKey2 As String = "CLSID\\{" + VBDBLoggingPlugin.ClassId + "}\\Properties"

    Private strSummaryOpenTag As String = "<Summary>"
    Private strSummaryCloseTag As String = "</Summary>"
    Private iMaxField As Integer = 52

    ' 
    ' TODO:
    '   Change strDataSource to your SQL server name
    '   Change strUserId to your SQL user name
    '   Change strPassword to your SQL user password
    '
    Private strDataSource As String = ""
    Private strUserId     As String = ""
    Private strPassword   As String = ""

    Private strDatabase   As String = "WmsLog"
    Private strTableName  As String = "WmsLogs"

    ' A creatable COM class must have a Public Sub New() 
    ' with no parameters, otherwise, the class will not be 
    ' registered in the COM registry and cannot be created 
    ' via CreateObject.
    Public Sub New()
        MyBase.New()
        m_strConnectString = "User ID=" + strUserId _
                            + ";Password=" + strPassword _
                            + ";Data Source=" + strDataSource _
                            + ";Initial Catalog=" + strDatabase
        Dim strCreateTable As Text.StringBuilder
        strCreateTable = New Text.StringBuilder()
        strCreateTable.AppendFormat("create table {0} (", strTableName)
        Dim i As Integer
        For i = 1 To iMaxField
            strCreateTable.AppendFormat("f{0:##} char(50),", i)
        Next
        strCreateTable.Chars(strCreateTable.Length - 1) = ")"
        m_strCreateTable = strCreateTable.ToString

    End Sub

    Public Overloads Function GetHandledEvents() As Object _
        Implements IWMSEventNotificationPlugin.GetHandledEvents

        Dim events As Integer() = {WMS_EVENT_TYPE.WMS_EVENT_LOG}
        GetHandledEvents = events
    End Function

    Public Overloads Sub OnEvent( _
        ByRef ServerEvent As WMS_EVENT, _
        ByVal UserCtx As IWMSContext, _
        ByVal PresentationCtx As IWMSContext, _
        ByVal CommandCtx As IWMSCommandContext) _
        Implements IWMSEventNotificationPlugin.OnEvent

        Dim CommandRequest As IWMSContext
        Dim ContextBody As Object
        Dim NsBuffer As INSSBuffer
        Dim strContext As String
        Dim iBegin As Integer
        Dim iEnd As Integer
        Dim strSummary As String

        Try
            CommandCtx.GetCommandRequest(CommandRequest)
            CommandRequest.GetIUnknownValue( _
               "@WMS_COMMAND_CONTEXT_BODY", 11, ContextBody, 0)

            NsBuffer = ContextBody
            If NsBuffer Is Nothing Then
                Exit Sub
            End If

            strContext = GetStringFromNSSBuffer(NsBuffer)

            ' find the beginning of the summary open tag
            iBegin = strContext.IndexOf(strSummaryOpenTag)
            If -1 = iBegin Then
                Exit Sub
            End If
            iBegin = iBegin + strSummaryOpenTag.Length

            ' find the end of the summary tag
            iEnd = strContext.IndexOf(strSummaryCloseTag)
            If -1 = iEnd Then
                Exit Sub
            End If

            If (iEnd > iBegin) Then
                strSummary = strContext.Substring(iBegin, iEnd - iBegin)
                LogToDb(strSummary)
            End If
        Catch
        End Try
    End Sub

    Private Function GetStringFromNSSBuffer(ByRef NsBuffer As INSSBuffer) As String
        Dim s As String = ""
        GetStringFromNSSBuffer = ""
        Try
            Dim uBufSize As UInt32
            Dim pbuf As IntPtr = New IntPtr()
            NsBuffer.GetBufferAndLength(pbuf, uBufSize)
            Dim iBufSize As Int32 = Convert.ToInt32(uBufSize) / 2
            s = Marshal.PtrToStringUni(pbuf, iBufSize)
            GetStringFromNSSBuffer = s
        Catch
        End Try
    End Function

    Private Function LogToDb(ByRef strSummary As String) As Boolean
        Dim ret As Boolean = False

        ' split the log fields
        Dim LogFields As String()
        LogFields = strSummary.Split(" ")

        Try
            ' construct the command string
            Dim cmd As System.Text.StringBuilder
            cmd = New Text.StringBuilder()
            cmd.AppendFormat("Insert into {0} values (", strTableName)
            Dim i As Integer

            For i = 0 To iMaxField - 1
                Dim strField As String
                strField = "-"
                If LogFields.Length > i Then
                    strField = LogFields(i)
                End If

                ' 50 is the max column length
                If 50 < strField.Length Then
                    strField = strField.Substring(0, 50)
                End If

                strField.Replace("\'", "\'\'")
                cmd.AppendFormat("'{0}',", strField)
            Next i
            cmd.Chars(cmd.Length - 1) = ")"

            Dim iAffectedRows As Integer = 0
            ret = ExecuteNonQueryCommand(cmd.ToString, iAffectedRows)
        Catch
        End Try
        LogToDb = ret

    End Function

    <ComRegisterFunctionAttribute()> _
    Shared Sub RegisterFunction(ByVal t As Type)
        Try
            Dim regHKLM As RegistryKey
            regHKLM = Registry.LocalMachine
            regHKLM = regHKLM.CreateSubKey(strSubKey1)
            regHKLM.SetValue(Nothing, "WMS SDK Sample VB.Net Database Logging Plugin")

            Dim regHKCR As RegistryKey
            regHKCR = Registry.ClassesRoot
            regHKCR = regHKCR.CreateSubKey(strSubKey2)
            regHKCR.SetValue("Name", "WMS SDK Sample VB.Net Database Logging Plugin")
            regHKCR.SetValue("Author", "Microsoft Corporation")
            regHKCR.SetValue("CopyRight", "Copyright (c) Microsoft Corporation.  All rights reserved.")
            regHKCR.SetValue("SubCategory", "Logging")
            regHKCR.SetValue("Description", "Enables you to log activity data for players that are connected over a unicast stream.")

        Catch e As Exception
            MsgBox(e.Message, MsgBoxStyle.OKOnly)
        End Try
    End Sub

    <ComUnregisterFunction()> _
    Shared Sub UnRegisterFunction(ByVal t As Type)

        Try
            Dim regHKLM As RegistryKey
            regHKLM = Registry.LocalMachine
            regHKLM.DeleteSubKey(strSubKey1)
        Catch
            ' ignore error
        End Try

        Try
            Dim regHKCR As RegistryKey
            regHKCR = Registry.ClassesRoot
            regHKCR.DeleteSubKeyTree(strSubKey2)
        Catch
            ' ignore error
        End Try

    End Sub

End Class

