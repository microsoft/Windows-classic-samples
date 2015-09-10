//
//  <copyright file="CredentialManager.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.WindowsServerSolutions.HostedEmail;

namespace Contoso.HostedEmail
{
    internal static class CredentialManager
    {
        private const string adminUserNameKey = "adminUserName";
        private const string adminPasswordKey = "adminPassword";
        // This is copied from ContosoHostedEmail.addin
        private const string addinId = "3983E9AC-B6D1-4A2A-881C-4B1CEFCA5266";
        // The following line is the result of executing the code 'Encoding.Unicode.GetBytes("P@ssw0rd");'
        static byte[] optionalEntropy = new byte[] { 80, 0, 64, 0, 115, 0, 115, 0, 119, 0, 48, 0, 114, 0, 100, 0 };
        static HostedEmailProtectedDataStore store = new HostedEmailProtectedDataStore(Guid.Parse(addinId), optionalEntropy);

        public static string AdminUserName
        {
            get
            {
                return store[adminUserNameKey];
            }
            set
            {
                store[adminUserNameKey] = value;
            }
        }

        public static string AdminPassword
        {
            get
            {
                return store[adminPasswordKey];
            }
            set
            {
                store[adminPasswordKey] = value;
            }
        }

        public static void ClearAll()
        {
            if (store != null) store.Destroy();
        }
    }
}
