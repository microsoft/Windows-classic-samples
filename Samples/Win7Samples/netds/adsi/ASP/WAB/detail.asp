<html>

<head>
<meta name="GENERATOR" content="Microsoft FrontPage 3.0">
<title>Detail Object Information</title>
</head>

<body>
<%
  'Parse the URL path into adsPath
  urlPath = Request.QueryString("anr")
  idx = 1
  adsPath = ""
  While idx <> 0
      idx = InStr(1, urlPath, "%20")
      If (idx = 0) Then
         adsPath = adsPath & Mid(urlPath, 1)
      Else
         adsPath = adsPath & Mid(urlPath, 1, idx - 1)
        adsPath = adsPath & " "
      End If
      urlPath = Mid(urlPath, idx + 3)
   Wend

   Set o = GetObject(adsPath);
   mgrPath = o.Get("manager")
   Set mgr = GetObject("GC://" & mgrPath )
   

   On Error Resume Next
%>

<p>&nbsp;</p>

<table border="0" width="100%" cellpadding="2" cellspacing="1">
  <tr>
    <td width="16%" bgcolor="#000080" align="right"><font face="Verdana" color="#FFFFFF"><strong><small>Name:
    </small></strong></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><strong><small><% Response.Write o.Get("cn") %></small></strong></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"><font face="Verdana" color="#FFFFFF"><small><strong>Phone</strong></small></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("telephoneNumber")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"><font face="Verdana" color="#FFFFFF"><small><strong>Office</strong></small></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("physicalDeliveryOfficeName")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"><font face="Verdana" color="#FFFFFF"><small><strong>Manager</strong></small></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><a
    href="detail.asp?anr=<%Response.Write mgr.AdsPath%>"><%Response.Write mgr.Get("cn")%></a></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"><font face="Verdana" color="#FFFFFF"><strong><small>Deparment</small></strong></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("department")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"><font face="Verdana" color="#FFFFFF"><strong><small>Company</small></strong></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("company")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"></td>
    <td width="84%"></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"><font face="Verdana" color="#FFFFFF"><small><strong>NT
    4.0 Logon</strong></small></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("samAccountName")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"><font face="Verdana" color="#FFFFFF"><small><strong>UPN</strong></small></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("userPrincipalName")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"></td>
    <td width="84%"></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"></td>
    <td width="84%"></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"></td>
    <td width="84%"></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right"></td>
    <td width="84%"></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right">Direct Report</td>
    <td width="84%"></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#000080" align="right">MemberOf</td>
    <td width="84%"></td>
  </tr>
</table>
</body>
</html>
