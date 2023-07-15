#pragma once
#include <vector>
#include <string>
#include <windows.h>
#include <wininet.h>

int StartConnection(HINTERNET& hConnect);
int SendRequest(HINTERNET& hRequest, const char* method, const char* endpoint, const char* requestData);
int UpdateTheMessage(const char* message, LPSTR id);
int SendTheMessage(const char* message, LPSTR &id);
int RecvTheMessage(const char* id, LPSTR &output);
LPSTR AddPrefixToResult(LPSTR str, LPSTR prefix);
int SendChildMessage(const char* message, LPSTR& id, LPSTR ParentMsgID);