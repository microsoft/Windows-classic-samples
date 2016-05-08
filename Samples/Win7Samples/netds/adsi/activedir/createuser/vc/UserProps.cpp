
#define INC_OLE2
#define UNICODE 1
#define _WIN32_DCOM

#include <windows.h>
#include <winuser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <activeds.h>

#include "UserProps.h"


////////////////////////////////////////////////////
//        Mandatory
////////////////////////////////////////////////////

WCHAR * parwszUserMandatoryProps []= 
{
    // Attrib Name              Type                                Multi or Single Valued
    L"cn",                      L"DirectoryString",                 L"SV",    
    L"instanceType",            L"INTEGER",                         L"SV",    
    L"nTSecurityDescriptor",    L"ObjectSecurityDescriptor",        L"SV",    
    L"objectCategory",          L"DN",                              L"SV",    
    L"objectClass",             L"OID",                             L"MV",    
    L"objectSid",               L"OctetString",                     L"SV",    
    L"sAMAccountName",          L"DirectoryString",                 L"SV"
};


////////////////////////////////////////////////////
//        OptionalProperties
////////////////////////////////////////////////////

WCHAR * parwszUserOptionalProps []= 
{
    // Attrib Name                      Type                        Multi or Single Valued
    L"accountExpires",                  L"INTEGER8",	            L"SV",       		
    L"aCSPolicyName",		            L"DirectoryString",	        L"SV",    
    L"adminCount",		                L"INTEGER",	                L"SV",    
    L"adminDescription",		        L"DirectoryString",	        L"SV",    
    L"adminDisplayName",		        L"DirectoryString",	        L"SV",    
    L"allowedAttributes",		        L"OID",	                    L"MV",    
    L"allowedAttributesEffective",		L"OID",	                    L"MV",    
    L"allowedChildClasses",		        L"OID",	                    L"MV",    
    L"allowedChildClassesEffective",	L"OID",	                    L"MV",    	
    L"alternateSecurityIdentities",		L"IA5String",	            L"MV",    
    L"altRecipient",		            L"DN",	                    L"SV",    
    L"altRecipientBL",		            L"DN",	                    L"MV",    
    L"altSecurityIdentities",		    L"DirectoryString",	        L"MV",    
    L"ancestorID",		                L"OctetString",	            L"SV",    
    L"assistant",		                L"DN",	                    L"SV",    
    L"assocRemoteDXA",		            L"DN",	                    L"MV",    
    L"attributeCertificate",		    L"OctetString",	            L"MV",    
    L"AuthOrig",		                L"ORName",	                L"MV",    
    L"AuthOrigBL",		                L"DN",	                    L"MV",    
    L"autoReply",		                L"Boolean",	                L"SV",    
    L"autoReplyMessage",		        L"DirectoryString",	        L"SV",    
    L"autoReplySubject",		        L"DirectoryString",	        L"SV",    
    L"badPasswordTime",		            L"INTEGER8",	            L"SV",    
    L"badPwdCount",		                L"INTEGER",	                L"SV",    
    L"bridgeheadServerListBL",		    L"DN",	                    L"MV",    
    L"c",		                        L"DirectoryString",	        L"SV",    
    L"CanCreatePFBL",		            L"DN",	                    L"MV",    
    L"CanCreatePFDLBL",		            L"DN",	                    L"MV",    
    L"CanNotCreatePFBL",		        L"DN",	                    L"MV",    
    L"CanNotCreatePFDLBL",		        L"DN",	                    L"MV",    
    L"canonicalName",		            L"DirectoryString",	        L"MV",    
    L"co",		                        L"DirectoryString",	        L"SV",    
    L"codePage",		                L"INTEGER",	                L"SV",    
    L"comment",		                    L"DirectoryString",	        L"SV",    
    L"controlAccessRights",		        L"OctetString",	            L"MV",    
    L"countryCode",		                L"INTEGER",	                L"SV",    
    L"createTimeStamp",		            L"GeneralizedTime",	        L"SV",    
    L"dBCSPwd",		                    L"OctetString",	            L"SV",    
    L"defaultClassStore",		        L"DN",	                    L"MV",    
    L"deletedItemFlags",		        L"INTEGER",	                L"SV",    
    L"delivContLength",		            L"INTEGER",	                L"SV",    
    L"delivEITs",		                L"OctetString",	            L"MV",    
    L"deliverAndRedirect",		        L"Boolean",	                L"SV",    
    L"delivExtContTypes",		        L"OctetString",	            L"MV",    
    L"department",		                L"DirectoryString",	        L"SV",    
    L"description",		                L"DirectoryString",	        L"MV",    
    L"desktopProfile",		            L"DirectoryString",	        L"SV",    
    L"destinationIndicator",		    L"PrintableString",	        L"MV",    
    L"directReports",		            L"DN",	                    L"MV",    
    L"displayName",		                L"DirectoryString",	        L"SV",    
    L"displayNamePrintable",		    L"PrintableString",	        L"SV",    
    L"distinguishedName",		        L"DN",	                    L"SV",    
    L"division",		                L"DirectoryString",	        L"SV",    
    L"DLMemRejectPerms",		        L"ORName",	                L"MV",    
    L"DLMemRejectPermsBL",		        L"DN",	                    L"MV",    
    L"DLMemSubmitPerms",		        L"ORName",	                L"MV",    
    L"DLMemSubmitPermsBL",		        L"DN",	                    L"MV",    
    L"dnQualifier",		                L"DirectoryString",	        L"SV",    
    L"dSASignature",		            L"OctetString",	            L"SV",    
    L"dynamicLDAPServer",		        L"DN",	                    L"SV",    
    L"employeeID",		                L"DirectoryString",	        L"SV",    
    L"expirationTime",		            L"UTCTime",	                L"SV",    
    L"extensionAttribute1",		        L"DirectoryString",	        L"SV",    
    L"extensionAttribute10",		    L"DirectoryString",	        L"SV",    
    L"extensionAttribute11",		    L"DirectoryString",	        L"SV",    
    L"extensionAttribute12",		    L"DirectoryString",	        L"SV",    
    L"extensionAttribute13",		    L"DirectoryString",	        L"SV",    
    L"extensionAttribute14",		    L"DirectoryString",	        L"SV",    
    L"extensionAttribute15",		    L"DirectoryString",	        L"SV",    
    L"extensionAttribute2",		        L"DirectoryString",	        L"SV",    
    L"extensionAttribute3",		        L"DirectoryString",	        L"SV",    
    L"extensionAttribute4",		        L"DirectoryString",	        L"SV",    
    L"extensionAttribute5",		        L"DirectoryString",	        L"SV",    
    L"extensionAttribute6",		        L"DirectoryString",	        L"SV",    
    L"extensionAttribute7",		        L"DirectoryString",	        L"SV",    
    L"extensionAttribute8",		        L"DirectoryString",	        L"SV",    
    L"extensionAttribute9",		        L"DirectoryString",	        L"SV",    
    L"extensionData",		            L"OctetString",	            L"MV",    
    L"extensionName",		            L"DirectoryString",	        L"MV",    
    L"extensionNameInherited",		    L"DirectoryString",	        L"MV",    
    L"facsimileTelephoneNumber",		L"DirectoryString",	        L"SV",    
    L"flags",		                    L"INTEGER",	                L"SV",    
    L"forwardingAddress",		        L"DirectoryString",	        L"SV",    
    L"fromEntry",		                L"Boolean",	                L"MV",    
    L"frsComputerReferenceBL",		    L"DN",	                    L"MV",    
    L"fRSMemberReferenceBL",		    L"DN",	                    L"MV",    
    L"fSMORoleOwner",		            L"DN",	                    L"SV",    
    L"garbageCollPeriod",		        L"INTEGER",	                L"SV",    
    L"gatewayProxy",		            L"DirectoryString",	        L"MV",    
    L"generationQualifier",		        L"DirectoryString",	        L"SV",    
    L"givenName",		                L"DirectoryString",	        L"SV",    
    L"gPLink",		                    L"DirectoryString",	        L"SV",    
    L"gPOptions",		                L"INTEGER",	                L"SV",    
    L"groupMembershipSAM",		        L"OctetString",	            L"SV",    
    L"groupPriority",		            L"DirectoryString",	        L"MV",    
    L"groupsToIgnore",		            L"DirectoryString",	        L"MV",    
    L"heuristics",		                L"INTEGER",	                L"SV",    
    L"homeDirectory",		            L"DirectoryString",	        L"SV",    
    L"homeDrive",		                L"DirectoryString",	        L"SV",    
    L"homeMDB",		                    L"DN",	                    L"SV",    
    L"homeMDBBL",		                L"DN",	                    L"MV",    
    L"homeMTA",		                    L"DN",	                    L"SV",    
    L"homePhone",		                L"DirectoryString",	        L"SV",    
    L"homePostalAddress",		        L"DirectoryString",	        L"SV",    
    L"iMOBuiltinAccountOpsGroup",		L"DN",	                    L"MV",    
    L"iMOBuiltinAdminGroup",		    L"DN",	                    L"MV",    
    L"iMOBuiltinBackupGroup",		    L"DN",	                    L"MV",    
    L"iMOBuiltinGuestGroup",		    L"DN",	                    L"MV",    
    L"iMOBuiltinPrintOpsGroup",		    L"DN",	                    L"MV",    
    L"iMOBuiltinReplicatorGroup",		L"DN",	                    L"MV",    
    L"iMOBuiltinServerOpsGroup",		L"DN",	                    L"MV",    
    L"iMOBuiltinUsersGroup",		    L"DN",	                    L"MV",    
    L"importedFrom",		            L"DirectoryString",	        L"SV",    
    L"incomingMsgSizeLimit",		    L"INTEGER",	                L"SV",    
    L"initials",		                L"DirectoryString",	        L"SV",    
    L"internationalISDNNumber",		    L"NumericString",	        L"MV",    
    L"internetEncoding",		        L"INTEGER",	                L"SV",    
    L"ipPhone",		                    L"DirectoryString",	        L"SV",    
    L"isCriticalSystemObject",		    L"Boolean",	                L"SV",    
    L"isDeleted",		                L"Boolean",	                L"SV",    
    L"isPrivilegeHolder",		        L"DN",	                    L"MV",    
    L"l",		                        L"DirectoryString",	        L"SV",    
    L"LabeledURI",		                L"DirectoryString",	        L"MV",    
    L"language",		                L"DirectoryString",	        L"SV",    
    L"languageCode",		            L"INTEGER",	                L"SV",    
    L"lastKnownParent",		            L"DN",	                    L"SV",    
    L"lastLogoff",		                L"INTEGER8",	            L"SV",    
    L"lastLogon",		                L"INTEGER8",	            L"SV",    
    L"legacyExchangeDN",		        L"CaseIgnoreString",	    L"SV",    
    L"lmPwdHistory",		            L"OctetString",	            L"MV",    
    L"localeID",		                L"INTEGER",	                L"MV",    
    L"lockoutTime",		                L"INTEGER8",	            L"SV",    
    L"logonCount",		                L"INTEGER",	                L"SV",    
    L"logonHours",		                L"OctetString",	            L"SV",    
    L"logonWorkstation",		        L"OctetString",	            L"SV",    
    L"mail",		                    L"DirectoryString",	        L"SV",    
    L"mailDrop",		                L"DirectoryString",	        L"SV",    
    L"mailNickname",		            L"DirectoryString",	        L"SV",    
    L"maintainAutoReplyHistory",		L"Boolean",	                L"SV",    
    L"managedObjects",		            L"DN",	                    L"MV",    
    L"manager",		                    L"DN",	                    L"SV",    
    L"mAPIRecipient",		            L"Boolean",	                L"SV",    
    L"maxStorage",		                L"INTEGER8",	            L"SV",    
    L"memberOf",		                L"DN",	                    L"MV",    
    L"mhsORAddress",		            L"DirectoryString",	        L"MV",    
    L"middleName",		                L"DirectoryString",	        L"SV",    
    L"mobile",		                    L"DirectoryString",	        L"SV",    
    L"modifyTimeStamp",		            L"GeneralizedTime",	        L"SV",    
    L"mSMQDigests",		                L"OctetString",	            L"MV",    
    L"mSMQDigestsMig",		            L"OctetString",	            L"MV",    
    L"mSMQSignCertificates",		    L"OctetString",	            L"SV",    
    L"mSMQSignCertificatesMig",		    L"OctetString",	            L"SV",    
    L"msNPAllowDialin",		            L"Boolean",	                L"SV",    
    L"msNPCallingStationID",		    L"IA5String",	            L"MV",    
    L"msNPSavedCallingStationID",		L"IA5String",	            L"MV",    
    L"msRADIUSCallbackNumber",		    L"IA5String",	            L"SV",    
    L"msRADIUSFramedIPAddress",		    L"INTEGER",	                L"SV",    
    L"msRADIUSFramedRoute",		        L"IA5String",	            L"MV",    
    L"msRADIUSServiceType",		        L"INTEGER",	                L"SV",    
    L"msRASSavedCallbackNumber",		L"IA5String",	            L"SV",    
    L"msRASSavedFramedIPAddress",		L"INTEGER",	                L"SV",    
    L"msRASSavedFramedRoute",		    L"IA5String",	            L"MV",    
    L"name",		                    L"DirectoryString",	        L"SV",    
    L"netbootSCPBL",		            L"DN",	                    L"SV",    
    L"networkAddress",		            L"CaseIgnoreString",	    L"MV",    
    L"nNTPCharacterSet",		        L"DirectoryString",	        L"SV",    
    L"nNTPContentFormat",		        L"DirectoryString",	        L"SV",    
    L"nNTPNewsfeeds",		            L"DN",	                    L"MV",    
    L"nonSecurityMemberBL",		        L"DN",	                    L"MV",    
    L"notes",		                    L"DirectoryString",	        L"SV",    
    L"ntPwdHistory",		            L"OctetString",	            L"MV",    
    L"o",		                        L"DirectoryString",	        L"MV",    
    L"objectGUID",		                L"OctetString",	            L"SV",    
    L"objectVersion",		            L"INTEGER",	                L"SV",    
    L"objViewContainers",		        L"DN",	                    L"MV",    
    L"operatorCount",		            L"INTEGER",	                L"SV",    
    L"otherFacsimileTelephoneNumber",	L"DirectoryString",	        L"MV",    	
    L"otherHomePhone",		            L"DirectoryString",	        L"MV",    
    L"otherIpPhone",		            L"DirectoryString",	        L"MV",    
    L"otherLoginWorkstations",		    L"DirectoryString",	        L"MV",    
    L"otherMailbox",		            L"DirectoryString",	        L"MV",    
    L"otherMobile",		                L"DirectoryString",	        L"MV",    
    L"otherPager",		                L"DirectoryString",	        L"MV",    
    L"otherTelephone",		            L"DirectoryString",	        L"MV",    
    L"ou",		                        L"DirectoryString",	        L"MV",    
    L"overrideNNTPContentFormat",		L"Boolean",	                L"SV",    
    L"ownerBL",		                    L"DN",	                    L"SV",    
    L"pager",		                    L"DirectoryString",	        L"SV",    
    L"partialAttributeDeletionList",	L"OctetString",	            L"SV",    	
    L"partialAttributeSet",		        L"OctetString",	            L"SV",    
    L"periodReplStagger",		        L"INTEGER",	                L"SV",    
    L"periodRepSyncTimes",		        L"OctetString",	            L"SV",    
    L"personalTitle",		            L"DirectoryString",	        L"SV",    
    L"physicalDeliveryOfficeName",		L"DirectoryString",	        L"SV",    
    L"pOPCharacterSet",		            L"DirectoryString",	        L"SV",    
    L"pOPContentFormat",		        L"DirectoryString",	        L"SV",    
    L"possibleInferiors",		        L"OID",	                    L"MV",    
    L"postalAddress",		            L"DirectoryString",	        L"MV",    
    L"postalCode",		                L"DirectoryString",	        L"SV",    
    L"postOfficeBox",		            L"DirectoryString",	        L"MV",    
    L"preferredDeliveryMethod",		    L"INTEGER",	                L"MV",    
    L"preferredOU",		                L"DN",	                    L"SV",    
    L"primaryGroupID",		            L"INTEGER",	                L"SV",    
    L"primaryInternationalISDNNumber",	L"DirectoryString",	        L"SV",    	
    L"primaryTelexNumber",		        L"DirectoryString",	        L"SV",    
    L"profilePath",		                L"DirectoryString",	        L"SV",    
    L"protocolSettings",		        L"DirectoryString",	        L"MV",    
    L"proxiedObjectName",		        L"DN",	                    L"SV",    
    L"proxyAddresses",		            L"DirectoryString",	        L"MV",    
    L"publicDelegatesBL",		        L"DN",	                    L"MV",    
    L"pwdLastSet",		                L"INTEGER8",	            L"SV",    
    L"queryPolicyBL",		            L"DN",	                    L"MV",    
    L"registeredAddress",		        L"OctetString",	            L"MV",    
    L"replicatedObjectVersion",		    L"INTEGER",	                L"SV",    
    L"replicationSensitivity",		    L"INTEGER",	                L"SV",    
    L"replPropertyMetaData",		    L"OctetString",	            L"SV",    
    L"replUpToDateVector",		        L"OctetString",	            L"SV",    
    L"repsFrom",		                L"OctetString",	            L"MV",    
    L"repsTo",		                    L"OctetString",	            L"MV",    
    L"repsToExt",		                L"OctetString",	            L"MV",    
    L"reverseMembership",		        L"OctetString",	            L"MV",    
    L"revision",		                L"INTEGER",	                L"SV",    
    L"rid",		                        L"INTEGER",	                L"SV",    
    L"sAMAccountType",		            L"INTEGER",	                L"SV",    
    L"scriptPath",		                L"DirectoryString",	        L"SV",    
    L"securityIdentifier",		        L"OctetString",	            L"SV",    
    L"securityProtocol",		        L"OctetString",	            L"MV",    
    L"seeAlso",		                    L"DN",	                    L"MV",    
    L"serverReferenceBL",		        L"DN",	                    L"SV",    
    L"servicePrincipalName",		    L"DirectoryString",	        L"MV",    
    L"showInAddressBook",		        L"DN",	                    L"MV",    
    L"showInAdvancedViewOnly",		    L"Boolean",	                L"SV",    
    L"sIDHistory",		                L"OctetString",	            L"MV",    
    L"siteObjectBL",		            L"DN",	                    L"MV",    
    L"sn",		                        L"DirectoryString",	        L"SV",    
    L"st",		                        L"DirectoryString",	        L"SV",    
    L"street",		                    L"DirectoryString",	        L"SV",    
    L"streetAddress",		            L"DirectoryString",	        L"SV",    
    L"submissionContLength",		    L"INTEGER",	                L"SV",    
    L"subRefs",		                    L"DN",	                    L"MV",    
    L"subSchemaSubEntry",		        L"DN",	                    L"MV",    
    L"supplementalCredentials",		    L"OctetString",	            L"MV",    
    L"supportedAlgorithms",		        L"OctetString",	            L"SV",    
    L"supportingStackBL",		        L"DN",	                    L"MV",    
    L"systemFlags",		                L"INTEGER",	                L"SV",    
    L"taggedCertificate",		        L"OctetString",	            L"MV",    
    L"telephoneNumber",		            L"DirectoryString",	        L"SV",    
    L"teletexTerminalIdentifier",		L"OctetString",	            L"MV",    
    L"telexNumber",		                L"OctetString",	            L"MV",    
    L"terminalServer",		            L"OctetString",	            L"SV",    
    L"textEncodedORAddress",		    L"DirectoryString",	        L"SV",    
    L"thumbnailLogo",		            L"OctetString",	            L"SV",    
    L"thumbnailPhoto",		            L"OctetString",	            L"SV",    
    L"title",		                    L"DirectoryString",	        L"SV",    
    L"UnauthOrig",		                L"ORName",	                L"MV",    
    L"UnauthOrigBL",		            L"DN",	                    L"MV",    
    L"unicodePwd",		                L"OctetString",	            L"SV",    
    L"url",		                        L"DirectoryString",	        L"MV",    
    L"userAccountControl",		        L"INTEGER",	                L"SV",    
    L"userCert",		                L"OctetString",	            L"SV",    
    L"userCertificate",		            L"OctetString",	            L"MV",    
    L"userParameters",		            L"DirectoryString",	        L"SV",    
    L"userPassword",		            L"OctetString",	            L"MV",    
    L"userPrincipalName",		        L"DirectoryString",	        L"SV",    
    L"userSharedFolder",		        L"DirectoryString",	        L"SV",    
    L"userSharedFolderOther",		    L"DirectoryString",	        L"MV",    
    L"userSMIMECertificate",		    L"OctetString",	            L"MV",    
    L"userWorkstations",		        L"DirectoryString",	        L"SV",    
    L"useServerValues",		            L"Boolean",	                L"SV",    
    L"uSNChanged",		                L"INTEGER8",	            L"SV",    
    L"uSNCreated",		                L"INTEGER8",	            L"SV",    
    L"uSNDSALastObjRemoved",		    L"INTEGER8",	            L"SV",    
    L"USNIntersite",		            L"INTEGER",	                L"SV",    
    L"uSNLastObjRem",		            L"INTEGER8",	            L"SV",    
    L"uSNSource",		                L"INTEGER8",	            L"SV",    
    L"viewContainer1",		            L"DirectoryString",	        L"SV",    
    L"viewContainer2",		            L"DirectoryString",	        L"SV",    
    L"viewContainer3",		            L"DirectoryString",	        L"SV",    
    L"viewSite",		                L"DirectoryString",	        L"SV",    
    L"voiceMailFlags",		            L"OctetString",	            L"MV",    
    L"voiceMailGreetings",		        L"DirectoryString",	        L"MV",    
    L"voiceMailPassword",		        L"DirectoryString",	        L"SV",    
    L"voiceMailRecordedName",		    L"OctetString",	            L"SV",    
    L"voiceMailRecordingLength",		L"INTEGER",	                L"MV",    
    L"voiceMailSpeed",		            L"INTEGER",	                L"SV",    
    L"voiceMailSystemGUID",		        L"OctetString",	            L"SV",    
    L"voiceMailUserID",		            L"DirectoryString",	        L"SV",    
    L"voiceMailVolume",		            L"INTEGER",	                L"SV",    
    L"wbemPath",		                L"DirectoryString",	        L"MV",    
    L"wellKnownObjects",		        L"1.2.840.113556.1.4.903",	L"MV",    
    L"whenChanged",		                L"GeneralizedTime",	        L"SV",    
    L"whenCreated",		                L"GeneralizedTime",	        L"SV",    
    L"wWWHomePage",		                L"DirectoryString",	        L"SV",    
    L"x121Address",		                L"NumericString",	        L"MV",    
    L"x500AccessControlList",		    L"OctetString",	            L"SV"
};

BOOL FindAttribFromName(LPWSTR pwszName,WCHAR * pwsArray[],int iSize,LPWSTR * ppwszRetAttType, LPWSTR * ppwszRetAttribMultiOrSingle)
{
    *ppwszRetAttType                  = NULL; 
    *ppwszRetAttribMultiOrSingle      = NULL;

    for (int x =0; x< iSize; x+=3)
        if (_wcsicmp(pwsArray[x],pwszName)==0)
        {
            *ppwszRetAttType              = pwsArray[x+1];
            *ppwszRetAttribMultiOrSingle  = pwsArray[x+2];
            return TRUE;             
        }
    return FALSE;
}
 
BOOL MapMandatoryUserAttribToType(LPWSTR pwszAttrib,LPWSTR *ppwszRetAttType, LPWSTR *ppwszRetAttribMultiOrSingle)
{
    return FindAttribFromName(pwszAttrib,parwszUserMandatoryProps,sizeof(parwszUserMandatoryProps),ppwszRetAttType, ppwszRetAttribMultiOrSingle);
}

BOOL MapOptionalUserAttribToType(LPWSTR pwszAttrib,LPWSTR *ppwszRetAttType, LPWSTR *ppwszRetAttribMultiOrSingle)
{
    return FindAttribFromName(pwszAttrib,parwszUserOptionalProps,sizeof(parwszUserOptionalProps),ppwszRetAttType, ppwszRetAttribMultiOrSingle );
}

BOOL MapUserAttribToType(LPWSTR pwszAttrib,LPWSTR *ppwszRetAttType, LPWSTR *ppwszRetAttribMultiOrSingle)
{
    BOOL bret;
    
    bret = MapMandatoryUserAttribToType(pwszAttrib,ppwszRetAttType, ppwszRetAttribMultiOrSingle);

    if (!bret)
        bret = MapOptionalUserAttribToType(pwszAttrib,ppwszRetAttType, ppwszRetAttribMultiOrSingle);

    return bret;
}

struct ADSTYPEsyntaxmapping
{
    LPWSTR pwszSyntax;
    ADSTYPE adsType;
};

ADSTYPEsyntaxmapping aSyntaxMap[] =
{
    { L"Boolean",                    ADSTYPE_BOOLEAN             },
    { L"Integer",              ADSTYPE_INTEGER             },
    { L"OctetString",          ADSTYPE_OCTET_STRING        },
                                                                
    // The following are                                        
    // in ADS only                                              
    { L"Counter",              ADSTYPE_INTEGER             },
    { L"ADsPath",              ADSTYPE_CASE_IGNORE_STRING  },
    { L"EmailAddress",         ADSTYPE_CASE_IGNORE_STRING  },
    { L"FaxNumber",            ADSTYPE_CASE_IGNORE_STRING  },  
    { L"Interval",             ADSTYPE_INTEGER             },
    { L"List",                 ADSTYPE_OCTET_STRING        },
    { L"NetAddress",           ADSTYPE_CASE_IGNORE_STRING  },
    { L"Path",                 ADSTYPE_CASE_IGNORE_STRING  },
    { L"PhoneNumber",          ADSTYPE_CASE_IGNORE_STRING  },
    { L"PostalAddress",        ADSTYPE_CASE_IGNORE_STRING  },
    { L"SmallInterval",        ADSTYPE_INTEGER             },
    { L"String",               ADSTYPE_CASE_IGNORE_STRING  },
    { L"Time",                 ADSTYPE_UTC_TIME            },
                                                                
    // The following are in         NTDS only                   
    { L"INTEGER8",             ADSTYPE_LARGE_INTEGER       },
    { L"UTCTime",              ADSTYPE_UTC_TIME            },
    { L"DN",                   ADSTYPE_DN_STRING           },
    { L"OID",                  ADSTYPE_CASE_IGNORE_STRING  },
    { L"DirectoryString",      ADSTYPE_CASE_IGNORE_STRING  },
    { L"PrintableString",      ADSTYPE_PRINTABLE_STRING    },
    { L"CaseIgnoreString",     ADSTYPE_CASE_IGNORE_STRING  },
    { L"NumericString",        ADSTYPE_NUMERIC_STRING      },
    { L"IA5String",            ADSTYPE_PRINTABLE_STRING    },
    { L"PresentationAddresses",ADSTYPE_CASE_IGNORE_STRING  },
    { L"ORName",               ADSTYPE_INVALID             },
    { L"AccessPointDN",        ADSTYPE_INVALID             },
    { L"NTSecurityDescriptor", ADSTYPE_OCTET_STRING        },
    { L"GeneralizedTime",      ADSTYPE_UTC_TIME            }
                                    
};

// TAkes a String for the Adstype and returns the appropriate ADSTYPE enum
ADSTYPE MapTypeToADSTYPE(LPWSTR pwszName)
{
    int iSize = sizeof(aSyntaxMap);

    for (int x =0; x< iSize; x++)
        if (_wcsicmp(aSyntaxMap[x].pwszSyntax,pwszName)==0)
            return aSyntaxMap[x].adsType;             
    return ADSTYPE_UNKNOWN ;
}


struct ADSTYPE_TO_STRING_ENTRY
{
    LPWSTR      pwszTypeName;
    ADSTYPE     AdsType;
};

ADSTYPE_TO_STRING_ENTRY pAdsTypeToStringList []= 
{ 
    L"CaseExactString",  ADSTYPE_CASE_EXACT_STRING ,
    L"CaseIgnoreString", ADSTYPE_CASE_IGNORE_STRING ,
    L"",                 ADSTYPE_PRINTABLE_STRING ,
    L"",                 ADSTYPE_NUMERIC_STRING ,
    L"Boolean",          ADSTYPE_BOOLEAN ,
    L"",                 ADSTYPE_INTEGER ,
    L"", ADSTYPE_OCTET_STRING ,
    L"", ADSTYPE_UTC_TIME ,
    L"", ADSTYPE_LARGE_INTEGER ,
    L"", ADSTYPE_PROV_SPECIFIC ,
    L"", ADSTYPE_OBJECT_CLASS ,
    L"", ADSTYPE_CASEIGNORE_LIST ,
    L"", ADSTYPE_OCTET_LIST ,
    L"", ADSTYPE_PATH ,
    L"", ADSTYPE_POSTALADDRESS ,
    L"", ADSTYPE_TIMESTAMP ,
    L"", ADSTYPE_BACKLINK ,
    L"", ADSTYPE_TYPEDNAME ,
    L"", ADSTYPE_HOLD ,
    L"", ADSTYPE_NETADDRESS ,
    L"", ADSTYPE_REPLICAPOINTER ,
    L"", ADSTYPE_FAXNUMBER ,
    L"", ADSTYPE_EMAIL ,
    L"", ADSTYPE_NT_SECURITY_DESCRIPTOR ,
    L"", ADSTYPE_UNKNOWN 
} ;
 

ADSTYPE MapAdsTypeFromString(LPWSTR pwszAdsType)
{
/*    //"1.2.840.113556.1.4.903",
	Boolean,
	CaseIgnoreString,
	DirectoryString,
	DN,
	GeneralizedTime,
	IA5String,
	INTEGER,
	INTEGER8,
	NumericString,
	ObjectSecurityDescriptor,
	OctetString,
	OID,
	ORName,
	PrintableString,
	UTCTime
*/
    if (_wcsicmp(pwszAdsType,L"Boolean")==0)
    {
        return ADSTYPE_BOOLEAN;
    }
    else if (_wcsicmp(pwszAdsType,L"CaseIgnoreString")==0)
    {
        return ADSTYPE_CASE_IGNORE_STRING;
    }
    else if (_wcsicmp(pwszAdsType,L"DirectoryString")==0)
    {
        return ADSTYPE_CASE_IGNORE_STRING;
    }
    else if (_wcsicmp(pwszAdsType,L"DN")==0)
    {
        return ADSTYPE_CASE_IGNORE_STRING;
    }
    else if (_wcsicmp(pwszAdsType,L"GeneralizedTime")==0)
    {
        return ADSTYPE_UTC_TIME;
    }
    else if (_wcsicmp(pwszAdsType,L"IA5String")==0)
    {
        return ADSTYPE_CASE_IGNORE_STRING;
    }
    else if (_wcsicmp(pwszAdsType,L"INTEGER")==0)
    {
        return ADSTYPE_INTEGER;
    }
    else if (_wcsicmp(pwszAdsType,L"INTEGER8")==0)
    {
        return ADSTYPE_LARGE_INTEGER;
    }
    else if (_wcsicmp(pwszAdsType,L"NumericString")==0)
    {
        return ADSTYPE_NUMERIC_STRING;
    }
    else if (_wcsicmp(pwszAdsType,L"ObjectSecurityDescriptor")==0)
    {
        return ADSTYPE_OCTET_STRING;
    }
    else if (_wcsicmp(pwszAdsType,L"OctetString")==0)
    {
        return ADSTYPE_OCTET_STRING ;
    }
    else if (_wcsicmp(pwszAdsType,L"OID")==0)
    {
        return ADSTYPE_OCTET_STRING;
    }
    else if (_wcsicmp(pwszAdsType,L"ORName")==0)
    {
        return ADSTYPE_CASE_IGNORE_STRING;
    }
    else if (_wcsicmp(pwszAdsType,L"PrintableString")==0)
    {
        return ADSTYPE_PRINTABLE_STRING;
    }
    else if (_wcsicmp(pwszAdsType,L"UTCTime")==0)
    {
        return ADSTYPE_UTC_TIME;
    }
    return ADSTYPE_UNKNOWN;
}
