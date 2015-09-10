// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Replica
{
    using System;
    using System.Globalization;
    using System.Reflection;

    class Program
    {
        /// <summary>
        /// Entry point of the program.
        /// </summary>
        /// <param name="args">Command line arguments.</param>
        static void
        Main(
            string[] args)
        {
            if (args.Length == 2)
            {
                if (string.Equals(args[0], "ModifyReplicationService", 
                    StringComparison.OrdinalIgnoreCase))
                {
                    bool enbleReplicationService = !(Int16.Parse(args[1], CultureInfo.CurrentCulture) == 0);
                    ModifyReplicationServiceSettings.ModifyServiceSettings(enbleReplicationService);
                }
                else if (string.Equals(args[0], "StartReplication", StringComparison.OrdinalIgnoreCase))
                {
                    ManageReplication.StartReplication(args[1]);
                }
                else if (string.Equals(args[0], "TestReplicaSystem", StringComparison.OrdinalIgnoreCase))
                {
                    ManageReplication.TestReplicaSystem(args[1]);
                }
                else if (string.Equals(args[0], "InitiateFailover", StringComparison.OrdinalIgnoreCase))
                {
                    ManageReplication.InitiateFailover(args[1]);
                }
                else if (string.Equals(args[0], "ReverseReplicationRelationship", 
                    StringComparison.OrdinalIgnoreCase))
                {
                    ManageReplication.ReverseReplicationRelationship(args[1]);
                }
                else if (string.Equals(args[0], "RemoveAuthorizationEntry",
                    StringComparison.OrdinalIgnoreCase))
                {
                    ModifyReplicationServiceSettings.RemoveAuthorizationEntry(args[1]);
                }
                else
                {
                    ShowUsage();
                }
            }
            else if (args.Length == 3)
            {
                if (string.Equals(args[0], "SetAuthorizationEntry", StringComparison.OrdinalIgnoreCase))
                {
                    ModifyReplicationServiceSettings.SetAuthorizationEntry(args[1], args[2]);
                }
                else if (string.Equals(args[0], "CreateReplicationRelationship", 
                    StringComparison.OrdinalIgnoreCase))
                {
                    ManageReplication.CreateReplicationRelationship(args[1], args[2]);
                }
                else if (string.Equals(args[0], "RemoveReplicationRelationshipEx",
                    StringComparison.OrdinalIgnoreCase))
                {
                    UInt16 relationshipType = UInt16.Parse(args[2], CultureInfo.CurrentCulture);
                    ManageReplication.RemoveReplicationRelationshipEx(args[1], relationshipType);
                }
                else if (string.Equals(args[0], "GetReplicationRelationshipInfo",
                    StringComparison.OrdinalIgnoreCase))
                {
                    UInt16 relationshipType = UInt16.Parse(args[2], CultureInfo.CurrentCulture);
                    ManageReplication.GetReplicationRelationshipInfo(args[1], relationshipType);
                }
                else
                {
                    ShowUsage();
                }
            }
            else if (args.Length == 4)
            {
                if (string.Equals(args[0], "AddAuthorizationEntry", StringComparison.OrdinalIgnoreCase))
                {
                    ModifyReplicationServiceSettings.AddAuthorizationEntry(args[1], args[2], args[3]);
                }
                else if (string.Equals(args[0], "RequestReplicationStateChangeEx",
                    StringComparison.OrdinalIgnoreCase))
                {
                    UInt16 requestedState = UInt16.Parse(args[2], CultureInfo.CurrentCulture);
                    UInt16 relationshipType = UInt16.Parse(args[3], CultureInfo.CurrentCulture);
                    ManageReplication.RequestReplicationStateChangeEx(args[1], requestedState, relationshipType);
                }
                else
                {
                    ShowUsage();
                }
            }
            else
            {
                ShowUsage();
            }
        }

        /// <summary>
        /// Displays the command line usage for the program.
        /// </summary>
        static void
        ShowUsage()
        {
            string moduleName = Assembly.GetExecutingAssembly().GetModules()[0].Name;

            Console.WriteLine("Usage:\t{0} <SampleName> <Arguments>\n", moduleName);

            Console.WriteLine("Supported SampleNames and Arguments:\n");
            Console.WriteLine("   ModifyReplicationService <Enable/Disable>");
            Console.WriteLine("   AddAuthorizationEntry <Fqdn> <TrustGroup> <ReplicaStoragePath>");
            Console.WriteLine("   RemoveAuthorizationEntry <Fqdn>");
            Console.WriteLine("   CreateReplicationRelationship <VmName> <ReplicaServer>");
            Console.WriteLine("   RemoveReplicationRelationshipEx <VmName> <RelationshipType (0/1)>");
            Console.WriteLine("   StartReplication <VmName>");
            Console.WriteLine("   TestReplicaSystem <VmName>");
            Console.WriteLine("   RequestReplicationStateChangeEx <VmName> <RequestedState> <RelationshipType (0/1)>");
            Console.WriteLine("   InitiateFailover <VmName>");
            Console.WriteLine("   SetAuthorizationEntry <VmName> <Fqdn>");
            Console.WriteLine("   ReverseReplicationRelationship <VmName>");
            Console.WriteLine("   GetReplicationRelationshipInfo <VmName> <RelationshipType (0/1)>\n");

            Console.WriteLine("Examples:\n");
            Console.WriteLine("   {0} ModifyReplicationService 1", moduleName);
            Console.WriteLine("   {0} AddAuthorizationEntry *.com AnyRequestIsAllowed C:\\ReplicaStorage", moduleName);
            Console.WriteLine("   {0} RemoveAuthorizationEntry *.com", moduleName);
            Console.WriteLine("   {0} CreateReplicationRelationship WIN8VM replica.contosa.com", moduleName);
            Console.WriteLine("   {0} RemoveReplicationRelationshipEx WIN8VM 0", moduleName);
            Console.WriteLine("   {0} StartReplication WIN8VM", moduleName);
            Console.WriteLine("   {0} TestReplicaSystem WIN8VM", moduleName);
            Console.WriteLine("   {0} RequestReplicationStateChangeEx WIN8VM 7 0", moduleName);
            Console.WriteLine("   {0} InitiateFailover WIN8VM", moduleName);
            Console.WriteLine("   {0} SetAuthorizationEntry WIN8VM replica.contosa.com", moduleName);
            Console.WriteLine("   {0} ReverseReplicationRelationship WIN8VM", moduleName);
            Console.WriteLine("   {0} GetReplicationRelationshipInfo WIN8VM 0\n", moduleName);
        }
    }
}
