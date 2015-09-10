/*
**++
**
** Copyright (c) Microsoft Corporation
**
**
** Module Name:
**
**	swriter.cpp
**
**
** Abstract:
**
**	Demonstrate a writer
**
**--
*/


///////////////////////////////////////////////////////////////////////////////
// Includes

#include "stdafx.h"
#include "swriter.h"

///////////////////////////////////////////////////////////////////////////////

// Initialize the writer
HRESULT STDMETHODCALLTYPE DepWriter::Initialize()
{
	HRESULT hr = CVssWriter::Initialize(DepWriterId, 		// WriterID
				            DepWriterName, 	        // wszWriterName
					    VSS_UT_USERDATA,		// ut
					    VSS_ST_OTHER);              // st
	if (FAILED(hr))	{
	    wprintf(L"CVssWriter::Initialize failed!");	
	    return hr;
	}

    // subscribe for events
	hr = Subscribe();
	if (FAILED(hr))
	    wprintf(L"CVssWriter::Subscribe failed!");	

	return hr;
}

// OnIdentify is called as a result of the requestor calling GatherWriterMetadata
bool STDMETHODCALLTYPE DepWriter::OnIdentify(IN IVssCreateWriterMetadata *pMetadata)
{
    // Set the restore method to restore if can replace
	HRESULT hr = pMetadata->SetRestoreMethod(VSS_RME_RESTORE_IF_CAN_REPLACE, 
						 NULL,
						 NULL, 
						 VSS_WRE_ALWAYS,
						 false);
	if (FAILED(hr))	{
	    wprintf(L"SetRestoreMethod failed!");	
	    return false;
	}


	// add simple FileGroup component
	hr = pMetadata->AddComponent(VSS_CT_FILEGROUP,		
				     NULL,
				     L"FilesComponent",
				     L"My FileGroup",
				     NULL,
				     0,
				     false,
				     true,
				     true,
			             true);
	if (FAILED(hr))	{
	    wprintf(L"AddComponent failed!");	
	    return false;
	}

    // add some sample files to the group	
	hr = pMetadata->AddFilesToFileGroup(NULL,
					    L"FilesComponent",
					    L"C:\\MyFiles",
				            L"*.dat",
					    false,
					    NULL);
	if (FAILED(hr))	{
	    wprintf(L"AddFilesToFileGroup failed!");	
	    return false;
	}


	return true;
}

// This function is called as a result of the requestor calling PrepareForBackup
// this indicates to the writer that a backup sequence is being initiated
bool STDMETHODCALLTYPE DepWriter::OnPrepareBackup(_In_ IVssWriterComponents*)
{
    return true;
}

// This function is called after a requestor calls DoSnapshotSet
// time-consuming actions related to Freeze can be performed here
bool STDMETHODCALLTYPE DepWriter::OnPrepareSnapshot()
{
    return true;
}

// This function is called after a requestor calls DoSnapshotSet
// here the writer is expected to freeze its store
bool STDMETHODCALLTYPE DepWriter::OnFreeze()
{

	return true;	
}	

// This function is called after a requestor calls DoSnapshotSet
// here the writer is expected to thaw its store
bool STDMETHODCALLTYPE DepWriter::OnThaw()
{
	return true;	
}

// This function is called after a requestor calls DoSnapshotSet
bool STDMETHODCALLTYPE DepWriter::OnPostSnapshot(_In_ IVssWriterComponents*)
{
    return true;
}

// This function is called to abort the writer's backup sequence.
// This should only be called between OnPrepareBackup and OnPostSnapshot
bool STDMETHODCALLTYPE DepWriter::OnAbort()
{
    return true;
}

// This function is called as a result of the requestor calling BackupComplete
bool STDMETHODCALLTYPE DepWriter::OnBackupComplete(_In_ IVssWriterComponents*)
{
	return true;	
}	

// This function is called at the end of the backup process.  This may happen as a result
// of the requestor shutting down, or it may happen as a result of abnormal termination 
// of the requestor.
bool STDMETHODCALLTYPE DepWriter::OnBackupShutdown(_In_ VSS_ID)
{
	return true;
}

// This function is called as a result of the requestor calling PreRestore
// This will be called immediately before files are restored
bool STDMETHODCALLTYPE DepWriter::OnPreRestore(_In_ IVssWriterComponents *)
{
    return true;
}

// This function is called as a result of the requestor calling PreRestore
// This will be called immediately after files are restored
bool STDMETHODCALLTYPE DepWriter::OnPostRestore(_In_ IVssWriterComponents*)
{
    return true;
}

