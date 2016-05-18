VShadow Sample

Building the Visual Studio 2005 project will produce vshadow.exe. Start
a cmd.exe window running as a local administrator, and run this program
to see the list of available commands.

Building solution outside of SDK directory requires MSSdk environment
variable defined or project file updated with paths to includes and
libraries. For more information about MSSdk environment variable,
please see the SetEnv.Cmd script in bin subdirectory inside SDK
installation path.

Itanium architecture is available as a target for server SKUs only.
Appropriate compiler is required.

Visual Studio 2005 solution is designed to work in Visual Studio 2008
as well after initial conversion performed by the IDE.

Building in Visual Studio 2005 against SDK libraries may require manual
reconfiguration of Visual Studio 2005 directories configuration if you
encounter linker errors related to uuid.lib. To do that, please select
Tools > Options in the main menu. Navigate to Projects and Solutions >
VC++ Directories branch and select "Library files" for your desired
architecture from the "Show directories for" dropdown menu. SDK path
will vary depending on the installation directory. By default it is
c:\Program Files\Microsoft SDKs\Windows\v7.0\Include. Select and move
this directory down the list using "Line Down" button in the upper
right corner of the dialog.
