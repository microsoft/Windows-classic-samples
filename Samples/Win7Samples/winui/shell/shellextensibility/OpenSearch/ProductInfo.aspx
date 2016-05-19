<%-- THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 Copyright (c) Microsoft Corporation. All rights reserved--%>
 
<%@ Page Language="C#" AutoEventWireup="true" CodeFile="ProductInfo.aspx.cs" Inherits="ProductInfo" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title></title>
    <style type="text/css">
        .style1
        {
            width: 400px;
            height: 120px;
        }
    </style>
</head>
<body>
    <form id="form1" runat="server">
    <div>
    <h1><img class="style1" src="aw.jpg" /><br /><%=HttpUtility.HtmlEncode(GetTitle()) %></h1>
    <img src="<%=HttpUtility.HtmlEncode(GetImageUrl()) %>" alt="Picture of this product."/>
    <%=GetDescription()%>
    </div>
    </form>
</body>
</html>
