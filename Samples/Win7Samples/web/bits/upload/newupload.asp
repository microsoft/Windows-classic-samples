<%@ LANGUAGE=JScript %>
<!--
//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Script name: 
//  newupload.asp
//
//  Purpose:
//  This script processes upload notifications for the BITS upload sample,
//  and sends a reply to the client (BITS Upload-Reply).
// 
//  Note that because the virtual directory was configured to trigger 
//  notifications of type "by-ref", we are processing the notification by
//  using paths that were passed to us.
//
//  We then use the uploadedFile and responseFile paths to directly access
//  these files via the file system: the xml file contents are read and
//  some information is sent back to the client.
//
//----------------------------------------------------------------------------
-->

<%

  // Create a FileSystemObject
  var FSO = new ActiveXObject ("Scripting.FileSystemObject")

  // Get the path for the file uploaded and also the location where we should
  // write our response to the BITS client
  var uploadedFile = Request.ServerVariables ("HTTP_BITS-Request-DataFile-Name")
  var responseFile = Request.ServerVariables ("HTTP_BITS-Response-DataFile-Name")
  var uploadURL    = Request.ServerVariables("HTTP_BITS-Original-Request-URL")
  var serverName   = Request.ServerVariables("HTTP_HOST")
  
  // constants needed for FSO
  var ForReading = 1
  var ForWriting = 2
  var OpenAsUnicode = -1
  var OpenAsAscii   = 0

  // local variables
  var fs           = null
  var xmlfs        = null
  var xmlnode      = null

  // Open response data file as text stream for writing
  fs = FSO.OpenTextFile(responseFile, ForWriting, true, OpenAsUnicode)

  //
  // We are going to send back to the client a "receipt" that we received the upload.
  // So output some data about the uploaded job
  //
  fs.Write("Request URL:  "                    + uploadURL + "\r\n")
  fs.Write("Server that received the upload: " + serverName + "\r\n")
  fs.Write("Uploaded File Name: "              + uploadedFile + "\r\n")
  fs.Write("File to write the response to: "   + responseFile + "\r\n")

  // if XMLDOM is installed on the server machine, we are going to
  // to load the XML and extract part of its content
  try
  {
	  var XML = new ActiveXObject ("Msxml2.DOMDocument")

	  // the upload sample app saves the file as ANSI, with UTF-8 encoding
	  xmlfs = FSO.OpenTextFile(uploadedFile, ForReading, true, OpenAsAscii)

	  XML.async = false
	  XML.loadXML(xmlfs.ReadAll())

	  xmlfs.close()

      xmlnode = XML.documentElement.selectSingleNode("//text")
	  fs.write("Text packed inside the XML file: '" + xmlnode.text + "'\r\n")
  }
  catch(err)
  {
	  fs.write("The XML file couldn't be loaded by the XMLDOM. The error was: " + err.description + "\r\n")
  }


  //
  // Copy the temporary uploaded file the the final file name.
  //
  // Note that when notifications are enabled for BITS uploads, it's up to the script processing the notification
  // to copy the uploaded file from the temporary location (that will be deleted as soon as the job completes)
  // to a final destination. Do so if you are interested in persisting the data uploaded.
  //
  try
  {
	  var format = /\w+:\/\/[^/]+(\/.+)/

	  if (format.exec(uploadURL))
	  {
		  var relativeURL = RegExp.$1
		  var finalDest   = Server.MapPath(relativeURL)

		  FSO.CopyFile(uploadedFile, finalDest, true)
		  fs.write("Uploaded file was copied to " + uploadURL + "\r\n")
	  }
  }
  catch(err)
  {
	  fs.write("Could not copy the uploaded file the destination URL. The error was:  " + err.description + "\r\n")
  }
  
  // close the file
  fs.Close()

%>
