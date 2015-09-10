//
//  <copyright file="SubTabPage.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Windows.Forms;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;

namespace WSSTopLevelTabWinformsSample
{
    public class SubTabPage : ControlRendererPage
    {
        public SubTabPage()
            : base(new Guid("ab80dc7c-8964-4a62-8c7a-1c50afad70cc"), // Put your fixed, static guid here
                   "WSS Winform SubTab Sample",
                   "WSS Dashboard Addin Sample consisting of a SubTab containing a Winform User Control")
        { 
        }

        protected override ControlRendererPageContent CreateContent()
        {
            return ControlRendererPageContent.Create(new MyCustomWinformControl());
        }
    }
}
