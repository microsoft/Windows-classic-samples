//
//  <copyright file="CustomSubTabPage.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Windows.Forms;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;

namespace TopLevelTab
{
    public class CustomSubTabPage : ControlRendererPage
    {
        public CustomSubTabPage()
            : base(new Guid("3b9c2df2-ce04-4c5c-a8e0-c9ebd4493e52"), // Put your fixed, static guid here
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
