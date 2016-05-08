// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __LIST_H__
#define __LIST_H__
                   /*----------------------
                   |         List          |
                    ----------------------*/

/* worth also looking at nt\public\sdk\inc\ntrtl.h which also has some
|  low level list pointer chaining stuff in it
*/
/* Note here that Modula-2 style comments (*like this*) are used
   within examples which are already within C comments to indicate
   where comments should go in the examples
*/

/*------------------------------------------------------------------------
| Abstract data type LIST OF (*untyped*) object.
| Different lists can have different types of object in them
| Different items in a list can have different types of object in them.
| The price of this lack of typing is that you have a slightly more
| awkward syntax and you get no help from the compiler if you try to
| put the wrong type of data into the list.
|
| The list is implemented as a collection of items.  Within the item
| somewhere is the object.
|
| Objects are stored UNALIGNED within items.
|
| Use:
|
|   #include <list.h>
|   . . .
|   LIST MyList; (* or LIST liMyList for Hungarians *)
|   . . .
|   MyList = List_Create();
|   List_AddLast(MyList,&MyObject,sizeof(OBJECT));
|
| In the abstract a LIST is a list of objects.  The representation
| is a linked collection of items.  The manner of the linking is
| implementation dependent (as I write this it's linear but when you
| read it it might be a tree (See Knuth for why a tree)).
|
| A LIST is a "handle" for a list which may be thought of as a POINTER
| (whether it is really a pointer or not is implementation dependent)
| so that it can be copied at the risk of creating an alias. e.g.
|
|   L = List_Create();
|   L1 = L;             (* L and L1 are both healthy and empty *)
|   List_AddFirst(L, &elem, sizeof(elem));
|   (* L1 may also appear to have one object, there again it may be sick *)
|   L1 = L;               (* Now they both surely see the one element *)
|   List_Destroy(&L1);    (* L is almost certainly sick now too *)
|   L1 = List_Create();   (* All bets off as to what L is like now
|                            but L1 is empty and healthy
|                         *)
|
| If two handles compare equal then the lists must be equal, but
| unequal handles could address two similar lists i.e. the same list
| of objects held in two different LISTs of items (like pointers).
|
| A LIST can be transferred from one variable to another like this:
|
|   NewList = OldList;           (* copy the handle *)
|   OldList = List_Create();     (* kill the old alias *)
|
| and the Create statement can be omitted if OldList is never touched again.
|
| Items are identified by Cursors.  A cursor is the address of an object
| within an item in the list. i.e. it is the address of the piece of your
| data that you had inserted.  (It is probably NOT the address of the item).
| It is typed as pointer to void here, but you should declare it as a pointer
| to whatever sort of object you are putting in the LIST.
|
| The operations AddFirst, AddLast, AddAfter and AddBefore
| all copy elements by direct assignment.  If an element is itself
| a complex structure (say a tree) then this will only copy a pointer
| or an anchor block or whatever and give all the usual problems of
| aliases.  Clear will make the list empty, but will only free the
| storage that it can "see" directly.  SplitBefore or Split After may
| also perform a Clear operation.  To deal with fancy data structures
| use New rather than Add calls and copy the data yourself
|   e.g.  P = List_NewLast(MyList, sizeof(MyArray[14])*(23-14+1));
|         CopyArraySlice(P, MyArray, 14, 23);
|
| The operations NewFirst, NewLast, NewAfter, NewBefore, First and Last
| all return pointers to elements and thus allow you to do any copying.
| This is how you might copy a whole list of fancy structures:
|
|    void CopyFancyList(LIST * To, LIST From)
|             (* Assumes that To has been Created and is empty *)
|    { PELEMENT Cursor;
|      PELEMENT P;
|
|      List_TRAVERSE(From, Cursor);
|      { P = List_NewLast(To, sizeof(element) );
|        FancyCopy(P, Cursor);    (* Copy so that *Cursor==*P afterwords *)
|      }
|    }
 --------------------------------------------------------------------*/

  typedef struct item_tag FAR * LIST;
  typedef LIST FAR * PLIST;

  void APIENTRY List_Init(void);
  /* MUST BE CALLED BEFORE ANY OF THE OTHER FUNCTIONS. Don't ask, just do it */

  void APIENTRY List_Term(void);
  /* Call at end of application (does some checking and resource freeing) */

  void APIENTRY List_Dump(LPSTR Header, LIST lst);
  /* Dump the internals to current output stream -- debug only */

  void APIENTRY List_Show(LIST lst);
  /* Dump hex representation of handle to current out stream -- debug only */

  LIST APIENTRY List_Create(void);
  /* Create a list.  It will be initially empty */

  void APIENTRY List_Destroy(PLIST plst);
  /* Destroy *plst.  It does not need to be empty first.
  |  All storage directly in the list wil be freed.
  */


  LPVOID APIENTRY List_NewFirst(LIST lst, UINT uLen);
  /* Return the address of the place for Len bytes of data in a new
  |  item at the start of *plst.
  |  The storage is zeroed BEFORE chaining it in.
  */

  void APIENTRY List_DeleteFirst(LIST lst);
  /* Delete the first item in lst.  Error if lst is empty */


  LPVOID APIENTRY List_NewLast(LIST lst, UINT uLen);
  /* Return the address of the place for uLen bytes of data in a new
  |  item at the end of lst
  |  The storage is zeroed BEFORE chaining it in.
  */

  void APIENTRY List_DeleteLast(LIST lst);
  /* Delete the last item in lst.  Error if lst is empty */

  void APIENTRY List_AddAfter( LIST lst
                    , LPVOID Curs
                    , LPVOID pObject
                    , UINT uLen
                    );
  /*--------------------------------------------------------------------
  | Add an item holding *pObject to lst immediately after Curs.
  | List_AddAfter(lst, NULL, pObject, Len) adds it to the start of the lst
   ---------------------------------------------------------------------*/

  LPVOID APIENTRY List_NewAfter(LIST lst, LPVOID Curs, UINT uLen);
  /*--------------------------------------------------------------------
  | Return the address of the place for uLen bytes of data in a new
  | item immediately after Curs.
  | List_NewAfter(Lst, NULL, uLen) returns a pointer
  | to space for uLen bytes in a new first element.
  | The storage is zeroed BEFORE chaining it in.
   ---------------------------------------------------------------------*/

  void APIENTRY List_AddBefore( LIST lst
                     , LPVOID Curs
                     , LPVOID pObject
                     , UINT uLen
                     );
  /*--------------------------------------------------------------------
  | Add an item holding Object to lst immediately before Curs.
  | List_AddBefore(Lst, NULL, Object, uLen) adds it to the end of the list
   ---------------------------------------------------------------------*/

  LPVOID APIENTRY List_NewBefore(LIST lst, LPVOID Curs, UINT uLen );
  /*--------------------------------------------------------------------
  | Return the address of the place for uLen bytes of data in a new
  | item immediately before Curs.
  | List_NewBefore(Lst, NULL, uLen) returns a pointer
  | to space for uLen bytes in a new last element.
  | The storage is zeroed BEFORE chaining it in.
   ---------------------------------------------------------------------*/

#if 0
// these functions are not actually defined...

  void APIENTRY List_DeleteAndNext(LPVOID * pCurs);
  /* Delete the item that *pCurs identifies and move *pCurs to the Next item */

  void APIENTRY List_DeleteAndPrev(LPVOID * pCurs);
  /* Delete the item that *pCurs identifies and move *pCurs to the Prev item */
#endif

  void APIENTRY List_Delete(LPVOID Curs);
  /*------------------------------------------------------------------
  | Delete the item that Curs identifies.
  | I'm not too sure about this:
  | This will be only a few (maybe as little as 3) machine instructions
  | quicker than DeleteAndNext or DeleteAndPrev but leaves Curs dangling.
  | It is therefore NOT usually to be preferred.
  | It may be useful when you have a function which returns an LPVOID
  | since the argument does not need to be a variable.
  |     Trivial example: List_Delete(List_First(L));
  | I am not sure which is more damaging, a dangling pointer which points
  | at garbage or one that points at something that is real live data.
   -------------------------------------------------------------------*/

  int APIENTRY List_ItemLength(LPVOID Curs);
  /* Return the length of the object identified by the cursor Curs */

  /*------------------------------------------------------------------
  | TRAVERSING THE ULIST
  |
  | LIST lst;
  | object * Curs;
  | . . .
  | Curs = List_First(lst);
  | while (Curs!=NULL)
  | {  DoSomething(*Curs);   (* Curs points to YOUR data not to chain ptrs *)
  |    Curs = List_Next(Curs);
  | }
  |
  | This is identically equal to
  | List_TRAVERSE(lst, Curs)  // note NO SEMI COLON!
  | {  DoSomething(*Curs); }
   -------------------------------------------------------------------*/

  #define List_TRAVERSE(lst, curs)  for(  curs=List_First(lst)            \
                                       ;  curs!=NULL                      \
                                       ;  curs = List_Next((LPVOID)curs)  \
                                       )
  #define List_REVERSETRAVERSE(lst, curs)  for(  curs=List_Last(lst)             \
                                              ;  curs!=NULL                      \
                                              ;  curs = List_Prev((LPVOID)curs)  \
                                              )

  LPVOID APIENTRY List_First(LIST lst);
  /*------------------------------------------------------------------
  | Return the address of the first object in lst
  |  If lst is empty then Return NULL.
  --------------------------------------------------------------------*/

  LPVOID APIENTRY List_Last(LIST lst);
  /*------------------------------------------------------------------
  | Return the address of the last object in lst
  | If lst is empty then return NULL.
  --------------------------------------------------------------------*/

  LPVOID APIENTRY List_Next(LPVOID Curs);
  /*------------------------------------------------------------------
  | Return the address of the object after Curs^.
  | List_Next(List_Last(lst)) == NULL;  List_Next(NULL) is an error.
  | List_Next(List_Prev(curs)) is illegal if curs identifies first el
  --------------------------------------------------------------------*/

  LPVOID APIENTRY List_Prev(LPVOID Curs);
  /*------------------------------------------------------------------
  | Return the address of the object after Curs^.
  | List_Prev(List_First(L)) == NULL;  List_Prev(NULL) is an error.
  | List_Prev(List_Next(curs)) is illegal if curs identifies last el
  --------------------------------------------------------------------*/

  /*------------------------------------------------------------------
  |  Whole list operations
   -----------------------------------------------------------------*/
  void APIENTRY List_Clear(LIST lst);
  /* arrange that lst is empty after this */

  BOOL APIENTRY List_IsEmpty(LIST lst);
  /* Return TRUE if and only if lst is empty */

  void APIENTRY List_Join(LIST l1, LIST l2);
  /*-----------------------------------------------------------------------
  | l1 := l1||l2; l2 := empty
  | The elements themselves are not moved, so pointers to them remain valid.
  |
  | l1 gets all the elements of l1 in their original order followed by
  | all the elements of l2 in the order they were in in l2.
  | l2 becomes empty.
   ------------------------------------------------------------------------*/

  void APIENTRY List_InsertListAfter(LIST l1, LIST l2, LPVOID Curs);
  /*-----------------------------------------------------------------------
  | l1 := l1[...Curs] || l2 || l1[Curs+1...]; l2 := empty
  | Curs=NULL means insert l2 at the start of l1
  | The elements themselves are not moved, so pointers to them remain valid.
  |
  | l1 gets the elements of l1 from the start up to and including the element
  | that Curs points at, in their original order,
  | followed by all the elements that were in l2, in their original order,
  | followed by the rest of l1
   ------------------------------------------------------------------------*/

  void APIENTRY List_InsertListBefore(LIST l1, LIST l2, LPVOID Curs);
  /*-----------------------------------------------------------------------
  | l1 := l1[...Curs-1] || l2 || l1[Curs...]; l2 := empty
  | Curs=NULL means insert l2 at the end of l1
  | The elements themselves are not moved, so pointers to them remain valid.
  |
  | l1 gets the elements of l1 from the start up to but not including the
  | element that Curs points at, in their original order,
  | followed by all the elements that were in l2, in their original order,
  | followed by the rest of l1.
   ------------------------------------------------------------------------*/

  void APIENTRY List_SplitAfter(LIST l1, LIST l2, LPVOID Curs);
  /*-----------------------------------------------------------------------
  | Let l1 be l1 and l2 be l2
  | Split l2 off from the front of l1:    final l2,l1 = original l1
  |
  | Split l1 into l2: objects of l1 up to and including Curs object
  |               l1: objects of l1 after Curs
  | Any original contents of l2 are freed.
  | List_Spilt(l1, l2, NULL) splits l1 before the first object so l1 gets all.
  | The elements themselves are not moved.
   ------------------------------------------------------------------------*/

  void APIENTRY List_SplitBefore(LIST l1, LIST l2, LPVOID Curs);
  /*----------------------------------------------------------------------
  | Split l2 off from the back of l1:  final l1,l2 = original l1
  |
  | Split l1 into l1: objects of l1 up to but not including Curs object
  |               l2: objects of l1 from Curs onwards
  | Any original contants of l2 are freed.
  | List_Spilt(l1, l2, NULL) splits l1 after the last object so l1 gets all.
  | The elements themselves are not moved.
   -----------------------------------------------------------------------*/

  int APIENTRY List_Card(LIST lst);
  /* Return the number of items in L */

  /*------------------------------------------------------------------
  | Error handling.
  |
  | Each list has within it a flag which indicates whether any illegal
  | operation has been detected (e.g. DeleteFirst when empty).
  | Rather than have a flag on every operation, there is a flag held
  | within the list that can be queried when convenient.  Many operations
  | do not have enough redundancy to allow any meaningful check.  This
  | is a design compromise (for instance to allow P = List_Next(P);
  | rather than P = List_Next(L, P); which is more awkward, especially
  | if L is actually a lengthy phrase).
  |
  | List_IsOK tests this flag (so is a very simple, quick operation).
  | MakeOK sets the flag to TRUE, in other words to accept the current
  | state of the list.
  |
  | It is possible for a list to be damaged (whether or not the flag
  | says OK) for instance by the storage being overwritten.
  |
  | List_Check attempts to verify that the list is sound (for instance where
  | there are both forward and backward pointers they should agree).
  |
  | List_Recover attempts to make a sound list out of whatever debris is left.
  | If the list is damaged, Recover may trap (e.g. address error) but
  | if the list was damaged then ANY operation on it may trap.
  | If Check succeeds without trapping then so will Recover.
   -----------------------------------------------------------------*/

  BOOL APIENTRY List_IsOK(LIST lst);
  /* Check return code */

  void APIENTRY List_MakeOK(LIST lst);
  /* Set return code to good */

  BOOL APIENTRY List_Check(LIST lst);
  /* Attempt to validate the chains */

  void APIENTRY List_Recover(PLIST plst);
  /* Desperate stuff.  Attempt to reconstruct something */

/*------------------------------------------------------------------
|  It is designed to be as easy to USE as possible, consistent
|  only with being an opaque type.
|
|  In particular, the decision to use the address of an object a list cursor
|  means that there is a small amount of extra arithmetic (in the
|  IMPLEMENTATION) in cursor operations (e.g. Next and Prev).
|  and spurious arguments are avoided whenever possible, even though
|  it would allow greater error checking.
|
| Of the "whole list" operations, Clear is given because it seems to be
| a common operation, even though the caller can implement it with almost
| the same efficiency as the List implementation module.
| Join, Split and InsertListXxx cannot be implemented efficiently without
| knowing the representation.
 --------------------------------------------------------------------*/
#endif
