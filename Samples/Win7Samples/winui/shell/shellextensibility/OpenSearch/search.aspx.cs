// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.

// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.HtmlControls;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Data.OleDb;
using System.Diagnostics;
using System.Xml;
using System.Xml.Linq;

public partial class search : System.Web.UI.Page
{
    protected void Page_Load(object sender, EventArgs e)
    {
        // Handle the query from the URL
        string query = Request.QueryString["q"];
        if (query == null)
            query = "";
        AdvWorksDataContext db = new AdvWorksDataContext();

        try
        {
            var latestItems = (from model in db.ProductModels
                               where model.CatalogDescription != null &&
                                   model.Products.Any(p => p.Name.ToLower().Contains(query.ToLower()) ||
                                   model.CatalogDescription.ToString().ToLower().Contains(query.ToLower()))
                               select model.Products.First());
            LatestItemsView.DataSource = latestItems;
            LatestItemsView.DataBind();
        }
        catch (Exception ex)
        {
            Response.Redirect("readme.txt");
            Debug.WriteLine(ex.Message);
            Debug.WriteLine("Did you forget to set up the AdventureWorks SQL Database?");
        }
    }
    protected void Button1_Click(object sender, EventArgs e)
    {
        if (Request.QueryString["type"] == "photos")
        {
            Response.Redirect("search.aspx?type=photos&q=" + searchBox.Text, true);
        }
        else
        {
            Response.Redirect("search.aspx?q=" + searchBox.Text, true);
        }
    }
}
