<%@ LANGUAGE="VBSCRIPT"%>
<!--#include virtual="ExpenseWeb/common.asp" -->

<HTML>
<HEAD>
<link rel="stylesheet" type="text/css" href="style.css" />
</HEAD>
<body style="background:white">

<%
Dim AzClient
Dim Results
Dim Names(5)
Dim Values(5)
Dim Scopes(5)
Dim Operations(10)
Dim Roles
Dim Role
Dim Submitter
Dim Approver
Dim Admin
Dim AccountName
Dim SamName

Initialize

SamName = GetClientSamName()


AccountName = Right( SamName, Len( SamName ) - InStr( SamName, "\" ))
Response.Write "<h3>Hello " & AccountName & "</h3>"


'--------------- Determine Role for User --------------
Set AzClient = GetAzCleintContext()

Roles = AzClient.GetRoles()

Approver = False
Admin = FALSE

If UBound(Roles) < 0 Then
   response.write "Sorry " & AccountName & " you have no Roles <Br>"
   response.write "Please contact your manager <Br>"
   
Else
   For each Role in Roles
 	If ( StrComp( Role, "Approver", 1 ) = 0 ) Then
		Approver = True
	Elseif ( StrComp( Role, "Administrator", 1 ) = 0 ) Then
		Admin = True
        Elseif ( StrComp( Role, "Submitter", 1 ) = 0 ) Then
                Submitter = True
	End If
   Next
End If

Set AzClient = Nothing

%>

<% If ( Submitter = True ) Then 
' display submit page
%>
<h2>Submitter</h2>

<center>
<a href="submit/Submit.asp" target=RightFrame> Submit Expense </a>
<br>
<br>
<a href="Status/Status.asp?Mode=0" target=RightFrame> Check Status </a>
</center>

<% End If %> 


<% If ( Approver = True ) Then 
' Allow only managers to approve expenses
%>

<br>
<h2>Approver</h2>

<center>
<a href="Status/Status.asp?Mode=1" target=RightFrame>View Pending</a>
</center>
<% End If %> 
</FORM>

</BODY>

<%

%>

</HTML>
