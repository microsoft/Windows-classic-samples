//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Script name: 
//  configure.js
//
//  Purpose:
//  This script configures the local IIS server for the upload sample.
//  It will create some virtual directories and adjust some settings
//  related to the BITS extension configuration.
//
//  To undo the effects of this script in your IIS server, just
//  delete the 3 virtual directories created, and also delete the 
//  directory provided as input to this script.
//
//  NOTE: You need to hold administrator privileges in order to execute
//  this script.
//  
/// For usage instructions, execute "cscript configure.js" 
//  at the command line.
//
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Global variables and constants used in the script
//----------------------------------------------------------------------------

//
// Names of the virtual directories that this script will create/configure
//

// upload without notifications
var UPLOAD_VIRTUAL_DIRECTORY               = "UploadSample"                   

// upload with notifications
var UPLOAD_NOTIFICATION_VIRTUAL_DIRECTORY  = "UploadSampleWithNotifications"  

// location of the scripts activated when a notification is received
var SCRIPTS_VIRTUAL_DIRECTORY              = "UploadSampleScripts"            

// 
// Other global settings
//

// this is the ASP script that will be called by BITS every time an upload job completes
var SCRIPT_NOTIFIED_ON_UPLOAD = "newupload.asp"

// name of the server that we will be uploading to 
var UPLOAD_SERVER             = "localhost"

// local path of the virtual directory is the first argument to the script
var g_LocalPath = null

var FSO  = new ActiveXObject("Scripting.FileSystemObject")
var WSH  = new ActiveXObject("WScript.Shell")


//----------------------------------------------------------------------------
//  MAIN EXECUTION
//----------------------------------------------------------------------------


if (WScript.Arguments.length < 1)
{
	Usage()
    WScript.Quit(1)
}

g_LocalPath = WScript.Arguments(0)

Msg("\n---------- Verifying parameters and dependent components")
ValidateUserInput()
VerifyIISInstalled()
VerifyBITSExtensionsInstalled()

Msg("\n---------- Creating IIS virtual directories")
CreateLocalDirs(g_LocalPath)
var dir1 = CreateVirtualDirectory(UPLOAD_VIRTUAL_DIRECTORY,               g_LocalPath)
var dir2 = CreateVirtualDirectory(UPLOAD_NOTIFICATION_VIRTUAL_DIRECTORY,  g_LocalPath)
var dir3 = CreateVirtualDirectory(SCRIPTS_VIRTUAL_DIRECTORY,              g_LocalPath)

Msg("\n---------- Copying notification script to the script virtual directory")
// assumes the script to be installed is at the current directory
CopyScriptToVdir(SCRIPT_NOTIFIED_ON_UPLOAD, dir3)

Msg("\n---------- Configuring virtual directories for upload")
ConfigureUploadVdir(dir1)
ConfigureUploadVdirWithNotifications(dir2)
ConfigureScriptsVdir(dir3)

Msg("\n---------- DONE!")
WScript.Quit(0)



//----------------------------------------------------------------------------
//  Auxiliary functions
//----------------------------------------------------------------------------

function Usage()
{
	WScript.Echo("\n=======================================================================")
	WScript.Echo("")
	WScript.Echo("  BITS UPLOAD SAMPLE CONFIGURATION SCRIPT")
    WScript.Echo("")
	WScript.Echo("  This script will assist you in configuring your IIS server for the ")
	WScript.Echo("  the BITS upload sample." )
	WScript.Echo("")
    WScript.Echo("  Usage: " + WScript.ScriptName + " <local_path>")
	WScript.Echo("")
	WScript.Echo("  where:")
	WScript.Echo("    <local_path> is the directory of the local machine where your sample ")
	WScript.Echo("                 upload files will be transferred. The directory will be")
	WScript.Echo("                 created by the script and cannot exist already.")
	WScript.Echo("")
	WScript.Echo("=======================================================================")
	WScript.Echo("")
}

function FatalError(msg, err)
{
	WScript.Echo("ERROR: " + msg)
	if (err)
	{
		WScript.Echo("The error message returned by the system is: " + err.description + " (" + err.number + ")")
	}

	WScript.Quit(1)
}

function Msg(msg)
{
	WScript.Echo(msg)
}

function ValidateUserInput()
{
	if (FSO.FolderExists(g_LocalPath))
	{
		FatalError("The directory " + g_LocalPath + " already exists. Please provide a new path as a parameter to the script.")
	}
}

function CreateDirectory(path)
{
	try
	{
		Msg("Creating the local directory " + path + ".")
		FSO.CreateFolder(path)
	}
	catch (err)
	{
		FatalError("The directory " + path + " cannot be created.", err)
	}
}

function CreateLocalDirs(rootpath)
{
	CreateDirectory(rootpath)
	CreateDirectory(rootpath + "\\" + UPLOAD_VIRTUAL_DIRECTORY)
	CreateDirectory(rootpath + "\\" + UPLOAD_NOTIFICATION_VIRTUAL_DIRECTORY)
	CreateDirectory(rootpath + "\\" + SCRIPTS_VIRTUAL_DIRECTORY)
}

function CopyScriptToVdir(scriptpath, oVdir)
{
	try
	{
		Msg("Copying " + scriptpath + " to " + oVdir.Path + ".")
		FSO.CopyFile(scriptpath, oVdir.Path + "\\")
	}
	catch(err)
	{
		FatalError("Couldn't copy the script " + scriptpath + " to " + oVdir.Path + ".")
	}
}

function VerifyIISInstalled()
{
	Msg("Verifying that IIS is installed on this machine...")

	// if this call fails an exception is going to be thrown
	try
	{
		var oIIS = GetObject("IIS://" + UPLOAD_SERVER + "/W3SVC/1/ROOT")
	}
	catch (err)
	{
		FatalError("This script could not verify that IIS is installed. Please verify your installation and try again.", err)
	}
}

function VerifyBITSExtensionsInstalled()
{
	Msg("Verifying that BITS Extensions is installed on this machine...")

	try
	{
		var oWWWRoot = GetObject("IIS://" + UPLOAD_SERVER + "/W3SVC/1/ROOT")
		var test     = oWWWRoot.BITSUploadEnabled
	}
	catch (err)
	{
		FatalError("This script could not verify that BITS Extensions are installed. BITS Extensions can be installed\n" +
				   "on Windows Server 2003 by using Control Panel's Add/Remove Windows Components or on Windows 2000 Server\n" +
				   "by installing the BITS SDK package.", err)
	}
}

//----------------------------------------------------------------------------
// IIS configuration for BITS upload
//----------------------------------------------------------------------------


function CreateVirtualDirectory(name, physicalpath)
{
    Msg("Creating the virtual directory '" + name + "'.")

	var oWWWRoot   = GetObject("IIS://" + UPLOAD_SERVER + "/W3SVC/1/ROOT")

	// test if the vdir already exists
	try
	{
		var test = GetObject("IIS://" + UPLOAD_SERVER + "/W3SVC/1/ROOT/" + name)

		FatalError("The virtual directory named " + name + " already exists. \n" + 
				   "Please delete it before using this script.")
	}
	catch (err)
	{
		// just continue in case of error
	}

	oVdir          = oWWWRoot.Create("IIsWebVirtualDir", name)
	oVdir.Path     = physicalpath + "\\" + name

	oVdir.AppIsolated       = 0
	oVdir.EnableDirBrowsing = false

	oVdir.SetInfo()
	return oVdir
}

function ConfigureUploadVdir(oVdir)
{
	Msg("Enabling BITS uploads on the virtual directory " + oVdir.Name)

	oVdir.AccessScript = false
	oVdir.AccessRead   = false
	oVdir.AccessWrite  = false     // not needed in order for upload to function!!

	oVdir.BITSMaximumUploadSize = 4294967296
	oVdir.EnableBITSUploads()
	oVdir.SetInfo()
}

function ConfigureScriptsVdir(oVdir)
{
	Msg("Setting script permissions on the virtual directory " + oVdir.Name)

	oVdir.AccessScript = true
	oVdir.AccessRead   = true
	oVdir.AccessWrite  = false 

	oVdir.AccessNoRemoteExecute = true
	oVdir.AccessNoRemoteScript  = true
	oVdir.SetInfo()
}

function ConfigureUploadVdirWithNotifications(oVdir)
{
	var notificationurl = "http://" + UPLOAD_SERVER + "/" + SCRIPTS_VIRTUAL_DIRECTORY + "/" + SCRIPT_NOTIFIED_ON_UPLOAD

	Msg("Enabling BITS uploads on the virtual directory " + oVdir.Name + ", and turning on upload notifications.")
	Msg("The URL that will be executed when a file is successfully uploaded is " + notificationurl + ".")

	oVdir.AccessScript = false
	oVdir.AccessRead   = false
	oVdir.AccessWrite  = false     // not needed in order for upload to function!!

	oVdir.BITSMaximumUploadSize = 4294967296
	oVdir.EnableBITSUploads()

	oVdir.BITSServerNotificationType = 1     // by-ref
	oVdir.BITSServerNotificationURL  = notificationurl
	oVdir.SetInfo()
}

