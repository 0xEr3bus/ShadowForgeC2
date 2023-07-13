#include <windows.h>
#include "commands.h"
#include <stdio.h>

BOOL cd(const char* directory) {
    return SetCurrentDirectoryA(directory) != 0;
}

LPSTR pwd() {
    LPSTR workingDir = NULL;
    DWORD bufferSize = GetCurrentDirectoryA(0, NULL);
    if (bufferSize > 0) {
        workingDir = (LPSTR)malloc(bufferSize * sizeof(CHAR));
        GetCurrentDirectoryA(bufferSize, workingDir);
        return workingDir;
    }
    else {
        return (LPSTR)"Cannot get Working Directory!\n";
    }
}


LPSTR ls(const char* directory) {
    char tempDirectory[MAX_PATH];
    LPSTR baseDirectory = NULL;
    DWORD result = 0;

    _snprintf_s(tempDirectory, sizeof(tempDirectory), "%s", directory);

    _snprintf_s(tempDirectory, sizeof(tempDirectory), "%s\\*",
        directory);

    directory = tempDirectory;

    baseDirectory = _strdup(directory);
    if (baseDirectory == NULL) {
        result = ERROR_NOT_ENOUGH_MEMORY;
        return (LPSTR)"ERROR_NOT_ENOUGH_MEMORY";
    }

    //printf("%s", directory);
    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA(directory, &data);      // DIRECTORY

    LPSTR directoryContent = (LPSTR)malloc(MAX_PATH * 10000);

    LPSTR currentPosition = directoryContent;

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // printf("%s", data.cFileName);
            int len = sprintf_s(currentPosition, MAX_PATH * 100, "%s|", data.cFileName);
            currentPosition += len;
        } while (FindNextFileA(hFind, &data));
        FindClose(hFind);
    }
    return directoryContent;
}