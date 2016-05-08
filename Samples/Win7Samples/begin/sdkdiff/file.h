// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __FILE_H__
#define __FILE_H__
/*
 * FILEDATA - represents a file name and its contents.
 *
 * The file name is held in the form of a DIRITEM which is managed
 * by SCANDIR (see scandir.h).  A DIRITEM must be supplied to initialise
 * a FILEDATA.
 *
 * On demand, the FILEDATA will return a handle to a list of lines.
 * These are the lines in the file in the form of LINE handles (see line.h)
 * this list can be discarded by a call to file_discardlines, or file_delete.
 */

/* handle to filedata */
typedef struct filedata FAR * FILEDATA;

/*
 * make a new FILEDATA, based on a DIRITEM. the filedata will retain
 * the diritem handle for use in fetching filenames and handles.
 *
 * if the bRead is set, the file will be read into memory. If not, this
 * will be done during the first call to file_getlines
 */
FILEDATA file_new(DIRITEM fiName, BOOL bRead);

/*
 * return a handle to the DIRITEM used to create this FILEDATA
 */
DIRITEM file_getdiritem(FILEDATA fi);


/* delete a FILEDATA and its associated list of LINEs. note that the
 * DIRITEM is NOT deleted.
 */
void file_delete(FILEDATA fi);

/*
 * return a list of the lines in the file. This is a standard list that can
 * be traversed with the list functions. The list should only be deleted
 * by calls to file_delete or file_discardlines for the owning FILEDATA.
 * The items in the list are LINE handles.
 *
 * this call will cause the file to be read into memory if necessary (if
 * the lines had been discarded using file_discardlines, or if bRead were
 * false in the initial call to file_new
 */
LIST file_getlinelist(FILEDATA fi);

/*
 * free up the line list and any associated memory until it is needed again.
 */
void file_discardlines(FILEDATA fi);

/*
 * force all lines in the line list to reset their hashcodes and any line
 * links. Does not cause the file to be re-read.
 */
void file_reset(FILEDATA fi);

/*
 * give me a checksum for the file. whether or not actually calculated by
 * dir_getchecksum(), it will be the same checksum as if it were.
 */
DWORD file_checksum(FILEDATA fi);

/*
 * Retrieve the checksum that we have for this file, valid or not.
 * Indicate in bValid whether it is actually valid or not.
 * Do NOT recalculate it or make any new attempt to read the file!
 */
DWORD file_retrievechecksum(FILEDATA fi, BOOL * bValid);

/* returns TRUE if the file is unicode */
BOOL file_IsUnicode(FILEDATA fd);


/* retrieve the filetime for the file */
FILETIME file_GetTime(FILEDATA fd);


#endif
