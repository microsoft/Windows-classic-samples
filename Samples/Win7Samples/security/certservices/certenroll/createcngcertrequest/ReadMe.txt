Sample to demonstrate how to create a CNG (Suite-B based) simple 
certificate request using CertEnroll classes.

NOTE: This sample requires Visual Studio 2005. Create a project and
in the menu click on Project -> Add Reference...
this will pop a dialog. Click on the COM tab
Select 'CertEnroll 1.0 Type Library' and click OK.

This will create an interop library which will be used by the C# code.

When you run this sample the base 64 encoded request will be displayed
on the console. You can copy this request and save it to a file.

Use the following command to verify the request:
certutil -dump [filename]
