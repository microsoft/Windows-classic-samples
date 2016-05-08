SchemaReader sample

This sample creates a tool that uses the Windows Media Player object model to retrieve and display information about metadata in the Windows Media Player library or a digital media file. The tool can save the results to a text file.

The main functionality the tool provides is to enumerate the metadata attribute names stored in the library for each supported schema (audio, video, playlist, photo, and other). The tool does this by finding an entry for a file from the given schema, counting the available attributes, and then retrieving the name, value, and writability for each attribute. This means that you must have a file of each schema type added to your library. If no file exists for a given schema, the tool cannot enumerate the attributes, so it displays an error message. 

The tool can also provide information about available attributes for playlists in the playlist collection (which are treated differently than playlists retrieved as media items), CD tracks and CD table of contents, DVD titles, chapters, and DVD table of contents, and individual media files (set as current media).

Using the tool

To use the tool, simply compile and run the progam. The tool automatically generates the Attributes by type output, which is the first option in the Capture menu. This is a query from the library for attribute names, representative values (if present), and writability (is the attribute read only?). To write the output to a text logging file, click File and then Save or Save as. By default, the log contains the schema names and attribute names. The Log options checkboxes enable you to customize the logging output by including writability and values.

To capture CD attributes, insert a CD into a drive and wait for it to load. Choose the letter of the drive into which you inserted the CD by using the CD/DVD Drive combo box. Use the Capture menu to enumerate CD table of contents or CD track attributes.

Capturing DVD attributes can be a bit tricky. Different DVDs can be authored differently. Usually, you can simply use the Capture menu commands to view table of contents or chapter information. You can use the DVD Menus buttons to navigate the DVD to its top menu or title menu, if they exist. For example, navigating to the title menu and then changing titles might enable you to view title attributes.

To view attributes for a particular digital media file or a playlist file, such as an ASX or WPL, use the Open media command from the File menu to browse to the file you want to inspect. Once the file is playing, use the capture menu to view the current media attributes or current playlist attributes. The resulting output will show you attributes present in the file and in the library for the current file.

By default, the tool uses the local library retrieved by using the mediaCollection property from the IWMPCore interface. You can choose to view results from the local library retrieved by using IWMPLibraryServices. To do this, uncheck the Use legacy library checkbox.

Copyright (c) Microsoft Corporation. All rights reserved.