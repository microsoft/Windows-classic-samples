This demo illustrates the use of the Authorization Manager store management
interfaces by implementing a store migration tool. Such a tool could be used to
migrate a store from one type to another, or could be used to install a store
of any given type from an xml store that ships with an application.

Usage:

AzMigrate <destination store> <source store> [flags]

Flags:
 /o : If destination store exists, it would be overwritten

 /a=[application name1,application name2,....] : Migrate specified
  applications only to destination store

 /l=[log file name] : Log all the operations performed during migration
  into specified log file

 /ip : Ignore all Policy assignments
 /im : Ignore all members

 /v : Verbose mode

 /? : Help
