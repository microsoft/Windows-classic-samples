//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//

#pragma once;

using namespace System;

using namespace System::Management::Automation;

namespace Microsoft
{
    namespace Samples
    {
        namespace PowerShell
        {
            namespace Commands
            {
                public ref class MatchInfo
                {
                private:
                    bool    ignoreCase;
                    UInt64  lineNumber;
                    String^ line;
                    String^ path;
                    bool    pathSet;
                    String^ pattern;

                    static String^ MatchFormat = "{0}:{1}:{2}";

                public:
                    /// <summary>
                    /// Indicates if the match was done ignoring case.
                    /// </summary>
                    /// <value>True if case was ignored.</value>
                    property bool IgnoreCase
                    {
                        bool get()
                        { 
                            return ignoreCase; 
                        }

                        void set(bool value)
                        { 
                            ignoreCase = value; 
                        }
                    }

                    /// <summary>
                    /// Returns the number of the matching line.
                    /// </summary>
                    /// <value>The number of the matching line.</value>
                    property UInt64 LineNumber
                    {
                        UInt64 get()
                        { 
                            return lineNumber; 
                        }

                        void set(UInt64 value)
                        { 
                            lineNumber = value; 
                        }
                    }

                    /// <summary>
                    /// Returns the text of the matching line.
                    /// </summary>
                    /// <value>The text of the matching line.</value>
                    property String^ Line
                    {
                        String^ get()
                        { 
                            return line; 
                        }

                        void set(String^ value)
                        { 
                            line = value; 
                        }
                    }

                    /// <summary>
                    /// The full path of the object(file) containing the matching line.
                    /// </summary>
                    /// <remarks>
                    /// It will be "inputStream" if the object came from the input 
                    /// stream.
                    /// </remarks>
                    /// <value>The path name</value>
                    property String^ Path
                    {
                        String^ get()
                        { 
                            return path; 
                        }

                        void set(String^ value)
                        {
                            pathSet = true;
                            path = value;
                        }
                    }

                    /// <summary>
                    /// Returns the pattern that was used in the match.
                    /// </summary>
                    /// <value>The pattern string</value>
                    property String^ Pattern
                    {
                        String^ get()
                        { 
                            return pattern; 
                        }

                        void set(String^ value)
                        { 
                            pattern = value; 
                        }
                    }

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
                    virtual String^ ToString() override 
                    {
                        if (pathSet)
                            return String::Format(
                            System::Threading::Thread::CurrentThread->CurrentCulture,
                            MatchFormat,
                            this->path,
                            this->lineNumber,
                            this->line
                            );
                        else
                            return this->line;
                    }
                };
            }
        }
    }
}