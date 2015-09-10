//
//  <copyright file="Program.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.WindowsServerSolutions.Common.ProviderFramework;

namespace BasicProvider1
{
    public class Program
    {
        static void Main(string[] args)
        {
            //This Initialize method is needed to link to Windows Server references that are not GACed
            Microsoft.WindowsServerSolutions.Common.WindowsServerSolutionsEnvironment.Initialize();

            ProviderHost host = new ProviderHost(typeof(ProviderService), "provider");
            host.Open();

            System.Console.ReadLine();
            host.Dispose();
        }
    }
}
