// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.IO;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Diagnostics.Eventing.Reader;

[assembly: CLSCompliant(true)]
[assembly: ComVisible(false)]

namespace Microsoft.Samples.EventLog.LogManagement
{
    //
    // The LogManagement class demonstrates some of the logging capabilities of Windows Event Log.  
    // This sample retrieves log information, exports events into a log archive, 
    // saves the localized message information for the exported log archive,
    // and clears the log.
    //

    public static class LogManagement
    {

        public static void Main(string[] args)
        {
            int exitCode = 0;
            String logPath = "Application";
            String query = "*/System[Level <= 3 and Level >= 1]"; // XPath selecting all events of level warning or higher.
            String targetFile = Environment.ExpandEnvironmentVariables("%USERPROFILE%\\export.evtx");
            String targetFileWithMessages = Environment.ExpandEnvironmentVariables("%USERPROFILE%\\exportWithMessages.evtx");

            try 
            {
                // 
                // Parse the command line.
                //
                if (args.Length > 0) 
                {
                    if (args[0] == "/?" || args[0] == "-?")
                    {
                        Console.WriteLine("Usage: LogManagement [<logname> [<exportFile> [<exportFileWithMessages>]]]\n" +
                                          "<logname> is the name of an existing event log.\n" +
                                          "When <logname> is not specified, Application is assumed.\n" +
                                          "EXAMPLE: LogManagement Microsoft-Windows-TaskScheduler/Operational archive.evtx archiveWithMessages.evtx\n");
                        Environment.Exit(0);
                    }
                    else
                    {
                        logPath = args[0];
                        if (args.Length > 1)
                        {
                            targetFile = args[1];
                        }
                        if (args.Length > 2)
                        {
                            targetFileWithMessages = args[2];
                        }                        
                    }
                }
                
                //
                // Get log information.
                //                
                EventLogSession session = new EventLogSession();
                EventLogInformation logInfo = session.GetLogInformation(logPath, PathType.LogName);
                Console.WriteLine("The {0} log contains {1} events.", logPath, logInfo.RecordCount );

                //
                // Export selected events from a log to a file.
                //
                if (File.Exists(targetFile))
                {
                    Console.WriteLine("Could not export log {0}: file {1} already exists", logPath, targetFile);
                    Environment.Exit(1);
                }
                else
                {
                    session.ExportLog(logPath, PathType.LogName, query, targetFile, true);
                    Console.WriteLine("Selected events from the {0} log have been exported to file {1}.", logPath, targetFile);
                }

                //
                // Capture localized event information so that the exported log can be viewed on 
                // systems that might not have some of the event providers installed.
                //
                if (File.Exists(targetFileWithMessages))
                {
                    Console.WriteLine("Could not archive log {0}: file {1} already exists", logPath, targetFileWithMessages);
                    Environment.Exit(1);
                }
                else
                {
                    session.ExportLogAndMessages(logPath, PathType.LogName, query, targetFileWithMessages, true, CultureInfo.CurrentCulture);
                    Console.WriteLine("The export file {0} has been localized into {1} for archiving.", targetFileWithMessages, CultureInfo.CurrentCulture.DisplayName);
                }

                //
                // Clear the log.
                //                
                session.ClearLog(logPath);                
                Console.WriteLine("The {0} log has been cleared.", logPath );

            }
            catch (UnauthorizedAccessException e)
            {                
                Console.WriteLine("You do not have the correct permissions. " +
                                  "Try re-running the sample with administrator privileges.\n" + e.ToString());
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
