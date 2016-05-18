//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Sample Name: SQuery (VBSQuery, JSQuery, QSample)
//                - Sample Indexing Service scripts
//
//--------------------------------------------------------------------------

Description
===========
  The SQuery sample consists of three scripts that execute using Windows
  Script Host.  VBSQuery is written in VBScript and performs a simple
  query.  JSQuery is a translation of VBSQuery to JScript.  QSample is
  written in VBScript and functions similarly to the C++ sample application
  Simple (QSample) with managing and querying functionality.
  
Path
====
  Source: mssdk\samples\winbase\indexing\SQuery\
  
User's Guide
============
  * To execute queries using the VBSQuery.vbs or JSQuery.js scripts
      1. Open a command window and change the directory to the path of the
         sample scripts.
      2. Submit a fixed query by entering, at the command-line prompt,

         cscript vbsquery.vbs
         or
         cscript jsquery.js
         
  * To execute queries using the QSample.vbs script
      1. Open a command window and change the directory to the path of the
         sample scripts.
      2. Formulate a query that you know will succeed.  You need to know
         the query text and, optionally, values for the machine, catalog,
         scope, columns, query language dialect, locale,sort order, and
         other argument (see list below).
      3. Submit a query by entering, at the command-line prompt,

         cscript qsample.vbs <query> [arguments]

         where
         
         <query>         is an Indexing Service query.
         
         and the arguments can optionally include:
         
         /c:<catalog>    is the name of the catalog.
                             Default is SYSTEM.
         /e:<locale>     is ISO locale identifier, e.g. EN-US.
                             Default is system locale.
         /f:(+|-)        + or - specifies forcing use of the index.
                             Default is +.
         /g              specifies forcing a master merge.
         /i:<inputfile>  specifies an input file to read with queries,
                             one per line.
         /j              specifies to return just files in the scope path,
                             and not subdirectories.
         /l:<dialect>    specifies the query language dialect, 1 or 2.
                             Default is 1.
         /m:<machine>    is the name of the computer.
                             Default is the local computer.
         /o:<columns>    is the output column list.  Default is path.
         /p:<scope>      is the scope path of the query, absolute or relative.
         /q              specifies to execute quietly.
                             Display only query results.
         /r:#            is the number of times to repeat the command.
         /s:<sort>       is the sort column list.  Default is none.
                             For example: write[d].  Append [a] for ascending
                             (default) or [d] for descending.
         /t              specifies to display catalog statistics.
         /u              specifies to check if the catalog is up to date.
         /x:<maxhits>    is the maximum number of hits to retrieve.
                             Default is no limit.
         
Programming Notes
=================
  VBSQuery and JSQuery
  --------------------
    The query executed by these scripts is embedded in the script.  The query
    uses the Query Helper API and consists of the following.
     *  Columns = filename, directory, size, write
     *  Query = #filename *.asp
     *  GroupBy = directory[a]
     *  Catalog = system
     *  CiScope = \
     *  CiFlags = DEEP
     *  OptimizeFor = recall,hitcount
     *  AllowEnumeration = True
     *  MaxRecords = 20000

    The JSQuery is a direct translation of the VBSQuery script
    to JScript.  Its output is identical to that of VBSQuery except
    for a difference in the way dates and times are represented.

  QSample.vbs
  -----------
    QSample uses the Query Helper API and is much more general and
    flexible than the VBSQuery and JSQuery scripts.  It also uses
    the Admin Helper API to perform some managing tasks.
    
    Arguments
    ---------
      You can specify the following columns with the /o argument.
        * attrib
        * create
        * directory
        * docauthor
        * dockeywords
        * doclastauthor
        * docsubject
        * doctitle
        * fileindex
        * filename
        * hitcount
        * path
        * rank
        * size
        * vpath
        * workid
        * write

      You can specify the following locales with the /e argument.
        * af
        * ar ar-ae ar-bh ar-dz ar-eg ar-iq ar-jo ar-kw ar-lb
             ar-ly ar-ma ar-om ar-qa ar-sa ar-sy ar-tn ar-ye
        * be bg ca cs da
        * de de-at de-ch de-li de-lu
        * en en-au en-bz en-ca en-gb en-ie en-jm en-nz en-tt
             en-us en-za
        * es es-ar es-bo es-c  es-co es-cr es-do es-ec es-gt
             es-hn es-mx es-ni es-pa es-pe es-pr es-py es-sv
             es-uy es-ve
        * et eu fa fi fo
        * fr fr-be fr-ca fr-ch fr-lu
        * gd gd-ie
        * he hi hr hu in is
        * it it-ch
        * ja ji ko ko lt lv mk ms mt n neutr
        * nl-be
        * no p
        * pt pt-br
        * rm
        * ro ro-mo
        * ru ru-mo
        * s sb sk sq sr
        * sv sv-fi
        * sx sz th tn tr ts uk ur ve vi xh
        * zh-cn zh-hk zh-sg zh-tw
        * zu

    Example Queries
    ---------------
      cscript qsample.vbs mango /o:size,path
        Finds all files in the "system" catalog on the local computer that
        contain the word "mango" and outputs the size and path values.
      cscript qsample.vbs "peach and not apple"" /s:rank[d] /p:.
        Finds all files in the "system" catalog on the local computer with
        the relative path "." that contain the word "peach" but not the
        word "apple" and outputs the path value sorted in order of
        increasing rank.
      cscript qsample.vbs "@size > 1000000"" /o:size,path /s:size[a] /m:dogfood
        Finds all files in the "system" catalog on the computer "dogfood"
        whose size is greater than 1000000 bytes and outputs the size and
        path values sorted in order of increasing size.
      cscript qsample.vbs "@docauthor joe"" /o:docauthor,path /s:docauthor,path
        Finds all files in the "system" catalog on the local computer whose
        docauthor property is "joe" and outputs the docauthor and path
        values sorted in order of ascending docauthor and then ascending
        path.    
      cscript qsample.vbs apricot /p:c:\\files
        Finds all files in the "system" catalog on the local computer with
        an absolute scope of c:\files and outputs the path in unsorted order.
      cscript qsample.vbs /m:index1 /c:sources pear
        Finds all files in the "sources" catalog on the computer "index1"
        containing the word "pear" and outputs the path value in unsorted
        order.

