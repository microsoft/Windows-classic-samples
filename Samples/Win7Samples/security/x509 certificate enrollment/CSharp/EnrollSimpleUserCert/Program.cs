//---------------------------------------------------------------------
//  This file is part of the Microsoft .NET Framework SDK Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
// 
//This source code is intended only as a supplement to Microsoft
//Development Tools and/or on-line documentation.  See these other
//materials for detailed information regarding Microsoft code samples.
// 
//THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//---------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Text;

namespace EnrollSimpleUserCert
{
    class Program
    {
        static void Usage()
        {
            Console.WriteLine("Usage:");
            Console.Write("EnrollSimpleUserCert <Template> ");
            Console.WriteLine("<SubjectName> <FriendlyName>");
            Console.Write("Example: EnrollSimpleUserCert ");
            Console.WriteLine("User \"cn=MyCert\" \"My Cert\"");
        }

        static void Main(string[] args)
        {
            if (args.Length != 3)
            {
                Usage();
                Environment.Exit(1);
            }

            EnrollCertificate.EnrollCert(args[0], args[1], args[2]);
        }
    }
}
