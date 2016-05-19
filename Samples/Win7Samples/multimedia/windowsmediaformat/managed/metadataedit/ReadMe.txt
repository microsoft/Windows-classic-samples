========================================================================
       Windows Application : ManagedMetadataEdit
========================================================================

This managed-code application can be used to view and edit metadata from Windows Media files. 

Before you can build this application, the assembly ManagedWMFSDKWrapper needs to be built.

The various command line options for this sample are as follows.

========================================================================

MetadataEdit     <filename> show <stream number>
                 <filename> show3 <stream number>
                 <filename> delete <stream number> <attrib index>
                 <filename> set <stream number> <attrib name> <attrib type> <attrib value>
                 <filename> add <stream number> <attrib name> <attrib type> <attrib value> <attrib language>
                 <filename> modify <stream number> <attrib index> <attrib type> <attrib value> <attrib language>

 Attrib Type can have one of the following values
         0 - WMT_TYPE_DWORD
         1 - WMT_TYPE_STRING
         2 - WMT_TYPE_BINARY
         3 - WMT_TYPE_BOOL
         4 - WMT_TYPE_QWORD
         5 - WMT_TYPE_WORD
         6 - WMT_TYPE_GUID

========================================================================

show displays all the attributes for the specified stream number obtained through the IWMHeaderInfo interface

show3 displays all the attributes for the specified stream number obtained through the IWMHeaderInfo3 interface

delete enables you to delete attributes using IWMHeaderInfo3::DeleteAttribute

set enables you to add or modify attributes using IWMHeaderInfo::SetAttribute

add enables you to add attributes using IWMHeaderInfo::AddAttribute

modify enables you to modify attribute values using IWMHeaderInfo3::ModifyAttribute





