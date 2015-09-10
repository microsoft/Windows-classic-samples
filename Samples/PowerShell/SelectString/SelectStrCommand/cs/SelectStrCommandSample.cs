// <copyright file="SelectStrCommandSample.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

using System;
using System.Text.RegularExpressions;
using System.Collections;
using System.Collections.ObjectModel;
using System.Management.Automation;
using System.Management.Automation.Provider;
using System.ComponentModel;


namespace Microsoft.Samples.PowerShell.Commands
{
    #region SelectStringCommand
    /// <summary>
    /// A cmdlet to search through PSObjects for particular patterns.
    /// </summary>
    /// <remarks>
    /// Can be used to search any object like a file or a variable
    /// whose provider exposes methods for reading and writing
    /// contents
    /// </remarks>
    [Cmdlet("Select", "Str", DefaultParameterSetName="PatternParameterSet")]
    public class SelectStringCommand : PSCmdlet
    {
        #region Parameters
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
        public string[] Path
        {
          get { return paths; }
          set { paths = value; }
        }
        private string[] paths;

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
        public string[] Pattern
        {
          get { return patterns; }
          set { patterns = value; }
        }
        private string[] patterns;
        private Regex[] regexPattern;
        private WildcardPattern[] wildcardPattern;
        
        /// <summary>
        /// A script block to call to perform the matching operations
        /// instead of the matching performed by the Cmdlet
        /// </summary>
        /// <value>Script block that will be called for matching</value>
        [Parameter(
          Position = 1,
          ParameterSetName = "ScriptParameterSet",
          Mandatory = true)]
        public ScriptBlock Script
        {
          set { script = value; }
          get { return script; }
        }
        ScriptBlock script;

        /// <summary> 
        /// If set, match pattern string literally. 
        /// If not (default), search using pattern as a Regular 
        /// Expression
        /// </summary>
        /// <value>True if matching literally</value>        
        [Parameter]
        public SwitchParameter SimpleMatch
        {
          get { return simpleMatch; }
          set { simpleMatch = value; }
        }
        private bool simpleMatch;

        /// <summary> 
        /// If true, then do case-sensitive searches.  False by default.
        /// </summary>
        /// <value>True, if case-sensitive searches are made</value>        
        [Parameter]
        public SwitchParameter CaseSensitive
        {
          get { return caseSensitive; }
          set { caseSensitive = value; }
        }
        private bool caseSensitive;

        /// <summary>
        /// Allows to include particular files.  Files not matching
        /// one of these (if specified) are excluded.
        /// </summary>
        [Parameter]
        [ValidateNotNullOrEmpty]
        public string[] Include
        {
            get
            {
                return includeStrings;
            }
            set
            {
                includeStrings = value;

                this.include = new WildcardPattern[includeStrings.Length];
                for (int i = 0; i < includeStrings.Length; i++)
                {
                    this.include[i] = new WildcardPattern(includeStrings[i], WildcardOptions.IgnoreCase);
                }
            }
        }

        internal string[] includeStrings = null;
        internal WildcardPattern[] include = null;

        /// <summary>
        /// Allows to exclude particular files.  Files matching
        /// one of these (if specified) are excluded.
        /// </summary>
        [Parameter]
        [ValidateNotNullOrEmpty]
        public string[] Exclude
        {
            get
            {
                return excludeStrings;
            }
            set
            {
                excludeStrings = value;

                this.exclude = new WildcardPattern[excludeStrings.Length];
                for (int i = 0; i < excludeStrings.Length; i++)
                {
                    this.exclude[i] = new WildcardPattern(excludeStrings[i], WildcardOptions.IgnoreCase);
                }
            }
        }
        internal string[] excludeStrings;
        internal WildcardPattern[] exclude;

        #endregion Parameters

        #region Overrides
        /// <summary>
        /// If regular expressions are used for pattern matching,
        /// then build an array of compiled regular expressions 
        /// at startup.This increases performance during scanning 
        /// operations when simple matching is not used.
        /// </summary>
        protected override void BeginProcessing()
        {
          WriteDebug("Validating patterns.");

         WriteVerbose("Search pattern(s) are valid.");

         // If it's not a simple match, then
         // compile the regular expressions once.
         if (!simpleMatch)
         {
             WriteDebug("Compiling search regular expressions.");

             RegexOptions regexOptions = RegexOptions.Compiled;

             if (!caseSensitive)
                 regexOptions |= RegexOptions.IgnoreCase;

             regexPattern = new Regex[patterns.Length];

             for (int i = 0; i < patterns.Length; i++)
             {
                 try
                 {
                     regexPattern[i] =
                        new Regex(patterns[i], regexOptions);
                 }
                 catch (ArgumentException ex)
                 {
                     ThrowTerminatingError(new ErrorRecord(
                        ex,
                        "InvalidRegularExpression",
                        ErrorCategory.InvalidArgument,
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

             WildcardOptions wildcardOptions = WildcardOptions.Compiled;

             if (!caseSensitive)
             {
                 wildcardOptions |= WildcardOptions.IgnoreCase;
             }

             wildcardPattern = new WildcardPattern[patterns.Length];
             for (int i = 0; i < patterns.Length; i++)
             {
                 wildcardPattern[i] =
                     new WildcardPattern(patterns[i], wildcardOptions);
             }

             WriteVerbose("Pattern(s) compiled into wildcard expressions.");
         }// if match is a simple match
        }// end of function BeginProcessing()

        /// <summary>
        /// Process the input and search for the specified patterns
        /// </summary>   
        protected override void ProcessRecord()
        {
          UInt64 lineNumber = 0;
          MatchInfo result;
          ArrayList nonMatches = new ArrayList();

          // Walk the list of paths and search the contents for
          // any of the specified patterns
          foreach (string psPath in paths)
          {
             // Once the filepaths are expanded, we may have more than one
             // path, so process all referenced paths.
             foreach(PathInfo path in 
                SessionState.Path.GetResolvedPSPathFromPSPath(psPath)
             )
             {
                WriteVerbose("Processing path " + path.Path);

                // Check if the path represented is one to be excluded
                // if so continue
                if (!MeetsIncludeExcludeCriteria(path.ProviderPath))
                    continue;

                // Get the content reader for the item(s) at the
                // specified path
                Collection<IContentReader> readerCollection = null;
                try
                {
                    readerCollection =
                        this.InvokeProvider.Content.GetReader(path.Path);
                }
                catch (PSNotSupportedException ex)
                {
                    WriteError(new ErrorRecord(ex,
                        "ContentAccessNotSupported",
                        ErrorCategory.NotImplemented,
                        path.Path));
                    continue;
                }

                foreach(IContentReader reader in readerCollection)
                {
                   // Reset the line number for this path.
                   lineNumber = 0;

                   // Read in a single block (line in case of a file) 
                   // from the object.
                   IList items = reader.Read(1);
                   
                   // Read and process one block(line) at a time until
                   // no more blocks(lines) exist
                   while (items != null && items.Count == 1) 
                   {
                      // Increment the line number each time a line is
                      // processed.
                      lineNumber++;

                      String message = String.Format("Testing line {0} : {1}",
                                            lineNumber, items[0]);

                      WriteDebug(message);

                      result = SelectString(items[0]);

                      if (result != null)
                      {
                          result.Path = path.Path;
                          result.LineNumber = lineNumber;

                          WriteObject(result);
                      }
                      else
                      {
                          // Add the block(line) that did notmatch to the 
                          // collection of non matches , which will be stored 
                          // in the SessionState variable $NonMatches
                          nonMatches.Add(items[0]);
                      }

                      // Get the next line from the object.
                      items = reader.Read(1);                  

                   }// read and process one line at a time
                }// loop through the reader collection
             }// process all referenced paths
          }// walk the list of paths

          // Store the list of non-matches in the
          // session state variable $NonMatches.
          try
          {
              this.SessionState.PSVariable.Set("NonMatches", nonMatches);
          }
          catch (SessionStateUnauthorizedAccessException ex)
          {
              WriteError(new ErrorRecord(ex,
                  "CannotWriteVariableNonMatches",
                     ErrorCategory.InvalidOperation,
                       nonMatches));
          }
                    
        }// protected override void ProcessRecord()

        #endregion Overrides

        #region PrivateMethods
        /// <summary>
        /// Check for a match using the input string and the pattern(s)
        /// specified.
        /// </summary>
        /// <param name="input">The string to test.</param>
        /// <returns>MatchInfo object containing information about 
        /// result of a match</returns>
        private MatchInfo SelectString(object input)
        {
          string line = null;

          try
          {
             // Convert the object to a string type
             // safely using language support methods
             line = (string)LanguagePrimitives.ConvertTo(
                      input,
                      typeof(string)
                   );
             line = line.Trim(' ','\t');
          }
          catch (PSInvalidCastException ex)
          {
             WriteError(new ErrorRecord(
                ex,
                "CannotCastObjectToString",
                ErrorCategory.InvalidOperation,
                input
             ));
             
             return null;
          }

          MatchInfo result = null;

          // If a scriptblock has been specified, call it 
          // with the path for processing.  It will return
          // one object.
          if (script != null)
          {
             WriteDebug("Executing script block.");

             Collection<PSObject> psObjects =
                script.Invoke(
                   line,
                   simpleMatch,
                   caseSensitive
                );

             foreach (PSObject psObject in psObjects)
             {
                if (LanguagePrimitives.IsTrue(psObject))
                {
                   result = new MatchInfo();
                   result.Line = line;
                   result.IgnoreCase = !caseSensitive;

                   break;
                }
             }
          }// if script block exists
          // See if this line matches any of the match
          // patterns.
          else
          {
             int patternIndex = 0;

             while (patternIndex < patterns.Length)
             {
                if ((simpleMatch && 
                      wildcardPattern[patternIndex].IsMatch(line))
                   || (regexPattern != null 
                      && regexPattern[patternIndex].IsMatch(line))
                )
                {
                   result = new MatchInfo();
                   result.IgnoreCase = !caseSensitive;
                   result.Line = line;
                   result.Pattern = patterns[patternIndex];

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
        private bool MeetsIncludeExcludeCriteria(string path)
        {
            bool ok = false;

            // see if the file is on the include list...
            if (this.include != null)
            {
                foreach (WildcardPattern patternItem in this.include)
                {
                    if (patternItem.IsMatch(path))
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
            if (this.exclude != null)
            {
                foreach (WildcardPattern patternItem in this.exclude)
                {
                    if (patternItem.IsMatch(path))
                    {
                        ok = false;
                        break;
                    }
                }
            }

            return ok;
        } //MeetsIncludeExcludeCriteria

        #endregion Private Methods

    }// class SelectStringCommand

    #endregion SelectStringCommand

    #region MatchInfo

    /// <summary>
    /// Class representing the result of a pattern/literal match
    /// that will be returned by the select-str command
    /// </summary>
    public class MatchInfo
    {
       /// <summary>
       /// Indicates if the match was done ignoring case.
       /// </summary>
       /// <value>True if case was ignored.</value>
       public bool IgnoreCase
       {
          get { return ignoreCase; }
          set { ignoreCase = value; }
       }
       private bool ignoreCase;

       /// <summary>
       /// Returns the number of the matching line.
       /// </summary>
       /// <value>The number of the matching line.</value>
       public UInt64 LineNumber
       {
          get { return lineNumber; }
          set { lineNumber = value; }
       }
       private UInt64 lineNumber;

       /// <summary>
       /// Returns the text of the matching line.
       /// </summary>
       /// <value>The text of the matching line.</value>
       public string Line
       {
          get { return line; }
          set { line = value; }
       }
       private string line;

       /// <summary>
       /// The full path of the object(file) containing the matching line.
       /// </summary>
       /// <remarks>
       /// It will be "inputStream" if the object came from the input 
       /// stream.
       /// </remarks>
       /// <value>The path name</value>
       public string Path
       {
          get { return path; }
          set
          {
             pathSet = true;
             path = value;
          }
       }
       private string path;
       private bool pathSet;

       /// <summary>
       /// Returns the pattern that was used in the match.
       /// </summary>
       /// <value>The pattern string</value>
       public string Pattern
       {
          get { return pattern; }
          set { pattern = value; }
       }
       private string pattern;

       private const string MatchFormat = "{0}:{1}:{2}";

       /// <summary>
       /// Returns the string representation of this object. The format
       /// depends on whether a path has been set for this object or 
       /// not.
       /// </summary>
       /// <remarks>
       /// If the path component is set, as would be the case when 
       /// matching in a file, ToString() would return the path, line 
       /// number and line text.  If path is not set, then just the 
       /// line text is presented.
       /// </remarks>
       /// <returns>The string representation of the match object</returns>
       public override string ToString()
       {
          if (pathSet)
             return String.Format(
                System.Threading.Thread.CurrentThread.CurrentCulture,
                MatchFormat,
                this.path,
                this.lineNumber,
                this.line
             );
          else
             return this.line;
       }
    }// class MatchInfo

    #endregion
} //namespace Microsoft.Samples.PowerShell.Commands;

