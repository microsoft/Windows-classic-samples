<!-- 
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 Copyright (c) Microsoft Corporation. All rights reserved.

 Carolyn Van Slyck 06/2003 - Created
 DaveMM - Updates 06/2005 - Tweaks, Updates, Fixes for SDK
-->
<%@ Page language="c#" Inherits="WebExpense.List" CodeFile="List.aspx.cs" %>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" >
<HTML>
	<HEAD>
		<title>List</title>
		<meta content="Microsoft Visual Studio .NET 7.1" name="GENERATOR">
		<meta content="C#" name="CODE_LANGUAGE">
		<meta content="JavaScript" name="vs_defaultClientScript">
		<meta content="http://schemas.microsoft.com/intellisense/ie5" name="vs_targetSchema">
	</HEAD>
	<body vLink="#000099" aLink="#000099" link="#000099">
		<form id="Form1" method="post" runat="server">
			<asp:hyperlink id="LogoLink" runat="server" NavigateUrl="../index.aspx" ImageUrl="../images/Treylogo.small.jpg">HyperLink</asp:hyperlink><asp:label id="TitleLbl" runat="server" Height="35px" ForeColor="#000099" Font-Bold="True">Expense Listing</asp:label>
			<HR width="100%" SIZE="1">
			<asp:label id="MSG" runat="server"></asp:label><BR>
			<BR>
			<asp:dropdownlist id="ModeSelect" runat="server" AutoPostBack="True" onselectedindexchanged="ModeSelect_SelectedIndexChanged">
				<asp:ListItem Value="ALL">All</asp:ListItem>
				<asp:ListItem Value="" Selected="True">Pending</asp:ListItem>
				<asp:ListItem Value="APPROVED">Approved</asp:ListItem>
				<asp:ListItem Value="REJECTED">Rejected</asp:ListItem>
			</asp:dropdownlist>
			<asp:label id="mode" runat="server" Visible="False"></asp:label>
			<P title="List Expenses">
				<asp:Table id="TransList" runat="server" GridLines="Both" BorderWidth="1px" BorderStyle="Solid"
					BorderColor="Black" CellSpacing="0" CellPadding="5">
					<asp:TableRow ForeColor="Black" BackColor="LightSteelBlue" Font-Bold="True">
						<asp:TableCell Wrap="False" Text="Select an Expense"></asp:TableCell>
						<asp:TableCell Text="Status"></asp:TableCell>
					</asp:TableRow>
				</asp:Table></P>
		</form>
		<P>
			<asp:HyperLink id="ReturnLink" style="TEXT-DECORATION: none" runat="server" NavigateUrl="../index.aspx">Return to Main Menu</asp:HyperLink></P>
	</body>
</HTML>
