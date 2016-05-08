<%@ LANGUAGE="VBSCRIPT" %>
<!--#include virtual="/ExpenseWeb/common.asp" -->

<%
function ValidateParams
Dim Amount,Desc,Date

	Amount = Request.QueryString("amount")
	Desc = Request.QueryString("desc")
	Date = Request.QueryString("date")

	If Len(Amount) > 9 or Len(Desc) > 100 Then
    		Response.Write "Amount field is too long, please try again."
	Else
   		' Validate amount submitted as a numeric value
   		Set reg = New RegExp 
   		reg.Pattern = "\D+" ' Look for something other than 0-9's.

   		if reg.Test(Amount) = False then 
            		ValidateParams = True
        	Else
           		Response.Write "Must enter number, please try again"
        	End if
        End if

End function

Sub QueueExpense
   Dim intNextIdNumber
   Dim ExpenseData

	ExpenseData = Request.ServerVariables("LOGON_USER") & "-"
	ExpenseData = ExpenseData & "Date,"& Request.QueryString("date")  & VBNewLine
	ExpenseData = ExpenseData & "Description," &  Request.QueryString("desc") & VBNewLine
        ExpenseData = ExpenseData & "Amount," & Request.QueryString("amount") & VBNewLine

	intNextIdNumber = AssignNextTransaction 
	SaveTransaction intNextIdNumber, ExpenseData
End Sub

Sub ShowResults (Result)
' Print the results
	Response.Write "<h3>The following expense was " & Result & "</h3>"
	Response.Write "<table>"
	Response.Write "<tr>"
		Response.Write "<th>Date</td>"
		Response.Write "<th>Description</td>"
		Response.Write "<th>Amount</td>"
	Response.Write "</tr>"
	Response.Write "<tr>"
		Response.Write "<td>" & Request.QueryString("date") & "</td>"
		Response.Write "<td>" & Request.QueryString("desc") & "</td>"
		%> <td align="right"> <% Response.write Request.QueryString("amount") & "</td>"
	Response.Write "</tr>"
	Response.Write "</table>"
End Sub


%>

<HTML>
<HEAD>
<link rel="stylesheet" type="text/css" href="/ExpenseWeb/style.css" />
</HEAD>
<BODY style="background:white">

<%
'THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
'ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
'TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
'PARTICULAR PURPOSE.


Dim AzClient

Dim BizNames(5)
Dim BizValues(5)
Dim Scopes(5)
Dim Operations(10)


Dim Reg
Dim Results
Dim Result : Result = "Denied."


Set AzClient = GetAzCleintContext()


'Setup biz rule params. 
'To optomize performance names/value pairs must be placed in Array alphabitically

BizNames(0) = "ExpAmount"
BizValues(0) = 0
Operations(0) = AZOP_SUBMIT


if ValidateParams Then
      BizValues(0) = Clng(Request.QueryString("amount"))


      Results = AzClient.AccessCheck("Submit Expense", Empty, Operations, BizNames, BizValues)
     
      If Results(0) = NO_ERROR Then 
	QueueExpense
	Result = "accepted."
      End If

      ShowResults Result
      
End if

Set AzClient = Nothing
%>

</BODY>
</HTML>
