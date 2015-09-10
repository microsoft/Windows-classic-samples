// <copyright file="Serialization01.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace Microsoft.Samples.PowerShell.Serialization
{
    using System;
    using System.IO;
    using System.Management.Automation;
    using System.Management.Automation.Runspaces;
    using PowerShell = System.Management.Automation.PowerShell;

    /// <summary>
    /// This class contains the Main entry point for this host application.
    /// </summary>
    internal class Serialization01
    {
        /// <summary>
        /// This sample looks at an existing .NET class and shows how to make sure that
        /// information from selected public properties of this class is preserved across
        /// serialization/deserialization.
        /// </summary>
        private static void Main()
        {
            string typesPs1XmlPath = Path.Combine(Environment.CurrentDirectory, "Serialization01.types.ps1xml");
            if (!File.Exists(typesPs1XmlPath))
            {
                Console.WriteLine("Building the project in Visual Studio should have created a types.ps1xml file at the following path:");
                Console.WriteLine("{0}", typesPs1XmlPath);
                Console.WriteLine();
                Console.WriteLine("Cannot continue without this file being present.");
                return;
            }

            // Demonstrate the effects of the types.ps1xml and DeserializingTypeConverter
            using (Runspace myRunspace = RunspaceFactory.CreateRunspace(InitialSessionState.CreateDefault()))
            {
                myRunspace.Open();

                // Demonstrate that the deserializing an exception results in a live object
                using (PowerShell powershell = PowerShell.Create())
                {
                    powershell.Runspace = myRunspace;
                    powershell.AddScript(@"
                        # Get an System.Drawing.Rectangle object
                        Add-Type -AssemblyName System.Drawing
                        $rectangle = New-Object System.Drawing.Rectangle 1,2,3,4
                        
                        # Without extra type.ps1xml Rectangle.Location property might get serialized as a string
                        Write-Output 'Below are serialization results without the extra types.ps1xml declarations: '
                        Export-CliXml -Input $rectangle -Depth 1 -Path Serialization01.xml
                        $deserializedRectangle = Import-CliXml Serialization01.xml
                        Write-Output ('$deserializedRectangle.Location is a ' + $deserializedRectangle.Location.PSTypeNames[0])
                        Write-Output '----------------------------------------'

                        # Update the system with the extra types.ps1xml declarations
                        Update-TypeData .\Serialization01.types.ps1xml

                        # After adding extra types.ps1xml declarations 
                        # chosen properties of Rectangle.Location will always get serialized
                        Write-Output 'Below are serialization results after adding the extra types.ps1xml declarations: '
                        Export-CliXml -Input $rectangle -Depth 1 -Path Serialization01.xml
                        $deserializedRectangle = Import-CliXml Serialization01.xml
                        Write-Output ('$deserializedRectangle.Location is a ' + $deserializedRectangle.Location.PSTypeNames[0])
                        if ($deserializedRectangle.Location.IsEmpty -eq $null)
                        {
                            Write-Output '$deserializedRectangle.Location.IsEmpty didnt get serialized'
                        }
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

