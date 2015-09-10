//
//  <copyright file="ProviderCore.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BasicProvider1
{
    public class ProviderCore
    {
        private HashSet<ProviderService> m_connections = new HashSet<ProviderService>();

        private object m_syncRoot = new object();

        public void SendOperation(string name, string text)
        {
            lock (m_syncRoot)
            {
                foreach (var connection in m_connections)
                {
                    connection.SendToClient(name, text);
                }
            }
        } 

        public void AddConnection(ProviderService connection)
        {
            lock (m_syncRoot)
            {
                m_connections.Add(connection);
            }

        }

        public void Disconnect(ProviderService connection)
        {
            lock (m_syncRoot)
            {
                m_connections.Remove(connection);
            }
        }
    }
}
