========================================================================
       WIN32 Console Application : DRMShow
========================================================================

Synopsis:
    DRMShow shows how to read DRM properties of a Windows Media file using the GetDRMProperty() method of the IWMDRMReader interface.
    
    NOTE: If you get errors such as "Failed to QI for DRM Reader interface", then you are likely building with wmvcore.lib instead of wmstubdrm.lib.  For information on obtaining a stublib, see the section "Getting a stublib" below.
    

Building the sample:
    To build the sample, open the project file DRMShow.sln in VC and build the project.
        
    NOTE: This sample requires the DRM stub library "wmstubdrm.lib" to build.  The project will need to be altered to link with this file instead of the wmvcore.lib library.  For information on obtaining a stublib, see the section "Getting a stublib" below.


Running the sample:
    To run the sample, use the following syntax:
        drmshow -i <input file>
        
    Note that a DRM-protected file cannot be opened without a license, so the DRMShow sample is unable to show properties for a file without a license.  Also, running DRMShow with a non-DRM file will cause the sample to fail to retrieve all of the properties except "IsDRM" with the HRESULT NS_E_DRM_UNSUPPORTED_PROPERTY. Every license associated with a file has a security level specifying the security level that an application must support in order to play the file. When you acquire a stublib, it has an associated security level. If the security level of your stublib does not meet the requirements in a license, the file cannot be played.
    
    
Getting a stublib:
    Contact wmla@microsoft.com to obtain the Windows Media Rights Managment setup, or for more information go to: http://www.msdn.microsoft.com/windowsmedia


Items demonstrated in this sample:
    - Use of the IWMDRMReader::GetDRMProperty() method
    - Properties that can be gotten from the DRM reader
    
    
What this sample does not show:
    - How to acquire a license for DRM-protected content

