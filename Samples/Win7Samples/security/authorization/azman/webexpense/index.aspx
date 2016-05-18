<!-- 
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 Copyright (c) Microsoft Corporation. All rights reserved.

 Carolyn Van Slyck 06/2003 - Created
 DaveMM - Updates 06/2005 - Tweaks, Updates, Fixes for SDK
-->
<%@ Page language="c#" Inherits="WebExpense.MainPage" CodeFile="index.aspx.cs" %>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" >
<HTML>
	<HEAD>
		<title>WebExpense</title>
		<meta content="Microsoft Visual Studio .NET 7.1" name="GENERATOR">
		<meta content="C#" name="CODE_LANGUAGE">
		<meta content="JavaScript" name="vs_defaultClientScript">
		<meta content="http://schemas.microsoft.com/intellisense/ie5" name="vs_targetSchema">
	</HEAD>
	<body vLink="#000099" aLink="#000099" link="#000099">
		<form id="Form1" method="post" runat="server">
			<P><asp:image id="logo" runat="server" ImageUrl="images/Treylogo.lg.jpg"></asp:image></P>
			<P>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<asp:label id="title" Runat="server">
					<FONT color="#000099"><FONT size="5"><EM>WebExpense</EM></FONT> - online expense 
						reporting and auditing</FONT>
				</asp:label></P>
			<HR width="100%" SIZE="1">
			<P><asp:label id="MSG" runat="server" Font-Bold="True" Font-Italic="True"></asp:label></P>
			<P>
				<asp:hyperlink id="SubmitLink" style="FONT-WEIGHT: bold; TEXT-DECORATION: none" runat="server"
					NavigateUrl="Submitter/Submit.aspx" Visible="False">
					<asp:Image id="SubmitImg" runat="server" ImageUrl="images\spreadsheet.gif" ImageAlign="Middle"></asp:Image>
					Submit an Expense</asp:hyperlink></P>
			<P>
				<asp:HyperLink id="ApproveLink" runat="server" Font-Bold="True" Style="TEXT-DECORATION:none" NavigateUrl="Approver/List.aspx"
					Visible="False">
					<asp:Image id="ApproveImg" runat="server" ImageUrl="images\checked.gif" ImageAlign="Middle"></asp:Image>
					Approve Expenses</asp:HyperLink></P>
			<P>
				<asp:HyperLink id="AdminLink" runat="server" Font-Bold="True" Style="TEXT-DECORATION:none" Visible="False"
					NavigateUrl="Administrator/Admin.aspx">
					<asp:Image id="AdminImg" runat="server" ImageUrl="images\settings.gif" ImageAlign="Middle"></asp:Image>
					Application Settings</asp:HyperLink></P>
		</form>
	</body>
</HTML>
