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
using System.IO;

public partial class GetOSDX : System.Web.UI.Page
{
    protected void Page_Load(object sender, EventArgs e)
    {
        // This ASPX page makes sure we return the OSDX content with the correct MIME type.
        if (!String.IsNullOrEmpty(Request.Params["file"]))
        {
            Response.ContentType = "application/opensearchdescription+xml";
            Response.WriteFile(Request.Params["file"]);
        }
        else
        {
            Response.Redirect("default.aspx");
        }
    }
}
