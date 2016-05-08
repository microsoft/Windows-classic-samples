<!-- 
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 Copyright (c) Microsoft Corporation. All rights reserved.


 Carolyn Van Slyck 06/2003 - Created
 DaveMM - Updates 06/2005 - Tweaks, Updates, Fixes for SDK
-->
<%@ Page language="c#" Inherits="WebExpense.Display" CodeFile="Display.aspx.cs" %>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" >
<HTML>
	<HEAD>
		<title>Display Expenses</title>
		<meta content="Microsoft Visual Studio .NET 7.1" name="GENERATOR">
		<meta content="C#" name="CODE_LANGUAGE">
		<meta content="JavaScript" name="vs_defaultClientScript">
		<meta content="http://schemas.microsoft.com/intellisense/ie5" name="vs_targetSchema">
	</HEAD>
	<body>
		<form id="Form1" method="post" runat="server">
			<P>
				<asp:hyperlink id="LogoLink" runat="server" ImageUrl="../images/Treylogo.small.jpg" NavigateUrl="../index.aspx">HyperLink</asp:hyperlink>
				<asp:label id="TitleLbl" runat="server" Height="35px" Font-Bold="True" ForeColor="#000099">View Expense</asp:label></P>
			<HR width="100%" SIZE="1">
			<P><asp:table id="TransData" runat="server" GridLines="Both" BorderColor="Black" BorderStyle="Solid"
					CellSpacing="0" CellPadding="5">
					<asp:TableRow ForeColor="Black" BackColor="LightSteelBlue" Font-Bold="True">
						<asp:TableCell Text="User"></asp:TableCell>
						<asp:TableCell Text="Description"></asp:TableCell>
						<asp:TableCell Text="Date"></asp:TableCell>
						<asp:TableCell Text="Amount"></asp:TableCell>
						<asp:TableCell Text="Comment"></asp:TableCell>
					</asp:TableRow>
				</asp:table></P>
			<P>
				<asp:Label id="Status" runat="server" ForeColor="Navy" Font-Bold="True" Font-Size="16px">Status: </asp:Label><asp:panel id="DecisionGroup" runat="server" Visible="False">
					<asp:Label id="CommentsLbl" runat="server">Additional Comments</asp:Label>
					<BR>
					<asp:TextBox id="Comment" runat="server" Height="96px" MaxLength="255" TextMode="MultiLine" Rows="4"
						Width="312px"></asp:TextBox>
					<BR>
					<BR>
					<asp:RadioButtonList id="Decision" runat="server" RepeatDirection="Horizontal">
						<asp:ListItem Value="approve">Approve</asp:ListItem>
						<asp:ListItem Value="reject">Reject</asp:ListItem>
					</asp:RadioButtonList>
					<BR>
					<asp:Button id="SubmitDecision" runat="server" Text="Submit" onclick="SubmitDecision_Click"></asp:Button>
					<asp:HyperLink id="ReturnLink" style="TEXT-DECORATION: none" runat="server" NavigateUrl="List.aspx">Return to Expense Listing</asp:HyperLink>
				</asp:panel></P>
		</form>
		<asp:Label id="Message" runat="server" ForeColor="Red" Font-Bold="True"></asp:Label>
	</body>
</HTML>