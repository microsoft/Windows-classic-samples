// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Storage;

/*++

    namespace Microsoft.Samples.FSRM.ManagedGetEnumProperties

Description:

    This namespace contains the program that displays FSRM properties.

--*/
namespace Microsoft.Samples.FSRM.ManagedGetEnumProperties
{

    /*++

        class Program

    Description:

        This is the only class in the namespace. It is responsible for displaying FSRM properties.

    --*/
    class Program
    {
        /*++

            Routine DisplayPropertyDefinition

        Description:

            This routine prints an FSRM property definitions
            
        Arguments:

            PropertyDefinition - An IFsrmPropertyDefinition object to pring
            
        Return value:

            void
            
        Notes:
            

        --*/
        static void DisplayPropertyDefinition(
            IFsrmPropertyDefinition PropertyDefinition
            )
        {
            string description;
            string name;
            Array possibleValues;
            Array valueDescriptions;

            _FsrmPropertyDefinitionType type;

            // get the members of the PropertyDefinition
            description = PropertyDefinition.Description;
            name = PropertyDefinition.Name;
            type = PropertyDefinition.Type;
            possibleValues = PropertyDefinition.PossibleValues;
            valueDescriptions = PropertyDefinition.ValueDescriptions;

            //getting all the values will have succeeded
            //print all values

            Console.WriteLine( "\tName:\t\t{0}", name );
            Console.WriteLine( "\tDefinition:\t{0}", description );
            Console.Write( "\tType:\t\t" );
            switch (type) {
                case _FsrmPropertyDefinitionType.FsrmPropertyDefinitionType_Unknown:
                    Console.Write( "Unknown\n" );
                    break;

                case _FsrmPropertyDefinitionType.FsrmPropertyDefinitionType_OrderedList:
                    Console.Write( "Ordered List\n" );

                    //For an ordered list, print all possible values
                    for (int i = 0; i < possibleValues.Length && i < valueDescriptions.Length; ++i) {
                        Console.Write( "\t\t{0} Value:\t {1}\n", i + 1, possibleValues.GetValue( i ) );
                        Console.Write( "\t\t{0} Description:\t {1}\n", i + 1, valueDescriptions.GetValue( i ) );
                    }
                    break;

                case _FsrmPropertyDefinitionType.FsrmPropertyDefinitionType_MultiChoiceList:
                    Console.Write( "Multichoice List\n" );

                    //For a multichoice property, print all possible values
                    for (int i = 0; i < possibleValues.Length && i < valueDescriptions.Length; ++i) {
                        Console.Write( "\t\t{0} Value:\t {1}\n", i + 1, possibleValues.GetValue( i ) );
                        Console.Write( "\t\t{0} Description:\t {1}\n", i + 1, valueDescriptions.GetValue( i ) );
                    }
                    break;

                case _FsrmPropertyDefinitionType.FsrmPropertyDefinitionType_Int:
                    Console.Write( "Int\n" );
                    break;

                case _FsrmPropertyDefinitionType.FsrmPropertyDefinitionType_Bool:
                    Console.Write( "Bool\n" );
                    break;

                case _FsrmPropertyDefinitionType.FsrmPropertyDefinitionType_Date:
                    Console.Write( "Date\n" );
                    break;

                case _FsrmPropertyDefinitionType.FsrmPropertyDefinitionType_MultiString:
                    Console.Write( "Multistring\n" );
                    break;

                case _FsrmPropertyDefinitionType.FsrmPropertyDefinitionType_String:
                    Console.Write( "String\n" );
                    break;

                default:
                    Console.Write( "Error type {0} not defined\n", type );
                    break;

            }
        }


        /*++

            Routine EnumerateProperties

        Description:

            This routine enumerates FSRM classification properties
            
        Arguments:

            FsrmManager - An IFsrmClassificationManager to retrieve the properties from.
            
        Return value:

            void
            
        Notes:
            

        --*/
        static void EnumerateProperties(
            IFsrmClassificationManager FsrmManager )
        {
            IFsrmCollection fsrmCollection;

            // Get the collection of property definition
            fsrmCollection = FsrmManager.EnumPropertyDefinitions( _FsrmEnumOptions.FsrmEnumOptions_None );

            // loop over all the properties in the collection printing each one
            for (int i = 0; i < fsrmCollection.Count; ++i) {

                // get the variant and convert it into a IFsrmPropertyDefinition
                IFsrmPropertyDefinition propertyDefinition;

                propertyDefinition = (IFsrmPropertyDefinition)fsrmCollection[i + 1];

                Console.Write( "Property {0}\n", 1 + i );
                DisplayPropertyDefinition( propertyDefinition );
                Console.WriteLine( "" );

            }
        }


        /*++

            Routine EnumerateProperties

        Description:

            This routine prints an FSRM property to the screen
            
        Arguments:

            Property - An IFsrmProperty pbject for the property.
            
        Return value:

            void
            
        Notes:
            

        --*/

        static void DisplayProperty(
            IFsrmProperty Property )
        {
            string name;
            string value;
            Array sources;
            int propertyFlag;

            // get each member for the property
            value = Property.Value;
            name = Property.Name;
            propertyFlag = Property.PropertyFlags;
            sources = Property.Sources;
            //print the properties


            Console.WriteLine( "\tName:\t\t{0}", name );
            Console.WriteLine( "\tDefinition:\t\t{0}", value );
            Console.Write( "\tProperty Flags:\t" );

            int FsrmPropertyFlags_Orphaned = 0x00000001;
            int FsrmPropertyFlags_RetrievedFromCache = 0x00000002;
            int FsrmPropertyFlags_RetrievedFromStorage = 0x00000004;
            int FsrmPropertyFlags_SetByClassifier = 0x00000008;
            int FsrmPropertyFlags_Deleted = 0x00000010;
            int FsrmPropertyFlags_Reclassified = 0x00000020;
            int FsrmPropertyFlags_AggregationFailed = 0x00000040;
            //int FsrmPropertyFlags_Existing = 0x00000080;
            int FsrmPropertyFlags_FailedLoadingProperties = 0x00000100;
            int FsrmPropertyFlags_FailedClassifyingProperties = 0x00000200;


            if (((int)propertyFlag & FsrmPropertyFlags_Orphaned) != 0) {
                Console.Write( "Orphaned " );
            }
            if (((int)propertyFlag & FsrmPropertyFlags_RetrievedFromCache) != 0) {
                Console.Write( "RetrievedFromCache " );
            }
            if (((int)propertyFlag & FsrmPropertyFlags_RetrievedFromStorage) != 0) {
                Console.Write( "RetrievedFromStorage " );
            }
            if (((int)propertyFlag & FsrmPropertyFlags_SetByClassifier) != 0) {
                Console.Write( "SetByClassifier " );
            }
            if (((int)propertyFlag & FsrmPropertyFlags_Deleted) != 0) {
                Console.Write( "Deleted " );
            }
            if (((int)propertyFlag & FsrmPropertyFlags_Reclassified) != 0) {
                Console.Write( "Reclassified " );
            }
            if (((int)propertyFlag & FsrmPropertyFlags_AggregationFailed) != 0) {
                Console.Write( "AggregationFailed" );
            }
            if (((int)propertyFlag & FsrmPropertyFlags_FailedLoadingProperties) != 0) {
                Console.Write( "FailedLoadingProperties " );
            }
            if (((int)propertyFlag & FsrmPropertyFlags_FailedClassifyingProperties) != 0) {
                Console.Write( "FailedClassifyingProperties" );
            }
            Console.Write( "\n" );

            Console.Write( "\tSources:\n" );
            for (int i = 0; i < sources.Length; ++i) {
                Console.Write( "\t\t{0} - {1}\n", i + 1, sources.GetValue( i ) );
            }
        }

        

        /*++

            Routine EnumerateFileProperties

        Description:

            This routine enumerates all FSRM properties on a file.
            
        Arguments:

            FilePath  - The file whose properties are to be enumerated.
            FsrmManager - The IFsrmClassificationManager object to retrieve the properties for the file.
            
        Return value:

            void
            
        Notes:
            

        --*/
        
        static void EnumerateFileProperties(
            string FilePath,
            IFsrmClassificationManager FsrmManager )
        {
            IFsrmCollection fsrmCollection;

            // get the list of properties for a file
            fsrmCollection = FsrmManager.EnumFileProperties(
                                    FilePath,
                                    _FsrmGetFilePropertyOptions.FsrmGetFilePropertyOptions_None
                                    );

            // loop over all the properties in the collection and print each
            for (int i = 0; i < fsrmCollection.Count; ++i) {

                IFsrmProperty property_ = (IFsrmProperty)fsrmCollection[i + 1];
                Console.WriteLine( "Property {0}", i + 1 );
                DisplayProperty( property_ );
                Console.Write( "\n" );

            }
        }



        /*++

            Routine GetFileProperty

        Description:

            This routine retrieves an FSRM property from a given file.
            
        Arguments:

            FilePath  - The file whose property is to be retrieved.
            PropertyName - The property to be retrieved
            FsrmManager - The IFsrmClassificationManager object to retrieve the property for the file.
            
        Return value:

            void
            
        Notes:
            

        --*/

        static void GetFileProperty(
            string FilePath,
            string PropertyName,
            IFsrmClassificationManager FsrmManager )
        {
            // Get the file property
            IFsrmProperty property_ = FsrmManager.GetFileProperty(
                                    FilePath,
                                    PropertyName,
                                    _FsrmGetFilePropertyOptions.FsrmGetFilePropertyOptions_None
                                    );
            DisplayProperty( property_ );
        }


        /*++

            Routine DisplayUsage

        Description:

            This routine displays the usage of the program.
            
        Arguments:

            void
            
        Return value:

            void
            
        Notes:
            

        --*/
        
        static void DisplayUsage()
        {
            Console.WriteLine( "Sample Usage:" );
            Console.WriteLine( "\t ManagedGetEnumProperties.exe -f c:\\foo\\cat.txt -p SomePropertyName -EnumerateProperties -GetFileProperty -EnumerateFileProperties" );
            Console.WriteLine( "" );
            Console.WriteLine( "You must specify atleast one of the following" );
            Console.WriteLine( "\t -EnumerateProperties, -EnumerateFileProperties, -GetFileProperty" );
            Console.WriteLine( "" );
            Console.WriteLine( "If specifying -EnumerateFileProperties" );
            Console.WriteLine( "\t The following must also be specified: " );
            Console.WriteLine( "\t\t -f <FilePath>" );
            Console.WriteLine( "If specifying -GetFileProperty" );
            Console.WriteLine( "\t The following must also be specified: " );
            Console.WriteLine( "\t\t -f <FilePath>" );
            Console.WriteLine( "\t\t -p <PropertyName>" );
            Console.WriteLine( "" );

        }


        /*++

            Routine Main

        Description:

            This is the main routine of the program.
            
        Arguments:

            args - Command line arguments
            
        Return value:

            void
            
        Notes:
            

        --*/

        static void Main(string [] args)
        {

            //Create an instance of the IFsrmClassificationManager
            FsrmClassificationManager fsrmManager = new FsrmClassificationManager();
            
            string filePath = "";
            string propertyName = "";           
            bool enumerateProperties = false;
            bool enumerateFileProperties = false;
            bool getFileProperty = false;

            //Parse command line arguments

            for(int i=0;i<args.Length;i++){

                if(args[i].Equals("-f", StringComparison.OrdinalIgnoreCase)
                    && i+1<args.Length)
                {
                    filePath = args[i+1];
                    i++;
                    continue;
                }

                if(args[i].Equals("-p", StringComparison.OrdinalIgnoreCase)
                    && i+1<args.Length)
                {
                    propertyName = args[i+1];
                    i++;
                    continue;
                }
                if(args[i].Equals("-enumerateProperties", StringComparison.OrdinalIgnoreCase))
                {
                    enumerateProperties = true;
                    continue;
                }
                if(args[i].Equals("-enumerateFileProperties", StringComparison.OrdinalIgnoreCase))
                {
                    enumerateFileProperties = true;
                    continue;
                }
                if(args[i].Equals("-getFileProperty", StringComparison.OrdinalIgnoreCase))
                {
                    getFileProperty = true;
                    continue;
                }
            }

            if (!(getFileProperty || enumerateProperties || enumerateFileProperties)) {

                DisplayUsage();
                System.Environment.Exit(-1);
            }
            
            if (enumerateFileProperties && filePath == "") {

                DisplayUsage();
                System.Environment.Exit( -1 );
            }

            if (getFileProperty && (filePath == "" || propertyName == "")) {

                DisplayUsage();
                System.Environment.Exit( -1 );
            }

            //Enumerate all FSRM properties
            if (enumerateProperties) {
                Console.WriteLine( "Enumerating Properties" );
                EnumerateProperties( fsrmManager );
            }

            //Enumerate all FSRM properties on a file
            if (enumerateFileProperties) {
                Console.WriteLine( "Enumerating File Properties - {0}",filePath );
                EnumerateFileProperties( filePath, fsrmManager );
            }

            //Get and print a particular FSRM property on a file
            if (getFileProperty) {
                Console.WriteLine( "Getting the {0} Property from file {1}",propertyName,filePath );
                GetFileProperty( filePath, propertyName, fsrmManager );
            }
        }
    }
}
