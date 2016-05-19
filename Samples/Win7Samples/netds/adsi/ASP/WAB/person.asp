<%
  'Parse the URL path into adsPath

  ADsDomain = Session("ADsDomain")
  'Session may time out
  If ADsDomain = "" then
    ADsDomain = Application("ADsDomain")
  End if

  urlPath = Request.QueryString("anr")
  userID = Application("UserID")
  password = Application("Password")
  domainPath = "LDAP://" & ADsDomain

  counter = 0
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
   Set dso = GetObject("LDAP:")



   if ( o.Class = "group" ) then
      Set dso = Nothing
      Set o = Nothing
      Response.Redirect "group.asp?anr=" & Request.QueryString("anr")
   end if 


%>
<html>

<head>
<meta name="GENERATOR" content="Microsoft FrontPage 3.0">
<title>Person Information</title>
</head>

<body>

<p><img src="banner.gif" width="232" height="52" alt="banner.gif (3494 bytes)">
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <a href="default.asp"><img src="search.gif"
width="39" height="39" alt="search.gif (1222 bytes)" border="0"></a></p>

<%
   On Error Resume Next
   mgrPath = o.Get("manager")
   Set mgr = dso.OpenDSObject(domainPath & "/" & mgrPath , userID, password, 1 )

   
   directs = o.Get("directReports")
   members = o.Get("memberOf")
   counter = 0
%>

<table border="0" width="100%" cellpadding="2" cellspacing="2">
  <tr>
    <td width="16%" bgcolor="#0080C0" align="right"><font face="Verdana" color="#FFFFFF"><strong><small>Name:
    </small></strong></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><strong><small><% Response.Write o.Get("cn") %></small></strong></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#0080C0" align="right"><font face="Verdana" color="#FFFFFF"><strong><small>E-Mail:
    </small></strong></font></td>
    <td width="84%"><small><font face="Verdana" color="#0080C0"><a
     href="mailto:<%Response.Write o.Get("mail")%>"><% Response.Write o.Get("mail")%></a></font></small></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#0080C0" align="right"><font face="Verdana" color="#FFFFFF"><small><strong>Phone</strong></small></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("telephoneNumber")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#0080C0" align="right"><font face="Verdana" color="#FFFFFF"><small><strong>Title</strong></small></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("title")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#0080C0" align="right"><font face="Verdana" color="#FFFFFF"><small><strong>Office</strong></small></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("physicalDeliveryOfficeName")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#0080C0" align="right"><font face="Verdana" color="#FFFFFF"><small><strong>Manager</strong></small></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><a
    href="person.asp?anr=<%Response.Write mgr.AdsPath%>"><%Response.Write mgr.Get("cn")%></a></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#0080C0" align="right"><font face="Verdana" color="#FFFFFF"><strong><small>Deparment</small></strong></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("department")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#0080C0" align="right"><font face="Verdana" color="#FFFFFF"><strong><small>Division</small></strong></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("extensionAttribute3")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#0080C0" align="right"><font face="Verdana" color="#FFFFFF"><strong><small>Company</small></strong></font></td>
    <td width="84%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("company")%></small></font></td>
  </tr>
  <tr>
    <td width="16%" bgcolor="#0080C0" align="right"><font face="Verdana" color="#FFFFFF"><strong><small>Employee
    ID</small></strong></font></td>
    <td width="84%"><small><font face="Verdana" color="#0080C0"><% Response.Write o.Get("extensionAttribute4")%></font></small></td>
  </tr>
</table>

<p>&nbsp;</p>

<table border="0" cellpadding="1" cellspacing="2" width="100%">
  <tr>
    <td width="100%" bgcolor="#0080C0"><font face="Verdana" color="#FFFFFF"><strong><small>&nbsp;&nbsp;
    Direct Reports</small></strong></font></td>
  </tr>
<% for each direct in directs 
     'Alternate background color for enhancing the appearance 
     counter = counter + 1 
      md = counter mod 2 
      if ( md = 0 ) then 
         bkColor = "#C9C9C9"
      else
         bkColor = "#E8E8E8"
      end if 

      if ( direct = "" ) then
         counter = 0
      end if
  %>
  <tr>
<%  
    idxStart = InStr(1, direct, "=") + 1
    idxEnd = InStr(1, direct, ",")
    rdnDirect = Mid(direct, idxStart, idxEnd - idxStart)
    refDirect = domainPath & "/" & direct 
%>
    <td width="100%" bgcolor="<%Response.Write bkColor%>"><a
    href="person.asp?anr=<%Response.Write refDirect%>"><font face="Verdana" %><small><%Response.Write rdnDirect %></small></font></td>
  </tr>
<% Next %>
</table>
<small><font face="Verdana"><%
  if ( counter > 0 ) then
    Response.Write counter & " direct report(s) found"
  else
    Response.Write "No direct report"
  end if 
  
%>
</font></small><%
   counter = 0
%>


<p>&nbsp;</p>

<table border="0" cellpadding="1" cellspacing="2" width="100%">
  <tr>
    <td width="100%" bgcolor="#0080C0"><font face="Verdana" color="#FFFFFF"><strong><small>&nbsp;&nbsp;
    Member Of</small></strong></font></td>
  </tr>
<% for each member in members 
     
     'Alternate background color for enhancing the appearance 
     counter = counter + 1 
      md = counter mod 2 
      if ( md = 0 ) then 
         bkColor = "#C9C9C9"
      else
         bkColor = "#E8E8E8"
      end if 

      if ( member = "" ) then
          counter = 0
      end if 
  %>
  <tr>
<%  
    idxStart = InStr(1, member, "=") + 1
    idxEnd = InStr(1, member, ",")
    rdnMember = Mid(member, idxStart, idxEnd - idxStart)
    refMember = domainPath & "/" & member 
%>
    <td width="100%" bgcolor="<%Response.Write bkColor%>"></a><a
    href="group.asp?anr=<%Response.Write refMember%>"><font face="Verdana" %><small><%Response.Write rdnMember %></small></font></td>
  </tr>
<% Next %>
</table>
<small><font face="Verdana"><%
  if ( counter > 0 ) then
    Response.Write " Member of " & counter &" group(s)"
  else
    Response.Write "Do not member of any group OR you may not have permission to view this property"
  end if 
  
%>
</font></small><%
  Set dso = Nothing
  Set o = Nothing
  Set mgr = Nothing
%>
</a>


<p></p>
<p></p>


<small><small><font face="Verdana">
<%
  Response.Write "Query was executed in: " & elapse & " second(s)"
  Response.Write "<br>"
  elapse = Timer - t
  Response.Write "Total Time (Execute, Enumeration and Rendering): " & elapse & " second(s)"

%>
</font></small></small>

</body>
</html>
