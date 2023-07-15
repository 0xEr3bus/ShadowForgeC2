#include <windows.h>

LPSTR ReplaceParameters(const char* message, const char* id);
LPSTR ReplaceChildParameters(const char* message, const char* id, const char* ParentMsgID);