//
//  memctrnm.h
//
//  Offset definition file for exensible counter objects and counters
//
//  These "relative" offsets must start at 0 and be multiples of 2 (i.e.
//  even numbers). In the Open Procedure, they will be added to the 
//  "First Counter" and "First Help" values fo the device they belong to, 
//  in order to determine the  absolute location of the counter and 
//  object names and corresponding help text in the registry.
//
//  this file is used by the extensible counter DLL code as well as the 
//  counter name and help text definition file (.INI) file that is used
//  by LODCTR to load the names into the registry.
//
#define APPMEMOBJ       0
#define APPMEMALLOC     2
#define APPALLOCS       4
#define APPALLOCSSEC    6
#define APPREALLOCS     8
#define APPREALLOCSSEC  10
#define APPFREES        12
#define APPFREESSEC     14
