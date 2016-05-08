<%
 Const ADS_GROUP_TYPE_GLOBAL_GROUP = &H2
 Const ADS_GROUP_TYPE_DOMAIN_LOCAL_GROUP = &H4
 Const ADS_GROUP_TYPE_UNIVERSAL_GROUP = &H8
 Const ADS_GROUP_TYPE_SECURITY_ENABLED = &H80000000

  ADsDomain = Session("ADsDomain")
  'Session may time out
  If ADsDomain = "" then
    ADsDomain = Application("ADsDomain")
  End if


  'Parse the URL path into adsPath
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

   if ( o.Class <> "group" ) then
        Set o = Nothing
        Set dso = Nothing
        Response.Redirect "person.asp?anr=" & Request.QueryString("anr")
   end if 

      
%>
<html>

<head>
<meta name="GENERATOR" content="Microsoft FrontPage 3.0">
<title>Group Information</title>
</head>

<body>

<p><img src="banner.gif" width="232" height="52" alt="banner.gif (3494 bytes)">
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <a href="default.asp"><img src="search.gif"
width="39" height="39" alt="search.gif (1222 bytes)" border="0"></a></p>


<%

  On Error Resume Next
   members = o.Get("member")
   groupType = o.Get("groupType")



   if ( groupType And  ADS_GROUP_TYPE_SECURITY_ENABLED ) then
     groupDesc = "Security Group "
  else
     groupDesc = "Distribution List "
  end if   

   if ( groupType And ADS_GROUP_TYPE_GLOBAL_GROUP ) Then
         groupDesc = groupDesc & "(Global)"
   ElseIf ( groupType And ADS_GROUP_TYPE_DOMAIN_LOCAL_GROUP ) Then
          groupDesc = groupDesc & "(Domain Local)"
   ElseIf ( groupType And ADS_GROUP_TYPE_UNIVERSAL_GROUP ) Then
          groupDesc = groupDesc & "(Universal)"
   end if 

    'Get the owner
    ownerPath = domainPath & "/" &o.Get("managedBy")
    if ( Err.Number = 0 ) then
      idxStart = InStr(1, ownerPath, "=")
      idxEnd   = InStr(idxStart, ownerPath, ",")
      owner = Mid(ownerPath, idxStart+1, idxEnd - idxStart -1 ) 
    end if 


%>


<table border="0" cellpadding="2" width="100%">
  <tr>
    <td width="17%" align="right" bgcolor="#0080C0"><font face="Verdana" color="#FFFFFF"><strong><small>Group</small></strong></font></td>
    <td width="83%"><font face="Verdana" color="#0080C0"><small><%Response.Write o.Get("cn")%></small></font></td>
  </tr>
  <tr>
    <td width="17%" align="right" bgcolor="#0080C0"><font face="Verdana" color="#FFFFFF"><strong><small>Group
    Type:</small></strong></font></td>
    <td width="83%"><font face="Verdana" color="#0080C0"><small><%Response.Write groupDesc%></small></font></td>
  </tr>
  <tr>
    <td width="17%" align="right" bgcolor="#0080C0"><font face="Verdana" color="#FFFFFF"><strong><small>Owner:</small></strong></font></td>
    <td width="83%"><font face="Verdana" color="#0080C0"><small>
    <a href="person.asp?anr=<%Response.Write ownerPath%>" ><%Response.Write owner%></a></small></font></td>
  </tr>

</table>

<p>&nbsp;</p>

<table border="0" cellpadding="1" cellspacing="2" width="100%">
  <tr>
    <td width="100%" bgcolor="#0080C0"><font face="Verdana" color="#FFFFFF"><strong><small>&nbsp;&nbsp;
    Group Members</small></strong></font></td>
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
  %>
  <tr>
<%  
    idxStart = InStr(1, member, "=") + 1
    idxEnd = InStr(1, member, ",")
    rdnMember = Mid(member, idxStart, idxEnd - idxStart)
    refMember = domainPath & "/" & member 

%>
    <td width="100%" bgcolor="<%Response.Write bkColor%>"><a
    href="person.asp?anr=<%Response.Write refMember%>"><font face="Verdana" %><small><%Response.Write rdnMember %></small></font></td>
  </tr>
<% Next %>
</table>
<small><font face="Verdana"><%
  if ( counter > 0 ) then
    Response.Write " Member count: " & counter &" user(s) and group(s)"
  else
    Response.Write "Do not have membership or permission to view"
  end if 
  counter = 0  'Reset counter
%></font></small>



<%
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
