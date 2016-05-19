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

public partial class searchrss : System.Web.UI.Page
{
    protected void Page_Load(object sender, EventArgs e)
    {
        // Handle the query from the URL
        string query = Request.QueryString["q"]; 
        if (query == null)
            query = "";
        bool returnPhotos = (Request.QueryString["type"] == "photos");
        AdvWorksDataContext db = new AdvWorksDataContext();

        try
        {
            Response.ContentType = "application/rss+xml";
            XNamespace nsMediaRSS = "http://search.yahoo.com/mrss/";
            XNamespace nsExample = "http://sdk.microsoft.com/example/";
            XNamespace nsDescription = "http://schemas.microsoft.com/sqlserver/2004/07/adventure-works/ProductModelDescription";
            XDocument xml;

            // Query DB and generate RSS output using LINQ
            xml =
               new XDocument(
                  new XElement("rss",
                     new XAttribute("version", "2.0"),
                     new XAttribute(XNamespace.Xmlns + "media", "http://search.yahoo.com/mrss/"),
                     new XAttribute(XNamespace.Xmlns + "example", "http://sdk.microsoft.com/example/"),
                     new XElement("channel",
                        new XElement("title", "AdventureWorks Product Search Results - " + query),
                        new XElement("link", Request.Url),
                        new XElement("description", "Example search results from an example database."),
                        from model in db.ProductModels
                        where model.CatalogDescription != null &&
                        (model.Products.Any(p => p.Name.ToLower().Contains(query.ToLower())) ||
                            model.CatalogDescription.ToString().ToLower().Contains(query.ToLower()))
                        select new XElement("item",
                           new XElement("title", model.Name),
                           new XElement("description", model.CatalogDescription.Element(nsDescription + "Summary").Value),
                           new XElement("link", String.Format(
                                                    "http://localhost:63000/AdventureSearch/productinfo.aspx?id={0}", 
                                                    model.Products.FirstOrDefault().ProductID)),
                           new XElement("pubDate", DateTime.Now.ToString("R")),

                           // add custom thumbnail
                           new XElement(nsMediaRSS + "thumbnail", 
                           new XAttribute("url", "http://localhost:63000/AdventureSearch/DisplayPicture.aspx?id=" +
                            model.Products.FirstOrDefault().ProductID + "&size=small")),

                            // add enclosure if type == photos
                            returnPhotos ? new XElement("enclosure",
                                new XAttribute("type", "image/jpeg"),
                                new XAttribute("url", "http://localhost:63000/AdventureSearch/DisplayPicture.aspx?id=" +
                            model.Products.FirstOrDefault().ProductID + "&size=large")) : null,

                            // add custom property
                           new XElement(nsExample + "color",
                              model.Products.FirstOrDefault().Color),
                           new XElement(nsExample + "copyright",
                              model.CatalogDescription.Element(nsDescription + "Manufacturer").Element(nsDescription + "Copyright").Value),
                           new XElement(nsExample + "company",
                              model.CatalogDescription.Element(nsDescription + "Manufacturer").Element(nsDescription + "Name").Value)
                          )
                        )
                     )
                  );

            XmlTextWriter writer = new XmlTextWriter(Response.Output);
            writer.Formatting = Formatting.Indented;
            xml.WriteTo(writer);
            writer.Close();
        }
        catch (Exception ex)
        {
            Response.Redirect("readme.txt");
            Debug.WriteLine(ex.Message);
            Debug.WriteLine("Did you forget to set up the AdventureWorks SQL Database?");
        }
    }
}
