The sample demonstrates the Windows Server 2003 Authorization Manager
Framework.

This is a simple web expense application. To run this sample:

  Copy the included files into a virtual directory for IIS.

  Place the AzStore.xml file in the root of the C drive or change the
  path to the AzStore.xml file in Common.asp (AZMAN_STORE_URL) to the
  location of the AzStore.xml file.

  Set the Directory Security Authentication Properties to Windows 
  Integrated Authentication

  Make sure that "Active Server Pages" is enabled. This is done in the
  IIS MMC UI. By selecting the "Web Service Extensions" container on the left
  pane and clicking on "Active Server Pages" in the list on right pane and
  selecting "Allow"

  Use Internet Explorer to browse to the sample.htm page in the
  virtual directory create above.


Note:

This sample demonstrates a web expense application running in the
context of a web service account. Usually ASP Applications will run in
the context of the web client user. While you can run this sample as the
with out IIS URL Authorization configured client user in practice the web 
application will not impersonate the client user when using Authorization 
Manager. Since ASP does not support you to configure the server to run in
a context other than impersonating the client user, use IIS 6.0 URL 
Authorization to configure this Application run in the context of the IIS 
Worker process corresponding to the IIS 6.0 Web Site that contains this 
sample.  You can setup a worker process to use an account that you want 
this sample to run as.

See IIS 6.0 Online help for more info on IIS 6.0 URL Authorization.

SetURLAuth.vbs demonstrates setting the IIS meta-base attributes.

For more information on IIS 6.0 URL Authorization see Microsoft Knowledge
Base Article Q326020.

For More information on Authorization Manager see the above Knowledge Base
Article and the Microsoft Platform SDK
