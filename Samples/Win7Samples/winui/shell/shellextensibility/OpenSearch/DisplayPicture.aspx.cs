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
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

public partial class DisplayPicture : System.Web.UI.Page
{
    protected void Page_Load(object sender, EventArgs e)
    {
        if (String.IsNullOrEmpty(Request.Params["id"]))
        {
            Response.Redirect("default.aspx");
        }

        AdvWorksDataContext db = new AdvWorksDataContext();
        Response.ContentType = "image/jpeg";
        var picture = (from p in db.ProductProductPhotos
                       where p.ProductID == int.Parse(Request.QueryString["id"])
                       select (Request.QueryString["size"] == "small") ? p.ProductPhoto.ThumbNailPhoto : p.ProductPhoto.LargePhoto).FirstOrDefault();

        System.Drawing.Image img = System.Drawing.Image.FromStream(new MemoryStream(picture.ToArray()));
        img.Save(Response.OutputStream, ImageFormat.Jpeg);
    }
}
