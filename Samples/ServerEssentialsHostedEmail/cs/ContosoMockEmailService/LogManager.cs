//
//  <copyright file="LogManager.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.IO;
using System.IO.Pipes;
using System.Globalization;

namespace Contoso.EmailService
{
    public sealed class LogManager : IDisposable
    {
        private NamedPipeClientStream clientStream = null;
        private StreamWriter streamWriter = null;

        public const string ConnectionName = "Contoso.HostedEmail.LogManager_3983E9AC-B6D1-4A2A-881C-4B1CEFCA5266";

        private static object singletonLock = new object();
        private static LogManager logManager = null;
        public static LogManager SingleInstance
        {
            get
            {
                if (logManager == null)
                {
                    lock (singletonLock)
                    {
                        if (logManager == null) logManager = new LogManager();
                    }
                }
                return logManager;
            }
        }

        private LogManager()
        {
            clientStream = new NamedPipeClientStream(".", ConnectionName, PipeDirection.Out);
            streamWriter = new StreamWriter(clientStream);
        }

        public void Log(string format, params object[] values)
        {
            try
            {
                if (!clientStream.IsConnected)
                {
                    clientStream.Connect(0);
                }
                if (clientStream.IsConnected)
                {
                    string timeStamp = string.Format(CultureInfo.InvariantCulture, "[{0}]: ", DateTime.Now);
                    streamWriter.WriteLine(timeStamp + format, values);
                    streamWriter.Flush();
                }
                else
                {
                    // NamedPipe server is not ready, do nothing
                }
            }
            catch (TimeoutException)
            {
                // NamedPipe server is not ready, do nothing
            }
            catch (IOException)
            {
                // Do nothing
            }
            catch (ObjectDisposedException)
            {
                // NamedPipe server is disposed, do nothing
            }
        }

        public void Dispose()
        {

            if (streamWriter != null)
            {
                streamWriter.Close();
                streamWriter = null;
            }
            else if (clientStream != null)
            {
                clientStream.Close();
                clientStream = null;
            }
        }
    }
}