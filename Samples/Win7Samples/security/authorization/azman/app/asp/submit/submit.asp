<%@ LANGUAGE="VBSCRIPT"%>

<HTML>

<HEAD>
<link rel="stylesheet" type="text/css" href="/Expenseweb/style.css" />
</HEAD>
<body style="background:white">

<FORM METHOD="Get" ACTION="SubmitEx.asp" target = "RightFrame">
<P><STRONG><FONT size=5>Submit Expense Report</FONT></STRONG></P>
<HR>
<table>
	<tr>
		<td align="right">Date:</td>
		<td><input type="text" name="Date" Value=<%response.write date%>></td>
	</tr>
	<tr>
		<td align="right">Description:</td>
		<td><input type="text" name="Desc"></td>
	</tr>
	<tr>
		<td align="right">Amount:</td>
		<td><input type="text" name="Amount"></td>
	</tr>
</table>
<br>
<center>
<input type="Submit" value="Submit">
</center>

</FORM>

</BODY>

<%

%>

</HTML>
