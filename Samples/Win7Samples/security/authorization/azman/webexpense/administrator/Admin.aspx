<!-- 
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 Copyright (c) Microsoft Corporation. All rights reserved.

 Carolyn Van Slyck 06/2003 - Created
 DaveMM - Updates 06/2005 - Tweaks, Updates, Fixes for SDK
-->
<%@ Page language="c#" Inherits="WebExpense.ExpenseWebAdministration" CodeFile="Admin.aspx.cs" %>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" >
<HTML>
	<HEAD>
		<title>Admin</title>
		<meta name="GENERATOR" Content="Microsoft Visual Studio .NET 7.1">
		<meta name="CODE_LANGUAGE" Content="C#">
		<meta name="vs_defaultClientScript" content="JavaScript">
		<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5">
	</HEAD>
	<body>
		<form id="Form1" method="post" runat="server">
			&nbsp;
			<asp:hyperlink id="LogoLink" runat="server" ImageUrl="../images/Treylogo.small.jpg" NavigateUrl="../index.aspx">HyperLink</asp:hyperlink>
			<asp:label id="AdministrationLbl" runat="server" Height="35px" Font-Bold="True" ForeColor="#000099">Administration</asp:label>
			<HR width="100%" SIZE="1">
			<asp:Panel id="AdminGroup" runat="server" Visible="False">
				<P>
					<asp:CheckBox id="self_approval" runat="server" Text="Allow Self Approval"></asp:CheckBox></P>
				<P>
					<asp:TextBox id="max_trans" runat="server" Width="32px"></asp:TextBox>
					<asp:Label id="MaxTransLbl" runat="server">Maximum Transactions (set to 0 for unlimited)</asp:Label>&nbsp;
					<asp:RangeValidator id="RangeValidator1" runat="server" ErrorMessage="Please enter a valid number of transactions"
						ControlToValidate="max_trans" Type="Integer" MaximumValue="100" MinimumValue="0"></asp:RangeValidator></P>
				<P>
					<asp:Button id="OK" runat="server" Text="Submit" onclick="OK_Click"></asp:Button>
					<asp:Button id="Cancel" runat="server" Text="Cancel" onclick="Cancel_Click"></asp:Button></P>
			</asp:Panel>
		</form>
		<asp:Label id="Message" runat="server" ForeColor="Red" Font-Bold="True"></asp:Label>
	</body>
</HTML>

