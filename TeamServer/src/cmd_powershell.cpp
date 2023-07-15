#include <windows.h>

#define MAX_BUFFER_SIZE 4087
#define MAX_VARIABLE_SIZE 500

// Function: execute_shell_commands
// Executes a shell command and returns a handle to the output pipe.
// Parameters:
// - command: Pointer to the null-terminated string specifying the shell command to execute.
// Returns:
// - HANDLE: Handle to the output pipe for the executed command.
//           Returns INVALID_HANDLE_VALUE if the command execution fails.
HANDLE execute_shell_commands(char* command) {
    HANDLE pipeIN, pipeOUT;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES Attrs;

    memset(&Attrs, 0, sizeof(Attrs));

    Attrs.nLength = sizeof(SECURITY_ATTRIBUTES);
    Attrs.bInheritHandle = TRUE;
    Attrs.lpSecurityDescriptor = NULL;

    CreatePipe(&pipeOUT, &pipeIN, &Attrs, 0);
    SetHandleInformation(pipeOUT, HANDLE_FLAG_INHERIT, 0);

    memset(&si, 0, sizeof(si));

    si.cb = sizeof(si);
    si.hStdError = pipeIN;
    si.hStdOutput = pipeIN;
    si.dwFlags |= STARTF_USESTDHANDLES;

    memset(&pi, 0, sizeof(pi));

    CreateProcessA(NULL, (LPSTR)command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, 20000);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(pipeIN);

    return pipeOUT;
}

// Function: ReadFromPipe
// Reads data from a pipe and stores it in the provided buffer.
// Parameters:
// - Buffer: Pointer to the buffer where the read data will be stored.
// - pipeOUT: Handle to the output pipe to read from.
// Returns:
// - DWORD: Total number of bytes read from the pipe.
DWORD ReadFromPipe(char* Buffer, HANDLE pipeOUT) {
    DWORD totalBytesRead = 0;
    DWORD bytesRead = 0;

    do {
        if (!ReadFile(pipeOUT, Buffer + totalBytesRead, 4096, &bytesRead, NULL)) {
            break;
        }

        totalBytesRead += bytesRead;
    } while (bytesRead > 0);

    CloseHandle(pipeOUT);

    return totalBytesRead;
}

// Function: SplitDataFromPipe
// Splits the data read from a pipe into separate variables based on a maximum variable size.
// Parameters:
// - Buffer: Pointer to the buffer containing the data read from the pipe.
// - bytesRead: Number of bytes read from the pipe.
// - numVariables: Pointer to an integer to store the number of variables created.
// Returns:
// - char**: Pointer to an array of strings representing the split variables.
//           The caller is responsible for freeing the memory of each string and the array itself.
char** SplitDataFromPipe(char* Buffer, DWORD bytesRead, int* numVariables) {
    int variableCount = 0;
    char** variables = NULL;
    int startIndex = 0;

    while (startIndex < (int)bytesRead) {
        char* variable = (char*)malloc(MAX_VARIABLE_SIZE + 1);

        if (variable != NULL) {
            int endIndex = startIndex + MAX_VARIABLE_SIZE;
            if (endIndex > (int)bytesRead)
                endIndex = bytesRead;

            strncpy_s(variable, MAX_VARIABLE_SIZE + 1, Buffer + startIndex, endIndex - startIndex);
            variable[endIndex - startIndex] = '\0';

            variables = (char**)realloc(variables, (variableCount + 1) * sizeof(char*));
            variables[variableCount] = variable;
            variableCount++;

            startIndex = endIndex;
        }
        else {
            break;
        }
    }

    *numVariables = variableCount;
    return variables;
}