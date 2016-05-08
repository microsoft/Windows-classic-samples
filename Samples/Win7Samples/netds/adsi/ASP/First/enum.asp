<html>

<head>
<meta name="GENERATOR" content="Microsoft FrontPage 3.0">
<title>Enumerating a Computer Object using ADSI and ASP</title>
</head>
<%
  compName = Request.Form("computer")
  usrName = Request.Form("userName")
  password = Request.Form("password")

  adsPath = "WinNT://" & compName & ",computer"
  Set dso = GetObject("WinNT:")
  Set comp = dso.OpenDSObject(adsPath, userName, password, 1)

%>

<body>

<p><small><font face="Verdana"><strong><font color="#0080C0">Computer Name</font></strong>:
<%Response.Write comp.Name%></font></small></p>

<p><font face="Verdana"><small>Contains the following objects:</small></font></p>

<table border="1" width="100%" cellspacing="0" cellpadding="0">
<%for each obj in comp %>
  <tr>
    <td width="35%"><small><font face="Verdana">&nbsp;<%Response.Write obj.Name%></font></small></td>
    <td width="65%"><small><font face="Verdana">&nbsp;<%Response.Write obj.Class%></font></small></td>
  </tr>
<% next %>
</table>
</body>
</html>
