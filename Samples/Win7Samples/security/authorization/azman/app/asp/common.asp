<%
Option Explicit
Const ForReading = 1
Const ForWriting = 2
Const ForAppending = 8
Const AZOP_SUBMIT = 1
Const AZOP_APPROVE = 2
Const AZOP_READ = 3
Const AZOP_LIST = 4
Const NO_ERROR = 0


Const DATASTORE_TRANSPREFIX = "Expense.Trans." 
Const DATASTORE_TRANSSUFFIX = ".approved"
Const DATASTORE_LASTTRANS = "Expense.LastTransaction"
Const DATASTORE_MAXTRANS = "Expense.MaxTrans"
Const EXPENSE_INIT = "Expense.Initialized"
Const AZMAN_STORE_URL = "msxml://c:\AzStore.xml"
Const AZMAN_STORE = "AzManStore"
Const AZMAN_APP_NAME = "Expense Web"
Const AZMAN_APP = "AzManApp"
Const AZMAN_CLIENT = "AzClient"
Const CLIENT_SAM_NAME = "ClientSamName"


''''''''''''''''''''''''''''''
'' Set the SELF_APPROVAL constant to False to enable the AD integration
''''''''''''''''''''''''''''''
Dim bInitialized

Sub PrintErrorMessage( strErrorMsg )
  Response.Write "<FONT size=5 color=""red""><STRONG>"
  Response.Write strErrorMsg
  Response.Write "</STRONG></FONT></P>"
  err.clear
End Sub


Function FormatStrong ( msg ) 
  FormatStrong = "<FONT color=""red""><STRONG>" & msg & "</STRONG></FONT>"

End Function

Sub Initialize
    Dim AzManStore
    Dim AzManApp
    Dim AzManClientContext

    Application.Lock
    if Not CBool( Application(EXPENSE_INIT) ) Then
        Application(DATASTORE_MAXTRANS) = 10
        Application(EXPENSE_INIT) = True
    end If

    Session( CLIENT_SAM_NAME ) = Request.ServerVariables("LOGON_USER")

    '--------------- Create Authorization Store Object --------------
    Set AzManStore = CreateObject("Azroles.AzAuthorizationStore")
    Set Session( AZMAN_STORE ) = AzManStore

    '--------------- Load the desired store  --------------
    AzManStore.Initialize 0, AZMAN_STORE_URL

    '--------------- Open our application --------------
    Set AzManApp = AzManStore.OpenApplication ( AZMAN_APP_NAME )
    Set Session( AZMAN_APP ) = AzManApp

    '--------------- Create Client Context --------------
    Set AzManClientContext = AzManApp.InitializeClientContextFromName( GetClientSamName() )
    Set Session( AZMAN_CLIENT ) = AzManClientContext

    Application.Unlock
End Sub

Function GetClientSamName
    GetClientSamName = Session(CLIENT_SAM_NAME)
End Function

Function GetAzCleintContext
    Set GetAzCleintContext = Session(AZMAN_CLIENT)
End Function

'''''''''''''''''''''''''''''''''''''''
'' Persistence functions
''
'' You should modify these functions to persist the transactions
'' to the data store of your choosing.  In this sample, they are
'' stored using the Application global objects.  The values in the
'' Application object are lost when the server is restarted.
''
'' AssignNextTransaction 
'' GetNextTransaction
'' SaveTransaction
'' GetTransData
'' IsTransactionApproved
'' ApproveTransaction
'' GetTransactionApprovalTime
''
'''''''''''''''''''''''''''''''''''''''

Function IsTransactionApproved ( intTransactionID )
   If Len ( Application(DATASTORE_TRANSPREFIX & intTransactionID & DATASTORE_TRANSSUFFIX ) ) > 0 Then
     IsTransactionApproved = True
   Else
     IsTransactionApproved = False
   End If
end Function

Sub ApproveTransaction ( intTransactionID )
   Application(DATASTORE_TRANSPREFIX & intTransactionID & DATASTORE_TRANSSUFFIX ) = Now
End Sub

Function GetTransactionApprovalTime ( intTransactionID )
   GetTransactionApprovalTime = Application(DATASTORE_TRANSPREFIX & intTransactionID & DATASTORE_TRANSSUFFIX )
End Function

Function AssignNextTransaction 
  if CInt( Application( DATASTORE_LASTTRANS ) )  >= GetMaxTransaction Then
    ClearTrans
  end if
  Application.Lock
  Application( DATASTORE_LASTTRANS ) = Application( DATASTORE_LASTTRANS ) + 1
  AssignNextTransaction = Application( DATASTORE_LASTTRANS ) 
  Application.Unlock
End Function

Function GetNextTransaction
  GetNextTransaction = Application( DATASTORE_LASTTRANS )
End Function

sub SaveTransaction ( intNextIdNumber, strSignedData )
  Application(DATASTORE_TRANSPREFIX & intNextIdNumber) =  strSignedData
end sub

Function GetTransData ( intTransactionID ) 
  GetTransData = Application(DATASTORE_TRANSPREFIX & intTransactionID )
End Function

Sub ClearTrans
  Application.Lock
  Dim count 
  Dim i 
  count = GetNextTransaction
  for i = 1 to count
    Application(DATASTORE_TRANSPREFIX & i & DATASTORE_TRANSSUFFIX ) = ""
    Application(DATASTORE_TRANSPREFIX & i ) = ""
    Application( DATASTORE_LASTTRANS ) = 0
  next 
  Application.Unlock
End Sub

Function GetMaxTransaction
  GetMaxTransaction = Application(DATASTORE_MAXTRANS)
End Function

Function SetMaxTransaction ( max )
  Application(DATASTORE_MAXTRANS) = CInt ( max )
End Function
''''''''''''''''''''''''''''''''''''''''
''
''  Approver Lookup function
''
''''''''''''''''''''''''''''''''''''''''
Function GetSelfApproval
    GetSelfApproval = CBool (Application (SELF_APPROVAL))
End Function

Function SetApproval ( setting ) 
    Application (SELF_APPROVAL) = setting
End Function

Function LookupMgrinAD( strEmailAddress ) 
on error resume next
  Dim strQuery, strNamingContext
  Dim oConnection
  Dim oCmd
  Dim oRecordset
  Dim strADOQuery
  Dim oUser
  Dim oRootDSE
  Dim strOutput
  Dim vProp
  Dim strProp
  if GetSelfApproval Then
     LookupMgrinAD = strEmailAddress
     exit function
  end if
  Set oRootDSE = GetObject("LDAP://RootDSE")
  strNamingContext = oRootDSE.Get("defaultNamingContext")
  
  strQuery = "<LDAP://" & strNamingContext & ">;(&(objectCategory=person)(objectClass=user)(mail=" &   strEmailAddress & "));cn,adspath;subtree"

  ' -- Set up the connection ---
  Set oConnection = CreateObject("ADODB.Connection")
  Set oCmd = CreateObject("ADODB.Command")

  oConnection.Provider = "ADsDSOObject"
  oConnection.Open "ADs Provider"
  Set oCmd.ActiveConnection = oConnection
  'oCmd.Properties("Chase referrals") = ADS_CHASE_REFERRALS_EXTERNAL ‘64
  oCmd.CommandText = strQuery
  Set oRecordset = oCmd.Execute
  If oRecordset.RecordCount = 0 Then
    PrintErrorMessage "The user could not be found in the directory<p>e-mail address:<tt>"& strEmailAddress & "</tt>"
    Exit function
  End If
  If oRecordset.RecordCount <> 1 Then
    PrintErrorMessage "Did not find exactly one " & strUserCommonName & vbLf & "<P>"
    Exit function
  End If
  oRecordset.MoveFirst
  Set oUser = GetObject(oRecordset.Fields("ADsPath"))  'Use the returned ADsPath to bind to the user
  Set oUser = GetObject("LDAP://" & oUser.Manager) 'Get the manager's object
  LookupMgrinAD = oUser.EmailAddress
End function


%>

