////////////////////////////////////////////////////////////////////
// Description
////////////////////////////////////////////////////////////////////
The ReadOnlyPlugin solution is a sample plugin for photo 
acquisition in Windows Vista.  The plugin is a DLL.
To register the plugin, run regsvr32.exe
The plugin, once registered, performs additional functionality as 
files are saved, following acquisition from a device.

////////////////////////////////////////////////////////////////////
// Files
////////////////////////////////////////////////////////////////////

The solution contains the following files:
ReadOnlyPlugin.sln	-- Visual Studio solution file.
ReadOnlyPlugin.vcproj	-- Visual Studio project file.
ReadOnlyPlugin.cpp	-- Implementation for photo acquisition plugin.
ReadOnlyPlugin.rc	-- Visual Studio generated resource file.
				Contains resource definition for plugin string.
resource.h		-- Visual Studio generated resource header.

////////////////////////////////////////////////////////////////////
// Build instructions
////////////////////////////////////////////////////////////////////

Using Visual Studio 2005, in Project Properties, under 
Configuration Properties->Linker->Input, make sure the following 
are added to Additional Dependencies: 

photoacquireuid.lib bufferoverflowu.lib

In Tools->Options->Projects and Solutions->VC++ directories
make sure that the SDK directories for library files and include files
are added.
