//
// Windows (Printing) SDK Samples
//
// Sample program demonstarting adding, setting, deleting, and enumerating 
// printer forms.
//
// NOTES: 
//  1. This sample must be run with admin privilege for AddForm() and SetForm()
//     to succeed.
//  2. To run this sample multiple times, you should either delete the forms
//     previously added by this sample.  You can delete the sample forms two
//     ways:
//       a. Manually from the "Printers and Faxes" folder using the server
//          properties option.
//       b. Run this applet with the /D command line switch.
//
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All Rights reserved.
//

#include <stdio.h>
#include <windows.h>
#include <winspool.h>

HANDLE  gHeap = NULL;

// Keywords are ANSI
CHAR szKeyword[] = "L2TForm1Keyword";

WCHAR wszLevel1FormName [] = L"Level 1 Test Form";
WCHAR wszLevel2FormName1[] = L"Level 2 Test Form #1";
WCHAR wszLevel2FormName2[] = L"Level 2 Test Form #2";
WCHAR wszDisplayNameEnUS[] = L"L2TestFormDisplayNameEnUS";
WCHAR wszDisplayNameSpPR[] = L"N2FormaParaPruebaNobreDeExhibirSpPR";

// Specify a localized resource file containing form display names
WCHAR wszSomeResourceDLL[] = L"ResourceFile.dll.mui";

// Specify a valid resource ID for the above resource file.
#define SOME_RESOURCE_ID 0x1001


void Usage(void)
{
    wprintf(L"Forms_Sample.exe\n\n");
    wprintf(L"  Usage: Forms_Sample [/d]\n\n");
    wprintf(L"    /d - Specifies the sample forms added previously by this");
    wprintf(L"         sample should be deleted.\n\n");
    wprintf(L"   Note: This sample must be run with administrator privilege."); 
    wprintf(L"\n\n"); 
}



// Display the data associated with a level 1 form.
void DisplayLevel1Form(__in PFORM_INFO_1W pfi1)
{
    wprintf(L"  Form Name:       %ws\n",           pfi1->pName);
    wprintf(L"  Flags:           0x%-d    : %ws\n",pfi1->Flags,
               (pfi1->Flags == FORM_BUILTIN) ? L"BUILTIN" :
               (pfi1->Flags == FORM_USER)    ? L"USER"    : L"PRINTER");
    wprintf(L"  Size:            cx     = %d\n",   pfi1->Size.cx);
    wprintf(L"                   cy     = %d\n",   pfi1->Size.cy);
    wprintf(L"  Imageable Area:  Left   = %d\n",   pfi1->ImageableArea.left);
    wprintf(L"                   Top    = %d\n",   pfi1->ImageableArea.top);
    wprintf(L"                   Right  = %d\n",   pfi1->ImageableArea.right);
    wprintf(L"                   Bottom = %d\n",   pfi1->ImageableArea.bottom);
}



// Display the data associated with a level 2 form.
void DisplayLevel2Form(__in PFORM_INFO_2W pfi2)
{
    wprintf(L"  Form Name:       %ws\n",           pfi2->pName);
    wprintf(L"  Flags:           0x%-d    : %ws\n",pfi2->Flags,
               (pfi2->Flags == FORM_BUILTIN) ? L"BUILTIN" :
               (pfi2->Flags == FORM_USER)    ? L"USER"    : L"PRINTER");
    wprintf(L"  Size:            cx     = %d\n",   pfi2->Size.cx);
    wprintf(L"                   cy     = %d\n",   pfi2->Size.cy);
    wprintf(L"  Imageable Area:  Left   = %d\n",   pfi2->ImageableArea.left);
    wprintf(L"                   Top    = %d\n",   pfi2->ImageableArea.top);
    wprintf(L"                   Right  = %d\n",   pfi2->ImageableArea.right);
    wprintf(L"                   Bottom = %d\n",   pfi2->ImageableArea.bottom);
    printf(  "  Keyword:         %s\n",            pfi2->pKeyword);
    wprintf(L"  StringType:      %ws\n", 
               ((pfi2->StringType == STRING_NONE)   ? L"STRING_NONE"     :
                (pfi2->StringType == STRING_MUIDLL) ? L"STRING_LANGPAIR" : 
                                                      L"STRING_MUIDLL"));
    wprintf(L"  MuiDll:          %ws\n", 
               pfi2->pMuiDll ? pfi2->pMuiDll : L"NULL");
    wprintf(L"  Resource ID:     %d\n",            pfi2->dwResourceId);
    wprintf(L"  Display Name:    %ws\n", 
               pfi2->pDisplayName ? pfi2->pDisplayName : L"NULL");
    wprintf(L"  Language ID:     %-#X\n\n",        pfi2->wLangId);
}




// Add a level 1 form to the form database
int __cdecl AddLevel1Form(__in HANDLE hPrinter)
{
    FORM_INFO_1W  fi1;

    // Build and add a level 1 form
    fi1.pName = wszLevel1FormName;

    fi1.Flags = FORM_USER;

    fi1.Size.cx = 215900;
    fi1.Size.cy = 279400;

    fi1.ImageableArea.left   = 12700;
    fi1.ImageableArea.top    = 12700;
    fi1.ImageableArea.right  = 203200;
    fi1.ImageableArea.bottom = 266700;
    
    if(!AddFormW(hPrinter, 1, (LPBYTE)&fi1))  {
        wprintf(L"Level 1 AddForm failed!\n");
        return (0);
    }

    return (1);
}



// Add some level 2 forms to the form database and modify one of the forms
// to contain display name info for an additional language.
int __cdecl AddLevel2Forms(__in HANDLE hPrinter)
{
    FORM_INFO_2W  fi2;


    // Build and add level 2 PRINTER form #1
    fi2.pName = wszLevel2FormName1;

    fi2.Size.cx = 215900;
    fi2.Size.cy = 279400;

    fi2.ImageableArea.left   = 12700;
    fi2.ImageableArea.top    = 12700;
    fi2.ImageableArea.right  = 203200;
    fi2.ImageableArea.bottom = 266700;
    
    // Only FORM_PRINTER forms can specify keywords
    fi2.Flags    = FORM_PRINTER;
    fi2.pKeyword = szKeyword;

    // Must be NULL and 0 for STRING_LANGPAIR
    fi2.pMuiDll      = NULL;
    fi2.dwResourceId = 0;

    fi2.StringType = STRING_LANGPAIR;

    // Set United States English as the LANGID
    fi2.wLangId = 0x409;
    fi2.pDisplayName = wszDisplayNameEnUS;

    if(!AddFormW(hPrinter, 2, (LPBYTE)&fi2))  {
        wprintf(L"Level 2 Test 1 AddForm failed!\n");
        return (0);
    }

    // Set an additional display name for Puerto Rico Spanish for this form.
    fi2.wLangId = 0x500a;
    fi2.pDisplayName = wszDisplayNameSpPR;

    if(!SetFormW(hPrinter, wszLevel2FormName1, 2, (LPBYTE)&fi2)) {
        wprintf(L"Level 2 Test 1 SetForm failed!\n");
        return (0);
    }


    // Build and add level 2 USER form #2
    fi2.Flags = FORM_USER;
    fi2.pName = wszLevel2FormName2;

    // No keyword allowed for non-FORM_PRINTER forms.
    fi2.pKeyword = NULL;

    // Must be NULL and 0 for STRING_MUIDLL
    fi2.pDisplayName = NULL;
    fi2.wLangId      = 0;

    fi2.StringType = STRING_MUIDLL;
    fi2.pMuiDll = wszSomeResourceDLL;
    fi2.dwResourceId = SOME_RESOURCE_ID;

    if(!AddFormW(hPrinter, 2, (LPBYTE)&fi2))  {
        wprintf(L"Level 2 Test 2 AddForm failed!\n");
        return (0);
    }

    return (1);
}



// Enumerate and display all the forms in the form database in 
// FORM_INFO_1 format.  Note that level 2 forms will also be enumerated 
// in level 1 format.
int EnumAndDisplayLevel1Forms(__in HANDLE hPrinter)
{
    PFORM_INFO_1W   pfi1       = NULL;
    DWORD           dwSizeBuf  = 0;
    DWORD           dwNumForms = 0;
    DWORD           i;

    // Query to get the buffer size required to contain the enumeration of
    // level 1 forms.
    if(!EnumFormsW(hPrinter, 1, NULL, 0, &dwSizeBuf, &dwNumForms)) {

        // Allocate the required buffer size.
        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)  {

            pfi1 = (PFORM_INFO_1W)HeapAlloc(gHeap, HEAP_ZERO_MEMORY, dwSizeBuf);

            if(!pfi1)      {
                wprintf(L"Level 1 HeapAlloc failed!\n");
                return (0);
            }
        }
        else  {
            wprintf(L"Level 1 EnumForms query call failed!\n");
            return (0);
        }
    }
     
    // Fill the buffer with the enumeration data
    if(!EnumFormsW(hPrinter, 
                  1, 
                  (LPBYTE)pfi1, 
                  dwSizeBuf, 
                  &dwSizeBuf, 
                  &dwNumForms))  {

        HeapFree(gHeap, HEAP_NO_SERIALIZE, pfi1);

        wprintf(L"Level 1 EnumForms enumeration call failed!\n");
        return (0);
    }
     
    // Display the info associated with each level 1 form.
    wprintf(L"\nRetrieved %d level 1 forms", dwNumForms);
    wprintf(L"\n-----------------------------\n");

    for(i = 0; i < dwNumForms; i++)  {

        wprintf(L"Form %d:\n",          i);

        DisplayLevel1Form(pfi1);

        pfi1 = pfi1 + 1;
    }

    HeapFree(gHeap, HEAP_NO_SERIALIZE, pfi1);

    return (1);
}




int EnumAndDisplayLevel2Forms(__in HANDLE hPrinter)
{
    PFORM_INFO_2W   pfi2       = NULL;
    DWORD           dwSizeBuf  = 0;
    DWORD           dwNumForms = 0;
    DWORD           i;

    // Query to get the buffer size required to contain the enumeration of
    // level 2 forms.
    if(!EnumFormsW(hPrinter, 2, NULL, 0, &dwSizeBuf, &dwNumForms)) {

        // Allocate the required buffer size.
        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)  {

            pfi2 = (PFORM_INFO_2W)HeapAlloc(gHeap, HEAP_ZERO_MEMORY, dwSizeBuf);

            if(!pfi2)      {
                wprintf(L"Level 2 HeapAlloc failed!\n");
                return (0);
            }
        }
        else  {
            wprintf(L"Level 2 EnumForms query call failed!\n");
            return (0);
        }
    }
     
    // Fill the buffer with the enumeration data
    if(!EnumFormsW(hPrinter, 
                   2, 
                   (LPBYTE)pfi2, 
                   dwSizeBuf, 
                   &dwSizeBuf, 
                   &dwNumForms))  {

        HeapFree(gHeap, HEAP_NO_SERIALIZE, pfi2);

        wprintf(L"Level 2 EnumForms enumeration call failed!\n");
        return (0);
    }
     
    // Display the info associated with each level 2 form.
    wprintf(L"\nRetrieved %d level 2 forms", dwNumForms);
    wprintf(L"\n-----------------------------\n");

    for(i = 0; i < dwNumForms; i++)  {

        wprintf(L"Form %d:\n",          i);

        DisplayLevel2Form(pfi2);

        pfi2 = pfi2 + 1;
    }

    HeapFree(gHeap, HEAP_NO_SERIALIZE, pfi2);

    return (1);
}



// Determines if the "delete sample forms" flag was specified on the command
// line.
BOOL ProcessCommandLine(__in              int    argc, 
                        __in_ecount(argc) WCHAR *argv[],
                        __out             BOOL  *pbDeleteSampleForms)
{
    BOOL bContinue = FALSE;

    *pbDeleteSampleForms = FALSE;

    // OK, just run the applet
    if(argc == 1) {
        bContinue = TRUE;
    }

    // look for /D switch
    else if(argc == 2) {

        if(argv[1][0] == L'/') {

            if(argv[1][1] == L'd' || argv[1][1] == L'D') {

                *pbDeleteSampleForms = TRUE;

                bContinue = TRUE;
            }
        }
    }

    if(!bContinue) {
        Usage();
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return (bContinue);
}



int DeleteSampleForms(HANDLE hPrinter)
{
    int dwRet = 0;

    if(DeleteFormW(hPrinter, wszLevel1FormName)) {

        if(DeleteFormW(hPrinter, wszLevel2FormName1)) {

            if(DeleteFormW(hPrinter, wszLevel2FormName2)) {
 
                dwRet = 1;
            }
        }
    }

    if(!dwRet) {
        wprintf(L"\nUnable to delete forms\n\n");
    }

    return (dwRet);
}



int __cdecl wmain (__in int argc, __in_ecount(argc) WCHAR *argv[])
{
    DWORD             dwRet              = 0;
    BOOL              bDeleteSampleForms = FALSE;
    BOOL              bContinue          = FALSE;
    HANDLE            hPrinter           = NULL;
    PRINTER_DEFAULTS  pd                 = {NULL, NULL, SERVER_ALL_ACCESS};
  

    bContinue = ProcessCommandLine(argc, argv, &bDeleteSampleForms);

    if(bContinue) {

        if(gHeap = GetProcessHeap()) {

            // Obtain a handle to the local print server
            if(OpenPrinter(NULL, &hPrinter, &pd))  {

                if(bDeleteSampleForms) {
                    dwRet = DeleteSampleForms(hPrinter);
                }
                
                else {

                    if(AddLevel1Form(hPrinter)) {

                        if(AddLevel2Forms(hPrinter)) {

                            if(EnumAndDisplayLevel1Forms(hPrinter)) {
    
                                if(EnumAndDisplayLevel2Forms(hPrinter)) {
                                    dwRet = 1;
                                }
                            }
                        }
                    }
                }
                ClosePrinter(hPrinter);
            }
            else {
                wprintf(L"OpenPrinter failed!\n");
            }
        }
        else {
            wprintf(L"GetProcessHeap failed!\n");
        }
    }

    if (!dwRet) {
        wprintf(L"Error code: %d\n\n", GetLastError());
    }

    return (dwRet);
}

