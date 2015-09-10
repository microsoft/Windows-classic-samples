//
//  <copyright file="TopLevelTab.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;

namespace WSSTopLevelTabWinformsSample
{
    [ContainsCustomControl] // Required attribute for custom tabs
    public class TopLevelTabPageProvider : PageProvider
    {
        public TopLevelTabPageProvider()
            : base(new Guid("bc6078e3-6848-49fc-a746-c5df3cf114c6"),  // Put your fixed, static guid here
                   "WSS Winform Tab Sample",
                   "WSS Dashboard Addin Sample for a top level tab containing a Winform User Control SubTab")
        {
        }

        protected override Icon CreateImage()
        {
            return SystemIcons.Information;
        }

        protected override object CreatePages()
        {
            return (new object[] { new SubTabPage() });
        }
    }
}
