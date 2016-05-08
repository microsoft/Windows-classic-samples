<%-- THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 Copyright (c) Microsoft Corporation. All rights reserved--%>
 
<%@ Page Language="C#" MasterPageFile="~/MasterPage.master" AutoEventWireup="true" CodeFile="Default.aspx.cs" Inherits="_Default" Title = "AdventureWorks Catalog" %>

<asp:Content ID="homepage" ContentPlaceHolderID="content" Runat="Server">

<h1>
    <img class="style1" src="aw.jpg" alt="Logo" /></h1>
    <br />
    <asp:TextBox ID="searchBox" runat="server" Width="200px" BorderStyle="Inset"></asp:TextBox>
    <asp:Button ID="Button1" runat="server" onclick="Button1_Click" Text="Search Products" />
    <asp:Button ID="Button2" runat="server" OnClick="Button2_Click" Text="Search Photos" />
    <p>
        <a href="getOSDX.aspx?file=Product%20Search.osdx">Search Products in Windows Explorer</a>
        <br />
        <a href="getOSDX.aspx?file=Photo%20Search.osdx">Search Photos in Windows Explorer</a>
    </p>
</asp:Content>
<asp:Content ID="Content1" runat="server" contentplaceholderid="head">

    <style type="text/css">
        .style1
        {
            width: 400px;
            height: 120px;
        }
    </style>

</asp:Content>

