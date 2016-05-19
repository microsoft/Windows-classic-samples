<%-- THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 Copyright (c) Microsoft Corporation. All rights reserved--%>

<%@ Page Language="C#" MasterPageFile="~/MasterPage.master" AutoEventWireup="true" CodeFile="search.aspx.cs" Inherits="search" %>

<asp:Content ID="searchpage" ContentPlaceHolderID="content" Runat="Server">
<link rel="alternate" type="application/rss+xml" title="RSS 2.0" href="searchrss.aspx?q=<%=Request.QueryString["q"] %>&type=<%=Request.QueryString["type"] %>" /> 

<h1>
    <img class="style1" src="aw.jpg" alt="Logo" /></h1>
<asp:ListView ID="LatestItemsView" runat="server">
<LayoutTemplate>
 <p>Here are the items that match your query:</p>
 <asp:PlaceHolder ID="itemPlaceholder" runat="server" />
 </LayoutTemplate>
<ItemTemplate>
 <div class="itemBoxSmall">
 <img src="DisplayPicture.aspx?size=small&id=<%#Eval("ProductID") %>"
 class="productImage" alt="Image of <%#Eval("Name") %>" />
 <a href="ProductInfo.aspx?id=<%#Eval("ProductID") %>" class="productLink">
 <%#Eval("Name") %>
 </a>
 <br />
 Color: <%#Eval("Color") %><br />
 List Price: <%#Eval("ListPrice", "{0:C}") %><br />
 </div>
</ItemTemplate>

</asp:ListView>
    <br />
    <asp:TextBox ID="searchBox" runat="server" BorderStyle="Inset" Width="200px"></asp:TextBox>
    <asp:Button ID="Button1" runat="server" onclick="Button1_Click" Text="Search" />
    <br />
    <br />
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

