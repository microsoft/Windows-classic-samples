// <copyright file="Serialization02.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

using System;
using System.IO;
using System.Management.Automation;
using System.Management.Automation.Runspaces;

namespace Microsoft.Samples.PowerShell.Serialization
{
    using PowerShell = System.Management.Automation.PowerShell;

    class Serialization02
    {
        /// <summary>
        /// By default serialization preserves all public properties of an object.
        ///
        /// This sample looks at an existing .NET class and shows how to make sure that
        /// information from instance of this class is preserved across serialization/deserialization
        /// when the information is not available in public properties of the class
        /// </summary>
        static void Main()
        {
            string typesPs1XmlPath = Path.Combine(Environment.CurrentDirectory, "Serialization02.types.ps1xml");
            if (!File.Exists(typesPs1XmlPath))
            {
                Console.WriteLine("Building the project in Visual Studio should have created a types.ps1xml file at the following path:");
                Console.WriteLine("{0}", typesPs1XmlPath);
                Console.WriteLine();
                Console.WriteLine("Cannot continue without this file being present.");
                return;
            }

            // Create a default InitialSessionState 
            InitialSessionState iss = InitialSessionState.CreateDefault();
            // Add our types.ps1xml file to the InitialSessionState
            // (one alternative would be to associate the file with a module or with a snap-in)
            iss.Types.Add(new SessionStateTypeEntry(typesPs1XmlPath));

            //
            // Demonstrate the effects of the types.ps1xml and DeserializingTypeConverter
            //

            using (Runspace myRunspace = RunspaceFactory.CreateRunspace(iss))
            {
                myRunspace.Open();

                //
                // Demonstrate that the deserializing an exception results in a live object
                //
                using (PowerShell powershell = PowerShell.Create())
                {
                    powershell.Runspace = myRunspace;
                    powershell.AddScript(@"
                        # Get an System.Drawing.Point object
                        Add-Type -AssemblyName System.Drawing
                        $point = New-Object System.Drawing.Point 12,34
                        
                        # Verify that the extra property is hidden by default
                        Write-Output 'Below are the results of running $point | Format-List *   :'
                        $point | Format-List * | Out-String
                        Write-Output '----------------------------------------'

                        # Serialize the object
                        $point | Export-CliXml .\Serialization02.xml

                        # Deserialize the object
                        $deserializedPoint = Import-CliXml .\Serialization02.xml

                        # Verify that the extra property got serialized
                        Write-Output 'Below are the results of running $deserializedPoint | Get-Member   :'
                        $deserializedPoint | Get-Member | Out-String
                        Write-Output '----------------------------------------'
                        ");
                    foreach (string s in powershell.Invoke<string>())
                    {
                        System.Console.WriteLine(s);
                    }
                }

                // Close the runspace and release any resources.
                myRunspace.Close();
            }

            System.Console.WriteLine("Hit any key to exit...");
            System.Console.ReadKey();
        }
    }
}

