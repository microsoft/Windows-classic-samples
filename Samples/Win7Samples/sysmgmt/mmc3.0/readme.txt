//=======================================================================================
//
//  README.TXT - Read me notes for MMC 3.0 SDK samples
//
//  This file and the source code found in the associated samples is only intended as 
//  a supplement to existing Microsoft documentation. 
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (C) 2005 Microsoft Corporation.  All Rights Reserved.
//
//=======================================================================================


Installation:
MMC requires some registry information for each snap-in before the snap-in will be available through the Add / Remove menu option.  Each sample dll must be installed using the .Net Framework InstallUtil.exe for it to show in the list. Also, each time a change is made to any snap-in attributes (eg SnapInSettings) in the sample, the InstallUtil.exe will need to be rerun in order to recognize the changes. In all cases the Microsoft.ManagementConsole.dll will need to be able to be referenced or in the same directory as the snap-in dll in order for InstallUtil to succeed.

=======================================================================================



LIST OF SAMPLES:
The list of samples attempts to gives full coverage of the API and how it could be used. Each sample below is rated for complexity where more '*'s is more complex. For those wanting to dive in head first see the ‘Services Manager Sample’ as an example of a complete system.

=======================================================================================

Actions Sample - (**) 

Purpose: 
Shows the various places 'custom' actions can be added to the snap-in: ScopeNodes, Views, ViewModes, ResultNode Selections. 

Description:
Loads a list of users into a MmcListView.  Adds actions as follows: ScopeNode has actions Add Child Node and Add to Root Node. MmcListView has actions Refresh and SortByName. The mode of the MmcListView has actions View as Large Icons, View As List, View As Small Icons and View as Report. The SelectionData of result nodes has the action ShowSelection.

=======================================================================================
	
Drag Drop Sample - (**)
Purpose: 
Shows how to do dragging and dropping with the MmcListView 

Description:
Loads a list of Users into the MmcListView and enables the standard verbs Copy and Paste. Dropped on nodes get their DisplayName changed to show they were dropped on.

=======================================================================================
	
Extendible SnapIn Sample - (*)
Purpose: 
Shows how to create a Primary snap in that can be extended

Description:
Uses PublishesNodeType and NodeType attributes on the snap in and its root node type respectively to create a SnapIn that can be extended.  (see also ‘Extension Sample’)

=======================================================================================
	
Extending Computer Management Sample - (*)
Purpose: 
Shows how to extend the Computer Management Snap In

Description:
Uses the ExtendsNodeType attribute and the Guid of the Computer Management SnapIn RootNode to create a NameSpaceExtension. Uses the shared data features to get the MMC_SNAPIN_MACHINE_NAME from the Computer Management SnapIn and use it in the Extension node’s DisplayName as it adds it under System Tools.

=======================================================================================
	
Extension Sample - (*)
Purpose: 
Shows how to create an extension to the Extendible SnapIn.

Description:
Uses the ExtendsNodeType attribute and the Guid of the ‘Extendible Sample’ root node to create a NameSpaceExtension. Uses the shared data features to get some data from the primary and use it in the Extension node’s DisplayName.

=======================================================================================
	
Extension to Property Sheet Sample - (***)
Purpose: 
Shows how to extend a property sheet with extra property pages.

Description:
Uses the ExtendsNodeType attribute to create a NameSpaceExtension. Uses the PropertySheetExtension class to add a new property page to a property sheet that already exists.
=======================================================================================
	
Help Sample - (*)
Purpose: 
Shows how to add to the help system by pointing at a .chm file.

Description:
Uses the SnapInHelp attribute to cause an external .chm help file to be added to the MMC help system. Also fills the HelpTopic property on the root node to jump right to a specific help page url (in this case the ScopeNode definition help page).

=======================================================================================
	
Html View Sample - (*)
Purpose: 
Shows an html view

Description:
Uses the HtmlView to show the site www.microsoft.com in the result pane.

=======================================================================================
	
Initialization Wizard Sample - (**)
Purpose: 
Show wizard at add snap-in to console time.

Description:
Uses the snapin .ShowInitializationWizard method to show a dialog that allows the user to enter a name which is then used in setting the root node DisplayName.

=======================================================================================
	
Localized SnapIn Sample - (***)
Purpose: 
Show how to localize the snap-in registration

Description:
Has two projects. The first defines a new resource.dll. The second uses the SnapInAbout attribute to define resources in the external dll to be used to register the snapIn.
	
=======================================================================================

Message View Sample - (*)
Purpose: 
Shows a message view

Description:
Uses the MessageView to show a simple message in the result pane.
	
=======================================================================================

Modal Dialog Sample - (**)
Purpose: 
Shows a ‘connect to’ modal dialog.

Description:
Uses the snapin Console.ShowDialog to show a simple dialog during a ‘connect to’ action.	

=======================================================================================

On Scope Node Change Sample - (***)
Purpose: 
Shows how to have Views recognize changes to ScopeNodes

Description:
Creates a Changed event on the Scope Node that Views can use to handle scope node status changes 

=======================================================================================

Persistence Sample - (*)
Purpose: 
Shows how to have MMC load and store data for a snap-in

Description:
Uses the snapin LoadCustomData and SaveCustomData methods to store and retrieve the root node’s DisplayName regardless of how it’s been renamed since. 	

=======================================================================================

Property Sheet Sample - (***)
Purpose: 
Shows how to add property pages.

Description:
Uses a MmcListView to load a list of Users. Adds a property page to the property sheet for the selected ResultNode.

=======================================================================================
	
Selection Form View Sample - (**)
Purpose: 
Show FormView multi-selection while publishing selection context

Description:
Uses a FormView containing a form with a WinForms.ListView. Adds Refresh and ShowSelected actions. 	

=======================================================================================

Selection List View Sample - (**)
Purpose: 
Show MmcListView with multi-selection.

Description:
Uses the MmcListView to display list of users. Adds Refresh and ShowSelection actions.	

=======================================================================================

Service Manager Sample - (****)
Purpose: 
Shows a complete example putting many things together.

Description:
A Complete Sample using many parts. Installs using localized handling. Uses a MmcListView to show Services on the Machine. Uses a property page to allow specifying the startup method for Services. Also has a property page for describing the service. Defines SelectionData Start, Stop, Pause, Resume actions to manage the selected service.

=======================================================================================
	
Simple SnapIn Sample - (*)
Purpose: 
Show minimal up-and-running snap-in

Description:
Creates the minimum for a Hello World snap in.	

=======================================================================================

Standard Verbs Sample - (**)
Purpose: 
Show standard verbs like cut, copy, paste, rename

Description:
Uses a MmcListView to hold a list of Users. Enables and handles the standard verbs Delete and Refresh.	

=======================================================================================

Status  Sample - (**)
Purpose: 
Shows how to use the status and update the progress bar.

Description:
Defines a new type of ScopeNode that adds 10 children to itself OnExpand one every couple of seconds. While adding, it updates the progress bar in Mmc and sets the status bar message.

=======================================================================================
 	
Threading Sample - (***)
Purpose: 
Shows how use a separate thread to expand nodes and update the status and progress bar.

Description:
Defines a new type of ScopeNode that adds 10 children to itself.  It does this by firing off a new Thread for the adding process. This adding routine then uses SnapIn.Invoke to have a delegate actually add the node.
	
=======================================================================================

View Switching Sample - (**)
Purpose: 
Shows how Views can interact with their parent scope node to switch views.

Description:
Defines a scopenode and views. The default view can be swapped out for a sample error handling view, which can then be swapped back.

=======================================================================================


