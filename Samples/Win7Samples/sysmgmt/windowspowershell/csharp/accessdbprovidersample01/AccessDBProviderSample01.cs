//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//

using System.Management.Automation;
using System.Management.Automation.Provider;
using System.ComponentModel;


namespace Microsoft.Samples.PowerShell.Providers
{
   #region AccessDBProvider

    /// <summary>
   /// Simple provider.
   /// </summary>
   [CmdletProvider("AccessDB", ProviderCapabilities.None)]
   public class AccessDBProvider : CmdletProvider
   {

   }

   #endregion AccessDBProvider
}

