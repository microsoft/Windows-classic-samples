WMPML Sample

This C++ sample demonstrates how to use the library functionality of the Windows Media Player object model to create a user interface.

The sample creates the ActiveX control with its user interface hidden. The Player control is remoted. Although this sample does not write metadata to the library, applications that write metadata should use a remoted control instance to ensure that media items in the library can update file-level metadata.

The Schema combo box enables you to view media from a particluar schema, such as audio or video.

The Library combo box enables you to view media from a particular library. The sample supports libraries from sources other than the local user, such as libraries available on the network, on discs, or on connected portable devices.

The tree view control is similar to the library user interface in the Player. The Media list box shows the media items in the currently selected tree node. 

You can click Advanced Query in the tree view control to create a custom query. The sample displays a dialog box that enables you to create custom queries by following a series of steps. To do this, use the combo box controls to create the query condition and use the text box control to type a value for the condition. Then, click Add Condition to add a condition to the list. You can repeat this process to add multiple conditions, which are appended using AND logic. You can start a new condition group by clicking Add Group. Condition groups are appended to the query using OR logic.

When you are satisfied with your query, click Show StringCollection to see the results.


Note that this sample uses ATL. This means you must install Microsoft Visual Studio to compile this sample.

Copyright (c) Microsoft Corporation. All rights reserved.
