EnumDesk Sample 

The EnumDesk sample demonstrates how to use the shell APIs and interfaces to enumerate and perform operations on shell objects. 

The EnumDesk sample has a user interface that is similar to Windows Explorer. The shell folders are displayed in a hierarchical tree on the left side while the contents of the selected folder are displayed in a list on the right side. The user can right-click on an item in the tree or the list to display the item's context menu and select any of the options in the menu. In the list, the user can also double-click an item to perform the default action of that item.

It is important to note that this sample does not implement a shell browser, so some operations, such as opening or exploring a folder, will cause a new Explorer window to be created. This is because these operations are normally just a navigation change within the current browser. Because this sample does not implement a browser, the actions are performed in a new window.

Another limitation is caused by the fact that the sample does not actually host the shell view. It instead only enumerates the items within a folder for display purposes. Because a shell view is not actually hosted, it is not possible to properly implement a details view because the header labels and contents of the sub-columns are not available through any exposed functions or interfaces.
