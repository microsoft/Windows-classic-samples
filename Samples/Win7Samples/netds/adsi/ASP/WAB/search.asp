<html>

<head>
<meta name="GENERATOR" content="Microsoft FrontPage 3.0">
<title>Searching in the Active Directory</title>
</head>

<body>
<%
  ADsDomain = Session("ADsDomain")
  'Session may time out
  If ADsDomain = "" then
    ADsDomain = Application("ADsDomain")
  End if

  userName = Request.Form("userName")
  Set con = CreateObject("ADODB.Connection")
  Set com =   CreateObject("ADODB.Command")
  domainPath = "LDAP://" & ADsDomain
    
  'Open the ADSI Connection.
  con.Provider = "ADsDSOObject"

  'Credentials  
   con.Properties("User ID") = Application("UserID")
   con.Properties("Password") = Application("Password")




   con.Open "Active Directory Provider"
   Set Com.ActiveConnection = con

   
   Com.CommandText = "<" & domainPath & ">;(&(anr=" & userName & ")(|(objectCategory=organizationalPerson)(objectCategory=group)));ADsPath,name,telephoneNumber,title,physicalDeliveryOfficeName,department,objectCategory;subtree"
  

   Com.Properties("Page Size") = 64
   Com.Properties("Timeout") = 30 'seconds
   Com.Properties("Cache Results") = False
   
      

%>

<p><img src="banner.gif" width="232" height="52" alt="banner.gif (3494 bytes)">
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <a href="default.asp"><img src="search.gif"
width="39" height="39" alt="search.gif (1222 bytes)" border="0"></a></p>

<p><strong><small><font face="Verdana" color="#0080C0">Search for: &nbsp;</font><font
face="Verdana" color="#800080"> </font><font face="Verdana" color="#8080C0"><% Response.Write userName%></font><font
face="Verdana" color="#0080C0">&nbsp;&nbsp;&nbsp;&nbsp; </font></small></strong></p>
<%
  
   '--- TIME BEGIN
   t = Timer
   Set rs = Com.Execute
   elapse = Timer - t
   '--- TIME END
   On Error Resume Next 
   counter = 0
%>

<table border="0" width="814" cellpadding="1" cellspacing="1">
  <tr>
    <td width="395" bgcolor="#000080"><p align="center"><font face="Verdana" color="#FFFFFF"><small><strong>Name</strong></small></font></td>
    <td width="178" bgcolor="#000080"><small><p align="center"><font face="Verdana"
    color="#FFFFFF"><strong>Phone</strong></font></small></td>
    <td width="286" bgcolor="#000080"><font face="Verdana" color="#FFFFFF"><small><strong><p
    align="center">Title</strong></small></font></td>
    <td width="286" bgcolor="#000080"><small><p align="center"><font face="Verdana"
    color="#FFFFFF"><strong>Office</strong></font></small></td>
    <td width="371" bgcolor="#000080"><font face="Verdana" color="#FFFFFF"><small><strong><p
    align="center">Department</strong></small></font></td>
  </tr>
<% While Not rs.EOF %>
  <tr>
    <td width="395"><% 
     'Increment the counter 
     counter = counter + 1 
     'Alternate background color for enhancing the appearance 
      md = counter mod 2 
      if ( md = 0 ) then 
         bkColor = "#C9C9C9"
      else
         bkColor = "#E8E8E8"
      end if 

      '-- Find the object category
      sCat = rs.Fields("objectCategory").Value
      idx = InStr( 1, sCat, "=Person" )
      if ( idx > 0 ) then
          bPerson = True
      else
          bPerson = False
      end if 
   %>
</td>
  </tr>
  <tr>
    <td width="395" bgcolor="<%Response.Write bkColor%>"><a
    href="<% if bPerson then                Response.Write "person.asp?anr="              else               Response.Write "group.asp?anr="             end if            %> <%Response.Write rs.Fields("AdsPath")%>"><small><font
    face="Verdana"><% Response.Write rs.Fields("Name").Value %></font></small></td>
    <td width="178" bgcolor="<%Response.Write bkColor%>"><small><font face="Verdana"><small><% if ( bPerson )then Response.Write rs.Fields("telephoneNumber").Value else Response.Write "(Group)" end if %></small></font></small></td>
    <td width="286" bgcolor="<%Response.Write bkColor%>"><small><small><font face="Verdana"><% Response.Write rs.Fields("title").Value %></font></small></small></td>
    <td width="286" bgcolor="<%Response.Write bkColor%>"><small><small><font face="Verdana"><% Response.Write rs.Fields("physicalDeliveryOfficeName").Value %></font></small></small></td>
    <td width="371" bgcolor="<%Response.Write bkColor%>"><small><small><font face="Verdana"><% Response.Write rs.Fields("department").Value %></font></small></small></td>
  </tr>
<% 
     rs.MoveNext
     Wend
  %>
</table>
<small><font face="Verdana"><%
  if ( counter > 0 ) then
    Response.Write counter & " object(s) found"
  else
    Response.Write "No object is found"
  end if 
  Set con = nothing
  Set com = nothing
%>

</font></small></a>


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
