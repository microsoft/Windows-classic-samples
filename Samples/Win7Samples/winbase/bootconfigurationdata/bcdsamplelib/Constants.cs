// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Samples.BcdSampleLib
{


	/// <summary>
	/// This class was originallys borrowed from Nick Elliot's excellent work on bcd 
	/// (Nick Elliot was a contractor who is not working for MS at this 
	/// time but did some great foundational work with bcd and C#, including this highly useful constants class).
	/// various enhancements and additions have been made.

	/// </summary>
	public class Constants
	{

		// Known GUIDS
		public static readonly string GUID_WINDOWS_BOOTMGR = "{9dea862c-5cdd-4e70-acc1-f32b344d4795}";
		public static readonly string GUID_DEBUGGER_SETTINGS_GROUP = "{4636856e-540f-4170-a130-a84776f4c654}";
		public static readonly string GUID_CURRENT_BOOT_ENTRY = "{fa926493-6f1c-4193-a414-58f0b2456d1e}"; 
		public static readonly string GUID_WINDOWS_LEGACY_NTLDR = "{466f5a88-0af2-4f76-9038-095b170dc21c}";


		// Known Types		 (Dunno About the format?)
		public static readonly UInt32 BCDE_DEVICE_TYPE_BOOT_DEVICE    =0x00000001;
		public static readonly UInt32 BCDE_DEVICE_TYPE_PARTITION      =0x00000002;
		public static readonly UInt32 BCDE_DEVICE_TYPE_FILE           =0x00000003;
		public static readonly UInt32 BCDE_DEVICE_TYPE_RAMDISK        =0x00000004;
		public static readonly UInt32 BCDE_DEVICE_TYPE_UNKNOWN        =0x00000005;


		public static readonly UInt32 BCD_COPY_CREATE_NEW_OBJECT_IDENTIFIER = 0x00000001;
		//
		// Apply to all 
		//

        public static readonly UInt32 BCDE_LIBRARY_TYPE_APPLICATIONPATH =
            MAKE_BCDE_DATA_TYPE(BCDE_CLASS.LIBRARY, BCDE_FORMAT.STRING, 0x000002);

        public static readonly UInt32 BCDE_LIBRARY_TYPE_APPLICATIONDEVICE =
            MAKE_BCDE_DATA_TYPE(BCDE_CLASS.LIBRARY, BCDE_FORMAT.DEVICE, 0x000001);

		public static readonly UInt32 BCDE_LIBRARY_TYPE_DESCRIPTION =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.LIBRARY, BCDE_FORMAT.STRING, 0x000004);

		public static readonly UInt32 BCDE_LIBRARY_TYPE_DEBUGGER_ENABLED =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.LIBRARY, BCDE_FORMAT.BOOLEAN, 0x000010);

		public static readonly UInt32 BCDE_LIBRARY_TYPE_DEBUGGER_TYPE =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.LIBRARY, BCDE_FORMAT.INTEGER, 0x000011);

		public static readonly UInt32 BCDE_LIBRARY_TYPE_DEBUGGER_PORT_ADDRESS =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.LIBRARY, BCDE_FORMAT.INTEGER, 0x000012);

		public static readonly UInt32 BCDE_LIBRARY_TYPE_DEBUGGER_PORT_NUMBER =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.LIBRARY, BCDE_FORMAT.INTEGER, 0x000013);

		public static readonly UInt32 BCDE_LIBRARY_TYPE_DEBUGGER_BAUDRATE =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.LIBRARY, BCDE_FORMAT.INTEGER, 0x000014);

		public static readonly UInt32 BCDE_LIBRARY_TYPE_DEBUGGER_1394_CHANNEL =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.LIBRARY, BCDE_FORMAT.INTEGER, 0x000015);

		
		//
		// Valid types for the Windows Boot Manager.
		//

		public static readonly UInt32 BCDE_BOOTMGR_TYPE_DISPLAY_ORDER  =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.APPLICATION, BCDE_FORMAT.OBJECT_LIST, 0x000001);

		public static readonly UInt32 BCDE_BOOTMGR_TYPE_BOOT_SEQUENCE =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.APPLICATION, BCDE_FORMAT.OBJECT_LIST, 0x000002);

		public static readonly UInt32 BCDE_BOOTMGR_TYPE_DEFAULT_OBJECT =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.APPLICATION, BCDE_FORMAT.OBJECT, 0x000003);

		public static readonly UInt32 BCDE_BOOTMGR_TYPE_TIMEOUT =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.APPLICATION, BCDE_FORMAT.INTEGER, 0x000004);

		//
		// Apply to OS Loader
		//

		public static readonly UInt32 BCDE_OSLOADER_TYPE_OS_DEVICE =
		MAKE_BCDE_DATA_TYPE(BCDE_CLASS.APPLICATION, BCDE_FORMAT.DEVICE, 0x000001);

		public static readonly UInt32 BCDE_OSLOADER_TYPE_SYSTEM_ROOT =
		MAKE_BCDE_DATA_TYPE(BCDE_CLASS.APPLICATION, BCDE_FORMAT.STRING, 0x000002);

		public static readonly UInt32 BCDE_OSLOADER_TYPE_KERNEL_PATH =
		MAKE_BCDE_DATA_TYPE(BCDE_CLASS.APPLICATION, BCDE_FORMAT.STRING, 0x000011);

		public static readonly UInt32 BCDE_OSLOADER_TYPE_KERNEL_DEBUGGER_ENABLED =
			MAKE_BCDE_DATA_TYPE(BCDE_CLASS.APPLICATION, BCDE_FORMAT.BOOLEAN, 0x0000A0);

		// new add - slaing - using hex values for speed but these could be created thru the MAKE TYPE macros also
		public static readonly UInt32 BCDE_VISTA_OS_ENTRY = 0x10200003;
		public static readonly UInt32 BCDE_LEGACY_OS_ENTRY = 0x10300006;
		
		// Enumerations
		public enum BCDE_CLASS
		{
			NONE			=	0x0,
			LIBRARY			=	0x1,
			APPLICATION		=	0x2,
			DEVICE			=	0x3,
			SETUP_TEMPLATE	=	0x4
		}

		public enum BCDE_FORMAT
		{
			NONE		=0x0,
			DEVICE      =0x1,
			STRING      =0x2,
			OBJECT      =0x3,
			OBJECT_LIST =0x4,
			INTEGER     =0x5,
			BOOLEAN     =0x6,
		}
			
		public enum BCDE_DEBUGGER_TYPE 
		{
			DebuggerSerial,
			Debugger1394,
			DebuggerUsb,
			DebuggerNone
		}

		public enum BCDE_OBJECT_TYPE
		{
			APPLICATION = 0x1,
			INHERITED = 0x2,
			DEVICE = 0x3
		}

		public enum BCDE_IMAGE_TYPE
		{
			FIRMWARE_APPLICATION            =    0x1,
			BOOT_APPLICATION                 =   0x2,
			LEGACY_LOADER                    =   0x3,
			REALMODE_CODE                   =    0x4
		}

		public enum BCDE_APPLICATION_TYPE
		{
			FIRMWARE_BOOT_MANAGER         = 0x00001,
			WINDOWS_BOOT_MANAGER          = 0x00002,
			WINDOWS_BOOT_LOADER           = 0x00003,
			WINDOWS_RESUME_APPLICATION    = 0x00004,
			MEMORY_TESTER                 = 0x00005,
			LEGACY_NTLDR                  = 0x00006,
			LEGACY_SETUPLDR               = 0x00007,
			BOOT_SECTOR                   = 0x00008,
			STARTUP_MODULE                = 0x00009,
			RESERVED                      = 0xFFFFF
		}


		// Conversion / utility routines
		// Composition of a type id
		public static UInt32 MAKE_BCDE_DATA_TYPE(BCDE_CLASS _Class, BCDE_FORMAT _Format, UInt32 _Subtype)
		{
			return
				(((((UInt32)_Class) & 0xF) << 28) |  
				((((UInt32)_Format) & 0xF) << 24) | 
				((_Subtype) & 0x00FFFFFF));
		}

		public static UInt32 MAKE_BCD_APPLICATION_OBJECT(BCDE_IMAGE_TYPE imageType, 
																										BCDE_APPLICATION_TYPE applicationType)
		{
			return
				(((((UInt32)BCDE_OBJECT_TYPE.APPLICATION) & 0xF) << 28) |
				((((UInt32)imageType) & 0xF) << 20) |
				(((UInt32)applicationType) & 0x000FFFFF));
		}

	//    #define MAKE_BCD_APPLICATION_OBJECT(_ImageType, _ApplicationType) \
	//(((ULONG)BCD_OBJECT_TYPE_APPLICATION << 28) | \
	// (((ULONG)(_ImageType) & 0xF) << 20) | \
	// ((ULONG)(_ApplicationType) & 0xFFFFF))

		public static BCDE_CLASS GET_BCDE_DATA_CLASS(UInt32 _DataType) 
		{
			return (BCDE_CLASS)((((_DataType)) >> 28) & 0xF);
		}
		public static BCDE_FORMAT GET_BCDE_DATA_FORMAT(UInt32 _DataType) 
		{
			return (BCDE_FORMAT)((((_DataType)) >> 24) & 0xF);
		}
		public static UInt32 GET_BCDE_DATA_SUBTYPE(UInt32 _DataType) 
		{
			return (((_DataType)) & 0x00FFFFFF);
		}
	}

    public class ElementConstants
    {

        public static readonly UInt32 BcdOSLoaderBoolean_SafeBootAlternateShell = 0x26000081;
        public static readonly UInt32 BcdOSLoaderInteger_SafeBoot = 0x25000080;

    }

}
