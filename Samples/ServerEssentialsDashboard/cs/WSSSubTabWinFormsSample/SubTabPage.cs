//
//  <copyright file="SubTabPage.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;
using System.Diagnostics.CodeAnalysis;

namespace WSSSubTabWinFormsSample
{
    public class SubTabPage : ControlRendererPage
    {
        public SubTabPage()
            : base(new Guid("620e54f5-f290-440a-941b-dabbd882fce3"), // Put your fixed, static GUID here
                   "WSS Winform SubTab Sample",
                   "WSS Dashboard Addin Sample consisting of a SubTab containing a Winform User Control")
        {
        }

        protected override ControlRendererPageContent CreateContent()
        {
            return ControlRendererPageContent.Create(new MyCustomWinformControl());
        }

        protected override HelpTopicInfo CreateHelpTopicInfo()
        {
            // Replace with a custom URL or CHM/HTML file path.
            return new HelpTopicInfo(new Uri("http://servername/help"));
        }
    }
}
