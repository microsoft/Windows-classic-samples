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

namespace Microsoft.Samples.EventLog.Configuration
{
    //
    // The ChannelConfig class demonstrates some of the configuration capabilities of  
    // Windows Event Log.  This sample reads and sets an event log's maximum size.
    // NOTE: setting the maximum log size requires running with administrative privileges.
    //

    public static class ChannelConfig
    {
        public static void Main(string[] args)
        {
            int exitCode = 0;
            String channelPath = "Application";

            try 
            {
                // 
                // Parse the command line.
                //
                if (args.Length > 0) 
                {
                    if (args[0] == "/?" || args[0] == "-?")
                    {
                        Console.WriteLine("Usage: ChannelConfig [<channelPath> [<newMaxLogSizeInBytes>]]\n" +
                                          "<channelPath> is the name of an existing event channel.\n" +
                                          "EXAMPLE: ChannelConfig Microsoft-Windows-TaskScheduler/Operational 10485760\n");
                        Environment.Exit(0);
                    }
                    else
                    {
                        channelPath = args[0];
                    }
                }

                //
                // Read a configuration property of the specified channel.
                //
                EventLogConfiguration config = new EventLogConfiguration(channelPath);
                Console.WriteLine("The {0} log's configured maximum size is {1} bytes.", channelPath, config.MaximumSizeInBytes);
                              
                //
                // Set and save a configuration property value: 
                // double the current maximum log size, if not supplied on the command line.
                //
                if (args.Length > 1)
                {
                    config.MaximumSizeInBytes = Convert.ToInt64(args[1], CultureInfo.InvariantCulture);
                }
                else
                {
                    config.MaximumSizeInBytes *= 2;
                }

                config.SaveChanges();
                Console.WriteLine("The {0} log's maximum size has been re-configured to {1} bytes.", channelPath, config.MaximumSizeInBytes);
            }
            catch (UnauthorizedAccessException e)
            {
                Console.WriteLine("You do not have the correct permissions. " +
                                  "Try re-running the sample with administrator privileges.\n" + e.ToString());
                exitCode = 1;
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
