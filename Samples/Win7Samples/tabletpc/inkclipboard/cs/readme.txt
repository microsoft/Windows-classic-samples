Ink Clipboard Sample
--------------------

This program demonstrates how to copy and paste ink into another application. It also enables the user to copy a selection of strokes and paste the result into the existing ink object.

The following clipboard modes are available:

* Ink serialized format (ISF) 
* Metafile
* Enhanced Metafile (EMF)
* Bitmap
* Text Ink
* Sketch Ink


Text ink and sketch ink are two types of ink controls used as text or drawing respectively. It is possible to paste ISF, text ink, and sketch ink into existing ink.

In addition to the clipboard, this sample also illustrates how to select strokes with the lasso tool. The user can move selected strokes and modify their drawing attributes. This functionality is a subset of the selection functionality already provided by the ink overlay control; it is implemented here for illustrative purposes.

The following features are used in this sample:

* The InkCollector object.
* Ink clipboard support.
* The use of the lasso with the HitTest method.


This sample demonstrates rendering ink, copying that ink, and then pasting the ink into another application such as Microsoft® Paint.

Note: If you have difficultly accessing the copy and paste features in this sample, you may need to install an updated version of Microsoft.Ink on your system.  See the knowledgebase article at this location for details:  http://support.microsoft.com/kb/900722/ 
