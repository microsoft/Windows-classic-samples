'+---------------------------------------------------------------------
'
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
' PARTICULAR PURPOSE.
'
' Copyright (c) Microsoft Corporation. All rights reserved.
'
' SCRIPT:   QSample.vbs
'
' PURPOSE:  Illustrates using VBScript, Query Helper API, and Admin
'           Helper API with Windows Scripting Host to query Indexing
'           Service and to determine the catalog status.  The script
'           functions similarly to the C++ Simple(QSample) sample.
'
' PLATFORM: Windows 2000
'
'----------------------------------------------------------------------

main

'----------------------------------------------------------------------
'
' Main entry point -- parses the arguments and drives the query
'

sub Main

    if WScript.Arguments.Count < 1 then call Usage end if

    ' set defaults for all arguments

    query = ""
    catalog = "system"
    locale = ""
    forceci = TRUE
    forcemerge = FALSE
    inputfile = ""
    shallow = FALSE
    dialect = 1
    machine = "."
    columns = "path"
    scope = "\"
    quiet = FALSE
    sort = ""
    groupby = ""
    stats = FALSE
    uptodate = FALSE
    maxhits = 0
    firsthits = FALSE
    repeat = 1

    ' parse command line arguments
    
    for i = 0 to WScript.Arguments.Count - 1

        arg = WScript.Arguments( i )
        first = left( arg, 1 )
        c = mid( arg, 2, 1 )

        if "/" = first or "-" = first then

            if ":" = mid( arg, 3, 1 ) then
    
                v = mid( arg, 4 )

                select case c
                    case "a" groupby = v
                    case "c" catalog = v
                    case "e" locale = v
                    case "f" forceci = ( v = "+" )
                    case "i" inputfile = v
                    case "l" dialect = v
                    case "m" machine = v
                    case "o" columns = v
                    case "p" scope = v
                    case "r" repeat = v
                    case "s" sort = v
                    case "x" maxhits = v
                    case "y" maxhits = v
                             firsthits = TRUE
                    case else Usage
                end select

            else

                select case c
                    case "g" forcemerge = TRUE
                    case "j" shallow = TRUE
                    case "q" quiet = TRUE
                    case "t" stats = TRUE
                    case "u" uptodate = TRUE
                    case else Usage
                end select

            end if

        else

            if "" = query then query = arg else Usage

        end if
    
    next

    ' Turn a relative scope path into an absolute path

    if "\" <> scope and "\\" <> left( scope, 2 ) then

        set fso = WScript.CreateObject( "Scripting.FileSystemObject" )
        scope = fso.GetAbsolutePathName( scope )

    end if

    for i = 1 to repeat

        if "" = inputfile then

            if "" = query and not ( stats or uptodate or forcemerge ) then
                Usage
            end if

            DoQuery query, catalog, locale, forceci, forcemerge, shallow, dialect, machine, columns, scope, quiet, sort, stats, uptodate, maxhits, firsthits, groupby

        else

            if "" <> query then call Usage

            ' Open the input file and treat each line as a query.
            ' Report errors, but don't stop reading queries.

            set fs = WScript.CreateObject( "Scripting.FileSystemObject" )
            set f = fs.OpenTextFile( inputfile, 1 )

            do until f.AtEndOfStream

                line = f.ReadLine
                on error resume next
     
                DoQuery line, catalog, locale, forceci, forcemerge, shallow, dialect, machine, columns, scope, quiet, sort, stats, uptodate, maxhits, firsthits, groupby

                if 0 <> Err.Number then

                    out Err.Description
                    out "The query '" & line & "' failed, error 0x" & Hex( Err.Number )
                    Err.Clear

                end if

                out ""

            loop

        end if

    next

end sub

'----------------------------------------------------------------------
'
' Echos a string to the output
'

sub Out( str )

    WScript.echo str

end sub

'----------------------------------------------------------------------
'
' Echos a number and a string to the output
'

sub Out2( num, str )

    out right( space( 9 ) & num, 9 ) & " " & str

end sub

'----------------------------------------------------------------------
'
' Displays usage information about the program
'

sub Usage

    out "usage: cscript qsample.vbs <query> [arguments]"
    out "    <query>       An Indexing Service query."
    out "  arguments:"
    out "    /a:groupby    Columns over which results are grouped."
    out "    /c:catalog    Name of the catalog, default is SYSTEM."
    out "    /e:locale     ISO locale identifier, e.g. EN-US; default is system locale."
    out "    /f:(+|-)      + or -, for force use of index.  Default is +."
    out "    /g            Force a master merge."
    out "    /i:inputfile  Text input file with queries, one per line."
    out "    /j            Just return files in the scope path, and not subdirectories."
    out "    /l:dialect    1 or 2, for old or new dialect, default is 1."
    out "    /m:machine    Name of the machine, default is local machine."
    out "    /o:columns    Output column list, default is path."
    out "    /p:scope      The scope path of the query, absolute or relative."
    out "    /q            Quiet.  Don't display info other than query results."
    out "    /r:#          Number of times to repeat the command."
    out "    /s:sort       Sort column list, default is none.  e.g.: write[d]."
    out "                  Append [a] for ascending (default) or [d] for descending."
    out "    /t            Display catalog statistics."
    out "    /u            Check if the catalog is up to date."
    out "    /x:maxhits    Maximum number of hits to retrieve, default is no limit."
    out "    /y:firsthits  Only look at the first N hits."
    out ""
    out "  examples: cscript qsample.vbs mango /o:size,path"
    out "            cscript qsample.vbs ""peach and not apple"" /s:rank[d] /p:."
    out "            cscript qsample.vbs ""@size > 1000000"" /o:size,path /s:size[a] /m:dogfood"
    out "            cscript qsample.vbs ""@docauthor joe"" /o:docauthor,path /s:docauthor,path"
    out "            cscript qsample.vbs apricot /p:c:\\files"
    out "            cscript qsample.vbs /m:index1 /c:sources pear"
    out ""
    out "  columns: path vpath directory filename write create size attrib"
    out "           rank hitcount workid fileindex"
    out "           docauthor doclastauthor dockeywords docsubject doctitle"
    out ""
    out "  locales: af ar-ae ar-bh ar-dz ar-eg ar-iq ar-jo ar-kw ar-lb"
    out "           ar-ly ar-ma ar-om ar-qa ar-sa ar-sy ar-tn ar-ye be"
    out "           bg ca cs da de de-at de-ch de-li de-lu e en en"
    out "           en-au en-bz en-ca en-gb en-ie en-jm en-nz en-tt"
    out "           en-us en-za es es es-ar es-bo es-c es-co es-cr"
    out "           es-do es-ec es-gt es-hn es-mx es-ni es-pa es-pe"
    out "           es-pr es-py es-sv es-uy es-ve et eu fa fi fo fr"
    out "           fr-be fr-ca fr-ch fr-lu gd gd-ie he hi hr hu in"
    out "           is it it-ch ja ji ko ko lt lv mk ms mt n neutr"
    out "           nl-be no no p pt pt-br rm ro ro-mo ru ru-mo s sb"
    out "           sk sq sr sr sv sv-fi sx sz th tn tr ts uk ur ve"
    out "           vi xh zh-cn zh-hk zh-sg zh-tw zu"

    WScript.Quit( 2 )

end sub

'----------------------------------------------------------------------
'
' Formats a value given its datatype
'

function FormatValue( v, t )

    if 7 = t or 137 = t then
        w = 21
    elseif 2 = t or 3 = t or 4 = t or 5 = t or 14 = t or 17 = t or 18 = t or 19 = t then
        w = 7
    elseif 20 = t or 21 = t then
        w = 12
    else
        w = 0
    end if

    if 0 = w then
        r = v
    else
        r = right( space( w ) & v, w )
    end if

    FormatValue = r

end function

'----------------------------------------------------------------------
'
' Displays a grouped (categorized) rowset and its child rowsets
' Recursive -- an argument is the level of the rowset
'

function DisplayGroupedRowset( rs, level )

    const cRowsToGet = 20
    rs.CacheSize = cRowsToGet
    cHits = 0

    do until rs.EOF

        row = ""
        fChild = FALSE

        for c = 0 to rs.Fields.Count - 1

            if rs( c ).Name <> "Chapter" then
                row = row & "  " & rs( c ).Value
            else
                set rsChild = rs.Fields( "Chapter" ).Value
                fChild = TRUE
            end if

        next

        out space( level * 2 ) & row
        cHits = cHits + 1

        if fChild then
            x = DisplayGroupedRowset( rsChild, level + 1 )
            rsChild.Close
            set rsChild = nothing
        end if

        rs.MoveNext

    loop

    DisplayGroupedRowset = cHits

end function

'----------------------------------------------------------------------
'
' Displays the contents of a rowset
'

function DisplayRowset( rs )

    ' Display the results, 20 rows at a time for performance

    const cRowsToGet = 20
    rs.CacheSize = cRowsToGet
    cHits = 0

    do until rs.EOF

         rows = rs.GetRows( cRowsToGet )

         for r = 0 to UBound( rows, 2 )

            row = ""

            for col = 0 to UBound( rows, 1 )

                if 0 <> col then row = row & "  "
                row = row & FormatValue( rows( col, r ), rs( col ).type )

            next

            out row
            cHits = cHits + 1

         next

    loop

    DisplayRowset = cHits

end function

'----------------------------------------------------------------------
'
' Issues a query given the parameters
'

sub DoQuery( query, catalog, locale, forceci, forcemerge, shallow, dialect, machine, columns, scope, quiet, sort, stats, uptodate, maxhits, firsthits, groupby )

    if "" <> query then

        ' Create the query object and set the query properties

        set q = WScript.CreateObject( "ixsso.Query" )
        q.Query = query
        q.Catalog = "query://" & machine & "/" & catalog
        q.AllowEnumeration = not forceci
        q.Dialect = dialect
        q.Columns = columns
        q.CiScope = scope

        if shallow then q.CiFlags = "shallow"

        if "" <> sort then q.SortBy = sort
        if "" <> groupby then q.GroupBy = groupby

        ' FirstRows is only supported on Windows XP and later versions, though

        if 0 <> maxhits then
            if firsthits then
                q.FirstRows = maxhits
            else
                q.MaxRecords = maxhits
            end if
        end if

        if "" <> locale then

            set u = WScript.CreateObject( "ixsso.Util" )
            q.LocaleId = u.ISOToLocaleID( locale )

        end if

        set rs = q.CreateRecordSet( "sequential" )

        if "" <> groupby then
            cHits = DisplayGroupedRowset( rs, 0 )
        else
            cHits = DisplayRowset( rs )
        end if

        ' Display query status information

        if not quiet then

            out CHR(10) & cHits & " files matched the query '" & query & "'"

            if q.OutOfDate then
                out "The index is out of date"
            end if

            if q.QueryTimedOut then
                out "The query timed out"
            end if

            if q.QueryIncomplete then
                out "The query results are incomplete; may require enumeration"
            end if

        end if

    end if

    ' Display catalog status information and/or force a merge

    if stats or uptodate or forcemerge then

        set a = WScript.CreateObject( "microsoft.ISAdm" )
        a.MachineName = machine
        set c = a.GetCatalogByName( catalog  )

        if forcemerge then c.ForceMasterMerge

        if stats then

            out "Machine:  " & machine
            out "Catalog:  " & catalog
            out "Location: " & c.CatalogLocation
            out2 c.TotalDocumentCount, "Documents in the catalog"
            out2 c.FreshTestCount, "Documents modified since the last master merge"
            out2 c.FilteredDocumentCount, "Documents filtered since the service started"
            out2 c.DocumentsToFilter, "Documents to filter"
            out2 c.DelayedFilterCount, "Documents deferred for filtering"
            out2 c.UniqueKeyCount, "Unique keys in the master index"
            out2 c.WordListCount, "Wordlists"
            out2 c.PersistentIndexCount, "Saved indexes"
            out2 c.QueryCount, "Queries executed since the service started"
            out2 c.IndexSize, "Megabytes used for index files"
            out2 c.PendingScanCount, "Scans scheduled"

            s = c.StateInfo

            if s and    &H1 then out "Shadow merge " & c.PctMergeComplete & "% complete"
            if s and    &H2 then out "Master merge " & c.PctMergeComplete & "% complete"
            if s and    &H8 then out "Annealing merge " & c.PctMergeComplete & "% complete"
            if s and   &H20 then out "Recovery in progress..."
            if s and   &H80 then out "Indexing paused due to low memory"
            if s and  &H100 then out "Indexing paused due to high system I/O"
            if s and  &H400 then out "Catalog is read-only"
            if s and  &H800 then out "Indexing paused due to running on battery power"
            if s and &H1000 then out "Indexing paused due to busy interactive user"
            if s and &H2000 then out "Indexing service is starting..."
            if s and &H4000 then out "Reading the NTFS USN log(s)"

        end if

        if uptodate then

            if c.IsUpToDate then
                out "The catalog is up to date."
            else
                out "The catalog is not up to date."
            end if

        end if

    end if

end sub

