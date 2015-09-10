// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.ResourcePools
{
    using System;
    using System.Globalization;
    using System.Management;
    using System.Reflection;
 
    class ResourcePools
    {
        /// <summary>
        /// Entry point of the program.
        /// </summary>
        /// <param name="args">Command line arguments.</param>
        static void 
        Main(
            string[] args)
        {
            if (args.Length > 0)
            {
                if (string.Equals(args[0], "EnumerateSupportedResources", StringComparison.OrdinalIgnoreCase) &&
                    args.Length == 1)
                {
                    ResourceUtilities.EnumerateSupportedResources();
                }
                else if (string.Equals(args[0], "CreatePool", StringComparison.OrdinalIgnoreCase) &&
                    args.Length == 6)
                {
                    MsvmResourcePoolConfigurationService.CreatePool(
                        args[1],
                        args[2],
                        args[3],
                        args[4],
                        args[5]);
                }
                else if (string.Equals(args[0], "DisplayPoolResources", StringComparison.OrdinalIgnoreCase) &&
                    args.Length == 3)
                {
                    MsvmResourceAllocationSettingData.DisplayPoolResourceAllocationSettingData(
                        args[1],
                        args[2]);
                }
                else if (string.Equals(args[0], "ModifyPoolResources", StringComparison.OrdinalIgnoreCase) &&
                    args.Length == 5)
                {
                    MsvmResourcePoolConfigurationService.ModifyPoolResources(
                        args[1],
                        args[2],
                        args[3],
                        args[4]);
                }
                else if (string.Equals(args[0], "DisplayPoolSettings", StringComparison.OrdinalIgnoreCase) &&
                    args.Length == 3)
                {
                    MsvmResourcePoolSettingData.DisplayPoolResourcePoolSettingData(
                        args[1],
                        args[2]);
                }
                else if (string.Equals(args[0], "ModifyPoolSettings", StringComparison.OrdinalIgnoreCase) &&
                    args.Length == 5)
                {
                    MsvmResourcePoolConfigurationService.ModifyPoolSettings(
                        args[1],
                        args[2],
                        args[3],
                        args[4]);
                }
                else if (string.Equals(args[0], "DeletePool", StringComparison.OrdinalIgnoreCase) &&
                    args.Length == 3)
                {
                    MsvmResourcePoolConfigurationService.DeletePool(
                        args[1],
                        args[2]);
                }
                else if (string.Equals(args[0], "DisplayPool", StringComparison.OrdinalIgnoreCase) &&
                    args.Length == 3)
                {
                    MsvmResourcePool.DisplayPoolVerbose(
                        args[1],
                        args[2]);
                }
                else if (string.Equals(args[0], "DisplayChildPools", StringComparison.OrdinalIgnoreCase) &&
                    args.Length == 3)
                {
                    MsvmResourcePool.DisplayChildPools(
                        args[1],
                        args[2]);
                }
                else if (string.Equals(args[0], "DisplayParentPools", StringComparison.OrdinalIgnoreCase) &&
                    args.Length == 3)
                {
                    MsvmResourcePool.DisplayParentPools(
                        args[1],
                        args[2]);
                }
                else if (string.Equals(args[0], "DisplayAllocationCapabilities", StringComparison.OrdinalIgnoreCase) &&
                    args.Length == 3)
                {
                    MsvmResourceAllocationSettingData.DisplayValidResourceAllocationSettingDataSettings(
                        args[1],
                        args[2]);
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
            Console.WriteLine("   EnumerateSupportedResources");
            Console.WriteLine("   CreatePool resourceName poolId poolName parentPoolIds parentPoolHostResources");
            Console.WriteLine("   DisplayPoolResources resourceName poolId");
            Console.WriteLine("   ModifyPoolResources resourceName poolId newParentPoolIds newParentPoolHostResources");
            Console.WriteLine("   DisplayPoolSettings resourceName poolId");
            Console.WriteLine("   ModifyPoolSettings resourceName poolId newPoolId newPoolName");
            Console.WriteLine("   DisplayPool resourceName poolId");
            Console.WriteLine("   DisplayChildPools resourceName poolId");
            Console.WriteLine("   DisplayParentPools resourceName poolId");
            Console.WriteLine("   DeletePool resourceName poolId");
            Console.WriteLine("   DisplayAllocationCapabilities resourceName poolId");

            Console.WriteLine("\nExamples:\n");
            Console.WriteLine("   {0} CreatePool VHD \"Pool Id 1\" \"Pool Name 1\" [p]\"\"[p] " +
                                      "[p][h]c:\\root\\base1[h][p]", moduleName);
            Console.WriteLine("   {0} CreatePool VHD \"Pool Id 2\" \"Pool Name 2\" [p]\"\"[p] " +
                                      "[p][h]c:\\root\\base2[h][p]", moduleName);
            Console.WriteLine("   {0} CreatePool VHD \"Pool Id 3\" \"Pool Name 3\" " + 
                                      "[p]\"Pool Id 1\"[p][p]\"Pool Id 2\"[p] " +
                                      "[p][h]c:\\root\\base1\\dira[h][h]c:\\root\\base1\\dirb[h][p][p][h]c:\\root\\base2[h][p]", moduleName);
            Console.WriteLine("   {0} DisplayParentPools VHD \"Pool Id 3\"", moduleName);
            Console.WriteLine("   {0} ModifyPoolSettings VHD \"Pool Id 3\" \"New Pool Id 3+\" \"New Pool Name 3+\"", moduleName);
            Console.WriteLine("Note:\nParent pools are specified as an array of pool Ids delimited " +
                               "by [p]. In the first example, we specify the primordial pool (\"\"). ");
            Console.WriteLine("In the third example, we specify two parent pools (Pool Id 1 and Pool Id 2) " + 
                              "as parent pools (multiple parents is supported only for the VHD resource).");

            Console.WriteLine("\nParent pool host resources are specified as an array of strings for " +
                              "each parent pool. Each set of parent resources is delimited by [p]; " +
                              "each seperate host resource is delimited by a [h]. In the third example, " +
                              "we assign c:\\root\\base1\\dira and c:\\root\\base1\\dirb from the first " +
                              "first parent pool (Pool Id 1) and c:\\root\\base2 from the second parent pool.");
        }
    }
}
