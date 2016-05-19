========================================================================
       WIN32 Console Application : DRMHeader
========================================================================

Synopsis:
    DRMHeader demonstrates how to use the IWMDRMEditor interface of the MetadataEditor Object in order to query for DRM attributes.

    The significance of this interface is that it allows access to certain DRM properties without the need to link against WMSTUBDRM.LIB. 
    Notice that this sample program is only linked against WMVCORE.LIB. It does not require WMSTUBDRM.LIB .


Building the sample:
    To build the sample, open the project file DRMHeader.sln in the Visual C++ environment and build the project.
        
Running the sample:
    To run the sample, use the following syntax:
        drmheader <input file> [Property Name]
        
    

Features demonstrated in this sample:
    - IWMDRMEditor::GetDRMProperty
    - IWMMetadataEditor::Open
    - DRM Properties that are accesible through the DRM Editor interface

