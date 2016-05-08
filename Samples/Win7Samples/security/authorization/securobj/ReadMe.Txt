Object Security

Notes:

 - This sample requires NT5, build 1671 (beta 1)
 - This sample may not work with other beta versions or the release version
   of NT5

The SecurObj sample demonstrates how to integrate NT security with an
application that needs to protect components or data. Some of the NT
Security concepts that this sample demonstrates:

 - Building hierarchical container/object security with automatic
   ACL inheritance using the Private Object security APIs

 - How to validate access for a client against an object

 - How to call the NT5 provided ACLUI interface for setting access
   permissions

Because of the inclusion of the ACLUI interface, this sample will run as-is
only on NT5. If you would like to run this sample on NT4, remove the ACLUI 
module and references to it in Main.cpp from the project. You will then have to
add code to view/set security on the objects.

Note to programmers: Some of you may notice that I am UI challenged. I apologize
in advance if you think my interface to the objects is clumsy, ugly, or
annoying. 
