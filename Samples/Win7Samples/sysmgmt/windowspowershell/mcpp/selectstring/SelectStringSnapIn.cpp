//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//

using namespace System;
using namespace System::ComponentModel;

using namespace System::Management::Automation;

// Select-Str sample SnapIn functionality
namespace Microsoft
{
    namespace Samples
    {
        namespace PowerShell
        {
            namespace Commands
            {
                /// <remarks>
                /// PSSnapIn related information for Select-Str.
                /// </remarks>
                [RunInstaller(true)]
                public ref class SelectStrSnapIn : PSSnapIn
                {
                public:
                    #pragma region PSSnapIn overrides

                    /// <summary>
                    /// Get a name for this PowerShell snap-in. This name will be used in registering
                    /// this PowerShell snap-in.
                    /// </summary>
                    property String^ Name 
                    {
                        virtual String^ get() override
                        {
                            return "SelectStrMCPPPSSnapIn";
                        }
                    }

                    /// <summary>
                    /// Vendor information for this PowerShell snap-in.
                    /// </summary>
                    property String^ Vendor
                    {
                        virtual String^ get() override
                        {
                            return "Microsoft";
                        }
                    }

                    /// <summary>
                    /// Description of this PowerShell snap-in.
                    /// </summary>
                    property String^ Description
                    {
                        virtual String^ get() override
                        {
                            return "This is a PowerShell snap-in that includes the select-str cmdlet.";
                        }
                    }

                    #pragma endregion
                };

            }
        }
    }
}