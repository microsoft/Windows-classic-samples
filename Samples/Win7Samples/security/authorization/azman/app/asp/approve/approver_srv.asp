<%@Language=VBScript %>
<!--#include virtual="ExpenseWeb/common.asp" -->
<%
Response.Expires = 0

Dim strFilename
Dim strSignedData
Dim strOriginalContent
Dim bFlag

Function Authorize ( strSignedData )
'  On Error Resume Next
  Dim retval
  Dim AzClient
  Dim Scopes(5)
  Dim Operations(5)
  Dim Names (2)
  Dim Values(2)
  Dim Results

	
  
  retval = False
  Err.Clear

  Set AzClient = GetAzCleintContext()

  Scopes(0) = ""     'empty string means application scope
  Operations(0) = AZOP_APPROVE  'Order bizRules in Array alphabitically
  Names(0) = "Submitter"
  Values(0) = "emailaddress"

  Results = AzClient.AccessCheck("Audit string",Scopes, Operations, Names, Values)


  if Results(0) = NO_ERROR then

     '
     ' access check succeeded
     '

     Response.Write "<h2>"  & "Expense approved"  & "</h2>"
     retval = True
  else
     Response.Write "<h2>" & FormatStrong ( "You are not authorized to approve this request" ) & "</h2>"
     retval = False
   End if

  Set AzClient = nothing

  Authorize = retval

End Function


%>

<HTML>
<HEAD>
<TITLE>The Online Expense Application - Expense approval Screen</TITLE>
<link rel="stylesheet" type="text/css" href="../style.css" />
</HEAD>
<BODY style="background:white">
<P><STRONG><FONT size=5>Approve Expense Report</FONT></STRONG></P>
<HR>
<P><STRONG><FONT size=4>Expense:&nbsp;&nbsp;&nbsp;
<%
dim intTransactionID
dim ExpenseData

intTransactionID = Request.Form("transactionID")
Response.Write intTransactionID

response.write "</FONT></STRONG></P>"

ExpenseData = Request.Form("signedData")

If IsTransactionApproved ( intTransactionID ) Then
  PrintErrorMessage "This transaction has already been approved."
Else  
    ' Authorization Manager AccessCheck Authorize
    if Authorize (ExpenseData) then
       ApproveTransaction intTransactionID
    End if
End If

%>

</BODY>
</HTML>
