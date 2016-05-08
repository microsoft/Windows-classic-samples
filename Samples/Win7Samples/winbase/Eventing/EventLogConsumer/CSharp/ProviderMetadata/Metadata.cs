// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


using System;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Diagnostics.Eventing.Reader;

[assembly: CLSCompliant(true)]
[assembly: ComVisible(false)]

namespace Microsoft.Samples.EventLog.ProviderMetadataSample
{
    //
    // The ProviderMetadataSample class demonstrates some of the provider and event discovery
    // capabilities of Windows Event Log.  This sample enumerates installed providers
    // and reads some of the provider and event properties.
    //

    public static class ProviderMetadataSample
    {
        public static void Main(string[] args)
        {
            int exitCode = 0;
            bool listProviders = true;            
            String providerName = "";            

            try 
            {
                // 
                // Get command line information.
                //
                if (args.Length > 0) 
                {
                    if (args[0] == "/?" || args[0] == "-?")
                    {
                        Console.WriteLine(
                            "Usage: ProviderMetadata [<providerName>]\n" +
                            "<providerName> is the name of an existing event provider\n" +
                            "When <providerName> is not specified, the names of the first 20 providers will be listed.\n" +
                            "EXAMPLE: ProviderMetadata.exe \"Application Error\"." );
                        Environment.Exit(0);
                    }
                    else
                    {
                        providerName = args[0];
                        listProviders = false;
                    }
                }                               

               if (listProviders)
               {
                    //
                    // List the first 20 providers.
                    //

                    EventLogSession session = new EventLogSession();
                    int count = 0;
                    foreach (string provName in session.GetProviderNames())
                    {
                        Console.WriteLine("\t{0}", provName);
                        if (++count >= 20) 
                        {
                            break;
                        }
                    }                    
               }               
               else
               {
                   //
                   // Read the specified provider's metadata.
                   //

                   ProviderMetadata providerMetadata = new ProviderMetadata(providerName);
                   Console.WriteLine("MessageFilePath for provider '{0}' is '{1}'.", providerName, providerMetadata.MessageFilePath);                               

                   //
                   // Read the provider's event metadata.
                   //                  

                   Console.WriteLine("Provider '{0}' contains event metadata for the following event ids:", providerName);
                   foreach (EventMetadata eventMetadata in providerMetadata.Events)                   
                   {
                       Console.WriteLine("\t{0}", eventMetadata.Id);                       
                   }                   
               }
            }
            catch(Exception e) 
            {
                Console.WriteLine(e.ToString());
                exitCode = 1;
            }
            
            Environment.Exit(exitCode);
        }

    }

}
