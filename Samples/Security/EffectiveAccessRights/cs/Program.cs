// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Text.RegularExpressions;

namespace Microsoft.Samples.DynamicAccessControl
{
    using Utility;

    internal class Program
    {
        const string CmdLnUsage = @"
Displays the effective access a user or group has on a file or folder.

{0} /path:<path>
    [/targetMachine:<machine-name> [/shareSD:<SDDL>]]
    /user:<user's account> [/userclaim:(<claim-detail>)]
      [/usergroup:<account>]
    [/device:<device's account> [/deviceclaims:(<claim-detail>)]
      [/devicegroup:<account>]

    /path:<path>    Path of the file or folder for which the effective access
                    is to be computed for.

    /targetMachine:<machine-name>
                    Name of the machine on which the file or folder is present.
                    Note: This must point to the actual machine on which the
                    file or folder resides. This has to accommodate mapped
                    drives and UNC paths that could be a DFS share.

    /shareSD:<SDDL> Share's security descriptor (SDDL).

    /user:<account> User account for whom effective access it to be determined
                    for. Refer below for the supported formats to identify an
                    account.

    /userclaim:(<claim-detail>)
                    Claim(s) to be included in the user's token for the purpose
                    determining effective access. Refer below for the format of
                    the claim.

    /usergroup:<account>
                    Additional user group to be included for determining
                    effective access.

    /device:<account>
                    Device account to be included in the evaluation of effective
                    access. Note: This is used to simulate as if the user was
                    accessing the file or folder from the specified device.

    /deviceclaim:(<claim-detail>)
                    Device claim(s) to be included in the user's token for the
                    purpose of computing effective access.

    /devicegroup:<account>
                    Additional device group to be used in evaluation of
                    effective access. Note: This is used to simulate the
                    device's group membership.

Supported format for parameter values:

<account>       Must be specified in one of the following formats:
                1) String SID for the domain / local account.
                2) Uniquely resolvable account name in the host machine's
                   domain.
                3) Uniquely resolvable qualified account name in the format -
                   <domain>\<account name>.

                Note: When specifying a device account, the device name must be
                      suffixed with the terminating '$'.

<claim-detail>  Must be specified in the following format:
                (<claim-id>,<claim-defn-type>,<value>)

    <claim-id>          Unique identifier (not display name) used to identify
                        a claim.

    <claim-defn-type>   Claim definition type. Permitted vales:
                        1. Integer
                        2. Boolean
                        3. String
                        4. MultiValuedString

    <value>             Value of the claim.

                        Integer values must be specified in decimal, octal
                        (prefixed with 0) or hex format (prefixed with 0x).

                        The only permitted values for Boolean are 0, false, 1
                        and true.

                        MultiValuedString values must be in the following
                        format:
                        [<value>[,<value>][,...]]

                Note: The semantics of the claims specified will not be
                      validated. E.g. claim definition type, claim-id or
                      value(s) mismatch will be ignored.

Sample claims:

    /userclaim:(_user_building,Integer,0x5)

    /userclaim:(_user_empid,Integer,07654231)

    /userclaim:(_user_code,Integer,-3)

    /deviceclaim:(_device_bitlocker_on,Boolean,true)

    /deviceclaim:(_device_is_hosted_desktop,Boolean,0)

    /deviceclaim:(_user_department,String,Finance)

    /deviceclaim:(_user_project,MultiValuedString,[X,""Fire & Ice""])
";

        //
        // Parameters obtained from command line
        //
        static string path = null;
        static string targetMachine = null;
        static RawSecurityDescriptor shareSD = null;
        static SecurityIdentifier userSid = null;
        static SecurityIdentifier deviceSid = null;
        static ClaimValueDictionary userClaims = new ClaimValueDictionary(ClaimDefinitionType.User);
        static ClaimValueDictionary deviceClaims = new ClaimValueDictionary(ClaimDefinitionType.Device);
        static GroupsCollection userGroups = new GroupsCollection(GroupType.User);
        static GroupsCollection deviceGroups = new GroupsCollection(GroupType.Device);

        static void Main(string[] args)
        {
            const string PATH_PREFIX = "/path:";
            const string TARGET_PREFIX = "/targetMachine:";
            const string SHARESD_PREFIX = "/shareSD:";
            const string USER_PREFIX = "/user:";
            const string USER_GROUP_PREFIX = "/usergroup:";
            const string USER_CLAIM_PREFIX = "/userclaim:";
            const string DEVICE_PREFIX = "/device:";
            const string DEVICE_GROUP_PREFIX = "/devicegroup:";
            const string DEVICE_CLAIM_PREFIX = "/deviceclaim:";
            const string CLAIM_PREFIX_REGEX = "^/(?<claimdefinitiontype>user|device)claim:";
            const string ATTRIBUTE_REGEX = @"\((?<id>[\w]+),(?<type>[\w]+),(?<value>.+)\)";

            string cmdLnPrefixInProcess = null;
            string cmdLnParam = null;
            bool showUsageAndExit = (args.Length == 0);

            try
            {
                foreach (var arg in args)
                {
                    cmdLnParam = arg;

                    if (arg.StartsWith(PATH_PREFIX, StringComparison.Ordinal))
                    {
                        cmdLnPrefixInProcess = PATH_PREFIX;
                        if (path == null)
                        {
                            path = arg.Substring(PATH_PREFIX.Length);
                        }
                        else
                        {
                            Helper.ReportDuplicateCmdLnParam(arg);
                        }
                        continue;
                    }

                    if (arg.StartsWith(TARGET_PREFIX, StringComparison.Ordinal))
                    {
                        cmdLnPrefixInProcess = TARGET_PREFIX;
                        if (targetMachine == null)
                        {
                            targetMachine = arg.Substring(TARGET_PREFIX.Length);
                        }
                        else
                        {
                            Helper.ReportDuplicateCmdLnParam(arg);
                        }
                        continue;
                    }

                    if (arg.StartsWith(SHARESD_PREFIX, StringComparison.Ordinal))
                    {
                        cmdLnPrefixInProcess = SHARESD_PREFIX;
                        if (shareSD == null)
                        {
                            shareSD = new RawSecurityDescriptor(arg.Substring(SHARESD_PREFIX.Length));
                        }
                        else
                        {
                            Helper.ReportDuplicateCmdLnParam(arg);
                        }
                        continue;
                    }

                    if (arg.StartsWith(USER_PREFIX, StringComparison.Ordinal))
                    {
                        cmdLnPrefixInProcess = USER_PREFIX;
                        if (userSid == null)
                        {
                            try
                            {
                                userSid = Helper.GetSidForObject(arg.Substring(USER_PREFIX.Length));
                            }
                            catch(IdentityNotMappedException)
                            {
                                Helper.ReportIgnoredAccount(arg.Substring(USER_PREFIX.Length), arg);
                            }
                        }
                        else
                        {
                            Helper.ReportDuplicateCmdLnParam(arg);
                        }
                        continue;
                    }

                    if (arg.StartsWith(DEVICE_PREFIX, StringComparison.Ordinal))
                    {
                        cmdLnPrefixInProcess = USER_PREFIX;
                        if (deviceSid == null)
                        {
                            try
                            {
                                deviceSid = Helper.GetSidForObject(arg.Substring(DEVICE_PREFIX.Length), true);
                            }
                            catch (IdentityNotMappedException)
                            {
                                Helper.ReportIgnoredAccount(arg.Substring(DEVICE_PREFIX.Length), arg);
                            }
                        }
                        else
                        {
                            Helper.ReportDuplicateCmdLnParam(arg);
                        }
                        continue;
                    }

                    if (arg.StartsWith(USER_CLAIM_PREFIX, StringComparison.Ordinal) ||
                        arg.StartsWith(DEVICE_CLAIM_PREFIX, StringComparison.Ordinal))
                    {
                        Match result = Regex.Match(arg, CLAIM_PREFIX_REGEX + "(?<claiminfo>" + ATTRIBUTE_REGEX + ")");

                        if (result.Success)
                        {
                            bool userClaimDefinitionType = result.Groups["claimdefinitiontype"].Value == "user";

                            cmdLnPrefixInProcess = userClaimDefinitionType ? USER_CLAIM_PREFIX : DEVICE_CLAIM_PREFIX;

                            string claimInfo = result.Groups["claiminfo"].Value;
                            string claimID = result.Groups["id"].Value;
                            string valueType = result.Groups["type"].Value;
                            string value = result.Groups["value"].Value;

                            ClaimValueDictionary claimsColln = userClaimDefinitionType ? userClaims : deviceClaims;

                            try
                            {
                                if (claimsColln.ContainsKey(claimID))
                                {
                                    Helper.ReportDuplicateClaim(claimID, arg);
                                }
                                else
                                {
                                    claimsColln.Add(claimID, new ClaimValue(
                                                                    (ClaimValueType)
                                                                    Enum.Parse(typeof(ClaimValueType), valueType),
                                                                    value));
                                }
                            }
                            catch (BadValueException e)
                            {
                                Helper.LogWarning("Ignoring claim - ");
                                Console.Write(claimInfo);
                                Helper.LogWarning(string.Format(CultureInfo.CurrentCulture, ". {0}", e.Message), true);
                            }
                            catch (Exception e)
                            {
                                Debug.Assert(e.GetType() == typeof(ArgumentException));

                                Helper.LogWarning("Ignoring claim - ");
                                Console.Write(claimInfo);

                                Helper.LogWarning(string.Format(CultureInfo.CurrentCulture,
                                                                ", with unrecognized value type - {0}",
                                                                valueType),
                                                  true);
                            }
                        }
                        else
                        {
                            Helper.LogWarning("Ignoring ill-formatted claim specification in parameter: ");
                            Console.WriteLine(arg);
                        }

                        continue;
                    }

                    if (arg.StartsWith(USER_GROUP_PREFIX, StringComparison.Ordinal) ||
                        arg.StartsWith(DEVICE_GROUP_PREFIX, StringComparison.Ordinal))
                    {
                        bool groupTypeUser = arg.StartsWith(USER_GROUP_PREFIX, StringComparison.Ordinal);
                        string groupInfo = arg.Substring(groupTypeUser
                                                         ? USER_GROUP_PREFIX.Length :
                                                         DEVICE_GROUP_PREFIX.Length);

                        cmdLnPrefixInProcess = groupTypeUser ? USER_GROUP_PREFIX : DEVICE_GROUP_PREFIX;

                        GroupsCollection groupsColln = groupTypeUser ? userGroups : deviceGroups;

                        try
                        {
                            groupsColln.Add(Helper.GetSidForObject(groupInfo, !groupTypeUser));
                        }
                        catch (IdentityNotMappedException)
                        {
                            Helper.ReportIgnoredAccount(groupInfo, arg);
                        }

                        continue;
                    }

                    Helper.LogWarning("Ignoring unrecognized parameter: ");
                    Console.WriteLine(arg);
                }

            }
            catch (ArgumentException e)
            {
                showUsageAndExit = true;

                switch (cmdLnPrefixInProcess)
                {
                    case USER_PREFIX:
                    case USER_GROUP_PREFIX:
                    case DEVICE_PREFIX:
                    case DEVICE_GROUP_PREFIX:
                        {
                            Helper.LogError("Invalid string SID in parameter: ");
                            Helper.LogWarning(cmdLnParam, true);
                            break;
                        }
                    case SHARESD_PREFIX:
                        {
                            Helper.LogError("The SDDL form of the security descriptor is invalid in parameter: ");
                            Helper.LogWarning(cmdLnParam, true);
                            break;
                        }
                    default:
                        {
                            showUsageAndExit = false;
                            Console.WriteLine(e.Message);
                            break;
                        }
                }
            }
            catch (Exception e)
            {
                showUsageAndExit = true;
                Console.WriteLine(e.Message);
                return;
            }
            finally
            {
                if (!showUsageAndExit)
                {
                    bool missingRequiredParam = (userSid == null);

                    if (string.IsNullOrEmpty(path))
                    {
                        missingRequiredParam = true;
                    }
                    else if (!Directory.Exists(path) && !File.Exists(path))
                    {
                        Helper.LogError("Could not find specified resource: ");
                        Helper.LogWarning(path, true);

                        //
                        // Don't report more than 1 errors
                        //
                        missingRequiredParam = false;
                    }
                    else if (deviceClaims.Count != 0 && deviceSid == null)
                    {
                        Helper.LogWarning("Device claims ignored since no valid device account was specified");
                    }
                    else if (shareSD != null && string.IsNullOrEmpty(targetMachine))
                    {
                        Helper.LogError("Share's Security descriptor(/shareSD) specified without specifying " +
                                        "/targetMachine.", true);
                        showUsageAndExit = true;
                    }

                    if (missingRequiredParam)
                    {
                        Helper.LogError("Required parameters missing", true);
                        showUsageAndExit = true;
                    }
                }
            }

            if (showUsageAndExit)
            {
                Console.WriteLine(CmdLnUsage,
                                  Path.GetFileNameWithoutExtension(Assembly.GetExecutingAssembly().CodeBase));
                return;
            }

            try
            {
                if (string.IsNullOrEmpty(targetMachine))
                {
                    targetMachine = "localhost";
                }

                var effPerm = new EffectiveAccess(path,
                                                  targetMachine,
                                                  shareSD,
                                                  userSid,
                                                  deviceSid,
                                                  userClaims,
                                                  deviceClaims,
                                                  userGroups,
                                                  deviceGroups);
                effPerm.Evaluate();
                effPerm.GenerateReport();
            }
            catch (Win32Exception win32Exp)
            {
                Helper.LogError("Win32 exception: ");
                Helper.LogWarning(win32Exp.Message);
            }
            catch (Exception e)
            {
                Helper.LogError("Unhandled exception: ");
                Helper.LogWarning(e.Message);
            }
        }
    }
}
