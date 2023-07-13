#define SECURITY_WIN32
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <lmcons.h>
#include <security.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <sddl.h>
#include "commands.h"

#pragma comment(lib, "Secur32.lib")
#pragma comment(lib, "psapi.lib")


LPSTR getuid() {
    DWORD usernameBufferSize = UNLEN + 1;
    LPSTR username = (char*)malloc(usernameBufferSize * sizeof(char));

    if (!GetUserNameExA(NameSamCompatible, username, &usernameBufferSize)) {
        free(username);
        return NULL;
    }

    return username;
}

LPSTR getprivs() {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        return (LPSTR)"Failed to open process token.\n";
    }

    DWORD bufferSize = 0;
    GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &bufferSize);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        CloseHandle(hToken);
        return (LPSTR)"Failed to get token information buffer size.\n";
    }

    PTOKEN_PRIVILEGES tokenPrivileges = (PTOKEN_PRIVILEGES)malloc(bufferSize);
    if (!GetTokenInformation(hToken, TokenPrivileges, tokenPrivileges, bufferSize, &bufferSize)) {
        CloseHandle(hToken);
        free(tokenPrivileges);
        return (LPSTR)"Failed to get token information.\n";
    }

    // Calculate the total length required for the formatted privileges string
    DWORD totalLength = 0;
    for (DWORD i = 0; i < tokenPrivileges->PrivilegeCount; i++) {
        LUID luid = tokenPrivileges->Privileges[i].Luid;
        DWORD nameLength = 0;
        LookupPrivilegeNameA(NULL, &luid, NULL, &nameLength);
        totalLength += nameLength + 3; // Length of privilege name + ": " + newline
    }

    LPSTR privilegesString = (LPSTR)malloc(totalLength + 1); // +1 for null terminator
    LPSTR currentPosition = privilegesString;

    for (DWORD i = 0; i < tokenPrivileges->PrivilegeCount; i++) {
        LUID luid = tokenPrivileges->Privileges[i].Luid;
        DWORD nameLength = 0;
        LookupPrivilegeNameA(NULL, &luid, NULL, &nameLength);
        LPSTR privilegeName = (LPSTR)malloc(nameLength);
        if (LookupPrivilegeNameA(NULL, &luid, privilegeName, &nameLength)) {
            sprintf_s(currentPosition, static_cast<long long>((totalLength)+1 - (currentPosition - privilegesString)), "%s|", privilegeName);
            currentPosition += nameLength + 1;
        }
        free(privilegeName);
    }

    CloseHandle(hToken);
    free(tokenPrivileges);

    return privilegesString;
}

BOOL GetProcessUserName(HANDLE hProcess, char* userName, DWORD* userNameSize) {
    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
        return FALSE;
    }

    DWORD dwSize = 0;
    if (!GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        CloseHandle(hToken);
        return FALSE;
    }

    PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(dwSize);
    if (pTokenUser == NULL) {
        CloseHandle(hToken);
        return FALSE;
    }

    if (!GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize)) {
        free(pTokenUser);
        CloseHandle(hToken);
        return FALSE;
    }

    SID_NAME_USE sidNameUse;
    DWORD cchName = MAX_PATH;
    DWORD cchDomainName = MAX_PATH;
    if (!LookupAccountSidA(NULL, pTokenUser->User.Sid, userName, &cchName, NULL, &cchDomainName, &sidNameUse)) {
        free(pTokenUser);
        CloseHandle(hToken);
        return FALSE;
    }

    free(pTokenUser);
    CloseHandle(hToken);
    return TRUE;
}

LPSTR ps(DWORD* processCount) {
    DWORD processIds[1024];
    DWORD cbNeeded;
    if (!EnumProcesses(processIds, sizeof(processIds), &cbNeeded)) {
        return (LPSTR)"Failed to enumerate processes.\n";
    }

    *processCount = cbNeeded / sizeof(DWORD);

    LPSTR processInfoString = (LPSTR)malloc(MAX_PATH * (*processCount) * 100);
    if (processInfoString == NULL) {
        return (LPSTR)"Failed to allocate memory.\n";
    }

    LPSTR currentPosition = processInfoString;

    for (DWORD i = 0; i < *processCount; i++) {
        DWORD pid = processIds[i];

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess == NULL) {
            continue;
        }

        char processName[MAX_PATH];
        if (GetModuleBaseNameA(hProcess, NULL, processName, MAX_PATH) == 0) {}

        int len = sprintf_s(currentPosition, MAX_PATH * 100, "%lu %s|",
            pid, processName);
        currentPosition += len;

        CloseHandle(hProcess);
    }

    return processInfoString;
}
