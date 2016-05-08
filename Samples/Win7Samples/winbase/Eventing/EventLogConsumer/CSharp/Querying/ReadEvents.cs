// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


using System;
using System.Globalization;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Diagnostics.Eventing.Reader;

[assembly: CLSCompliant(true)]
[assembly: ComVisible(false)]

namespace Microsoft.Samples.EventLog.Querying
{   
    //
    // The ReadEvents class demonstrates the use of querying and rendering capabilities 
    // of Windows Event Log.  This sample queries a log or log file and displays the
    // selected events in the Xml format fully or just the provider name and description.
    // 
    // This sample is similar to wevtutil.exe command-line utility.
    //
    // You may need to run this code with administrator privileges 
    // to be able to view events stored in certain logs (depending on the log's security settings).
    //
    public static class ReadEvents
    {
        static void PrintUsage()
        {
            Console.WriteLine(
                "\nUsage:\n\n" + 
                "   ReadEvents PATH [/OPTION:VALUE [/OPTION:VALUE] ...]\n\n" +
                "   PATH is the name of an existing event log or log file to read.  For a log file, the /lf option must be specified.\n" +
                "\nOptions:\n\n" +
                "   /logfile(lf)            Indicates that PATH is a log file instead of a log name.\n" +
                "   /query(q):VALUE         Filters the events to read with the XPath expression specified by VALUE.  Default is to read all events.\n" +
                "   /reversedirection(rd)   Reads events from the end of the log to the beginning (instead of the default beginning to end).\n" +
                "   /count(c):VALUE         Reads only the number of events specified by VALUE.\n" +
                "   /format(f):VALUE        Displays the events in the format specified by VALUE.  VALUE can be XML or Text (which simply displays the event source and description).\n" +
                "EXAMPLE: ReadEvents Application /q:\"*[System/Provider[@Name=\'Windows Error Reporting\']]\" /c:5 /f:Text /rd\n" +
                "(Displays 5 most recent error events.)"
                );
        }

        public static void Main(string[] args)
        {
            int exitCode = 0;
            String path = "";
            String query = "*";
            bool reverseDirection = false;
            UInt32 count = UInt32.MaxValue;
            String format = "xml";
            PathType pathType =  PathType.LogName;

            try 
            {
                // 
                // Parse the command line.
                //
                if (args.Length == 0) 
                {
                    Console.WriteLine("Error: No parameters provided.");
                    PrintUsage();
                    Environment.Exit(1);
                }
                if (args[0] == "/?" || args[0] == "-?")
                {
                    PrintUsage();
                    Environment.Exit(1);
                }

                path = args[0];
                char[] delimiters = {':'};

                for (int i=1 ; i < args.Length; i++)
                {
                    String option = args[i].Substring(1);
                    String[] words = option.Split(delimiters, 2);
                    words[0] = words[0].ToLower(CultureInfo.InvariantCulture);

                    switch (words[0])
                    {
                        case "logfile":
                        case "lf":
                            pathType = PathType.FilePath;                            
                            break;

                        case "query":
                        case "q":
                            query = words[1];
                            break;

                        case "reversedirection":
                        case "rd":
                            reverseDirection = true;
                            break;

                        case "count":
                        case "c":
                            count = Convert.ToUInt32(words[1], CultureInfo.InvariantCulture);
                            break;

                        case "format":
                        case "f":
                            format = words[1].ToLower(CultureInfo.InvariantCulture);
                            if (format != "text" && format != "xml")
                            {
                                throw (new Exception(String.Format(CultureInfo.InvariantCulture, "Unrecognized format option: {0}.", format)));
                            }
                            break;

                        default:                            
                            throw (new Exception( String.Format(CultureInfo.InvariantCulture, "Unrecognized parameter option: {0}.", words[0]) ));
                    }
                }

                // 
                // Query the event log.
                //
                EventLogQuery queryObj = new EventLogQuery(path, pathType, query);
                queryObj.ReverseDirection = reverseDirection;
                queryObj.TolerateQueryErrors = true;  // Continue to read events even if an error occurs.

                EventLogReader reader = new EventLogReader(queryObj);
                TimeSpan timeout = new TimeSpan(0, 0, 5);

                EventRecord eventRecord = null;
                UInt32 eventsRead = 0;
                while (((eventRecord = reader.ReadEvent(timeout)) != null) && (eventsRead++ < count))
                {
                    if (format == "xml")
                    {
                        Console.WriteLine("\n" + eventRecord.ToXml());
                    }
                    else if (format == "text")
                    {
                        Console.WriteLine("Source:\t\t" + eventRecord.ProviderName);
                        Console.WriteLine("Description:\t" + eventRecord.FormatDescription() + "\n");
                    }
                }
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
