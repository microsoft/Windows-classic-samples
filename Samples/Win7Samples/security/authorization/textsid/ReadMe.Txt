Textual Representation of a SID


SUMMARY
=-=====

The TEXTSID sample demonstrates how to obtain the textual representation
of a binary SID. This is useful when it is not appropriate to obtain the
name associated with the SID, or, when the network is not available to obtain
such information. The Windows NT Event Viewer displays SIDs in textual form
when it cannot look up the user name. Furthermore, lookup of local user
profiles is facilitated through conversion of the user SID to textual form.

This sample obtains the SID of the current user and then displays the SID
in textual notation.

MORE INFORMATION
================

A standardized shorthand notation for SIDs makes it simpler to visualize their
components:

   S-R-I-S-S...

In the notation shown above,

   S identifies the series of digits as an SID,
   R is the revision level,
   I is the identifier-authority value,
   S is subauthority value(s).

An SID (representing the local administrators group) could be written in this
notation as follows: S-1-5-32-544

In this example,

   the SID has a revision level of 1,
   an identifier-authority value of 5,
   first subauthority value of 32,
   second subauthority value of 544.

The GetTextualSid function converts the binary SID to a textual string.

The resulting string takes one of two forms. If the IdentifierAuthority value
is less than 2^32, then the SID is in the form:

   S-1-5-21-2127521184-1604012920-1887927527-19009
     ^ ^ ^^ ^^^^^^^^^^ ^^^^^^^^^^ ^^^^^^^^^^ ^^^^^
     | | |      |          |          |        |
     +-+-+------+----------+----------+--------+--- Decimal

Otherwise it will take the form:

   S-1-0x206C277C6666-21-2127521184-1604012920-1887927527-19009
     ^ ^^^^^^^^^^^^^^ ^^ ^^^^^^^^^^ ^^^^^^^^^^ ^^^^^^^^^^ ^^^^^
     |       |        |      |          |          |        |
     |   Hexidecimal  |      |          |          |        |
     +----------------+------+----------+----------+--------+--- Decimal
