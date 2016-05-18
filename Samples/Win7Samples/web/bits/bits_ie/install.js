WScript.Echo("Installing the BITS IE Plug-in sample.");
var WshShell = new ActiveXObject("WScript.Shell");

WScript.Echo("Copying files...");
WshShell.Run("cmd /c \"copy bits_ie.htm %windir%\\system32 /Y\"",1,true);
WshShell.Run("cmd /c \"copy bits_ie.exe %windir%\\system32 /Y\"",1,true);

WScript.Echo("Configuring registry...");
var WINDIR = WshShell.ExpandEnvironmentStrings("%windir%");
var RunURL = WINDIR + "\\system32\\bits_ie.htm";
WshShell.RegWrite( "HKCU\\Software\\Microsoft\\Internet Explorer\\MenuExt\\Background Download As\\", RunURL, "REG_SZ" );
WshShell.RegWrite( "HKCU\\Software\\Microsoft\\Internet Explorer\\MenuExt\\Background Download As\\Contexts", 32, "REG_DWORD" );

WScript.Echo("Sample installed.");
WScript.Quit(0);