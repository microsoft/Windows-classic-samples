
OPC Music Bundle sample
=======================

     This sample demonstrates how to use OPC APIs to produce and consume an OPC Package based on a custom defined OPC file format.


Sample Language Implementations
===============================

     This sample is available in the following language implementations:

     C++


Files
=====

     MusicBundle.cpp - main entry point for the sample application

     MusicBundle.h - main header file, also contains a description of the custom OPC file format used in the sample

     MusicBundleProduction.cpp - contains sample code showing how to produce a package

     MusicBundleConsumption.cpp - contains sample code showing how to consume a package

     util.cpp / util.h - common utility functions for working with OPC packages & managing resources

     MusicBundle.vcproj - build configuration for this sample

     ConsumptionData\SampleMusicBundle.zip - An OPC package containing sample music files that can be consumed using the sample application.
                                             .zip extension has been used to make content type of the entire package obvious. The content
                                             creator can use another extension specific to their scenario.

     ProductionData\TrackList.wpl
     ProductionData\AlbumArt\jacqui.jpg
     ProductionData\Lyrics\CrystalFree.txt
     ProductionData\Lyrics\Sire.txt
     ProductionData\Lyrics\SmallPines.txt
     ProductionData\Lyrics\Valparaiso.txt
     ProductionData\Tracks\CrystalFree.wma
     ProductionData\Tracks\Sire.wma
     ProductionData\Tracks\SmallPines.wma
     ProductionData\Tracks\Valparaiso.wma
          - sample music and other files required by the custom OPC file format in the correct directory structure. These can be used with the
            sample application to produce an OPC music bundle package.
    
To build the sample using the command prompt:
=============================================
    
     1. Open the Command Prompt window and navigate to the directory containing MusicBundle.vcproj

     2. Type: msbuild MusicBundle.vcproj


To build the sample using Visual Studio 2008 (preferred method):
================================================================

     1. Open Windows Explorer and navigate to the directory.

     2. Double-click the icon for the MusicBundle.vcproj file to open the file in Visual Studio.

     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
==================

     1. Open a command line window and navigate to the directory where the MusicBundle.exe is built.

     2. To use the sample to produce a music bundle package use command:
        
             MusicBundle.exe -p <Input Directory> <Output Package Path>
        
             Input Directory : Provide full path to the directory with the files to be used as input to create a package. Use the ProductionData\*
                               files with the correct directory structure as provided with the sample.

             Output Package Path : Provide full path and name for the package you want to create

     3. To use the sample to consume a music bundle package use command:

             MusicBundle.exe -c <Input Package Path> <Output Directory>

             Input Package Path : Provide full path and name to the package you want to read. Use the ConsumptionData\SampleMusicBundle.zip
                                  provided with sample.

             Output Directory : Full path to the directory where you want the files read from the package to be placed.