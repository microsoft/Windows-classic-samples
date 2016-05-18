//
// ChangeProperties_WMI.js
//
// The PrintUsage function is listed first so as not to repeat information.
// The meat of the WMI code starts at the comment, 'Connect to the machine via WMI.'
//

function PrintUsage() {
var s = "";

s = s + "\n ChangeProperties_WMI.js";
s = s + "\n ";
s = s + "\n \t Description:";
s = s + "\n Sample script that uses WMI to change properties on machines as listed in a tab-delimited file.";
s = s + "\n ";
s = s + "\n \t Syntax:";
s = s + "\n ChangeProperties_WMI.js <file name>";
s = s + "\n ";
s = s + "\n \t Example:";
s = s + "\n Cscript /nologo ChangeProperties_WMI.js c:\\mymachines.txt";
s = s + "\n Cscript /nologo ChangeProperties_WMI.js mymachines.txt";
s = s + "\n ";
s = s + "\n \t Syntax of File:";
s = s + "\n <machine name> \t <metabase path> \t <property name> \t <value> \t <node type>";
s = s + "\n ";
s = s + "\n \t Example of File:";
s = s + "\n Server1 \t w3svc \t ConnectionTimeout \t 999 \t IIsWebServiceSetting";
s = s + "\n Server2 \t w3svc\/1 \t ServerComment \t My Default Server \t IIsWebServerSetting";
s = s + "\n Server2 \t w3svc\/1\/root \t Path \t c:\\webroot \t IIsWebVirtualDirSetting";
s = s + "\n Server1 \t msftpsvc \t ConnectionTimeout \t 999 \t IIsFtpServiceSetting";
s = s + "\n ";
s = s + "\n \t Notes:";
s = s + "\n - The property must exist at the level you specify in the file.";
s = s + "\n - Not all properties propogate to child nodes. ConnectionTimeout does, ServerComment does not.";
s = s + "\n - The property value must be of a string, boolean, or integer data type.";
s = s + "\n - The file must be in ANSI format";
s = s + "\n - Each line in the file corresponds to one property change. A quick way to create the file that has repeated text is to use Excel where the first column is the machine name, the second column is the metabase path, the third column is the property name, the fourth column is the value you want set, and the fifth column is the node type. Then, copy all the fields and paste into Notepad. Each line is automatically tab-delimited.";
s = s + "\n - The user of the script must be an administrator on all of the machines that are listed in the file. If the user account is not an administrator on all of the machines, but there is an account that is an administrator on all of the machines, alter the call to ConnectServer in this script to add a user name and password, or any other parameters like Locale ID.";
s = s + "\n ";
s = s + "\n \t Node Types:";
s = s + "\n Node types correspond to the element classes of the IIS WMI provider CIM_Setting class.";
s = s + "\n Use CIM Studio from the WMI SDK to view these classes, or open %systemroot%\\system32\\inetsrv\\mbschema.xml, and search for '<Collection InternalName ='. Any InternalName value with the word 'Setting' in it indicates a class with writable properties. Those properties are listed underneath as '<Property'.";
s = s + "\n ";

WScript.Echo(s);
return 1;
}


//
// Main body of ChangeProperties.js script
//


// Get the argument.
var WshArgs = WScript.Arguments;

if (WshArgs.Length == 1) {

  // Try to open the file whose name was passed to the script.
  var FSO = new ActiveXObject("Scripting.FileSystemObject");  

  if (FSO.FileExists( WshArgs(0) )) {

    // Open the file.
    fFile = FSO.OpenTextFile( WshArgs(0), 1, false );

    // Get the WMI locator object which will connect to each machine.
    var locatorObj = new ActiveXObject("WbemScripting.SWbemLocator");

    // For each line in the file, change the property and report failure or success.
    var iLines = 0;
    WScript.Echo();
    while(!fFile.AtEndOfStream) {

      // Increment the counter.
      iLines++;

      // Get the parameters from the line in the file.
      var aLine = new Array();
      var sLine = fFile.ReadLine();
      var aLine = sLine.split("\t");
      
      if (aLine.length == 5) {

        var sMachineName = aLine[0];
        var sMetabasePath = aLine[1];
        var sPropertyName = aLine[2];
        var sValue = aLine[3];
        var sNodeName = aLine[4];

        // At least test if the NodeName has the word 'Setting' in it.
        var regExp = /setting/i;
        var iPos = sNodeName.search(regExp);
        if (0 < iPos) {

          // Use a try-catch block in case there are any errors when trying to set a property.
          try
          {

            // Connect to the machine via WMI.
            // If you need to use a username and password, use this line:
            // var providerObj = locatorObj.ConnectServer(sMachineName, "root/MicrosoftIISv2", "<userdomain>\\<username>", "<password>");
            var providerObj = locatorObj.ConnectServer(sMachineName, "root/MicrosoftIISv2");

            // Get an instance of the sNodeName object.
            // The next line should work out to something like var nodeObj = providerObj.Get("IIsWebServiceSetting='W3SVC'");
            var nodeObj = providerObj.Get(sNodeName + "='" + sMetabasePath + "'");
            var oldValue = nodeObj.Properties_(sPropertyName).Value;

            // Set the property.
            nodeObj.Properties_(sPropertyName).Value = sValue;

            // Save the changes.
            nodeObj.Put_();
            var newValue = nodeObj.Properties_(sPropertyName).Value;

            // Write out what was done.
            WScript.Echo("Success: " + sMachineName + " " + sMetabasePath + "/" + sPropertyName + ", " + oldValue + " -> " + newValue);        

          }
          catch(e)
          {

            WScript.Echo("*** Error: setting property, line " + iLines + " ***");
            WScript.Echo(e.number + " - " + e.description + "");
            WScript.Echo(sMachineName + " might not support WMI, or " + sPropertyName + " can not be set at " + sMetabasePath + ", or " + sMetabasePath + " is not of type " + sNodeName + ".");

          }

        }  // end of if (0 < iPos) {
        else {

          WScript.Echo("*** Error: " + sNodeName + " is not a WMI element class that contains writable properties (ie, not in the CIM_Setting class). Line " + iLines + ". ***");

        }

      }  // end of if (aLine.length == 5) {
      else {

        WScript.Echo("*** Error: improper number of arguments or arguments not tab-delimited in line " + iLines + " ***")

      }

    }  // end of while(!fFile.AtEndOfStream)

  }  // end of if (FSO.FileExists( WshArgs(0) )) {
  else {

    WScript.Echo("\n*** Error: can not find file, '" + WshArgs(0) + "' ***\n");

  }

}  // end of if (WshArgs.Length == 1) {
else {

  PrintUsage();

}


