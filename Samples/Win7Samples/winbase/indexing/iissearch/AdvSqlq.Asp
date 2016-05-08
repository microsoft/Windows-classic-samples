<%@ LANGUAGE = VBScript %>
<% ' Option Explicit %>
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 3.0//EN" "html.dtd">
<HTML>
<HEAD>
<!--#INCLUDE FILE="adovbs.inc" -->

<BODY background="is2bkgnd.gif" OnLoad="SetAll()" LANGUAGE=JavaScript>
<TITLE>Advanced Indexing Service Search Form</TITLE>
<LINK REL=STYLESHEET HREF="is2style.css" TYPE="text/css">
<META NAME="DESCRIPTION" CONTENT="Sample SQL query form for Indexing Service">
<META NAME="AUTHOR"      CONTENT="OLE DB Provider for Indexing Service Team">
<META NAME="KEYWORDS"    CONTENT="sql, query, content, hit">
<META NAME="SUBJECT"     CONTENT="sample form">
<META NAME="MS.CATEGORY" CONTENT="Internet">
<META NAME="MS.LOCALE"   CONTENT="EN-US">
<META HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=Windows-1252">
<%
Const STAT_BUSY = 0
Const STAT_ERROR = &H1
Const STAT_DONE = &H2
Const STAT_REFRESH      = &H3
Const STAT_PARTIAL_SCOPE        = &H8
Const STAT_NOISE_WORDS  = &H10
Const STAT_CONTENT_OUT_OF_DATE  = &H20
Const STAT_REFRESH_INCOMPLETE   = &H40
Const STAT_CONTENT_QUERY_INCOMPLETE     = &H80
Const STAT_TIME_LIMIT_EXCEEDED  = &H100

Function GetCiOutOfDate(value)
    GetCiOutOfDate = value And STAT_CONTENT_OUT_OF_DATE
end Function

Function GetCiQueryIncomplete(value)
    GetCiQueryIncomplete = value And STAT_CONTENT_QUERY_INCOMPLETE
end Function

Function GetCiQueryTimedOut(value)
    GetCiQueryTimedOut = value And STAT_TIME_LIMIT_EXCEEDED
end Function

' Customization variables
DebugFlag = FALSE          ' set TRUE for debugging
UseSessions = FALSE        ' set FALSE to disable use of session variables
RecordsPerPage = 10        ' number of results on a page
MaxResults = 300           ' total number of results returned
FormLogo = "is2logo.gif"   ' picture at top of page

' Hard-code some parameters that could be taken from the form
' SortBy = " order by rank desc"       ' sort order

' Set initial  conditions
NewQuery = FALSE
UseSavedQuery = FALSE
SearchString = ""
QueryForm = Request.ServerVariables("PATH_INFO")
if Request.ServerVariables("REQUEST_METHOD") = "POST" then
    SearchString = Request.Form("SearchString")
    DocAuthorRestriction = Request.Form("DocAuthorRestriction")
    FSRest = Request.Form("FSRest")
    FSRestVal = Request.Form("FSRestVal")
    FSRestOther = Request.Form("FSRestOther")
    FMMod = Request.Form("FMMod")
    FMModDate = Request.Form("FMModDate")
    SortBy = Request.Form("SortBy")
    ' NOTE: this will be true only if the button is actually pushed.
    if Request.Form("Action") = "Execute" then
        NewQuery = TRUE
        NextPageNumber = -1
    elseif Request.Form("pg") <> "" then
        NextPageNumber = Request.Form("pg")
        UseSavedQuery = UseSessions
        NewQuery = not UseSessions
    end if
End if
%>
</HEAD>

<TABLE>
    <TR><TD><A HREF="http://www.microsoft.com/ntserver/search" target="_top">
            <IMG SRC ="is2logo.gif" border=0 VALIGN=MIDDLE ALIGN="LEFT"></a></TD></TR>
    <TR><TD ALIGN="RIGHT"><H3>Advanced SQL Sample</H3></TD></TR>
</TABLE>


<SCRIPT LANGUAGE=JavaScript>
<!--
function SetAll()
{
    document.QForm.SearchString.value = "<%=SearchString%>";

    <%select case FSRest%>
    <%case " > "%>
        document.QForm.FSRest.selectedIndex = 1;
    <%case else%>
        document.QForm.FSRest.selectedIndex = 0;
    <%end select%>

    <%select case FSRestVal%>
    <%case "any"%>
        document.QForm.FSRestVal.selectedIndex = 0;
    <%case "100"%>
        document.QForm.FSRestVal.selectedIndex = 1;
    <%case "1024"%>
        document.QForm.FSRestVal.selectedIndex = 2;
    <%case "10240"%>
        document.QForm.FSRestVal.selectedIndex = 3;
    <%case "102400"%>
        document.QForm.FSRestVal.selectedIndex = 4;
    <%case "1048576"%>
        document.QForm.FSRestVal.selectedIndex = 5;
    <%case "10485760"%>
        document.QForm.FSRestVal.selectedIndex = 6;
    <%case "104857600"%>
        document.QForm.FSRestVal.selectedIndex = 7;
    <%case "other"%>
        document.QForm.FSRestVal.selectedIndex = 8;
    <%case else%>
        document.QForm.FSRestVal.selectedIndex = 0;
    <%end select%>

    document.QForm.FSRestOther.value = "<%=FSRestOther%>";

    <%select case FMMod%>
    <%case "any"%>
        document.QForm.FMMod.selectedIndex = 0;
    <%case "DATEADD(minute, -10, GETGMTDATE())"%>
        document.QForm.FMMod.selectedIndex = 1;
    <%case "DATEADD(hour, -1, GETGMTDATE())"%>
        document.QForm.FMMod.selectedIndex = 2;
    <%case "DATEADD(day, -1, GETGMTDATE())"%>
        document.QForm.FMMod.selectedIndex = 3;
    <%case "DATEADD(week, -1, GETGMTDATE())"%>
        document.QForm.FMMod.selectedIndex = 4;
    <%case "DATEADD(month, -1, GETGMTDATE())"%>
        document.QForm.FMMod.selectedIndex = 5;
    <%case "DATEADD(quarter, -1, GETGMTDATE())"%>
        document.QForm.FMMod.selectedIndex = 6;
    <%case "DATEADD(year, -1, GETGMTDATE())"%>
        document.QForm.FMMod.selectedIndex = 7;
    <%case "since"%>
        document.QForm.FMMod.selectedIndex = 8;
    <%case else%>
        document.QForm.FMMod.selectedIndex = 0;
    <%end select%>

    document.QForm.FMModDate.value = "<%=FMModDate%>";

    document.QForm.DocAuthorRestriction.value = "<%=DocAuthorRestriction%>";
    document.QForm.SearchString.focus();
}
//-->
</SCRIPT>


<FORM NAME="QForm" ACTION="<%=QueryForm%>" METHOD=POST>
<table width=500>

    <tr>
        <td>Enter your query: </td>
        <td><INPUT TYPE="TEXT" NAME="SearchString" SIZE="51" MAXLENGTH="100" VALUE=""></td>
    </tr>

    <tr>
        <td align="right" colspan=2>Document author: 
                <INPUT TYPE="TEXT" NAME="DocAuthorRestriction" SIZE="27" MAXLENGTH="100" VALUE="">
    </tr>

    <tr>
        <td align="right" colspan=2>Where File Size is: 
           <SELECT NAME="FSRest">
               <!-- <OPTION VALUE=" &lt; " <%if FSRest = " < " then%>SELECTED <%end if%>>Less Than
                    <OPTION VALUE=" &gt; " <%if FSRest = " > " then%>SELECTED <%end if%>>Greater Than
               -->
               <OPTION VALUE=" &lt; " SELECTED>Less Than
               <OPTION VALUE=" &gt; " >Greater Than
           </SELECT>

           <SELECT NAME="FSRestVal" DEFAULT VALUE="any">
               <OPTION VALUE="any" SELECTED>any size
               <OPTION VALUE="100" >100 bytes
               <OPTION VALUE="1024" >1K bytes
               <OPTION VALUE="10240" >10K bytes
               <OPTION VALUE="102400" >100K bytes
               <OPTION VALUE="1048576" >1M byte
               <OPTION VALUE="10485760" >10M bytes
               <OPTION VALUE="104857600" >100M bytes
               <OPTION VALUE="other" >Other
           </SELECT>
           <INPUT TYPE="TEXT" NAME="FSRestOther" SIZE="12" MAXLENGTH="9" VALUE="">
        </td>
    </tr>

    <tr>
        <td align="right" colspan=2>Modified:
            <SELECT NAME="FMMod">
                <OPTION VALUE="any" SELECTED >at any time.
                <OPTION VALUE="DATEADD(minute, -10, GETGMTDATE())">in the last 10 minutes.
                <OPTION VALUE="DATEADD(hour, -1, GETGMTDATE())">in the last hour.
                <OPTION VALUE="DATEADD(day, -1, GETGMTDATE())">in the last day.
                <OPTION VALUE="DATEADD(week, -1, GETGMTDATE())">in the last week.
                <OPTION VALUE="DATEADD(month, -1, GETGMTDATE())">in the last month.
                <OPTION VALUE="DATEADD(quarter, -1, GETGMTDATE())">in the last quarter.
                <OPTION VALUE="DATEADD(year, -1, GETGMTDATE())">in the last year.
                <OPTION VALUE="since">since GMT (yy/mm/dd)        .
            </SELECT>
            <INPUT TYPE="TEXT" NAME="FMModDate" SIZE="12" MAXLENGTH="9" VALUE="">
        </td>
    </tr>

     <tr>
        <td align="left"><a href="ixtipsql.htm">SQL Syntax Tips</a></td>
        <td align="right">
            <INPUT TYPE="SUBMIT" NAME="Action" VALUE="Execute">
            <INPUT TYPE="RESET"  NAME="Clear" VALUE="Clear">
        </td>
    </tr>
</table>
        <INPUT TYPE="HIDDEN" NAME="SortBy" VALUE=" order by rank desc">

</FORM>

<BR>
<%if DebugFlag then%>
    <PRE>
    SearchString         = <%=SearchString%>
    DocAuthorRestriction = <%=DocAuthorRestriction%>
    FSRest               = <%=FSRest%>
    FSRestVal            = <%=FSRestVal%>
    FSRestOther          = <%=FSRestOther%>
    FMMod                = <%=FMMod%>
    FMModDate            = <%=FMModDate%>
    SortBy               = <%=SortBy%>
    NewQuery             = <%=CStr(NewQuery)%>
    UseSavedQuery        = <%=CStr(UseSavedQuery)%>
    </PRE>
<%end if%>

<%

if NewQuery then
    ' First Reset the Connection, command and recordSet Object.
    if UseSessions then
        set session("AdoConnection") = nothing
        set session("AdoCommand") = nothing
        set session("Recordset") = nothing
    end if
    NextRecordNumber = 1
    set AdoConnection = Server.CreateObject("ADODB.Connection")
    set AdoCommand = Server.CreateObject("ADODB.Command")
    AdoConnection.ConnectionString = "provider=msidxs"
    AdoConnection.Open
    set AdoCommand.ActiveConnection = AdoConnection

    SelectColumns = "DocTitle, vpath, filename, size, write, characterization"
    SelectString = "Select " + SelectColumns + " from Web..Scope('Deep traversal of ""/""') "

    Composer = ""
    TheQuery = ""

    if SearchString <> "" then
        TheQuery = "Contains('""" + SearchString + """')"
        Composer = " and "
    end if

    if FSRestVal <> "any" then
        if FSRestVal <> "other" then
            if FSRestVal <> "" then
                TheQuery = "Size " + FSRest + FSRestVal + " " + Composer + TheQuery
            end if
        else
            TheQuery = "Size " + FSRest + FSRestOther + " " + Composer + TheQuery
        end if
        Composer = " and "
    end if

    if DocAuthorRestriction <> "" then
        TheQuery = "Contains(DocAuthor,  '""" + DocAuthorRestriction + """') " + Composer + TheQuery
        Composer = " and "
    end if

    if FMMod <> "" AND FMMod <> "any" then
        if FMMod <> "since" then
            TheQuery = "(Write > " + FMMod + ") " + Composer + TheQuery
        else
            TheQuery = "(Write > '" + FMModDate + "') " + Composer + TheQuery
        end if
    end if

    if TheQuery <> "" then
        SelectString = SelectString +  " where " + TheQuery +" "+ SortBy
    else
        SelectString = SelectString + SortBy +" "
    end if

if DebugFlag then
    Response.Write "SelectString = " + Server.HTMLEncode( SelectString ) + "<BR>"
end if

    AdoCommand.CommandText = SelectString
    set RS = Server.CreateObject("AdoDB.Recordset")
    RS.CursorType = adOpenKeyset
    AdoCommand.Properties("Bookmarkable") = True
    if MaxResults <> -1 then
        RS.MaxRecords=MaxResults
    end if
    RS.Open AdoCommand
    RS.PageSize = RecordsPerPage
    ActiveQuery = TRUE
    %>
    <!-- BEGIN column header -->
    <%
elseif UseSavedQuery then
    if IsObject( Session("AdoConnection") ) And IsObject( Session("AdoCommand")) AND IsObject( Session("RecordSet") ) then
        set AdoConnection = Session("AdoConnection")
        set AdoCommand = Session("AdoCommand")
        set RS = Session("RecordSet")
        ActiveQuery = TRUE
    else
        Response.Write "ERROR - No saved query"
    end if
end if
%>

<!-- BEGIN first row of query results table -->
<%
if ActiveQuery then
    if RS.RecordCount <> -1 and NextPageNumber <> -1 then
        RS.AbsolutePage = NextPageNumber
        NextRecordNumber = RS.AbsolutePosition
    end if
    if not RS.EOF then
 %>

<p>
<HR WIDTH=80% ALIGN=center SIZE=3>
<p>

<%
        LastRecordOnPage = NextRecordNumber + RS.PageSize - 1
        CurrentPage = RS.AbsolutePage
        if RS.RecordCount <> -1 AND RS.RecordCount < LastRecordOnPage then
            LastRecordOnPage = RS.RecordCount
        end if

        Response.Write "Documents " & NextRecordNumber & " to " & LastRecordOnPage
        if RS.RecordCount <> -1 then
            Response.Write " of " & RS.RecordCount
        end if

        Response.Write " matching the query: "
        Response.Write "<P>" & chr(34) & Server.HTMLEncode( AdoCommand.CommandText ) & "</I>" & chr(34) & ".<P>"

%>
<TABLE width=500>
<%
        Do While Not RS.EOF and NextRecordNumber <= LastRecordOnPage %>

            <%
            ' This is the detail portion for Title, Abstract, URL, Size, and
            ' Modification Date.
            ' If there is a title, display it, otherwise display the virtual path.
            %>
    <tr class="RecordTitle">
        <td align="right" valign="top" class="RecordTitle">
            <%= NextRecordNumber%>.
        </td>
        <td><b class="RecordTitle">
            <%if VarType(RS("DocTitle")) = 1 or RS("DocTitle") = "" then%>
            <b><a href="<%=RS("vpath")%>"><%= Server.HTMLEncode( RS("filename") )%></a></b>
            <%else%>
                <b><a href="<%=RS("vpath")%>"><%= Server.HTMLEncode(RS("DocTitle"))%></a></b>
            <%end if%>
        </td>
    </tr>
    <tr>
        <td></td>
        <td>
            <%if VarType(RS("characterization")) = 8 and RS("characterization") <> "" then%>
                <b><i>Abstract:  </I></b><%= Server.HTMLEncode(RS("characterization"))%>
                <br>
            <%end if%>
        </td>
    </tr>
    <tr>
        <td></td>
        <td class="RecordStats" height=40>
            <a href="<%=RS("vpath")%>" class="RecordStats" style="color:blue;">http://<%=Request("server_name")%><%=RS("vpath")%></a>
                <br><%if RS("size") = "" then%>(size and time unknown)<%else%>size <%=RS("size")%> bytes - <%=RS("write")%> GMT<%end if%></font>

            <%if SearchString <> "" then%>
                <%
                QueryString = CStr(RS.Properties("Query Restriction"))
                %>
                <BR>
                <%
                ' Construct the URL for hit highlighting

                WebHitsQuery = "CiWebHitsFile=" & Server.URLEncode( RS("vpath") )
                WebHitsQuery = WebHitsQuery & "&CiRestriction=" & Server.URLEncode( QueryString )
                WebHitsQuery = WebHitsQuery & "&CiUserParam3=" & QueryForm
                %>
                <a href="qsumrhit.htw?<%= WebHitsQuery %>">
                <IMG src="hilight.gif" align="left" alt="Highlight matching terms in document using Summary mode."> Summary</a>
                <br>
                <a href="qfullhit.htw?<%= WebHitsQuery %>&CiHiliteType=Full">
                <IMG src="hilight.gif" align="left" alt="Highlight matching terms in document."> Full</a>
            <%end if%>
            <br>
        </td>
    </tr>

            <%
            RS.MoveNext
            NextRecordNumber = NextRecordNumber+1
        Loop
        %>
</TABLE>
        <%
    else
       if NextRecordNumber = 1 then
           Response.Write "No documents matched the query<P>"
       else
           Response.Write "No more documents in the query<P>"
       end if
    end if ' NOT RS.EOF

if NOT GetCiOutofDate(RS.Properties("Rowset Query Status")) then
'   If the index is current, display the fact %>

    <P>
    <I><B>The index is up to date.</B></I><BR>
<%end if

if GetCiQueryIncomplete(RS.Properties("Rowset Query Status")) then
'   If the query was not executed because it needed to enumerate to
'   resolve the query instead of using the index, but AllowEnumeration
'   was FALSE, let the user know

 %>
    <P>
    <I><B>The query is too expensive to complete.</B></I><BR>
<%end if

if GetCiQueryTimedOut(RS.Properties("Rowset Query Status")) then
'   If the query took too long to execute (for example, if too much work
'   was required to resolve the query), let the user know %>

    <P>
    <I><B>The query took too long to complete.</B></I><BR>
<%end if%>
<TABLE>
<%

'   This is the "previous" button.
'   This retrieves the previous page of documents for the query.

  SaveQuery = FALSE
  if CurrentPage > 1 and RS.RecordCount <> -1 then %>
    <td align="left">
        <form action="<%=QueryForm%>" method="post">
            <INPUT TYPE="HIDDEN" NAME="SearchString" VALUE="<%=SearchString%>">
            <INPUT TYPE="HIDDEN" NAME="DocAuthorRestriction"  VALUE="<%=DocAuthorRestriction%>">
            <INPUT TYPE="HIDDEN" NAME="FSRest" VALUE="<%=FSRest%>">
            <INPUT TYPE="HIDDEN" NAME="FSRestVal" VALUE="<%=FSRestVal%>">
            <INPUT TYPE="HIDDEN" NAME="FSRestOther" VALUE="<%=FSRestOther%>">
            <INPUT TYPE="HIDDEN" NAME="FMMod" VALUE="<%=FMMod%>">
            <INPUT TYPE="HIDDEN" NAME="FMModDate" VALUE="<%=FMModDate%>">
            <INPUT TYPE="HIDDEN" NAME="SortBy" VALUE="<%=SortBy%>">
            <INPUT TYPE="HIDDEN" NAME="Scope" VALUE="<%=Scope%>">
            <INPUT TYPE="HIDDEN" NAME="pg" VALUE="<%=CurrentPage-1%>" >

            <input type="submit" value="Previous <%=RS.PageSize%> documents">
        </form>
    </td>
    <%SaveQuery = UseSessions%>
<%end if%>

<%if Not RS.EOF then
'   This is the "next" button.
'   This button retrieves the next page of documents for the query.

 %>
    <td align="right">
        <form action="<%=QueryForm%>" method="post">
            <INPUT TYPE="HIDDEN" NAME="SearchString" VALUE="<%=SearchString%>">
            <INPUT TYPE="HIDDEN" NAME="DocAuthorRestriction"  VALUE="<%=DocAuthorRestriction%>">
            <INPUT TYPE="HIDDEN" NAME="FSRest" VALUE="<%=FSRest%>">
            <INPUT TYPE="HIDDEN" NAME="FSRestVal" VALUE="<%=FSRestVal%>">
            <INPUT TYPE="HIDDEN" NAME="FSRestOther" VALUE="<%=FSRestOther%>">
            <INPUT TYPE="HIDDEN" NAME="FMMod" VALUE="<%=FMMod%>">
            <INPUT TYPE="HIDDEN" NAME="FMModDate" VALUE="<%=FMModDate%>">
            <INPUT TYPE="HIDDEN" NAME="SortBy" VALUE="<%=SortBy%>">
            <INPUT TYPE="HIDDEN" NAME="Scope" VALUE="<%=Scope%>">
            <INPUT TYPE="HIDDEN" NAME="pg" VALUE="<%=CurrentPage+1%>" >
            <%
                NextString = "Next "
                if RS.RecordCount <> -1 then
                    NextSet = (RS.RecordCount - NextRecordNumber) + 1
                    if NextSet > RS.PageSize then
                        NextSet = RS.PageSize
                    end if
                    NextString = NextString & NextSet & " documents"
                else
                    NextString = NextString & " page of documents"
                end if
             %>
            <input type="submit" value="<%=NextString%>">
        </form>
    </td>
    <%SaveQuery = UseSessions%>
<%end if%>
</TABLE>


<% ' Display the page number

if RS.PageCount <> 0 then
  Response.Write "Page " & CurrentPage
  if RS.PageCount <> -1 then
     Response.Write " of " & RS.PageCount
  end if
end if

    ' If either of the previous or back buttons were displayed, save
    ' the query and the recordset in session variables.
    if SaveQuery then
        set Session("AdoConnection") = AdoConnection
        set Session("AdoCommand") = AdoCommand
        set Session("RecordSet") = RS
    else
        RS.close
        AdoConnection.Close
        Set RS = Nothing
        Set AdoCommand = Nothing
        Set AdoConnection = Nothing
        if UseSessions then
            set Session("AdoConnection") = Nothing
            set Session("AdoCommand") = Nothing
            set Session("RecordSet") = Nothing
        end if
    end if
end if
%>
<BR>
<!--#include file ="is2foot.inc"-->
</BODY>
</HTML>
