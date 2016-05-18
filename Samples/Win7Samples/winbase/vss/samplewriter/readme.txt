Sample Writer

Building the Visual Studio project will produce Sample Writer.exe. Start
a cmd.exe window running as a local administrator, and run this program.

Sample Writer.exe will continue executing until a Ctr+C is entered into the
console. The Writer can be viewed by executing the following command from
another window

    vssadmin list writers

BeTest included in the SDK can be used to verify Writer's behavior during
backup and restore.

The Writer will show up with ID {079462f2-1079-48dd-b3fb-ccb2f2934ec0}, under
the name "Sample Writer". This Writer will search for the user profiles
on the machine and expose then as components. Each of them may have one or more
subcomponents with file groups exposing one of the handled file types (*.docx,
*.doc, *.jpg, etc. - for full reference please look into the code). Detailed
implementation can be found within the comments in the code. It is important to
understand that Writers perform on-disk operations during restore and changes
to the Writer may cause it to overwrite existing data. Sample code like this
should be always executed in the sandbox environment (e.g. test machine.)


Building solution outside of SDK directory requires MSSDK environment variable
defined or project file updated with paths to includes and libraries. For more
information about MSSDK environment variable, please see the SetEnv.cmd script
in the bin subdirectory inside the SDK installation path.

Itanium architecture is available as a target for server SKUs only. Appropriate
compiler is required.


