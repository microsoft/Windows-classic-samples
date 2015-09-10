//
//  <copyright file="Program.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.IO;
using System.IO.Pipes;

namespace Contoso.EmailService.LogMonitor
{
    class Program
    {
        static void Main(string[] args)
        {
            // Create a name pipe
            NamedPipeServerStream pipeStream = null;
            try
            {
                pipeStream = new NamedPipeServerStream(LogManager.ConnectionName, PipeDirection.InOut, 1, PipeTransmissionMode.Message);
                Console.WriteLine("Log Monitor started: " + pipeStream.GetHashCode());

                // Wait for a connection
                pipeStream.WaitForConnection();
                Console.WriteLine("Connection established");

                using (StreamReader sr = new StreamReader(pipeStream))
                {
                    pipeStream = null;

                    string temp;
                    // We read a line from the pipe and print it together with the current time
                    while ((temp = sr.ReadLine()) != null)
                    {
                        Console.WriteLine(temp);
                    }
                }
            }
            finally
            {
                if (pipeStream != null)
                {
                    pipeStream.Dispose();
                }
            }

            Console.WriteLine("Connection lost");
            Console.WriteLine("Press <ENTER> to quit");
            Console.ReadLine();
        }
    }
}
