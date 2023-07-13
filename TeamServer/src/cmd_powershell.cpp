#include <windows.h>

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

void ReadFromPipe(char* Buffer, HANDLE pipeOUT) {
    DWORD dwRead = 0;
    ReadFile(pipeOUT, Buffer, 4096, &dwRead, NULL);
    CloseHandle(pipeOUT);
}