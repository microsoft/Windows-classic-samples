/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples.
*       Copyright 1993 - 2000 Microsoft Corp.
*       All rights not expressly granted in the SDK license are reserved.
*       
*       THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
*       ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
*       THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*       PARTICULAR PURPOSE.
\******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RETURN_SUCCESS  0
#define RETURN_FAILURE  1
#define MAX_STRLEN		128

#define SUCCESS_PRINT(X,Y,Z)	{ \
								printf("%s: %s SUCCEEDED: Return %lu: line %d\n",X,Y,Z, __LINE__); \
								fflush(stdout); \
								}

#define FAIL_PRINT(X,Y,Z)	{ \
							printf("%s: %s FAILED: Error %lu: line %d\n",X,Y,Z, __LINE__); \
							fflush(stdout); \
							}

HINSTANCE Hinstance;
DWORD Error, Count, dwThreadId;
HANDLE Handle, hThread;
OVERLAPPED Overlapped, OverlappedUnenableRouter;
INT i;
DWORD (WINAPI* EnableRouter)(PHANDLE, LPOVERLAPPED);
DWORD (WINAPI* UnenableRouter)(LPOVERLAPPED, LPDWORD);

// prototypes
void CommandLineUsage(void);
DWORD WINAPI NullHandleCase(LPVOID);
DWORD WINAPI NullOverlappedCase(LPVOID);
DWORD WINAPI AllNullParamCase(LPVOID);
int UnreadableRand(size_t length, char szTestNumber[MAX_STRLEN]);
int UnwriteableRand(size_t length, char szTestNumber[MAX_STRLEN]);

int __cdecl
main(
    int argc,
    char* argv[]
    )
{
	Hinstance = LoadLibrary(TEXT("IPHLPAPI.DLL"));
    if (!Hinstance)
	{
        printf("LoadLibrary: %d\n", GetLastError());
		return RETURN_FAILURE;
    }
	else
	{
		EnableRouter = 
			(DWORD (WINAPI*)(PHANDLE, LPOVERLAPPED))
				GetProcAddress(Hinstance, "EnableRouter");
		UnenableRouter =
			(DWORD (WINAPI*)(LPOVERLAPPED, LPDWORD))
				GetProcAddress(Hinstance, "UnenableRouter");
		if (!EnableRouter || !UnenableRouter)
		{
			printf("GetProcAddress: %d\n", GetLastError());
			return RETURN_FAILURE;
		}
	}

// Step through one iteration if there are no command line switches

	if (argc < 2)
	{
		ZeroMemory(&Overlapped, sizeof(Overlapped));
		Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!Overlapped.hEvent)
		{
			printf("CreateEvent: %d\n", GetLastError());
			return RETURN_FAILURE;
		}
		else
		{
			Error = EnableRouter(&Handle, &Overlapped);
			if (Error != ERROR_IO_PENDING)
			{
				printf("EnableRouter: %d\n", Error);
				return RETURN_FAILURE;
			}
			else
			{
				printf("Press <Enter> to disable routing");
				getchar();
				Error = UnenableRouter(&Overlapped, &Count);
				if (Error)
				{
					printf("UnenableRouter: %d\n", Error);
					return RETURN_FAILURE;
				}
				else
				{
					printf("UnenableRouter: %d references left\n", Count);
				}
			}
			CloseHandle(Overlapped.hEvent);
		}
		FreeLibrary(Hinstance);
		return RETURN_SUCCESS;
	}

// Loop continuously for the 'stress' command line switch

	if ((argv[1][0] == 's') || (argv[1][0] == 'S'))
	{
		i = 1;
		while (TRUE)
		{
			ZeroMemory(&Overlapped, sizeof(Overlapped));
			Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			if (!Overlapped.hEvent)
			{
				printf("CreateEvent: %d\n", GetLastError());
				return RETURN_FAILURE;
			}
			else
			{
				Error = EnableRouter(&Handle, &Overlapped);
				if (Error != ERROR_IO_PENDING)
				{
					printf("EnableRouter: %d\n", Error);
				}
				else
				{
					Error = UnenableRouter(&Overlapped, &Count);
					if (Error)
					{
						printf("UnenableRouter: %d\n", Error);
					}
					else
					{
						if ((i % 100) == 0)
						{
							printf("Enable/UnenableRouter Stress: %d iterations\n", i);
						}
						i++;
					}
				}
				CloseHandle(Overlapped.hEvent);
			}
		}
		FreeLibrary(Hinstance);
		return RETURN_SUCCESS;
	}

// Run the API regressions for the 'regress' command line switch

	if ((argv[1][0] == 'r') || (argv[1][0] == 'R'))
	{
		srand( (unsigned)time( NULL ) );
		rand();        // throw out the first number because of its predictability

// Regression test EnableRouter()

		printf("\n\n\tRegression tests for EnableRouter()\n\n");

// Test 1: Do not zero memory for the Overlapped structure

		printf("\nRegression Test 1: Do not zero memory for the Overlapped structure\n");

		printf("\nRegression Test 1: ZeroMemory(&Overlapped, sizeof(Overlapped)) NOT called\n");

		Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!Overlapped.hEvent)
		{
			FAIL_PRINT("Regression Test 1", "CreateEvent", GetLastError());
		}
		else
		{
			SUCCESS_PRINT("Regression Test 1", "CreateEvent", GetLastError());
			Error = EnableRouter(&Handle, &Overlapped);
			if (Error != ERROR_IO_PENDING)
			{
				FAIL_PRINT("Regression Test 1", "EnableRouter", Error);
			}
			else
			{
				SUCCESS_PRINT("Regression Test 1", "EnableRouter", Error);
				Error = UnenableRouter(&Overlapped, &Count);
				if (Error)
				{
					FAIL_PRINT("Regression Test 1", "UnenableRouter", Error);
				}
				else
				{
					SUCCESS_PRINT("Regression Test 1", "UnenableRouter", Error);
					printf("Regression Test 1: UnenableRouter: %d references left\n", Count);
				}
			}
			CloseHandle(Overlapped.hEvent);
		}

// Test 2: Overlapped.hEvent is zero

		printf("\nRegression Test 2: Set Overlapped.hEvent equal to zero\n");

		printf("\nRegression Test 2: Overlapped.hEvent = 0\n");

		ZeroMemory(&Overlapped, sizeof(Overlapped));
		Overlapped.hEvent = 0;
		Error = EnableRouter(&Handle, &Overlapped);
		if (Error != ERROR_IO_PENDING)
		{
			FAIL_PRINT("Regression Test 2", "EnableRouter", Error);
		}
		else
		{
			SUCCESS_PRINT("Regression Test 2", "EnableRouter", Error);
			Error = UnenableRouter(&Overlapped, &Count);
			if (Error)
			{
				FAIL_PRINT("Regression Test 2", "UnenableRouter", Error);
			}
			else
			{
				SUCCESS_PRINT("Regression Test 2", "UnenableRouter", Error);
				printf("Regression Test 2: UnenableRouter: %d references left\n", Count);
			}
		}

// Test 3: Pass NULL parameters to EnableRouter()

		printf("\nRegression Test 3: Pass NULL parameters to EnableRouter()\n");

		printf("\nRegression Test 3.1: EnableRouter(NULL, &Overlapped)\n");

		hThread = NULL;
		dwThreadId = 0;
		hThread = CreateThread(NULL, 0, NullHandleCase, NULL, CREATE_SUSPENDED, &dwThreadId);
		if (hThread == NULL)
		{
			FAIL_PRINT("Regression Test 3.1", "CreateThread", GetLastError());
		}
		else
		{
			Error = ResumeThread(hThread);
			if (Error == 0xFFFFFFFF)
			{
				FAIL_PRINT("Regression Test 3.1", "ResumeThread", GetLastError());
			}
			else
			{
				Error = WaitForSingleObject(hThread, 10000);  // wait 10 seconds for thread to complete
				if ((Error == WAIT_FAILED) || (Error == WAIT_ABANDONED))
				{
					FAIL_PRINT("Regression Test 3.1", "WaitForSingleObject", GetLastError());
				}
				if (Error == WAIT_OBJECT_0)
				{
					FAIL_PRINT("Regression Test 3.1", "WaitForSingleObject", GetLastError());
					printf("Regression Test 3.1: WAIT_OBJECT_0 was unexpected\n");
				}
				if (Error == WAIT_TIMEOUT)
				{
					SUCCESS_PRINT("Regression Test 3.1", "WaitForSingleObject", Error);
					printf("Regression Test 3.1: WAIT_TIMEOUT was expected\n");
				}
			}
			CloseHandle(hThread);
		}

		printf("\nRegression Test 3.2: EnableRouter(&Handle, NULL)\n");

		hThread = NULL;
		dwThreadId = 0;
		hThread = CreateThread(NULL, 0, NullOverlappedCase, NULL, CREATE_SUSPENDED, &dwThreadId);
		if (hThread == NULL)
		{
			FAIL_PRINT("Regression Test 3.2", "CreateThread", GetLastError());
		}
		else
		{
			Error = ResumeThread(hThread);
			if (Error == 0xFFFFFFFF)
			{
				FAIL_PRINT("Regression Test 3.2", "ResumeThread", GetLastError());
			}
			else
			{
				Error = WaitForSingleObject(hThread, 10000);  // wait 10 seconds for thread to complete
				if ((Error == WAIT_FAILED) || (Error == WAIT_ABANDONED))
				{
					FAIL_PRINT("Regression Test 3.2", "WaitForSingleObject", GetLastError());
				}
				if (Error == WAIT_OBJECT_0)
				{
					FAIL_PRINT("Regression Test 3.2", "WaitForSingleObject", GetLastError());
					printf("Regression Test 3.2: WAIT_OBJECT_0 was unexpected\n");
				}
				if (Error == WAIT_TIMEOUT)
				{
					SUCCESS_PRINT("Regression Test 3.2", "WaitForSingleObject", Error);
					printf("Regression Test 3.2: WAIT_TIMEOUT was expected\n");
				}
			}
			CloseHandle(hThread);
		}

		printf("\nRegression Test 3.3: EnableRouter(NULL, NULL)\n");

		hThread = NULL;
		dwThreadId = 0;
		hThread = CreateThread(NULL, 0, AllNullParamCase, NULL, CREATE_SUSPENDED, &dwThreadId);
		if (hThread == NULL)
		{
			FAIL_PRINT("Regression Test 3.3", "CreateThread", GetLastError());
		}
		else
		{
			Error = ResumeThread(hThread);
			if (Error == 0xFFFFFFFF)
			{
				FAIL_PRINT("Regression Test 3.3", "ResumeThread", GetLastError());
			}
			else
			{
				Error = WaitForSingleObject(hThread, 10000);  // wait 10 seconds for thread to complete
				if ((Error == WAIT_FAILED) || (Error == WAIT_ABANDONED))
				{
					FAIL_PRINT("Regression Test 3.3", "WaitForSingleObject", GetLastError());
				}
				if (Error == WAIT_OBJECT_0)
				{
					FAIL_PRINT("Regression Test 3.3", "WaitForSingleObject", GetLastError());
					printf("Regression Test 3.3: WAIT_OBJECT_0 was unexpected\n");
				}
				if (Error == WAIT_TIMEOUT)
				{
					SUCCESS_PRINT("Regression Test 3.3", "WaitForSingleObject", Error);
					printf("Regression Test 3.3: WAIT_TIMEOUT was expected\n");
				}
			}
			CloseHandle(hThread);
		}

// Regression test UnenableRouter()

		printf("\n\n\tRegression tests for UnenableRouter()\n\n");

// Test 4: Use an Overlapped that is different from the one used for EnableRouter

		printf("\nRegression Test 4: Use a different Overlapped than the one used for EnableRouter\n");

		printf("\nRegression Test 4: Declare OVERLAPPED OverlappedUnenableRouter\n");

		ZeroMemory(&Overlapped, sizeof(Overlapped));
		Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!Overlapped.hEvent)
		{
			FAIL_PRINT("Regression Test 4", "CreateEvent", GetLastError());
		}
		else
		{
			SUCCESS_PRINT("Regression Test 4", "CreateEvent", GetLastError());
			Error = EnableRouter(&Handle, &Overlapped);
			if (Error != ERROR_IO_PENDING)
			{
				FAIL_PRINT("Regression Test 4", "EnableRouter", Error);
			}
			else
			{
				SUCCESS_PRINT("Regression Test 4", "EnableRouter", Error);
				ZeroMemory(&OverlappedUnenableRouter, sizeof(OverlappedUnenableRouter));
				OverlappedUnenableRouter.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
				if (!OverlappedUnenableRouter.hEvent)
				{
					FAIL_PRINT("Regression Test 4", "CreateEvent", GetLastError());
				}
				else
				{
					SUCCESS_PRINT("Regression Test 4", "CreateEvent", GetLastError());
					Error = UnenableRouter(&OverlappedUnenableRouter, &Count);
					if (Error)
					{
						FAIL_PRINT("Regression Test 4", "UnenableRouter", Error);
					}
					else
					{
						SUCCESS_PRINT("Regression Test 4", "UnenableRouter", Error);
						printf("Regression Test 4: UnenableRouter: %d references left\n", Count);
					}
					CloseHandle(OverlappedUnenableRouter.hEvent);
				}
			}
			CloseHandle(Overlapped.hEvent);
		}


		FreeLibrary(Hinstance);
		return RETURN_SUCCESS;
	}
	CommandLineUsage();
	FreeLibrary(Hinstance);
	return RETURN_FAILURE;
}

void
CommandLineUsage(
                 void
                 )
{
   fprintf( stderr, "\nEnableRouter [stress, regress]\n\n" );
   fprintf( stderr, "  'enablerouter' with no parameters will step through one iteration of\n" );
   fprintf( stderr, "                 enabling and disabling the router\n\n" );
   fprintf( stderr, "  'enablerouter stress' will enable and disable the router in an infinite loop\n\n" );
   fprintf( stderr, "  'enablerouter regress' will perform the EnableRouter() and UnenableRouter()\n" );
   fprintf( stderr, "                         regression tests\n\n" );
   fprintf( stderr, "  Any other parameter will display this help message\n\n" );
}

DWORD WINAPI
NullHandleCase(
			   LPVOID lpNotUsed
			   )
{
	ZeroMemory(&Overlapped, sizeof(Overlapped));
	Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!Overlapped.hEvent)
	{
		FAIL_PRINT("Regression Test 3.1", "CreateEvent", GetLastError());
	}
	else
	{
		SUCCESS_PRINT("Regression Test 3.1", "CreateEvent", GetLastError());
		printf("Regression Test 3.1: Call to EnableRouter(NULL, &Overlapped) is expected to block\n");
		Error = EnableRouter(NULL, &Overlapped);
		if (Error != ERROR_IO_PENDING)
		{
			FAIL_PRINT("Regression Test 3.1", "EnableRouter", Error);
		}
		else
		{
			SUCCESS_PRINT("Regression Test 3.1", "EnableRouter", Error);
			Error = UnenableRouter(&Overlapped, &Count);
			if (Error)
			{
				FAIL_PRINT("Regression Test 3.1", "UnenableRouter", Error);
			}
			else
			{
				SUCCESS_PRINT("Regression Test 3.1", "UnenableRouter", Error);
				printf("Regression Test 3.1: UnenableRouter: %d references left\n", Count);
			}
		}
		CloseHandle(Overlapped.hEvent);
	}
	return RETURN_FAILURE;
}

DWORD WINAPI
NullOverlappedCase(
				   LPVOID lpNotUsed
				   )
{
	ZeroMemory(&Overlapped, sizeof(Overlapped));
	Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!Overlapped.hEvent)
	{
		FAIL_PRINT("Regression Test 3.2", "CreateEvent", GetLastError());
	}
	else
	{
		SUCCESS_PRINT("Regression Test 3.2", "CreateEvent", GetLastError());
		printf("Regression Test 3.2: Call to EnableRouter(&Handle, NULL) is expected to block\n");
		Error = EnableRouter(&Handle, NULL);
		if (Error != ERROR_IO_PENDING)
		{
			FAIL_PRINT("Regression Test 3.2", "EnableRouter", Error);
		}
		else
		{
			SUCCESS_PRINT("Regression Test 3.2", "EnableRouter", Error);
			Error = UnenableRouter(&Overlapped, &Count);
			if (Error)
			{
				FAIL_PRINT("Regression Test 3.2", "UnenableRouter", Error);
			}
			else
			{
				SUCCESS_PRINT("Regression Test 3.2", "UnenableRouter", Error);
				printf("Regression Test 3.2: UnenableRouter: %d references left\n", Count);
			}
		}
		CloseHandle(Overlapped.hEvent);
	}
	return RETURN_FAILURE;
}

DWORD WINAPI
AllNullParamCase(
				 LPVOID lpNotUsed
				 )
{
	ZeroMemory(&Overlapped, sizeof(Overlapped));
	Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!Overlapped.hEvent)
	{
		FAIL_PRINT("Regression Test 3.3", "CreateEvent", GetLastError());
	}
	else
	{
		SUCCESS_PRINT("Regression Test 3.3", "CreateEvent", GetLastError());
		printf("Regression Test 3.3: Call to EnableRouter(NULL, NULL) is expected to block\n");
		Error = EnableRouter(NULL, NULL);
		if (Error != ERROR_IO_PENDING)
		{
			FAIL_PRINT("Regression Test 3.3", "EnableRouter", Error);
		}
		else
		{
			SUCCESS_PRINT("Regression Test 3.3", "EnableRouter", Error);
			Error = UnenableRouter(&Overlapped, &Count);
			if (Error)
			{
				FAIL_PRINT("Regression Test 3.3", "UnenableRouter", Error);
			}
			else
			{
				SUCCESS_PRINT("Regression Test 3.3", "UnenableRouter", Error);
				printf("Regression Test 3.3: UnenableRouter: %d references left\n", Count);
			}
		}
		CloseHandle(Overlapped.hEvent);
	}
	return RETURN_FAILURE;
}
