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

namespace Microsoft.Samples.EventLog.Subscribe
{
    //
    // The SubscribeEvents class demonstrates some of the subscription capabilities of Windows Event Log.  
    // This sample allows the user to subscribe to a given channel with an optional query
    // filter and displays the events written to the log inside a callback method.
    //
    public static class SubscribeEvents
    {
        static void PrintUsage()
        {
            Console.WriteLine(
                "\nUsage:\n\n" + 
                "   Subscribe PATH [/OPTION:VALUE [/OPTION:VALUE] ...]\n" +
                "   PATH is the name of an existing event log.\n" +
                "\nOptions:\n\n" +
                "   /query(q):VALUE                 Filters the events to subscribe with the XPath expression specified by VALUE.  Default is to read all events.\n" +
                "   /structuredquery(sq):VALUE      Filters the events to subscribe with the structured query from the xml file specified by VALUE.  Cannot be used with /query.\n" +
                "\nEXAMPLE:\n\n Subscribe SYSTEM  /sq:StructuredQuery.xml"
                );
        }

        public static void Main(string[] args)
        {
            int exitCode = 0;
            IntPtr session = IntPtr.Zero;
            string channelpath;
            string query = null;
            EventLogWatcher watcher = null;

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

                channelpath = args[0];
                char[] delimiters = { ':' };

                for (int i = 1; i < args.Length; i++)
                {
                    String option = args[i].Substring(1);
                    String[] words = option.Split(delimiters, 2);
                    words[0] = words[0].ToLower(CultureInfo.InvariantCulture);

                    switch (words[0])
                    {
                        case "query":
                        case "q":
                            if (query != null)
                            {
                                Console.WriteLine("Options '/query' and '/structuredquery' cannot both be specified.");
                                PrintUsage();
                                Environment.Exit(1);
                            }
                            query = words[1];
                            break;

                        case "structuredquery":
                        case "sq":
                            if (query != null)
                            {
                                Console.WriteLine("Options '/query' and '/structuredquery' cannot both be specified.");
                                PrintUsage();
                                Environment.Exit(1);
                            }

                            using (StreamReader sr = new StreamReader(words[1]))
                            {
                                String line;
                                while ((line = sr.ReadLine()) != null)
                                {
                                    query += line;
                                }
                            }
                            break;

                        default:
                            Console.WriteLine("Unrecognized parameter option: {0}.", words[0]);
                            PrintUsage();
                            Environment.Exit(1);
                            break;
                    }
                }

                //
                // Initialize the query string to retrieve all future events.
                //
                if (query == null)
                {
                    query = "*";
                }

                // 
                // Subscribe to the event log and start watching for new events.
                //
                EventLogQuery subscriptionQuery = new EventLogQuery(channelpath, PathType.LogName, query);               

                watcher = new EventLogWatcher(subscriptionQuery);
                watcher.EventRecordWritten += new EventHandler<EventRecordWrittenEventArgs>(DisplayEventCallback);               
                watcher.Enabled = true;

                // 
                // Continue watching until user stops the subscription.
                //
                Console.WriteLine("\nPress <enter> to stop subscription.\n");
                Console.Read();
            }
            catch (UnauthorizedAccessException e)
            {
                Console.WriteLine("You do not have the correct permissions. " +
                                  "Try re-running the sample with administrator privileges.\n" + e.ToString());
                exitCode = 1;
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
                exitCode = 1;
            }
            finally
            {  
                //
                // Stop the event subscription.
                //
                if (watcher != null)
                {
                    watcher.Enabled = false;
                    watcher.Dispose();
                }
            }
            Environment.Exit(exitCode);
        }
        
        //
        // Subscription callback displaying the delivered event
        // or an exception that occurred while retrieving the event.
        //
        private static void DisplayEventCallback(object value, EventRecordWrittenEventArgs callbackArg)
        {
            // Make sure there was no error reading the event.
            if (callbackArg.EventRecord != null)
            {
                Console.WriteLine("Received an event from the subscription:\n{0}\nFull event content in Xml:\n{1}\n\n", 
                                  callbackArg.EventRecord.FormatDescription(), 
                                  callbackArg.EventRecord.ToXml());
            }
            else
            {
                Console.WriteLine("**Subscription encountered an error: \n{0}", 
                                  callbackArg.EventException.ToString());
            }
        }
    }
}
