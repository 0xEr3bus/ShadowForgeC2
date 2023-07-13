#pragma once
#include <windows.h>
#include <string>

typedef struct {
    DWORD pid;
    DWORD ppid;
    char processName[MAX_PATH];
    char userName[MAX_PATH];
} ProcessInfo;

LPSTR getuid();
LPSTR getprivs();
LPSTR ps(DWORD* processCount);
BOOL cd(const char* directory);
LPSTR pwd();
LPSTR ls(const char* currentDir);
LPSTR cp(const char* sourceFile, const char* destinationFile);
LPSTR mkdir(const char* directoryPath);
LPSTR rm(const char* path);
HANDLE execute_shell_commands(char* command);
void ReadFromPipe(char* Buffer, HANDLE pipeOUT);