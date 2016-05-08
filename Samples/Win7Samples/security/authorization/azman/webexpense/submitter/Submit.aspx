<!-- 
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 Copyright (c) Microsoft Corporation. All rights reserved.

 Carolyn Van Slyck 06/2003 - Created
 DaveMM - Updates 06/2005 - Tweaks, Updates, Fixes for SDK
-->
<%@ Page language="c#" Inherits="WebExpense.Submit" CodeFile="Submit.aspx.cs" %>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" >
<HTML>
	<HEAD>
		<title>Submit</title>
		<meta name="GENERATOR" Content="Microsoft Visual Studio .NET 7.1">
		<meta name="CODE_LANGUAGE" Content="C#">
		<meta name="vs_defaultClientScript" content="JavaScript">
		<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5">
	</HEAD>
	<body vLink="#000099" aLink="#000099" link="#000099">
		<FORM id="Form1" method="post" runat="server">
			<P title="Submit Expense">
				<asp:HyperLink id="LogoLink" runat="server" ImageUrl="../images/Treylogo.small.jpg" NavigateUrl="../index.aspx">HyperLink</asp:HyperLink>
				<asp:Label id="TitleLbl" runat="server" Height="35px" Font-Bold="True" ForeColor="#000099">Expense Submission Tool</asp:Label></P>
			<HR width="100%" SIZE="1">
			<P title="Submit Expense">
				<asp:Label id="MSG" runat="server"></asp:Label></P>
			<asp:Panel id="SubmitGroup" runat="server">
				<P>
					<asp:Label id="DescriptionLbl" runat="server" Width="96px">Description:</asp:Label>
					<asp:TextBox id="Description" runat="server" Width="308px" MaxLength="50"></asp:TextBox>&nbsp;&nbsp;&nbsp;&nbsp;
					<asp:RequiredFieldValidator id="DescriptionVal" runat="server" ErrorMessage="Please enter a Description" ControlToValidate="Description"></asp:RequiredFieldValidator></P>
				<P>
					<asp:Label id="AmountLbl" runat="server" Width="96px">Amount:</asp:Label>
					<asp:TextBox id="Amount" runat="server" MaxLength="9"></asp:TextBox>
					<asp:RegularExpressionValidator id="AmountVal" runat="server" ErrorMessage="Please enter a valid amount." ControlToValidate="Amount"
						ValidationExpression="\$?[1-9][0-9]*(\.[0-9]{0,2})?"></asp:RegularExpressionValidator>
					<asp:RequiredFieldValidator id="AmountRequiredVal" runat="server" ErrorMessage="Please enter an amount." ControlToValidate="Amount"></asp:RequiredFieldValidator></P>
				<P>
					<asp:Label id="DateLbl" runat="server" Width="96px">Date:</asp:Label>
					<asp:TextBox id="Date" runat="server"></asp:TextBox>
					<asp:RangeValidator id="DateVal" runat="server" Width="168px" ErrorMessage="Please enter a valid date."
						ControlToValidate="Date" Type="Date" MinimumValue="01/01/1900" MaximumValue="12/31/2999"></asp:RangeValidator>
					<asp:RequiredFieldValidator id="DateRequiredVal" runat="server" ErrorMessage="Please enter a date." ControlToValidate="Date"></asp:RequiredFieldValidator></P>
				<P>
					<asp:Label id="CommentLbl" runat="server" Height="71px" Width="96px">Comments:</asp:Label>
					<asp:TextBox id="Comment" runat="server" Width="312px" MaxLength="255" Rows="4" TextMode="MultiLine"></asp:TextBox></P>
				<P>
					<asp:Button id="SubmitBtn" runat="server" Text="Submit" onclick="SubmitBtn_Click"></asp:Button>&nbsp;
					<asp:HyperLink id="ReturnLink" style="TEXT-DECORATION: none" runat="server" NavigateUrl="../index.aspx">Return to Main Menu</asp:HyperLink></P>
			</asp:Panel>
			<P>&nbsp;</P>
		</FORM>
	</body>
</HTML>
