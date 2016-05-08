Sample program that uses WFP diagnostic API to print diagnostic events. The functionality in individual files is described below 

Item		Description
------------	---------------------------------------------------------------
diagevents.c    Contains functions that utilize WFP Diagnostic API to enumerate events
WfpEventUtil.c  Contains utility functions for managing program input through command line arguments and printing program output in form of diagnostic event details.
WfpEventUtil.h  Contains function declarations implemented by utilized WfpEventUtil.c and utilized by diagevents.c

Notes for running the sample:
It should be noted that the samples are meant to be run on a machine running Windows Vista (or future versions). Run "diagevents.exe -?" for more usage options.

