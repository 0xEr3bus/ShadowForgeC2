#include <windows.h>
#include "commands.h"
#include <stdio.h>

// Function: cp
// Copies a file from the source path to the destination path.
// Parameters:
// - sourceFile: Pointer to the null-terminated string specifying the source file path.
// - destinationFile: Pointer to the null-terminated string specifying the destination file path.
// Returns:
// - LPSTR: Pointer to the dynamically allocated string containing the status message.
//          The caller is responsible for freeing the memory.
//          Returns NULL if the copy operation fails or memory allocation fails.
LPSTR cp(const char* sourceFile, const char* destinationFile) {
    char* buffer = (char*)malloc(75);
    if (CopyFileA(sourceFile, destinationFile, FALSE)) {
        return (LPSTR)"File copied successfully.\n";
    }
    else {
        sprintf_s(buffer, sizeof(buffer), "Failed to copy file. Error code: %lu", GetLastError());
        return buffer;
    }
}

// Function: mkdir
// Creates a directory at the specified path.
// Parameters:
// - directoryPath: Pointer to the null-terminated string specifying the directory path.
// Returns:
// - LPSTR: Pointer to the dynamically allocated string containing the status message.
//          The caller is responsible for freeing the memory.
//          Returns NULL if the directory creation fails or memory allocation fails.
LPSTR mkdir(const char* directoryPath) {
    char* buffer = (char*)malloc(100);
    if (CreateDirectoryA(directoryPath, NULL)) {
        return (LPSTR)"Directory created successfully.\n";
    }
    else {
        sprintf_s(buffer, sizeof(buffer), "Failed to create directory. Error code: %lu", GetLastError());
        return buffer;
    }
}

// Function: RecursiveDeleteDirectory
// Recursively deletes a directory and its contents.
// Parameters:
// - path: Pointer to the null-terminated string specifying the directory path.
// Returns:
// - const char*: Pointer to the dynamically allocated string containing the status message.
//                The caller is responsible for freeing the memory.
//                Returns NULL if the directory deletion fails or memory allocation fails.
const char* RecursiveDeleteDirectory(const char* path) {
    WIN32_FIND_DATAA findData;
    HANDLE findHandle;
    char filePath[MAX_PATH];
    char* buffer = (char*)malloc(500);

    snprintf(filePath, MAX_PATH, "%s\\*", path);

    findHandle = FindFirstFileA(filePath, &findData);
    if (findHandle == INVALID_HANDLE_VALUE) {
        sprintf_s(buffer, sizeof(buffer), "Failed to find the first file/directory. Error code: %lu\n", GetLastError());
        return buffer;
    }

    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
            continue;
        }

        snprintf(filePath, MAX_PATH, "%s\\%s", path, findData.cFileName);

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            RecursiveDeleteDirectory(filePath);
        }
        else {
            if (!DeleteFileA(filePath)) {
                sprintf_s(buffer, sizeof(buffer), "Failed to delete file: %s. Error code: %lu\n", path, GetLastError());
                return buffer;
            }
        }
    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);

    if (!RemoveDirectoryA(path)) {
        sprintf_s(buffer, sizeof(buffer), "Failed to remove directory: %s. Error code: %lu\n", path, GetLastError());
        return buffer;
    }
    return (LPSTR)"Directory removed successfully.\n";
}

// Function: rm
// Removes a file or directory at the specified path.
// Parameters:
// - path: Pointer to the null-terminated string specifying the file or directory path.
// Returns:
// - LPSTR: Pointer to the dynamically allocated string containing the status message.
//          The caller is responsible for freeing the memory.
//          Returns NULL if the removal fails or memory allocation fails.
LPSTR rm(const char* path) {
    DWORD attributes = GetFileAttributesA(path);
    char buffer[500];

    if (attributes == INVALID_FILE_ATTRIBUTES) {
        sprintf_s(buffer, sizeof(buffer), "Failed to get file/directory attributes. Error code: %lu\n", GetLastError());
        return buffer;
    }

    if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
        RecursiveDeleteDirectory(path);
        return (LPSTR)"Directory removed successfully.\n";
    }
    else {
        if (DeleteFileA(path)) {
            return (LPSTR)"File removed successfully.\n";
        }
        else {
            sprintf_s(buffer, sizeof(buffer), "Failed to remove file. Error code: %lu\n", GetLastError());
            return buffer;
        }
    }
}