WScript.Echo("Removing the BITS IE Plug-in sample.");
var WshShell = new ActiveXObject("WScript.Shell");

WScript.Echo("Deleting files...");
WshShell.Run("cmd /c \"del %windir%\\system32\\bits_ie.htm\"",1,true);
WshShell.Run("cmd /c \"del %windir%\\system32\\bits_ie.exe\"",1,true);

WScript.Echo("Removing registry entries...");
WshShell.RegDelete( "HKCU\\Software\\Microsoft\\Internet Explorer\\MenuExt\\Background Download As\\" );

WScript.Echo("Sample removed.");
WScript.Quit(0);