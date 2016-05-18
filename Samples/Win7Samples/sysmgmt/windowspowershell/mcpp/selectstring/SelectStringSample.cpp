//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//

#include "MatchInfo.h"

using namespace System;
using namespace System::Collections;
using namespace System::Collections::ObjectModel;
using namespace System::Text::RegularExpressions;
using namespace System::Globalization;

using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;

// This sample demonstrates the following:
// 1.Usage of PSPath
// 2.Usage of Scriptblocks
// 3.Usage of Session state
namespace Microsoft
{  
    namespace Samples
    {
        namespace PowerShell
        {
            namespace Commands
            {
                /// <summary>
                /// A cmdlet to search through PSObjects / Paths for particular patterns.
                /// </summary>
                /// <remarks>
                /// Can be used to search any object like a file or a variable
                /// whose provider exposes methods for reading and writing
                /// contents
                /// </remarks>
                [Cmdlet("Select","Str",
                    DefaultParameterSetName="PatternParameterSet")]
                public ref class SelectStrCmdlet : public PSCmdlet
                {
                private:
                    #pragma region Private Data

                    array<String^>^ paths;
                    array<String^>^ patterns;
                    array<Regex^>^  regexPattern;
                    array<WildcardPattern^>^ wildcardPattern;
                    ScriptBlock^ script;        
                    bool simpleMatch;
                    bool caseSensitive;
                    array<String^>^ includeStrings;
                    array<WildcardPattern^>^ include;
                    array<String^>^ excludeStrings;
                    array<WildcardPattern^>^ exclude;

                    #pragma endregion

                public:
                    #pragma region Parameters
                    /// <summary>
                    /// The Path of objects(files) to be searched
                    /// for the specified string/pattern.
                    /// </summary>
                    /// <value>Path of the object(s) to search</value>
                    [Parameter(
                        Position = 0,
                        ParameterSetName = "ScriptParameterSet",
                        Mandatory = true)]                  
                    [Parameter(
                        Position = 0,
                        ParameterSetName = "PatternParameterSet",
                        ValueFromPipeline = true,
                        Mandatory = true)]
                    [Alias("PSPath")]
                    property array<String^>^ Path
                    {
                        array<String^>^ get()
                        {
                            return paths;
                        }

                        void set(array<String^>^ value)
                        {
                            paths = value;
                        }
                    }

                    /// <summary>
                    /// The pattern(s) used to find a match from the string 
                    /// representation of the object. A result will be returned
                    /// if either of the patterns match (OR matching)
                    /// </summary>
                    /// <remarks>
                    /// The patterns will be compiled into an array of wildcard
                    /// patterns if its a simple match (literal string matching)
                    /// else it will be converted into an array of compiled
                    /// regular expressions.
                    /// </remarks>
                    /// <value>Array of patterns to search.</value>
                    [Parameter(
                        Position = 1,
                        ParameterSetName = "PatternParameterSet",
                        Mandatory = true)]
                    property array<String^>^ Pattern
                    {
                        array<String^>^ get()
                        {
                            return patterns;
                        }

                        void set(array<String^>^ value)
                        {
                            patterns = value;
                        }
                    }

                    /// <summary>
                    /// A script block to call to perform the matching operations
                    /// instead of the matching performed by the Cmdlet
                    /// </summary>
                    /// <value>Script block that will be called for matching</value>
                    [Parameter(
                        Position = 1,
                        ParameterSetName = "ScriptParameterSet",
                        Mandatory = true)]
                    property ScriptBlock^ Script
                    {
                        ScriptBlock^ get()
                        {
                            return script;
                        }

                        void set(ScriptBlock^ value)
                        {
                            script = value;
                        }
                    }

                    /// <summary> 
                    /// If set, match pattern string literally. 
                    /// If not (default), search using pattern as a Regular 
                    /// Expression
                    /// </summary>
                    /// <value>True if matching literally</value>        
                    [Parameter]
                    property SwitchParameter SimpleMatch
                    {
                        SwitchParameter get()
                        {
                            return simpleMatch;
                        }

                        void set(SwitchParameter value)
                        {
                            simpleMatch = value;
                        }
                    }

                    /// <summary> 
                    /// If true, then do case-sensitive searches.  False by default.
                    /// </summary>
                    /// <value>True, if case-sensitive searches are made</value>        
                    [Parameter]
                    property SwitchParameter CaseSensitive
                    {
                        SwitchParameter get()
                        {
                            return caseSensitive;
                        }

                        void set(SwitchParameter value)
                        {
                            caseSensitive = value;
                        }
                    }

                    /// <summary>
                    /// Allows to include particular files.  Files not matching
                    /// one of these (if specified) are excluded.
                    /// </summary>
                    [Parameter]
                    [ValidateNotNullOrEmpty]
                    property array<String^>^ Include
                    {
                        array<String^>^ get()
                        {
                            return includeStrings;
                        }

                        void set(array<String^>^ value)
                        {
                            includeStrings = value;

                            this->include = gcnew array<WildcardPattern^>(includeStrings->Length);
                            for (int i = 0; i < includeStrings->Length; i++)
                            {
                                this->include[i] = gcnew WildcardPattern(includeStrings[i], WildcardOptions::IgnoreCase);
                            }
                        }
                    }

                    /// <summary>
                    /// Allows to exclude particular files.  Files matching
                    /// one of these (if specified) are excluded.
                    /// </summary>
                    [Parameter]
                    [ValidateNotNullOrEmpty]
                    property array<String^>^ Exclude
                    {
                        array<String^>^ get()
                        {
                            return excludeStrings;
                        }

                        void set(array<String^>^ value)
                        {
                            excludeStrings = value;

                            this->exclude = gcnew array<WildcardPattern^>(excludeStrings->Length);
                            for (int i = 0; i < excludeStrings->Length; i++)
                            {
                                this->exclude[i] = gcnew WildcardPattern(excludeStrings[i], WildcardOptions::IgnoreCase);
                            }
                        }
                    }

                    #pragma endregion

                protected:
                    #pragma region CmdletOverrides
                    /// <summary>
                    /// If regular expressions are used for pattern matching,
                    /// then build an array of compiled regular expressions 
                    /// at startup.This increases performance during scanning 
                    /// operations when simple matching is not used.
                    /// </summary>
                    virtual void BeginProcessing() override
                    {
                        WriteVerbose("Search pattern(s) are valid.");

                        // If it's not a simple match, then
                        // compile the regular expressions once.
                        if (!simpleMatch)
                        {
                            WriteDebug("Compiling search regular expressions.");
                            RegexOptions regexOptions = RegexOptions::Compiled;
                            if (!caseSensitive)
                            {
                                regexOptions = regexOptions | RegexOptions::IgnoreCase;
                            }
                            regexPattern = gcnew array<Regex^>(patterns->Length);
                            for (int i = 0; i < patterns->Length; i++)
                            {
                                try
                                {
                                    regexPattern[i] =
                                        gcnew Regex(patterns[i], regexOptions);
                                }
                                catch (ArgumentException^ ex)
                                {
                                    ThrowTerminatingError(gcnew ErrorRecord(
                                        ex,
                                        "InvalidRegularExpression",
                                        ErrorCategory::InvalidArgument,
                                        patterns[i]
                                    ));
                                }
                            } //loop through patterns to create RegEx objects
                            WriteVerbose("Pattern(s) compiled into regular expressions.");
                        }// if not a simple match
                        // If it's a simple match, then compile the 
                        // wildcard patterns once
                        else
                        {
                            WriteDebug("Compiling search wildcards.");
                            WildcardOptions wildcardOptions = WildcardOptions::Compiled;
                            if (!caseSensitive)
                            {
                                wildcardOptions = wildcardOptions | WildcardOptions::IgnoreCase;
                            }
                            wildcardPattern = gcnew array<WildcardPattern^>(patterns->Length);
                            for (int i = 0; i < patterns->Length; i++)
                            {
                                wildcardPattern[i] =
                                    gcnew WildcardPattern(patterns[i], wildcardOptions);
                            }
                            WriteVerbose("Pattern(s) compiled into wildcard expressions.");
                        }// if match is a simple match
                    }

                    /// <summary>
                    /// Process the input and search for the specified patterns
                    /// </summary>   
                    virtual void ProcessRecord() override
                    {
                        UInt64 lineNumber = 0;
                        MatchInfo^ result;
                        ArrayList^ nonMatches = gcnew ArrayList();

                        // Walk the list of paths and search the contents for
                        // any of the specified patterns
                        for each (String^ psPath in paths)
                        {
                            // Once the filepaths are expanded, we may have more than one
                            // path, so process all referenced paths.
                            for each(PathInfo^ path in 
                                SessionState->Path->GetResolvedPSPathFromPSPath(psPath))
                            {
                                WriteVerbose("Processing path " + path->Path);

                                // Check if the path represented is one to be excluded
                                // if so continue
                                if (!MeetsIncludeExcludeCriteria(path->ProviderPath))
                                    continue;

                                // Get the content reader for the item(s) at the
                                // specified path
                                Collection<IContentReader^>^ readerCollection = nullptr;
                                try
                                {
                                    readerCollection =
                                        this->InvokeProvider->Content->GetReader(path->Path);
                                }
                                catch (PSNotSupportedException^ ex)
                                {
                                    WriteError(gcnew ErrorRecord(ex,
                                        "ContentAccessNotSupported",
                                        ErrorCategory::NotImplemented,
                                        path->Path));                    
                                    continue;
                                }

                                for each(IContentReader^ reader in readerCollection)
                                {
                                    // Reset the line number for this path.
                                    lineNumber = 0;

                                    // Read in a single block (line in case of a file) 
                                    // from the object.
                                    IList^ items = reader->Read(1);

                                    // Read and process one block(line) at a time until
                                    // no more blocks(lines) exist
                                    while ((items != nullptr) && (items->Count == 1))
                                    {
                                        // Increment the line number each time a line is
                                        // processed.
                                        lineNumber++;

                                        String^ message = String::Format(CultureInfo::InvariantCulture,
                                            "Testing line {0} : {1}", lineNumber, items[0]);

                                        WriteDebug(message);

                                        result = SelectString(items[0]);

                                        if (result != nullptr)
                                        {
                                            result->Path = path->Path;
                                            result->LineNumber = lineNumber;

                                            WriteObject(result);
                                        }
                                        else
                                        {
                                            // Add the block(line) that did notmatch to the 
                                            // collection of non matches , which will be stored 
                                            // in the SessionState variable $NonMatches
                                            nonMatches->Add(items[0]);
                                        }

                                        // Get the next line from the object.
                                        items = reader->Read(1);                  

                                    }// read and process one line at a time
                                }// loop through the reader collection
                            }// process all referenced paths
                        }// walk the list of paths

                        // Store the list of non-matches in the
                        // session state variable $NonMatches.
                        try
                        {
                            this->SessionState->PSVariable->Set("NonMatches", nonMatches);
                        }
                        catch (SessionStateUnauthorizedAccessException^ ex)
                        {
                            WriteError(gcnew ErrorRecord(ex,
                                "CannotWriteVariableNonMatches",
                                ErrorCategory::InvalidOperation,
                                nonMatches));
                        }
                    }
                    #pragma endregion           

                private:
                    #pragma region Private methods

                    /// <summary>
                    /// Check for a match using the input string and the pattern(s)
                    /// specified.
                    /// </summary>
                    /// <param name="input">The string to test.</param>
                    /// <returns>MatchInfo object containing information about 
                    /// result of a match</returns>
                    MatchInfo^ SelectString(Object^ input)
                    {
                        String^ line = nullptr;

                        try
                        {
                            // Convert the object to a string type
                            // safely using language support methods
                            line = (String^)LanguagePrimitives::ConvertTo(
                                input,
                                System::String::typeid,
                                CultureInfo::InvariantCulture);
                            line = line->Trim(' ','\t');
                        }
                        catch (PSInvalidCastException^ ex)
                        {
                            WriteError(gcnew ErrorRecord(
                                ex,
                                "CannotCastObjectToString",
                                ErrorCategory::InvalidOperation,
                                input
                                ));

                            return nullptr;
                        }

                        MatchInfo^ result = nullptr;

                        // If a scriptblock has been specified, call it 
                        // with the line for processing.  It will return
                        // one object.
                        if (script != nullptr)
                        {
                            WriteDebug("Executing script block.");

                            Collection<PSObject^>^ psObjects =
                                script->Invoke(
                                line,
                                simpleMatch,
                                caseSensitive);

                            for each (PSObject^ psObject in psObjects)
                            {
                                if (LanguagePrimitives::IsTrue(psObject))
                                {
                                    result = gcnew MatchInfo();
                                    result->Line = line;
                                    result->IgnoreCase = !caseSensitive;

                                    break;
                                }
                            }
                        }// if script block exists
                        // See if this line matches any of the match
                        // patterns.
                        else
                        {
                            int patternIndex = 0;

                            while (patternIndex < patterns->Length)
                            {
                                if ((simpleMatch && 
                                    wildcardPattern[patternIndex]->IsMatch(line)) ||
                                    (regexPattern != nullptr && 
                                    regexPattern[patternIndex]->IsMatch(line))
                                    )
                                {
                                    result = gcnew MatchInfo();
                                    result->IgnoreCase = !caseSensitive;
                                    result->Line = line;
                                    result->Pattern = patterns[patternIndex];

                                    break;
                                }

                                patternIndex++;

                            }// loop through patterns and do a match
                        }// no script block specified

                        return result;
                    }// end of SelectString        

                    /// <summary>
                    /// Check whether the supplied name meets the include/exclude criteria.
                    /// That is - it's on the include list if there is one and not on
                    /// the exclude list if there was one of those.
                    /// </summary>
                    /// <param name="path">path to validate</param>
                    /// <returns>True if the path is acceptable.</returns>
                    bool MeetsIncludeExcludeCriteria(String^ path)
                    {
                        bool ok = false;

                        // see if the file is on the include list...
                        if (this->include != nullptr)
                        {
                            for each (WildcardPattern^ patternItem in this->include)
                            {
                                if (patternItem->IsMatch(path))
                                {
                                    ok = true;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            ok = true;
                        }

                        if (!ok)
                            return false;

                        // now see if it's on the exclude list...
                        if (this->exclude != nullptr)
                        {
                            for each (WildcardPattern^ patternItem in this->exclude)
                            {
                                if (patternItem->IsMatch(path))
                                {
                                    ok = false;
                                    break;
                                }
                            }
                        }

                        return ok;
                    } //MeetsIncludeExcludeCriteria

                    #pragma endregion

                };
            }
        }
    }
}