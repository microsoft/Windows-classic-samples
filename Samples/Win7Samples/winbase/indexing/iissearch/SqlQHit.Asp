<HTML>
<HEAD>
<TITLE>Query Results from OLE DB Provider for Indexing Service</TITLE>
<LINK REL=STYLESHEET HREF="is2style.css" TYPE="text/css">
</HEAD>
<BODY BGCOLOR=#FFFFFF>

<TABLE>
    <TR><TD><A HREF="http://www.microsoft.com/ntserver/search" target="_top"><IMG SRC ="is2logo.gif" VALIGN=MIDDLE ALIGN=LEFT border=0></a></TD></TR>
    <TR><TD ALIGN="RIGHT"><H3>Simple SQL Search Results</H3></TD></TR>
</TABLE>

<P>

<%' !--#INCLUDE VIRTUAL="/iissamples/issamples/ADOVBS.INC"--%>
<%
QueryForm = "/iissamples/issamples/sqlqhit.htm"
Set Conn = Server.CreateObject("ADODB.Connection")
Conn.ConnectionString =  "provider=msidxs;"
Conn.Open
Set AdoCommand = Server.CreateObject("ADODB.Command")
set AdoCommand.ActiveConnection = Conn
%>
Executing the following query:

<% if Request.QueryString("CiRestriction")="" then %>
	<% if Request.QueryString("CiOrderBy")="" then %>
		<% AdoCommand.CommandText = "Select "&Request.QueryString("CiColumns")&" from " &Request.QueryString("CiScope")  %>
	<% else %>
		<% AdoCommand.CommandText = "Select "&Request.QueryString("CiColumns")&" FROM " &Request.QueryString("CiScope")&" ORDER BY " &Request.QueryString("CiOrderBy") %>
	<% end if %>
<% else %>
	<% if Request.QueryString("CiOrderBy")="" then %>
		<% AdoCommand.CommandText = "Select "&Request.QueryString("CiColumns")&" from " &Request.QueryString("CiScope")&" where "&Request.QueryString("CiRestriction") %>
	<% else %>
		<% AdoCommand.CommandText = "Select "&Request.QueryString("CiColumns")&" FROM " &Request.QueryString("CiScope")&" WHERE "&Request.QueryString("CiRestriction")&" ORDER BY " &Request.QueryString("CiOrderBy") %>
	<% end if %>
<% end if%>

<P><I><%=AdoCommand.CommandText%></I>
<%
Set RS = Server.CreateObject("ADODB.RecordSet")
AdoCommand.Properties("Bookmarkable") = True
RS.CursorType = adOpenKeyset
RS.MaxRecords = 300
RS.open AdoCommand

CiSearchString = CStr(RS.Properties("Query Restriction"))
%>
<P>
<TABLE BORDER=1>
<TR>
<% For i = 0 to RS.Fields.Count - 1 %>
	<TH class=RecordTitle><B><% = RS(i).Name %></B></TH>
<% Next %>
</TR>

<% Do While Not RS.EOF %>
	<TR>
	<% For i = 0 to RS.Fields.Count - 1 %>
		<%if RS(i).Name = "PATH" or RS(i).Name="VPATH" then %>
			<%if RS(i).Name="VPATH" and CiSearchString <> "" then %>
				<%
				' Construct the URL for hit highlighting

				WebHitsQuery = "CiWebHitsFile=" & Server.URLEncode( RS("vpath") )
				WebHitsQuery = WebHitsQuery & "&CiRestriction=" & Server.URLEncode( CiSearchString )
				WebHitsQuery = WebHitsQuery & "&CiBeginHilite=" & Server.URLEncode( "<b class=Hit>" )
				WebHitsQuery = WebHitsQuery & "&CiEndHilite=" & Server.URLEncode( "</b>" )
				WebHitsQuery = WebHitsQuery & "&CiUserParam3=" & QueryForm
				'       WebHitsQuery = WebHitsQuery & "&CiLocale=" & Q.LocaleID
				 %>
				<TD>
				<TABLE>
				<TR><TD><b><a href="<%=RS(i)%>"><% = RS(i) %></a></b></TD></TR>
				<TR NOWRAP><TD><I>
					Hit Highlighting:<br>
					<a href="qsumrhit.htw?<%= WebHitsQuery %>"><IMG src="hilight.gif" align=left alt="Highlight matching terms in document using Summary mode."> Summary</a><br>
					<a href="qfullhit.htw?<%= WebHitsQuery %>&CiHiliteType=Full"><IMG src="hilight.gif" align=left alt="Highlight matching terms in document."> Full</a>
				</I></TD></TR>
				</TABLE>
			<%else%>
				<TD><b><a href="<%=RS(i)%>"><% = RS(i) %></a></b></TD>
			<%end if%>
		<%else%>
			<TD><% = RS(i) %></TD>
		<%end if%>
	<% Next %>
	</TR>
	<%
	RS.MoveNext
    Loop
RS.Close
Conn.Close
%>
</TABLE>

<!--#include file ="is2foot.inc"-->

</BODY>
</HTML>
