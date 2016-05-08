==========================
DHTML - HTML Editor
==========================
Last Updated: April 22, 2001              

DESCRIPTION
============
This sample demonstrates how to use features of Microsoft® Internet Explorer to create an HTML editor application. 
The sample application contains a content editable region where users can type and format text with all of the 
standard formatting commands. A toolbar provides a number of formatting buttons, buttons for opening and saving files, 
and drop-down list boxes for font and block formatting selections. A menu provides users another way to access the 
functionality exposed by the toolbar buttons. Users can create HTML documents, format them, and save them to disk 
in either HTML or text format. If saved as an .htm file, the saved documents are viewable in Internet Explore 6.0 or 
in the HTML editor application. Documents saved as .txt files can be viewed in any text editor.

BROWSER/PLATFORM COMPATIBILITY
===============================
In order to successfully run this sample, you must have Microsoft® Internet Explorer 6.O on a Win32® platform.

USAGE
=====
To use HTMLEditor, complete the following steps. 
1. Download and Install Internet Explorer 6.0 if you are not running the minimum browser required.  
2. Download the files from this sample and copy them all to the same folder.
3. Double-click HTML_editor.hta to run the application. If you do not have the CommonDialog Control installed on your 
   machine, you will be prompted to download this file upon first usage of the application.  
4. To save files on a network file share, you must have write permission 
for that share. 

Security Note
=============
This sample is an HTML Application (HTA) and therefore is a fully trusted application. HTAs carry out actions that Internet Explorer would never permit in a Web page. For example, this sample can open and save files to the user's hard drive. To learn more about HTAs and the security implications of using them, see Introduction to HTML Applications (HTAs). 

SOURCE FILES
=============
HTML_editor.hta
Toolbar.htc
Menu.htc
dCheckForSave.htm

OTHER FILES
============
Comdlg.lpk
Readme.txt

SEE ALSO
=========
DHTML Behaviors
	http://msdn.microsoft.com/workshop/author/behaviors/overview.asp
Element Behaviors
	http://msdn.microsoft.com/workshop/author/behaviors/overview/elementb_ovw.asp
HTC Reference
	http://msdn.microsoft.com/workshop/components/htc/reference/htcref.asp
Internet Explorer WebControls References
	http://msdn.microsoft.com/workshop/webcontrols/webcontrols_entry.asp
Dialog Helper Object
	http://Zanzibar/workshop/author/dhtml/reference/objects/dialoghelper.asp
HTML Applications
	http://msdn.microsoft.com/workshop/author/hta/overview/htaoverview.asp

========================
© Microsoft Corporation

  





