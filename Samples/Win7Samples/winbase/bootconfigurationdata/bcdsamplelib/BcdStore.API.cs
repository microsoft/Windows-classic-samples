// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


// note please read the readme.txt file for notes on escalation/LUA 
using System;
using System.Collections.Generic;
using System.Text;
using System.Management;
using ROOT.WMI;

namespace Microsoft.Samples.BcdSampleLib
{
	public class BcdStore_API
	{

		/// <summary>
		/// Entry types enum, for ease of usage of a method call or two.
		/// </summary>
		public enum OSEntryTypes
		{
			Legacy,
			Vista
		}

		/// static constructor to make sure that static calls into the ROOT.WMI.BcdStore class have the
		/// correct setting for StaticScope property.  Otherwise all calls utilizing ROOT.WMI.BcdStore class that call the classes
		/// static methods will all fail.
		static BcdStore_API()
		{

			ROOT.WMI.BcdStore.StaticScope = BcdStore_API.ImpersonationScope;

		}

		/// <summary>
		/// this is the standard impersonation scope required to make most bcd calls work, put into a property for convenient access
		/// </summary>
		public static ManagementScope ImpersonationScope
		{
			get
			{
				ConnectionOptions options = new ConnectionOptions();
				options.Impersonation = ImpersonationLevel.Impersonate;
				options.EnablePrivileges = true;

				ManagementScope MScope = new ManagementScope("root\\WMI", options);
				return MScope;

			}

		}

		public static bool BackupSystemStoreToFile(string newStorePath)
		{
			bool success = ROOT.WMI.BcdStore.ExportStore(newStorePath);
			return success;
		}

		public static bool RestoreSystemStoreFromBackupFile(string newStorePath)
		{
			bool success = ROOT.WMI.BcdStore.ImportStore(newStorePath);
			return success;
		}

		public static bool DeleteOsEntry(string osEntryId)
		{
			return DeleteOsEntry(osEntryId, "");
		}

		public static bool DeleteOsEntry(string osEntryId, string storePath)
		{

			ROOT.WMI.BcdStore bcdStore = new ROOT.WMI.BcdStore(BcdStore_API.ImpersonationScope, storePath);
			bool bTestResult = bcdStore.DeleteObject(osEntryId);
			return bTestResult;
		}

		public static List<string> GetBootManagerGUIDEntriesDisplayListAsList()
		{
			return GetBootManagerGUIDEntriesDisplayListAsList("");
		}

		public static List<string> GetBootManagerGUIDEntriesDisplayListAsList(string storePath)
		{

			List<string> guidList = null;

			BcdObject bcdObject = new BcdObject(BcdStore_API.ImpersonationScope, Constants.GUID_WINDOWS_BOOTMGR, storePath);

			ManagementBaseObject mboOut;
			bool success = bcdObject.GetElement(Constants.BCDE_BOOTMGR_TYPE_DISPLAY_ORDER, out mboOut);

			if (success)
			{

				string[] oSList = (string[])mboOut.GetPropertyValue("Ids");
				guidList = new List<string>();
				foreach (string s in oSList)
					guidList.Add(s);

			}
			return guidList;

		}

		public static List<string> EnumerateObjectsByType( uint bcdType, string storePath )
		{

			List<string> objectGuidList = null;

			ROOT.WMI.BcdStore bcdStore = new ROOT.WMI.BcdStore(BcdStore_API.ImpersonationScope, storePath);
			ManagementBaseObject[] mboArray;

			bool success = bcdStore.EnumerateObjects(bcdType, out mboArray);
			if (success)
			{
				objectGuidList = new List<string>();

				foreach (ManagementBaseObject mbo in mboArray)
				{

					objectGuidList.Add((string)mbo.GetPropertyValue("Id"));

				}
			}
			return objectGuidList;

		}

		public static bool CreateNewOSEntry(BcdStore_API.OSEntryTypes osEntryType, string storePath, out string newGuidOut)
		{

			newGuidOut = ""; // default to empty guid for failure

			ROOT.WMI.BcdStore bcdStore = new ROOT.WMI.BcdStore(BcdStore_API.ImpersonationScope, storePath);

			string newGuid = "{" + Guid.NewGuid().ToString() + "}";

			ManagementBaseObject newObject;
			bool success;

			if (osEntryType == OSEntryTypes.Legacy)
				success = bcdStore.CreateObject(newGuid, Constants.BCDE_LEGACY_OS_ENTRY, out newObject);
			else
				success = bcdStore.CreateObject(newGuid, Constants.BCDE_VISTA_OS_ENTRY, out newObject);

			if (success)
			{
				newGuidOut = newGuid;
			}
			return success;
		}

		/// <summary>
		/// Overwrite the given bcd store's OS Display List (what displays on system load) with the given string array of guids.
		/// Note this is very powerful make sure you know what you are doing.
		/// </summary>
		/// <param name="arrNewGuidArray"></param>
		/// <param name="storePath"></param>
		/// <param name="errorDetails"></param>
		/// <returns></returns>
		public static bool SetOSDisplayListGuids(string[] arrNewGuidArray, string storePath, out string errorDetails)
		{

			// optional additional info on any errors encountered, sent back as output variable
			errorDetails = string.Empty; // default to empty string ( no errors encountered)

			if (arrNewGuidArray.Length == 0)
			{
				errorDetails = "This will delete ALL OS displayed list items, effectively making system boot NON-FUNCTIONAL.  Disabling this functionality per this version of library...";
				return false;
			}

			BcdObject bcdBootMgr = new BcdObject(BcdStore_API.ImpersonationScope, Constants.GUID_WINDOWS_BOOTMGR, storePath);

			ManagementBaseObject mboListElements;
			bool successGetDisplayOrderObj = bcdBootMgr.GetElement(Constants.BCDE_BOOTMGR_TYPE_DISPLAY_ORDER,
							out mboListElements);

			if (!successGetDisplayOrderObj)
			{
				errorDetails = "Trouble getting boot manager display order object";
				return false;

			}
			else
			{

				BcdObjectListElement listElement = new BcdObjectListElement(mboListElements);

				// save new display list via bcdbootmgr object's SetObjectListElement method.
				bool setElementStatus = bcdBootMgr.SetObjectListElement(arrNewGuidArray, Constants.BCDE_BOOTMGR_TYPE_DISPLAY_ORDER);
				if (!setElementStatus)
				{
					errorDetails = "Trouble setting new display order list via SetObjectListElement call";
					return false;
				}

			}

			return true;
		}

		/// <summary>
		/// Adds the passed in (valid store) GUID to the end of the current OS entry display list for store.
		/// </summary>
		/// <param name="newGuid"></param>
		/// <param name="storePath"></param>
		/// <param name="errorDetails"></param>
		/// <returns></returns>
		public static bool AddGuidToOSDisplayList(string newGuid, string storePath, out string errorDetails)
		{

			// optional additional info on any errors encountered, sent back as output variable
			errorDetails = string.Empty; // default to empty string ( no errors encountered)

			BcdObject bcdBootMgr = new BcdObject(BcdStore_API.ImpersonationScope, Constants.GUID_WINDOWS_BOOTMGR, storePath);

			ManagementBaseObject mboListElements;
			bool successGetDisplayOrderObj = bcdBootMgr.GetElement(Constants.BCDE_BOOTMGR_TYPE_DISPLAY_ORDER,
							out mboListElements);

			if (! successGetDisplayOrderObj)
			{
				errorDetails = "Trouble getting boot manager display order object";
				return false;

			}
			else
			{

				BcdObjectListElement listElement = new BcdObjectListElement(mboListElements);
				string[] osDisplayList = listElement.Ids;

				int len = osDisplayList.Length;
				string[] newOsDisplayList = new string[len + 1];
				for (int i = 0; i < len; i++)
				{
					newOsDisplayList[i] = osDisplayList[i];
				}

				newOsDisplayList[len] = newGuid;

				//if (_updateGuidListViaObject)
				//{
				//    // this code needs research to figure out why it's not working:

				//    //listElement.AutoCommit = true;
				//    //listElement.Ids = newOsDisplayList;
				//    //listElement.CommitObject();

				//    // DOES NOT WORK, error in WMI classes?
				//}
				//else
				//{

					// save new display list via bcdbootmgr object's SetObjectListElement method.
					bool setElementStatus = bcdBootMgr.SetObjectListElement(newOsDisplayList, Constants.BCDE_BOOTMGR_TYPE_DISPLAY_ORDER);
					if ( !setElementStatus)
					{
						errorDetails = "Trouble setting new display order list via SetObjectListElement call";
						return false;
					}

				//}

			}

			return true;
		}

		public static bool CloneExistingOSEntry(string osEntryToCloneGuid, string newOSEntryDescriptionName, string storePath, out string errorDetails)
		{

			errorDetails = string.Empty; // default error string to empty string/no errors
			ROOT.WMI.BcdStore bcdStore = new ROOT.WMI.BcdStore(BcdStore_API.ImpersonationScope, storePath);
			string newGuid = "{" + Guid.NewGuid().ToString() + "}";

			ManagementBaseObject newObject;

			bool success = bcdStore.CopyObject(Constants.BCD_COPY_CREATE_NEW_OBJECT_IDENTIFIER, osEntryToCloneGuid, storePath, out newObject);
			if (!success)
			{
				errorDetails = "failure with bcdstore copy object call";
				return false;
			}

			BcdObject newBcdObject = new BcdObject(newObject);
			string newGuid2 = newBcdObject.Id.ToString();

			if (success)
			{

				string errorString;
				bool successfulAddToOsList = AddGuidToOSDisplayList(newGuid2, storePath, out errorString);
				if (!successfulAddToOsList)
				{
					errorDetails = errorString;
					return false;
				}

				bool setDescripStatus = ChangeOSEntryDescription(newBcdObject.Id, newOSEntryDescriptionName, storePath);

				if (!setDescripStatus)
				{
					errorDetails = "failure with setting new OS entry description, otherwise success";
					return false;
				}

			}

			return true;


		}

		public static bool ChangeOSEntryDescription(string guid, string newDescription, string storePath)
		{

			BcdObject nonEmbeddedObjRef = new BcdObject(BcdStore_API.ImpersonationScope, guid, storePath);

			bool setDescripStatus = nonEmbeddedObjRef.SetStringElement(newDescription, Constants.BCDE_LIBRARY_TYPE_DESCRIPTION);

			return setDescripStatus;

		}

		public static string GetDescriptionForGuid(string guid, string storePath)
		{

			string descripFound = null;

			BcdObject nonEmbeddedObjRef = new BcdObject(BcdStore_API.ImpersonationScope, guid, storePath);

			System.Management.ManagementBaseObject Element;

			bool getDescripStatus = nonEmbeddedObjRef.GetElement(Constants.BCDE_LIBRARY_TYPE_DESCRIPTION, out Element);
			if ( getDescripStatus )
			{
				descripFound = Element.GetPropertyValue("String").ToString();
			}

			return descripFound;

		}

	}

}
