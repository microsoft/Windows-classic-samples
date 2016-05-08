'
' Copyright (c) 2006 Microsoft Corporation. All rights reserved.
' 
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
' PARTICULAR PURPOSE.
'
Imports System
Imports System.Text.RegularExpressions
Imports System.Collections
Imports System.Collections.ObjectModel
Imports Microsoft.VisualBasic
Imports System.Management.Automation
Imports System.Management.Automation.Provider
Imports System.ComponentModel

Namespace Microsoft.Samples.PowerShell.Commands

#Region "SelectStringCommand"
    ''' <summary>
    ''' A cmdlet to search through PSObjects for particular patterns.
    ''' </summary>
    ''' <remarks>
    ''' Can be used to search any object like a file or a variable
    ''' whose provider exposes methods for reading and writing
    ''' contents
    ''' </remarks>
    <Cmdlet("Select", "Str", DefaultParameterSetName:="PatternParameterSet")> _
    Public Class SelectStringCommand
        Inherits PSCmdlet

#Region "Parameters"

        ''' <summary>
        ''' The Path of objects(files) to be searched
        ''' for the specified string/pattern.
        ''' </summary>
        ''' <value>Path of the object(s) to search</value>

        <Parameter(Position:=0, ParameterSetName:="ScriptParameterSet", _
        Mandatory:=True), _
        Parameter(Position:=0, ParameterSetName:="PatternParameterSet", _
        ValueFromPipeline:=True, Mandatory:=True), [Alias]("PSPath")> _
        Public Property Path() As String()
            Get
                Return paths
            End Get
            Set(ByVal value As String())
                paths = value
            End Set
        End Property
        Private paths() As String

        ''' <summary>
        ''' The pattern(s) used to find a match from the string 
        ''' representation of the object. A result will be returned
        ''' if either of the patterns match (OR matching)
        ''' </summary>
        ''' <remarks>
        ''' The patterns will be compiled into an array of wildcard
        ''' patterns if its a simple match (literal string matching)
        ''' else it will be converted into an array of compiled
        ''' regular expressions.
        ''' </remarks>
        ''' <value>Array of patterns to search.</value>
        <Parameter(Position:=1, ParameterSetName:="PatternParameterSet", _
        Mandatory:=True)> _
        Public Property Pattern() As String()
            Get
                Return patterns
            End Get
            Set(ByVal value As String())
                patterns = value
            End Set
        End Property
        Private patterns() As String
        Private regexPattern() As Regex
        Private wildcardPattern() As WildcardPattern

        ''' <summary>
        ''' A script block to call to perform the matching operations
        ''' instead of the matching performed by the Cmdlet
        ''' </summary>
        ''' <value>Script block that will be called for matching</value>
        <Parameter(Position:=1, ParameterSetName:="ScriptParameterSet", _
        Mandatory:=True)> _
        Public Property Script() As ScriptBlock
            Get
                Return myScript
            End Get
            Set(ByVal value As ScriptBlock)
                myScript = value
            End Set
        End Property
        Private myScript As ScriptBlock = Nothing

        ''' <summary> 
        ''' If set, match pattern string literally. 
        ''' If not (default), search using pattern as a Regular 
        ''' Expression
        ''' </summary>
        ''' <value>True if matching literally</value>        
        <Parameter()> _
        Public Property SimpleMatch() As SwitchParameter
            Get
                Return mySimpleMatch
            End Get
            Set(ByVal value As SwitchParameter)
                mySimpleMatch = value
            End Set
        End Property
        Private mySimpleMatch As Boolean

        ''' <summary> 
        ''' If true, then do case-sensitive searches.  False by default.
        ''' </summary>
        ''' <value>True, if case-sensitive searches are made</value>        
        <Parameter()> _
        Public Property CaseSensitive() As SwitchParameter
            Get
                Return myCaseSensitive
            End Get
            Set(ByVal value As SwitchParameter)
                myCaseSensitive = value
            End Set
        End Property
        Private myCaseSensitive As Boolean

        ''' <summary>
        ''' Allows to include particular files.  Files not matching
        ''' one of these (if specified) are excluded.
        ''' </summary>
        <Parameter(), ValidateNotNullOrEmpty()> _
        Public Property Include() As String()
            Get
                Return includeStrings
            End Get
            Set(ByVal value As String())
                includeStrings = value

                ReDim myInclude(includeStrings.Length - 1)
                Dim i As Integer
                For i = 0 To includeStrings.Length - 1
                    myInclude(i) = New WildcardPattern( _
                        includeStrings(i), WildcardOptions.IgnoreCase)
                Next i
            End Set
        End Property

        Friend includeStrings As String() = Nothing
        Friend myInclude As WildcardPattern() = Nothing

        ''' <summary>
        ''' Allows to exclude particular files.  Files matching
        ''' one of these (if specified) are excluded.
        ''' </summary>

        <Parameter(), ValidateNotNullOrEmpty()> _
        Public Property Exclude() As String()
            Get
                Return excludeStrings
            End Get
            Set(ByVal value As String())
                excludeStrings = value

                ReDim myExclude(excludeStrings.Length - 1)
                Dim i As Integer
                For i = 0 To excludeStrings.Length - 1
                    myExclude(i) = New WildcardPattern( _
                        excludeStrings(i), WildcardOptions.IgnoreCase)
                Next i
            End Set
        End Property
        Friend excludeStrings() As String
        Friend myExclude() As WildcardPattern

#End Region

#Region "Overrides"
        ''' <summary>
        ''' If regular expressions are used for pattern matching,
        ''' then build an array of compiled regular expressions 
        ''' at startup.This increases performance during scanning 
        ''' operations when simple matching is not used.
        ''' </summary>
        Protected Overrides Sub BeginProcessing()
            WriteDebug("Validating patterns.")

            If Not (patterns Is Nothing) Then
                Dim pattern As String
                For Each pattern In patterns
                    If pattern Is Nothing Then
                        ThrowTerminatingError(New ErrorRecord( _
                            New ArgumentNullException( _
                                "Search pattern cannot be null."), _
                            "NullSearchPattern", _
                             ErrorCategory.InvalidArgument, pattern))
                    End If
                Next pattern

                WriteVerbose("Search pattern(s) are valid.")

                ' If it's not a simple match, then
                ' compile the regular expressions once.
                If Not simpleMatch.ToBool() Then
                    WriteDebug("Compiling search regular expressions.")

                    Dim regexOptions As RegexOptions = RegexOptions.Compiled

                    If Not caseSensitive.ToBool() Then
                        regexOptions = regexOptions Or RegexOptions.IgnoreCase
                    End If
                    regexPattern = New Regex(patterns.Length) {}

                    Dim i As Integer
                    For i = 0 To patterns.Length - 1
                        Try
                            regexPattern(i) = New Regex(patterns(i), regexOptions)
                        Catch ex As ArgumentException
                            ThrowTerminatingError(New ErrorRecord(ex, _
                                "InvalidRegularExpression", _
                                 ErrorCategory.InvalidArgument, patterns(i)))
                        End Try
                    Next i
                    'loop through patterns to create RegEx objects
                    WriteVerbose("Pattern(s) compiled into regular expressions.")
                    ' if not a simple match
                    ' If it's a simple match, then compile the 
                    ' wildcard patterns once
                Else
                    WriteDebug("Compiling search wildcards.")

                    Dim wildcardOptions As WildcardOptions = _
                        WildcardOptions.Compiled

                    If Not caseSensitive.ToBool() Then
                        wildcardOptions = wildcardOptions Or _
                            WildcardOptions.IgnoreCase
                    End If

                    wildcardPattern = New WildcardPattern(patterns.Length) {}
                    Dim i As Integer
                    For i = 0 To patterns.Length - 1
                        wildcardPattern(i) = New WildcardPattern( _
                            patterns(i), wildcardOptions)
                    Next i

                    WriteVerbose("Pattern(s) compiled into wildcard expressions.")
                End If ' if match is a simple match
            End If
            ' if valid patterns are available
        End Sub 'BeginProcessing
        ' end of function BeginProcessing()

        ''' <summary>
        ''' Process the input and search for the specified patterns
        ''' </summary>   
        Protected Overrides Sub ProcessRecord()
            Dim lineNumber As UInt64 = 0
            Dim result As MatchInfo
            Dim nonMatches As New ArrayList()

            ' Walk the list of paths and search the contents for
            ' any of the specified patterns
            Dim psPath As String
            For Each psPath In paths
                ' Once the filepaths are expanded, we may have more than one
                ' path, so process all referenced paths.
                Dim path As PathInfo
                For Each path In _
                    SessionState.Path.GetResolvedPSPathFromPSPath(psPath)

                    WriteVerbose(("Processing path " + path.Path))

                    ' Check if the path represented is one to be excluded
                    ' if so continue
                    If Not MeetsIncludeExcludeCriteria(path.ProviderPath) Then
                        GoTo ContinueForEach2
                    End If
                    ' Get the content reader for the item(s) at the
                    ' specified path
                    Dim readerCollection As Collection(Of IContentReader) = _
                        Nothing
                    Try
                        readerCollection = _
                            Me.InvokeProvider.Content.GetReader(path.Path)
                    Catch ex As PSNotSupportedException
                        WriteError(New ErrorRecord(ex, _
                           "ContentAccessNotSupported", _
                           ErrorCategory.NotImplemented, path.Path))
                        Return
                    End Try

                    Dim reader As IContentReader
                    For Each reader In readerCollection
                        ' Reset the line number for this path.
                        lineNumber = 0

                        ' Read in a single block (line in case of a file) 
                        ' from the object.
                        Dim items As IList = reader.Read(1)

                        ' Read and process one block(line) at a time until
                        ' no more blocks(lines) exist
                        While Not (items Is Nothing) AndAlso items.Count = 1
                            ' Increment the line number each time a line is
                            ' processed.
                            lineNumber += 1

                            Dim message As String = String.Format( _
                                "Testing line {0} : {1}", lineNumber, items(0))

                            WriteDebug(message)

                            result = SelectString(items(0))

                            If Not (result Is Nothing) Then
                                result.Path = path.Path
                                result.LineNumber = lineNumber

                                WriteObject(result)
                            Else
                                ' Add the block(line) that did notmatch to the 
                                ' collection of non-matches, which will be stored 
                                ' in the SessionState variable $NonMatches
                                nonMatches.Add(items(0))
                            End If

                            ' Get the next line from the object.
                            items = reader.Read(1)
                        End While
                    Next reader
ContinueForEach2:   ' read and process one line at a time
                Next path ' loop through the reader collection
            Next psPath ' process all referenced paths
            ' walk the list of paths
            ' Store the list of non-matches in the
            ' session state variable $NonMatches.
            Try
                Me.SessionState.PSVariable.Set("NonMatches", nonMatches)
            Catch ex As SessionStateUnauthorizedAccessException
                WriteError(New ErrorRecord(ex, _
                    "CannotWriteVariableNonMatches", _
                    ErrorCategory.InvalidOperation, nonMatches))
            End Try

        End Sub 'ProcessRecord

        ' protected override void ProcessRecord()
#End Region

#Region "PrivateMethods"
        ''' <summary>
        ''' Check for a match using the input string and the pattern(s)
        ''' specified.
        ''' </summary>
        ''' <param name="input">The string to test.</param>
        ''' <returns>MatchInfo object containing information about 
        ''' result of a match</returns>
        Private Function SelectString(ByVal input As Object) As MatchInfo
            Dim line As String = Nothing

            Try
                ' Convert the object to a string type
                ' safely using language support methods
                line = CStr(LanguagePrimitives.ConvertTo(input, GetType(String)))
                line = line.Trim(" "c, vbTab)
            Catch ex As PSInvalidCastException
                WriteError(New ErrorRecord(ex, _
                    "CannotCastObjectToString", _
                    ErrorCategory.InvalidOperation, input))

                Return Nothing
            End Try

            Dim result As MatchInfo = Nothing

            ' If a scriptblock has been specified, call it 
            ' with the path for processing.  It will return
            ' one object.
            If Not (script Is Nothing) Then
                WriteDebug("Executing script block.")

                Dim psObjects As Collection(Of PSObject) = _
                    script.Invoke(line, simpleMatch, caseSensitive)

                Dim psObject As PSObject
                For Each psObject In psObjects
                    If LanguagePrimitives.IsTrue(psObject) Then
                        result = New MatchInfo()
                        result.Line = line
                        result.IgnoreCase = Not caseSensitive.ToBool()

                        Exit For
                    End If
                Next psObject
                ' if script block exists
                ' See if this line matches any of the match
                ' patterns.
            Else
                Dim patternIndex As Integer = 0

                While patternIndex < patterns.Length
                    If simpleMatch.ToBool() AndAlso _
                           wildcardPattern(patternIndex).IsMatch(line) OrElse _
                           (Not (regexPattern Is Nothing) AndAlso _
                           regexPattern(patternIndex).IsMatch(line)) Then

                        result = New MatchInfo()
                        result.IgnoreCase = Not caseSensitive.ToBool()
                        result.Line = line
                        result.Pattern = patterns(patternIndex)

                        Exit While
                    End If

                    patternIndex += 1
                End While
            End If ' loop through patterns and do a match
            ' no script block specified
            Return result

        End Function 'SelectString

        ' end of SelectString        
        ''' <summary>
        ''' Check whether the supplied name meets the include/exclude criteria.
        ''' That is - it's on the include list if there is one and not on
        ''' the exclude list if there was one of those.
        ''' </summary>
        ''' <param name="path">path to validate</param>
        ''' <returns>True if the path is acceptable.</returns>
        Private Function MeetsIncludeExcludeCriteria(ByVal path As String) _
            As Boolean

            Dim ok As Boolean = False

            ' see if the file is on the include list...
            If Not (Me.include Is Nothing) Then
                Dim patternItem As WildcardPattern
                For Each patternItem In myInclude
                    If patternItem.IsMatch(path) Then
                        ok = True
                        Exit For
                    End If
                Next patternItem
            Else
                ok = True
            End If

            If Not ok Then
                Return False
            End If
            ' now see if it's on the exclude list...
            If Not (myExclude Is Nothing) Then
                Dim patternItem As WildcardPattern
                For Each patternItem In myExclude
                    If patternItem.IsMatch(path) Then
                        ok = False
                        Exit For
                    End If
                Next patternItem
            End If

            Return ok

        End Function 'MeetsIncludeExcludeCriteria 'MeetsIncludeExcludeCriteria
#End Region
    End Class 'SelectStringCommand 
#End Region

#Region "MatchInfo"

    ''' <summary>
    ''' Class representing the result of a pattern/literal match
    ''' that will be returned by the select-str command
    ''' </summary>
    Public Class MatchInfo
        ''' <summary>
        ''' Indicates if the match was done ignoring case.
        ''' </summary>
        ''' <value>True if case was ignored.</value>

        Public Property IgnoreCase() As Boolean
            Get
                Return myIgnoreCase
            End Get
            Set(ByVal value As Boolean)
                myIgnoreCase = value
            End Set
        End Property
        Private myIgnoreCase As Boolean

        ''' <summary>
        ''' Returns the number of the matching line.
        ''' </summary>
        ''' <value>The number of the matching line.</value>

        Public Property LineNumber() As UInt64
            Get
                Return myLineNumber
            End Get
            Set(ByVal value As UInt64)
                myLineNumber = value
            End Set
        End Property
        Private myLineNumber As UInt64

        ''' <summary>
        ''' Returns the text of the matching line.
        ''' </summary>
        ''' <value>The text of the matching line.</value>

        Public Property Line() As String
            Get
                Return myLine
            End Get
            Set(ByVal value As String)
                myLine = value
            End Set
        End Property
        Private myLine As String

        ''' <summary>
        ''' The full path of the object(file) containing the matching line.
        ''' </summary>
        ''' <remarks>
        ''' It will be "inputStream" if the object came from the input 
        ''' stream.
        ''' </remarks>
        ''' <value>The path name</value>

        Public Property Path() As String
            Get
                Return myPath
            End Get
            Set(ByVal value As String)
                pathSet = True
                myPath = value
            End Set
        End Property
        Private myPath As String
        Private pathSet As Boolean

        ''' <summary>
        ''' Returns the pattern that was used in the match.
        ''' </summary>
        ''' <value>The pattern string</value>

        Public Property Pattern() As String
            Get
                Return myPattern
            End Get
            Set(ByVal value As String)
                myPattern = value
            End Set
        End Property
        Private myPattern As String

        Private Const MatchFormat As String = "{0}:{1}:{2}"


        ''' <summary>
        ''' Returns the string representation of this object. The format
        ''' depends on whether a path has been set for this object or 
        ''' not.
        ''' </summary>
        ''' <remarks>
        ''' If the path component is set, as would be the case when 
        ''' matching in a file, ToString() would return the path, line 
        ''' number and line text.  If path is not set, then just the 
        ''' line text is presented.
        ''' </remarks>
        ''' <returns>The string representation of the match object</returns>
        Public Overrides Function ToString() As String
            If pathSet Then
                Return String.Format( _
                    System.Threading.Thread.CurrentThread.CurrentCulture, _
                    MatchFormat, Me.path, Me.lineNumber, Me.line)
            Else
                Return Me.line
            End If

        End Function 'ToString 
    End Class 'MatchInfo
#End Region

#Region "PowerShell snap-in"

    ''' <summary>
    ''' Create this sample as a PowerShell snap-in
    ''' </summary>
    <RunInstaller(True)> _
    Public Class SelectStringPSSnapIn
        Inherits PSSnapIn

        ''' <summary>
        ''' Create an instance of the SelectStrPSSnapin
        ''' </summary>
        Public Sub New()

        End Sub 'New

        ''' <summary>
        ''' Get a name for this PowerShell snap-in. This name will be used
        ''' in registering this PowerShell snap-in.
        ''' </summary>
        Public Overrides ReadOnly Property Name() As String
            Get
                Return "SelectStrPSSnapIn"
            End Get
        End Property

        ''' <summary>
        ''' Vendor information for this PowerShell snap-in.
        ''' </summary>
        Public Overrides ReadOnly Property Vendor() As String
            Get
                Return "Microsoft"
            End Get
        End Property

        ''' <summary>
        ''' Gets resource information for vendor. This is a string of format: 
        ''' resourceBaseName,resourceName. 
        ''' </summary>
        Public Overrides ReadOnly Property VendorResource() As String
            Get
                Return "SelectStrSnapIn,Microsoft"
            End Get
        End Property

        ''' <summary>
        ''' Description of this PowerShell snap-in.
        ''' </summary>
        Public Overrides ReadOnly Property Description() As String
            Get
                Return "This is a PowerShell snap-in that " & _
                    "includes the select-str cmdlet."
            End Get
        End Property
    End Class 'SelectStringPSSnapIn

#End Region

End Namespace


