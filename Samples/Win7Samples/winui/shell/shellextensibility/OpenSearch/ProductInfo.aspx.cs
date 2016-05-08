// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.

// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

public partial class ProductInfo : System.Web.UI.Page
{
    AdvWorksDataContext db = new AdvWorksDataContext();
    protected void Page_Load(object sender, EventArgs e)
    {

    }

    public String GetTitle()
    {
         var title = (from p in db.Products
                           where p.ProductID == int.Parse(Request.QueryString["id"])
                           select p.Name).FirstOrDefault();
         return title;
    }
    public String GetDescription()
    {
        var price = (from p in db.Products
                     where p.ProductID == int.Parse(Request.QueryString["id"])
                     select p.ListPrice).FirstOrDefault();
        var weight = (from p in db.Products
                     where p.ProductID == int.Parse(Request.QueryString["id"])
                     select p.Weight).FirstOrDefault();
        return "<h3>Price: " + price.ToString("C") + "</h3><p>Weight: " + weight.ToString() + "lbs</p>";
    }
    public String GetImageUrl()
    {
        return "http://localhost:63000/AdventureSearch/DisplayPicture.aspx?id=" + Request.QueryString["id"];
    }
}
