Using the XPS Object Model to combine XPS documents


SUMMARY
=======

The XpsRollup sample is a simple utility which combines multiple XPS documents
into a single document.  It handles renaming of resources to avoid part name
conflicts.

MORE INFORMATION
================

Usage
-----

XpsRollup.exe is a console application.  It takes as the first argument a
filename for the new XPS file to be created.  Each argument thereafter is the
filename of an XPS document to be combined.  Each input document is opened in
the order specified on the command line, and all pages in the document are added
to the output document along with their required resources.
