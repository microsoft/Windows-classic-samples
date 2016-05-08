<%@ Language=VBScript CODEPAGE=65001 %>
<!--#include file="include/wmsLocStrings.inc"-->
<!--#include file="include/wmsServerHash.inc"-->
<!--#include file="include/wmsPlugins.inc"-->
<!--#include file="include/wmsHeader.inc"-->
<!--#include file="include/wmsPageBanner.inc"-->
<!--#include file="include/wmsError.inc"-->
<!--#include file="plugin_loading.inc"-->
<%
'+-------------------------------------------------------------------------
'
'  Microsoft Windows Media
'  Copyright (C) Microsoft Corporation. All rights reserved.
'
'  File:       ContextSample.asp
'
'  Contents:
'
'--------------------------------------------------------------------------
ConnectToPlugin
ConnectToPluginAdmin

BeginErrorHandling

Dim bError
Dim strOutputPath
Dim bUserContext
Dim bPresContext
Dim bCmdReqContext
Dim bCmdResponseContext
Dim dwContextTypes

'typedef [public] enum WMS_CONTEXT_PLUGIN_CONTEXT_TYPE
Const WMS_CONTEXT_PLUGIN_NO_CONTEXT = 0
Const WMS_CONTEXT_PLUGIN_USER_CONTEXT = 1
Const WMS_CONTEXT_PLUGIN_PRESENTATION_CONTEXT = 2
Const WMS_CONTEXT_PLUGIN_COMMAND_REQUEST_CONTEXT = 4
Const WMS_CONTEXT_PLUGIN_COMMAND_RESPONSE_CONTEXT = 8

strOutputPath = g_objPluginAdmin.OutputPath
dwContextTypes = g_objPluginAdmin.ContextTypes
bUserContext = CBool( 0 <> ( WMS_CONTEXT_PLUGIN_USER_CONTEXT and dwContextTypes ) )
bPresContext = CBool( 0 <> ( WMS_CONTEXT_PLUGIN_PRESENTATION_CONTEXT and dwContextTypes ) )
bCmdReqContext = CBool( 0 <> ( WMS_CONTEXT_PLUGIN_COMMAND_REQUEST_CONTEXT and dwContextTypes ) )
bCmdResponseContext = CBool( 0 <> ( WMS_CONTEXT_PLUGIN_COMMAND_RESPONSE_CONTEXT and dwContextTypes ) )

dwTabIndex = 0

HandlePluginLoadingChanges()
on error resume next

Dim szOp
szOp = trim( posting( "submit" ) )
if( 0 < Len( szOp ) ) then
    Do
        strOutputPath = CStr( trim( posting( "outputPath" ) ) )
        if( 0 <> StrComp( g_objPluginAdmin.OutputPath, strOutputPath, vbTextCompare ) ) then
            g_objPluginAdmin.OutputPath = strOutputPath
            if( ErrorDetected( "outputPath" ) ) then
                exit Do
            end if
        end if

        Dim dwNewContextTypes
        dwNewContextTypes = WMS_CONTEXT_PLUGIN_NO_CONTEXT

        bUserContext = CBool( "" <> posting( "userContext" ) )
        if( bUserContext ) then
            dwNewContextTypes = WMS_CONTEXT_PLUGIN_USER_CONTEXT
        end if

        bPresContext = CBool( "" <> posting( "presContext" ) )
        if( bPresContext ) then
            dwNewContextTypes = CInt( dwNewContextTypes or WMS_CONTEXT_PLUGIN_PRESENTATION_CONTEXT )
        end if

        bCmdReqContext = CBool( "" <> posting( "cmdRequest" ) )
        if( bCmdReqContext ) then
            dwNewContextTypes = CInt( dwNewContextTypes or WMS_CONTEXT_PLUGIN_COMMAND_REQUEST_CONTEXT )
        end if

        bCmdResponseContext = CBool( "" <> posting( "cmdResponse" ) )
        if( bCmdResponseContext ) then
            dwNewContextTypes = CInt( dwNewContextTypes or WMS_CONTEXT_PLUGIN_COMMAND_RESPONSE_CONTEXT )
        end if

        if( dwNewContextTypes <> dwContextTypes ) then
            g_objPluginAdmin.ContextTypes = dwNewContextTypes
        end if
        
        if ( 0 = Session( "ErrorNumber" ) ) then
            if( "" <> g_strEncodedPubPointName ) then
                Response.Redirect "../pubpoints/pubpoint_props.asp?server=" & g_strQueryStringServer & "&ppID=" & g_strPubPointID & "&category=" & g_strCategory & "&pluginIndex=" & qs("pluginIndex")
            else
                Response.Redirect "../server_props.asp?server=" & g_strQueryStringServer & "&category=" & g_strCategory & "&pluginIndex=" & qs("pluginIndex")
            end if
        end if
    Loop Until TRUE
end if

WriteHTMLHeader( g_strDecodedServerName ) 
%>
<link rel="stylesheet" type="text/css" href="<%= Session( "cssName" ) %>">
<% WritePluginJSUtils %>
</head>
<body class="pluginBody" oncontextmenu="JavaScript:event.cancelBubble=true;return false;">
<% DrawPluginBanner %>
<table width="<%= STDTABLEWIDTH %>" class="propgroupbox">
<tr>
    <td valign=top>

        <% WriteStdPluginForm %>
        
        <p>
        &nbsp;
        
        <table cellspacing=1 cellpadding=0 border=0>
        <tr>
            <td class="defaultcursor">
                <% RenderWithErrorCheck Server.HTMLEncode( "Output Path:" ), "outputPath" %>
            </td>
        </tr>
        <tr>
            <td class="defaultcursor"><% dwTabIndex = dwTabIndex + 1 %>
                <input type="text" name="outputPath" value="<%= RemoveSpecifiedChars( strOutputPath, REGEXP_DANGEROUS_CHARS ) %>" tabindex=<%= dwTabIndex %> >
            </td>
        </tr>    
        </table>

        <p>
        <span class="helptext">
            Context types to show:
        </span>
        <table cellspacing=1 cellpadding=0 border=0>
        <tr>
            <td class="defaultcursor"><% dwTabIndex = dwTabIndex + 1 %>
                <input type="checkbox" name="userContext" value="on" <% if bUserContext then %>checked<% end if %> tabindex=<%= dwTabIndex %> >
            </td>
            <td align=left width=10>
                &nbsp;
            </td>
            <td class="defaultcursor">
                User Context
            </td>
        </tr>
        <tr>
            <td class="defaultcursor"><% dwTabIndex = dwTabIndex + 1 %>
                <input type="checkbox" name="presContext" value="on" <% if bPresContext then %>checked<% end if %> tabindex=<%= dwTabIndex %> >
            </td>
            <td align=left width=10>
                &nbsp;
            </td>
            <td class="defaultcursor">
                Presentation Context
            </td>
        </tr>    
        <tr>
            <td class="defaultcursor"><% dwTabIndex = dwTabIndex + 1 %>
                <input type="checkbox" name="cmdRequest" value="on" <% if bCmdReqContext then %>checked<% end if %> tabindex=<%= dwTabIndex %> >
            </td>
            <td align=left width=10>
                &nbsp;
            </td>
            <td class="defaultcursor">
                Command Request Context
            </td>
        </tr>    
        <tr>
            <td class="defaultcursor"><% dwTabIndex = dwTabIndex + 1 %>
                <input type="checkbox" name="cmdResponse" value="on" <% if bCmdResponseContext then %>checked<% end if %> tabindex=<%= dwTabIndex %> >
            </td>
            <td align=left width=10>
                &nbsp;
            </td>
            <td class="defaultcursor">
                Command Response Context
            </td>
        </tr>    
        </table>
    </td>
</tr>
<tr>
    <td>
        &nbsp;<br>&nbsp;
    </td>
    <td>
        &nbsp;
    </td>
<tr>
    <td valign=bottom><% dwTabIndex = dwTabIndex + 1 %>
        <INPUT type="submit" align="baseline" name="ok" value="<%= Server.HTMLEncode( L_OKAYBUTTON_TEXT ) %>" tabIndex=<%= dwTabIndex %> > <% dwTabIndex = dwTabIndex + 1 %>
        <INPUT type="button" align="baseline" name="cancel" onclick="Cancel()" value="<%= Server.HTMLEncode( L_CANCELBUTTONSPACED_TEXT ) %>" tabIndex=<%= dwTabIndex %> > <% dwTabIndex = dwTabIndex + 1 %>
    </td>
</tr>
</table>
</font>
<% RenderPluginLoadingOptions() %>
</form>
<% 
AlertUserWithPopupErrorDialog
OnErrorGoBack 
DrawStdFooter
%>
</body>
</html>
<%
LatchCurrentPage "plugins/ContextSample.asp", qs
EndErrorHandling "ContextSample.asp" 

on error resume next
PluginsASPCleanup
%>