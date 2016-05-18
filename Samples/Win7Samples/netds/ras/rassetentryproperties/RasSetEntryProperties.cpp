// RasSetEntryProperties.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ras.h>
#include <raserror.h>

int main(int argc, CHAR* argv[])
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwSize = 0, dwEntries;
    LPRASENTRYNAMEA pRasEntryName = NULL;
    LPRASENTRYA pRasEntry = NULL;
    DWORD i;
    
    dwErr = RasEnumEntriesA(NULL, NULL, NULL, &dwSize, &dwEntries);

    if (dwErr == ERROR_BUFFER_TOO_SMALL)
    {
        pRasEntryName = (LPRASENTRYNAMEA) LocalAlloc(LPTR, dwSize);
        if (pRasEntryName == NULL)
        {
            printf("Alloc failed for EnumEntries %d", GetLastError());
            goto Done;
        }

        pRasEntryName->dwSize = sizeof (RASENTRYNAMEA);
        dwErr = RasEnumEntriesA(NULL, NULL, pRasEntryName, &dwSize, &dwEntries);
        if (dwErr != ERROR_SUCCESS)
        {
            printf("RasEnumEntries failed with error %d", dwErr);
            goto Done;
        }
        printf("Number of Entries %d\n", dwEntries);

        for (i = 0; i < dwEntries; i++)
        {
            dwSize = 0;
            dwErr = RasGetEntryPropertiesA(pRasEntryName[i].szPhonebookPath, 
                                          pRasEntryName[i].szEntryName,
                                          NULL,
                                          &dwSize,
                                          NULL, NULL);
            if (dwErr == ERROR_BUFFER_TOO_SMALL)
            {
                pRasEntry = (LPRASENTRYA ) LocalAlloc(LPTR, dwSize);
                if (pRasEntry == NULL)
                {
                    printf("Alloc failed for RasEntry\n");
                    goto Done;
                }

                pRasEntry->dwSize = sizeof (RASENTRYA);
                dwErr = RasGetEntryPropertiesA(pRasEntryName[i].szPhonebookPath, 
                                              pRasEntryName[i].szEntryName,
                                              pRasEntry,
                                              &dwSize,
                                              NULL, 0);
                if (dwErr != ERROR_SUCCESS)
                {
                    printf("RasGetEntryProperties failed with error %d for entry %s\n", dwErr, pRasEntryName[i].szEntryName);
                    goto Done;
                }
            }
            else
            {
                printf("RasGetEntryProperties failed with error %d for entry %s\n", dwErr, pRasEntryName[i].szEntryName);
                goto Done;
            }

            pRasEntry->dwfOptions2 |= RASEO2_DontUseRasCredentials;
            dwErr = RasSetEntryPropertiesA(pRasEntryName[i].szPhonebookPath, 
                                          pRasEntryName[i].szEntryName, 
                                          pRasEntry,
                                          dwSize,
                                          NULL, 0);
            if (dwErr != ERROR_SUCCESS)
            {
                printf("RasSetEntryProperties failed with error %d for entry %s\n", dwErr, pRasEntryName[i].szEntryName);
                goto Done;
            }
            else
            {
                printf("RasSetEntryProperties is succeeded for entry %s\n", pRasEntryName[i].szEntryName);
            }

            LocalFree(pRasEntry);
            pRasEntry = NULL;
        }
    }
    else
    {

        goto Done;
    }

Done:
    if (pRasEntryName)
    {
        LocalFree(pRasEntryName);
    }
    if (pRasEntry)
    {
        LocalFree(pRasEntry);
    }
    return dwErr;
}

