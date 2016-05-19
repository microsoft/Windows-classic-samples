//
// ChangeProperties_ADSI.js
//
// The PrintUsage function is listed first so as not to repeat information.
// The meat of the WMI code starts at the comment, 'Connect to the machine via WMI.'
//

function PrintUsage() {
var s = "";

s = s + "\n ChangeProperties_ADSI.js";
s = s + "\n ";
s = s + "\n \t Description:";
s = s + "\n Sample script that uses ADSI to change properties on machines as listed in a tab-delimited file.";
s = s + "\n ";
s = s + "\n \t Syntax:";
s = s + "\n ChangeProperties_ADSI.js <file name>";
s = s + "\n ";
s = s + "\n \t Example:";
s = s + "\n Cscript /nologo ChangeProperties_ADSI.js c:\\mymachines.txt";
s = s + "\n Cscript /nologo ChangeProperties_ADSI.js mymachines.txt";
s = s + "\n ";
s = s + "\n \t Syntax of File:";
s = s + "\n <machine name> \t <metabase path> \t <property name> \t <value>";
s = s + "\n ";
s = s + "\n \t Example of File:";
s = s + "\n Server1 \t w3svc \t ConnectionTimeout \t 999";
s = s + "\n Server2 \t w3svc\/1 \t ServerComment \t My Default Server";
s = s + "\n Server2 \t w3svc\/1\/root \t Path \t c:\\webroot";
s = s + "\n Server1 \t msftpsvc \t ConnectionTimeout \t 999";
s = s + "\n ";
s = s + "\n \t Notes:";
s = s + "\n - The property must exist at the level you specify in the file.";
s = s + "\n - Not all properties propogate to child nodes. ConnectionTimeout does, ServerComment does not.";
s = s + "\n - The property value must be of a string, boolean, or integer data type.";
s = s + "\n - The file must be in ANSI format";
s = s + "\n - Each line in the file corresponds to one property change. A quick way to create the file that has repeated text is to use Excel where the first column is the machine name, the second column is the metabase path, the third column is the property name and the fourth column is the value you want set. Then, copy all the fields and paste into Notepad. Each line is automatically tab-delimited.";
s = s + "\n - The user of the script must be an administrator on all of the machines that are listed in the file. If the user account is not an administrator on all of the machines, but there is an account that is an administrator on all of the machines, alter the call to ConnectServer in this script to add a user name and password, or any other parameters like Locale ID.";
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
      
      if (aLine.length == 4) {

        var sMachineName = aLine[0];
        var sMetabasePath = aLine[1];
        var sPropertyName = aLine[2];
        var sValue = aLine[3];

        // Use a try-catch block in case there are any errors when trying to set a property.
        try
        {

          // Connect to the machine via ADSI.
          var providerObj = GetObject("IIS://" + sMachineName + "/" + sMetabasePath);

          // Get the old value.
          var oldValue = providerObj.Get(sPropertyName);

          // Set the property.
          providerObj.Put(sPropertyName, sValue);

          // Save the changes.
          providerObj.SetInfo();
          var newValue = providerObj.Get(sPropertyName);

          // Write out what was done.
          WScript.Echo("Success: " + sMachineName + " " + sMetabasePath + "/" + sPropertyName + ", " + oldValue + " -> " + newValue);        

        }
        catch(e)
        {

          WScript.Echo("*** Error: setting property, line " + iLines + " ***");
          WScript.Echo(e.number + " - " + e.description + "");
          WScript.Echo(sPropertyName + " can not be set at " + sMetabasePath + ".");

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


