#include <sddl.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#define PIPE_NAME L"\\\\.\\pipe\\IPCDemoPipe"
#define BUFFER_SIZE 512

// Function to get and display the current process integrity level and
// AppContainer status
void DisplayProcessSecurityInfo() {
  HANDLE hToken = nullptr;

  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
    printf("[Child] Failed to open process token. Error: %lu\n",
           GetLastError());
    return;
  }

  // Get integrity level
  DWORD dwLengthNeeded = 0;
  GetTokenInformation(hToken, TokenIntegrityLevel, nullptr, 0, &dwLengthNeeded);

  TOKEN_MANDATORY_LABEL *pTIL =
      (TOKEN_MANDATORY_LABEL *)LocalAlloc(0, dwLengthNeeded);
  if (pTIL) {
    if (GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwLengthNeeded,
                            &dwLengthNeeded)) {
      DWORD dwIntegrityLevel = *GetSidSubAuthority(
          pTIL->Label.Sid,
          (DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1));

      const char *integrityLevelStr = "Unknown";
      if (dwIntegrityLevel < SECURITY_MANDATORY_LOW_RID)
        integrityLevelStr = "Untrusted";
      else if (dwIntegrityLevel < SECURITY_MANDATORY_MEDIUM_RID)
        integrityLevelStr = "Low";
      else if (dwIntegrityLevel < SECURITY_MANDATORY_HIGH_RID)
        integrityLevelStr = "Medium";
      else if (dwIntegrityLevel >= SECURITY_MANDATORY_HIGH_RID)
        integrityLevelStr = "High/System";

      printf("[Child] *** Process Integrity Level: %s (0x%X) ***\n",
             integrityLevelStr, dwIntegrityLevel);

      // Convert SID to string for display
      LPSTR szIntegritySid = nullptr;
      if (ConvertSidToStringSidA(pTIL->Label.Sid, &szIntegritySid)) {
        printf("[Child] *** Integrity SID: %s ***\n", szIntegritySid);
        LocalFree(szIntegritySid);
      }
    }
    LocalFree(pTIL);
  }

  // Check if running in AppContainer
  DWORD dwIsAppContainer = 0;
  dwLengthNeeded = sizeof(DWORD);
  if (GetTokenInformation(hToken, TokenIsAppContainer, &dwIsAppContainer,
                          sizeof(DWORD), &dwLengthNeeded)) {
    printf("[Child] *** Running in AppContainer: %s ***\n",
           dwIsAppContainer ? "YES" : "NO");
  }

  // Get AppContainer SID if applicable
  if (dwIsAppContainer) {
    dwLengthNeeded = 0;
    GetTokenInformation(hToken, TokenAppContainerSid, nullptr, 0,
                        &dwLengthNeeded);

    TOKEN_APPCONTAINER_INFORMATION *pAppContainerInfo =
        (TOKEN_APPCONTAINER_INFORMATION *)LocalAlloc(0, dwLengthNeeded);
    if (pAppContainerInfo) {
      if (GetTokenInformation(hToken, TokenAppContainerSid, pAppContainerInfo,
                              dwLengthNeeded, &dwLengthNeeded)) {
        LPWSTR szAppContainerSid = nullptr;
        if (ConvertSidToStringSidW(pAppContainerInfo->TokenAppContainer,
                                   &szAppContainerSid)) {
          printf("[Child] *** AppContainer SID: %ls ***\n", szAppContainerSid);
          LocalFree(szAppContainerSid);
        }
      }
      LocalFree(pAppContainerInfo);
    }
  }

  CloseHandle(hToken);
}

bool SendMessage(HANDLE hPipe, const char *message) {
  DWORD bytesWritten;
  DWORD messageLen = (DWORD)strlen(message) + 1;

  printf("[Child] Sending: %s\n", message);

  if (!WriteFile(hPipe, message, messageLen, &bytesWritten, nullptr)) {
    printf("[Child] Failed to write to pipe. Error: %lu\n", GetLastError());
    return false;
  }

  return true;
}

bool ReceiveMessage(HANDLE hPipe, char *buffer, DWORD bufferSize) {
  DWORD bytesRead;

  if (!ReadFile(hPipe, buffer, bufferSize, &bytesRead, nullptr)) {
    printf("[Child] Failed to read from pipe. Error: %lu\n", GetLastError());
    return false;
  }

  printf("[Child] Received: %s\n", buffer);
  return true;
}

int main() {
  printf("=== IPC Demo - Child Process (Sandboxed) ===\n");
  printf("[Child] Starting...\n");

  // Display security information
  printf("\n[Child] === Security Context Information ===\n");
  DisplayProcessSecurityInfo();
  printf("[Child] ==========================================\n\n");

  // Wait for pipe to be available
  printf("[Child] Waiting for pipe to become available...\n");

  int retries = 0;
  while (!WaitNamedPipeW(PIPE_NAME, 5000)) {
    printf("[Child] Pipe not available yet, retrying... (attempt %d)\n",
           ++retries);
    if (retries > 10) {
      printf("[Child] Timeout waiting for pipe\n");
      return 1;
    }
  }

  printf("[Child] Pipe is available, connecting...\n");

  // Open the named pipe
  HANDLE hPipe = ::CreateFileW(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0,
                               nullptr, OPEN_EXISTING, 0, nullptr);

  if (hPipe == INVALID_HANDLE_VALUE) {
    printf("[Child] Failed to open pipe. Error: %lu\n", GetLastError());
    return 1;
  }

  printf("[Child] Connected to pipe\n");

  // Set pipe to message mode
  DWORD mode = PIPE_READMODE_MESSAGE;
  if (!SetNamedPipeHandleState(hPipe, &mode, nullptr, nullptr)) {
    printf("[Child] Failed to set pipe mode. Error: %lu\n", GetLastError());
    CloseHandle(hPipe);
    return 1;
  }

  char buffer[BUFFER_SIZE];

  // Message loop
  while (true) {
    // Receive message from parent
    if (!ReceiveMessage(hPipe, buffer, BUFFER_SIZE)) {
      break;
    }

    // Check for exit command
    if (strcmp(buffer, "EXIT") == 0) {
      printf("[Child] Received exit command\n");
      break;
    }

    // Echo the message back
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "Echo: %s", buffer);

    if (!SendMessage(hPipe, response)) {
      break;
    }
  }

  // Close pipe
  printf("[Child] Closing pipe\n");
  CloseHandle(hPipe);

  printf("[Child] Child process exiting\n");

  // Wait for user input before closing the console
  printf("\n[Child] *** Press Enter to close this window... ***\n");
  getchar();

  return 0;
}
