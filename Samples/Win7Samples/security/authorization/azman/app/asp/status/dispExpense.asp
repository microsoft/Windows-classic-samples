<%@Language=VBScript %>
<!--#include virtual="/ExpenseWeb/common.asp" -->

<SCRIPT LANGUAGE="VBscript">

Function ApproveIt
on error resume next
  Dim oSignedData
  Dim strDataToSign
  
  'strDataToSign = Transaction.rawData.value 
  'Transaction.signedData.value = 
  ApproveIt = True

End function
</SCRIPT>

<%

Sub ParseOriginalContent (strOriginalContent )
  
  Dim arrLineArray
  Dim arrFieldArray
  Dim strDetail
  Dim i
 
  'Response.Write strOriginalContent
  arrLineArray = Split(strOriginalContent, vbNewLine, -1, 1)
  for i = 0 to uBound(arrLineArray) -1
    arrFieldArray = Split(arrLineArray(i), ",", -1, 1)
    strDetail = "<tt>"
    strDetail = strDetail & arrFieldArray(0)  & "&nbsp;&nbsp;&nbsp;&nbsp;" & arrFieldArray(1) 
    strDetail = strDetail & "</tt><BR>"
    Response.Write strDetail
    
  next

end sub

Sub DisplayTrans ( ShowApprove )
  on error resume next
  Dim intTransactionID
  Dim ExpenseData
  Dim strOriginalContent 
  Dim oSignedData
  Dim approvalTime
  Dim AccountName

  intTransactionID = Request.QueryString("transactionId")
 
  ExpenseData = GetTransData( intTransactionID )
  

  AccountName = Left( ExpenseData , InStr( ExpenseData, "-" ) - 1 )

  Response.Write "<h3>Expense " & intTransactionID  & "</h3>"
  Response.Write "<table border=1><tr><td><b> Submitter Name</b></td><td><b>Details</b></td></tr>"
  Response.Write "<tr><td>" & AccountName & "&nbsp&nbsp</td><td>"
  ParseOriginalContent Right ( ExpenseData , Len( ExpenseData ) - InStr( ExpenseData, "-" ) )

  approvalTime = GetTransactionApprovalTime ( intTransactionID )
  if Len ( approvalTime ) > 0 Then
    Response.Write "<tr><td>" & FormatStrong ( "Approved" ) & "</td><td>" & FormatStrong ( approvalTime ) & "</td></tr>"
    Response.Write "</td></tr></table>"
  Else 
    Response.Write "</td></tr></table>"

    If ShowApprove then
       Response.Write "<h3>Click the button to approve the transaction</h3>"
       Response.Write "<FORM NAME=""Transaction"" ACTION=""/expenseweb/approve/approver_srv.asp"" METHOD=""post"" >"
       Response.Write "<INPUT TYPE=""Hidden"" NAME=""transactionID"" VALUE=""" & intTransactionID &""" >"
       Response.Write "<INPUT TYPE=""Hidden"" NAME=""signedData"" >"
       Response.Write "<INPUT TYPE=""Hidden"" NAME=""rawdata"" value=""" & ExpenseData & """ >"
       Response.Write "<P><INPUT id=submit1 style='LEFT: 10px; TOP: 213px' type=submit value=Approve name=submit1 onClick=""ApproveIt""></P>"
       Response.Write " </FORM>"
    End If
  End If
  Set oSignedData = Nothing

End Sub

%>
<HTML>
<HEAD>
<TITLE>Online Expense Application - Display Expense Details</TITLE>
<link rel="stylesheet" type="text/css" href="/ExpenseWeb/style.css" />
</HEAD>
<BODY style="background:white">
<P><STRONG><FONT size=5>Display Expense Details</FONT></STRONG></P><STRONG><FONT size=5>
<HR>
</FONT></STRONG>

<%

  DisplayTrans CInt( Request.QueryString("ShowApproval") )
%>

</BODY>
</HTML>
