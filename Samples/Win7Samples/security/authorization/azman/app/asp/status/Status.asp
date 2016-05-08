<%@Language=VBScript %>
<!--#include virtual="/ExpenseWeb/common.asp" -->

<%

Sub PrintPendingTranactions
  Dim numTrans 
  Dim content
  Dim i
  Dim PageMode

  PageMode = Request.QueryString( "Mode")
  numTrans = GetNextTransaction()
  Response.Write "<p> This sample will reset itself after " & CInt( GetMaxTransaction( )  ) & " Expenses."
  Response.Write "<p> There are " & CInt( numTrans ) & " pending expense reports."
  if numTrans > 0 then
    Response.Write "<table border=1><tr><td><b>Select an expense to view</b></td><td><b>Approved</b></td></tr>"
    for i = 1 to numTrans
      Response.Write "<tr><td><a href='dispExpense.asp?transactionId=" & i & "&ShowApproval=" & PageMode & "'>Expense "& i & "</td>"
      Response.Write "<td> " & GetTransactionApprovalTime ( i ) & "</td></tr>"
    next
    Response.Write "</table>"    
  end if
End Sub

%>
<HTML>
<HEAD>
<TITLE>Online Expense Application</TITLE>
<link rel="stylesheet" type="text/css" href="/Expenseweb/style.css" />
</HEAD>
<BODY style="background:white">
<P><STRONG><FONT size=5>Pending Expense Reports</FONT></STRONG></P>
<HR>
<%
  'Call Initialize
  PrintPendingTranactions
%>


</BODY>
</HTML>
