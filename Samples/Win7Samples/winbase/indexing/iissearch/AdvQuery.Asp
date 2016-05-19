<%@ LANGUAGE = VBScript %>
<% ' Option Explicit %>
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 3.0//EN" "html.dtd">
<HTML>
<HEAD>

    <TITLE>Advanced Indexing Service Search Form</TITLE>
        <LINK REL=STYLESHEET HREF="is2style.css" TYPE="text/css">
    <META NAME="DESCRIPTION" CONTENT="Sample query form for Indexing Service">
    <META NAME="AUTHOR"      CONTENT="Indexing Service Team">
    <META NAME="KEYWORDS"    CONTENT="query, content, hit">
    <META NAME="SUBJECT"     CONTENT="sample form">
    <META NAME="MS.CATEGORY" CONTENT="Internet">
    <META NAME="MS.LOCALE"   CONTENT="EN-US">
    <META HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=Windows-1252">
<%
    ' Customization variables
    DebugFlag = FALSE          ' set TRUE for debugging
    UseSessions = FALSE        ' set FALSE to disable use of session variables
    RecordsPerPage = 10        ' number of results on a page
    MaxResults = -1            ' total number of results returned
    FormLogo = "is2logo.gif"   ' picture at top of page

    ' Hard-code some parameters that could be taken from the form
    ' SortBy = "rank[d]"       ' sort order
    Scope = "/"                ' query scope

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
     '  Scope = Request.Form("Scope")
        RankBase = Request.Form("RankBase")
        ' NOTE: this will be true only if the button is actually pushed.
        if Request.Form("Action") = "Execute" then
            NewQuery = TRUE
            NextPageNumber = -1
        elseif Request.Form("pg") <> "" then
            NextPageNumber = Request.Form("pg")
            UseSavedQuery = UseSessions
            NewQuery = not UseSessions
        end if
    end if
 %>
</HEAD>

<BODY background="is2bkgnd.gif" OnLoad="SetAll()" LANGUAGE=JavaScript>

<TABLE>
    <TR><TD><A HREF="http://www.microsoft.com/ntserver/search" target="_top">
            <IMG SRC ="<%=FormLogo%>" border=0 VALIGN=MIDDLE ALIGN="LEFT"></a></TD></TR>
    <TR><TD ALIGN="RIGHT"><H3>Advanced ASP Sample</H3></TD></TR>
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
    <%case "-10n"%>
        document.QForm.FMMod.selectedIndex = 1;
    <%case "-1h"%>
        document.QForm.FMMod.selectedIndex = 2;
    <%case "-1d"%>
        document.QForm.FMMod.selectedIndex = 3;
    <%case "-1w"%>
        document.QForm.FMMod.selectedIndex = 4;
    <%case "-1m"%>
        document.QForm.FMMod.selectedIndex = 5;
    <%case "-1q"%>
        document.QForm.FMMod.selectedIndex = 6;
    <%case "-1y"%>
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

function msieversion()
{
    var ua   = window.navigator.userAgent;
    var msie = ua.indexOf("MSIE");
    if (msie>0)
        return parseInt(ua.substring(msie+5, ua.indexOf(".", msie)));
    else
        return 0;
}

function NAF(szUrl, szQuery)
{
    if ( (msieversion() >= 5) && (szQuery.length > 0) )
        window.external.NavigateAndFind(szUrl, szQuery, "_main");
    else
        window.navigate(szUrl, target="_main");
}
//-->
</SCRIPT>


<FORM NAME="QForm" ACTION="<%=QueryForm%>" METHOD=POST>

<!--     -->

<table width=500>
    <tr>
        <td>Enter your query:</td>
        <td><INPUT TYPE="TEXT" NAME="SearchString" SIZE="51" MAXLENGTH="100" VALUE=""></td>
    </tr>

    <tr>
        <td align="right" colspan=2>Document author:
            <INPUT TYPE="TEXT" NAME="DocAuthorRestriction" SIZE="27" MAXLENGTH="100" VALUE="">
        <td>
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

            <SELECT NAME="FSRestVal">
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
                <OPTION VALUE="-10n">in the last 10 minutes.
                <OPTION VALUE="-1h">in the last hour.
                <OPTION VALUE="-1d">in the last day.
                <OPTION VALUE="-1w">in the last week.
                <OPTION VALUE="-1m">in the last month.
                <OPTION VALUE="-1q">in the last quarter.
                <OPTION VALUE="-1y">in the last year.
                <OPTION VALUE="since">since GMT (yy/mm/dd)       .
            </SELECT>
            <INPUT TYPE="TEXT" NAME="FMModDate" SIZE="12" MAXLENGTH="9" VALUE="">
        </td>
    </tr>

    <tr>
        <td align="right"><A HREF="ixtiphlp.htm">Tips for searching</A></td>
        <td align="right">
            <BUTTON TYPE=SUBMIT NAME="Action">Execute</BUTTON>
            <INPUT TYPE="RESET"  NAME="Clear" VALUE="Clear">
        </td>
    </tr>
</table>

    <INPUT TYPE="HIDDEN" NAME="SortBy" VALUE="rank[d]">
    <INPUT TYPE="HIDDEN" NAME="Scope" VALUE="/">
    <INPUT TYPE="HIDDEN" NAME="ColChoice" VALUE="1">
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
    Scope                = <%=Scope%>
    NewQuery             = <%=CStr(NewQuery)%>
    UseSavedQuery        = <%=CStr(UseSavedQuery)%>
    </PRE>
<%end if%>

<%
    if NewQuery then
        if UseSessions then
            set Session("Query") = nothing
            set Session("Recordset") = nothing
        end if
        NextRecordNumber = 1
        set Q = Server.CreateObject("ixsso.Query")

        Composer = ""
        TheQuery = ""

        if SearchString <> "" then
            if Left( SearchSting, 1 ) <> "@" AND Left( SearchString, 1 ) <> "#" AND Left( SearchString, 1 ) <> "$" then
                TheQuery = "(@Contents " + SearchString + ")"
            else
                TheQuery = "(" + SearchString + ")"
            end if

            Composer = " & "
        end if

        if FSRestVal <> "any" then
            if FSRestVal <> "other" then
                TheQuery = "(@Size " + FSRest + FSRestVal + ") " + Composer + TheQuery
            else
                TheQuery = "(@Size " + FSRest + FSRestOther + ") " + Composer + TheQuery
            end if

            Composer = " & "
        end if

        if DocAuthorRestriction <> "" then
            TheQuery = "(@DocAuthor " + DocAuthorRestriction + ") " + Composer + TheQuery

            Composer = " & "
        end if

        if FMMod <> "" AND FMMod <> "any" then
            if FMMod <> "since" then
                TheQuery = "(@Write > " + FMMod + ") " + Composer + TheQuery
            else
                TheQuery = "(@Write > " + FMModDate + ") " + Composer + TheQuery
            end if
        end if

        if DebugFlag then
            Response.Write "TheQuery = " & Server.HTMLEncode( TheQuery ) & "<BR>"
        end if

        Q.Catalog = "web"
        Q.Query = TheQuery
        Q.SortBy = SortBy

        Q.Columns = "DocTitle, vpath, path, filename, size, write, characterization, rank"

        if MaxResults <> -1 then
            Q.MaxRecords = MaxResults
        end if

        set Util = Server.CreateObject("ixsso.Util")

        if Scope <> "\" then
            Util.AddScopeToQuery Q, Scope, "DEEP"
        end if

        set RS = Q.CreateRecordSet("nonsequential")
        RS.PageSize = RecordsPerPage
        ActiveQuery = TRUE
    elseif UseSavedQuery then
        if IsObject( Session("Query") ) And IsObject( Session("RecordSet") ) then
            set Q = Session("Query")
            set RS = Session("RecordSet")
            ActiveQuery = TRUE
        else
            Response.Write "ERROR - No saved query"
        end if
    end if

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

        if SearchString <> "" then
            Response.Write " matching the query " & chr(34) & "<I>"
            Response.Write Server.HTMLEncode( SearchString ) & "</I>" & chr(34) & ".<P>"
        end if
 %>

<% if Not RS.EOF and NextRecordNumber <= LastRecordOnPage then%>
<table border=0>
<colgroup width=105>
<%end if%>

<% Do While Not RS.EOF and NextRecordNumber <= LastRecordOnPage

    ' This is the detail portion for Title, Abstract, URL, Size, and
    ' Modification Date.
    ' If there is a title, display it, otherwise display the filename.
%>
    <p>
<%
    ' Graphically indicate rank of document with list of stars (*'s).

        if NextRecordNumber = 1 then
            RankBase=RS("rank")
        end if

        if RankBase>1000 then
            RankBase=1000
        elseif RankBase<1 then
            RankBase=1
        end if

        NormRank = RS("rank")/RankBase

        if NormRank > 0.80 then
            stars = "rankbtn5.gif"
        elseif NormRank > 0.60 then
            stars = "rankbtn4.gif"
        elseif NormRank > 0.40 then
            stars = "rankbtn3.gif"
        elseif NormRank >.20 then
            stars = "rankbtn2.gif"
        else
            stars = "rankbtn1.gif"
        end if
%>
        <tr class="RecordTitle">
            <td align="right" valign=top class="RecordTitle">
                <%= NextRecordNumber%>.
            </td>
            <td><b class="RecordTitle">
                <%if VarType(RS("DocTitle")) = 1 or RS("DocTitle") = "" then%>
                    <a href="javascript:NAF('http://<%=Request.ServerVariables("SERVER_NAME")%><%=RS("vpath")%>','<%=SearchString%>')" class="RecordTitle"><%= Server.HTMLEncode( RS("filename") )%></a>
                <%else%>
                    <a href="javascript:NAF('http://<%=Request.ServerVariables("SERVER_NAME")%><%=RS("vpath")%>','<%=SearchString%>')" class="RecordTitle"><%= Server.HTMLEncode(RS("DocTitle"))%></a>
                <%end if%>
            </b></td>
        </tr>
        <tr>
            <td valign=top align="left">
                <IMG SRC="<%=stars%>">
                <br>
<%
    ' Construct the URL for hit highlighting
    WebHitsQuery = "CiWebHitsFile=" & Server.URLEncode( RS("vpath") )
    WebHitsQuery = WebHitsQuery & "&CiRestriction=" & Server.URLEncode( Q.Query )
    WebHitsQuery = WebHitsQuery & "&CiUserParam3=" & QueryForm
    WebHitsQuery = WebHitsQuery & "&CiLocale=" & Util.LocaleIDToISO( Q.LocaleID )
 %>
                <a href="qsumrhit.htw?<%= WebHitsQuery %>">
                <IMG src="hilight.gif" align="left" alt="Highlight matching terms in document using Summary mode."> Summary</a>
                <br>
                <a href="qfullhit.htw?<%= WebHitsQuery %>&CiHiliteType=Full">
                <IMG src="hilight.gif" align="left" alt="Highlight matching terms in document."> Full</a>
            </td>
            <td valign=top>
                <%if VarType(RS("characterization")) = 8 and RS("characterization") <> "" then%>
                <b><i>Abstract:  </i></b>
                   <%= Server.HTMLEncode(RS("characterization"))%>
                <%end if%>
                <p>
                <i class="RecordStats"><a href="javascript:NAF('http://<%=Request.ServerVariables("SERVER_NAME")%><%=RS("vpath")%>','<%=SearchString%>')" class="RecordStats" style="color:blue;">http://<%=Request("server_name")%><%=RS("vpath")%></a>
                <br><%if RS("size") = "" then%>(size and time unknown)<%else%>size <%=RS("size")%> bytes - <%=RS("write")%> GMT<%end if%></i>
            </td>
        </tr>
        <tr>
        </tr>
<%
          RS.MoveNext
          NextRecordNumber = NextRecordNumber+1
      Loop
 %>

</table>

<P><BR>

<%
  else   ' NOT RS.EOF
      if NextRecordNumber = 1 then
          Response.Write "No documents matched the query<P>"
      else
          Response.Write "No more documents in the query<P>"
      end if

  end if ' NOT RS.EOF


if NOT Q.OutOfDate then
'   If the index is current, display the fact %>
    <P>
    <I><B>The index is up to date.</B></I><BR>
<%end if

if Q.QueryIncomplete then
'    If the query was not executed because it needed to enumerate to
'    resolve the query instead of using the index, but AllowEnumeration
'    was FALSE, let the user know
 %>
    <P>
    <I><B>The query is too expensive to complete.</B></I><BR>
<%end if

if Q.QueryTimedOut then
'    If the query took too long to execute (for example, if too much work
'    was required to resolve the query), let the user know %>
    <P>
    <I><B>The query took too long to complete.</B></I><BR>
<%end if%>


<TABLE>

<%

'    This is the "previous" button.
'    This retrieves the previous page of documents for the query.

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
            <INPUT TYPE="HIDDEN" NAME="RankBase" VALUE="<%=RankBase%>">

            <input type="submit" value="Previous <%=RS.PageSize%> documents">
        </form>
    </td>
    <%SaveQuery = UseSessions%>
<%end if%>

<%if Not RS.EOF then
    ' This is the "next" button.
    ' This button retrieves the next page of documents for the query.

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
            <INPUT TYPE="HIDDEN" NAME="RankBase" VALUE="<%=RankBase%>">
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
        set Session("Query") = Q
        set Session("RecordSet") = RS
    else
        RS.close
        Set RS = Nothing
        Set Q = Nothing
        Set Util = Nothing
        if UseSessions then
            set Session("Query") = Nothing
            set Session("RecordSet") = Nothing
        end if
    end if
end if
 %>
<BR>
<!--#include file ="is2foot.inc"-->

</BODY>
</HTML>

