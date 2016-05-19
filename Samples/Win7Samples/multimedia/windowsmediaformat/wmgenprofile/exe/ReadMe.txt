========================================================================
       WIN32 MFC Dialog Application : GenProfile
========================================================================

Synopsis:
    The GenProfile sample demonstrates the usage of the GenProfile static library.  It also serves as a tool for the creation of profiles.
    

Building the sample:
    To build the sample, open the GenProfile solution and build the GenProfile_lib project and then the GenProfile_exe project.
        
    NOTE: The GenProfile_Exe project depends upon the GenProfile.lib static library.  If the static library is not built first, the GenProfile_Exe project will not build correctly.  Also, the file wmsdkidl.h requires the latest version of rpcndr.h.  If you receive an error saying, "incorrect <rpcndr.h> version", you are not using the most recent version of the rpcndr.h file.  You can get a newer version by installing the latest platform SDK.  Alternatively, you can use midl on wmsdkidl.idl to generate a compatible wmsdkidl.h.


Running the sample:
    To run the sample, run "GenProfile.exe"


Items demonstrated in this sample:
    - Use of the GenProfile static library
    - Saving a profile as a PRX file
    
    
What this sample does not show:
    - MFC application programming - This example is not meant as an example of MFC programming
    - Recommended UI for profile creation - This tool is meant for developers familiar with the Windows Media Format.  The UI has not been tested for user experience and is not meant as a recommendation about how to present this information to a user.

