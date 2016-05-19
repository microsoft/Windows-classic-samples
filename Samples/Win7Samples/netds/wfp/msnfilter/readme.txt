Sample program that uses WFP API's to block all incoming traffic except traffic destined to msn messenger. Traffic can still
flow in based on active outbound connections, but no incoming connections can be initiated while the filters are present
The functionality in individual files is described below 

Item		Description
------------	---------------------------------------------------------------
msnfilter.cpp    Contains functions that utilize WFP API to block all incoming traffic except traffic destined to msn messenger.


Notes for running the sample:
It should be noted that the samples are meant to be run on a machine running Windows Vista (or future versions). Run msnfilter.exe.

