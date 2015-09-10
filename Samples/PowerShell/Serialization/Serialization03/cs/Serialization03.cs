// <copyright file="Serialization03.cs" company="Microsoft Corporation">
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

    public class DeserializingTypeConverter : PSTypeConverter
    {
        #region Conversion / rehydration methods

        /// <summary>
        /// Determines if the converter can rehydrate <paramref name="sourceValue"/>
        /// </summary>
        /// <param name="sourceValue">The value to convert from (deserialized property bag)</param>
        /// <param name="destinationType">The type to convert to (artificial and ignored - this is always DeserializingTypeConverter class)</param>
        /// <returns>True if the converter can rehydrate the <paramref name="sourceValue"/> parameter</returns>
        public override bool CanConvertFrom(PSObject sourceValue, Type destinationType)
        {
            return sourceValue.TypeNames.Contains("Deserialized.System.ArgumentException");
        }

        /// <summary>
        /// Rehydrates the <paramref name="sourceValue"/> parameter
        /// </summary>
        /// <param name="sourceValue">The value to convert from (deserialized property bag)</param>
        /// <param name="destinationType">The type to convert to (artificial and ignored - this is always DeserializingTypeConverter class)</param>
        /// <param name="formatProvider" />
        /// <param name="ignoreCase" />
        /// <returns>the <paramref name="sourceValue"/> parameter rehydrated into a live object</returns>
        /// <exception cref="InvalidCastException">if no conversion was possible (deserializer is going to use the deserialized property bag instead of a live object)</exception>
        public override object ConvertFrom(PSObject sourceValue, Type destinationType, IFormatProvider formatProvider, bool ignoreCase)
        {
            // go through the types, from the most specific/derived, to the most broad/base type
            foreach (string deserializedTypeName in sourceValue.TypeNames)
            {
                // Get rid of the "Deserialized." prefix from the type names
                const string deserializedPrefix = "Deserialized.";
                string typeName = deserializedTypeName;
                if (typeName.StartsWith(deserializedPrefix, StringComparison.OrdinalIgnoreCase))
                {
                    typeName = typeName.Substring(deserializedPrefix.Length);
                }

                // Get the original type associated with the deserialized property bag
                Type originalType = Type.GetType(typeName, false /* throwOnError */);

                // originalType might be null in the following cases:
                // - the assembly containing the type is not loaded into the current AppDomain
                // - the typeName is artificially introduced by PowerShell's Extended Type System 
                //   (i.e. Selected.System.ArgumentOutOfRangeException)
                if (originalType == null)
                {
                    continue;
                }

                // Only handle the types that we know how to rehydrate
                if (!originalType.IsSubclassOf(typeof(System.ArgumentException)))
                {
                    continue;
                }

                // Get the exception message from one of deserialized properties.
                // It is ok to throw exceptions here - i.e. when
                // 1) the property doesn't exist or 
                // 2) the property value is of a wrong type
                // Type convertion in PowerShell engine will simply fail the conversion
                // if our type converter throws any exception.
                string exceptionMessage = (string)sourceValue.Properties["Message"].Value;

                // rehydrate the property bag and return the live object
                System.Console.WriteLine("Rehydrating an ArgumentException object");
                return Activator.CreateInstance(originalType, exceptionMessage);
            }

            // this property bag didn't get recognized by the loop above
            throw new InvalidCastException("Cannot rehydrate sourceValue");
        }

        #endregion Conversion / rehydration methods

        #region Methods from PSTypeConverter that are not needed and can be left out empty

        /// <summary>
        /// This method is not implemented - CanConvertFrom method is used instead.
        /// </summary>
        public override bool CanConvertTo(object sourceValue, Type destinationType)
        {
            return false;
        }

        /// <summary>
        /// This method is not implemented - ConvertFrom method is used instead.
        /// </summary>
        public override object ConvertTo(object sourceValue, Type destinationType, IFormatProvider formatProvider, bool ignoreCase)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// This method is not implemented - an overload taking a PSObject is implemented instead
        /// </summary>
        public override bool CanConvertFrom(object sourceValue, Type destinationType)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// This method is not implemented - an overload taking a PSObject is implemented instead
        /// </summary>
        public override bool CanConvertTo(PSObject sourceValue, Type destinationType)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// This method is not implemented - an overload taking a PSObject is implemented instead
        /// </summary>
        public override object ConvertFrom(object sourceValue, Type destinationType, IFormatProvider formatProvider, bool ignoreCase)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// This method is not implemented - an overload taking a PSObject is implemented instead
        /// </summary>
        public override object ConvertTo(PSObject sourceValue, Type destinationType, IFormatProvider formatProvider, bool ignoreCase)
        {
            throw new NotImplementedException();
        }

        #endregion Methods from PSTypeConverter that are not needed and can be left out empty
    }

    class Serialization03
    {
        /// <summary>
        /// This sample looks at an existing .NET class and shows how to make sure that
        /// instances of this class and of derived classes are deserialized (rehydrated)
        /// into live .NET objects.
        /// </summary>
        static void Main()
        {
            string typesPs1XmlPath = Path.Combine(Environment.CurrentDirectory, "Serialization03.types.ps1xml");
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
                        # Get an object derived from System.ArgumentException
                        $exception = New-Object System.ArgumentOutOfRangeException 'ParameterName'
                        
                        # Serialize the object to the disk
                        # (remoting would also serialize the object before sending it to the remote session)
                        $exception | Export-CliXml .\Serialization03.xml

                        # Deserialize the object
                        $deserializedException = Import-CliXml .\Serialization03.xml

                        # Verify that the object is rehydrated
                        Write-Output ('Deserialized object is of type: ' + $deserializedException.GetType().FullName)
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

