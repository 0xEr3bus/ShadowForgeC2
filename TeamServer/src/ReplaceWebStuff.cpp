#include <windows.h>
#include <stdio.h>

LPSTR ReplaceParameters(const char* message, const char* id) {
	CHAR* requestData = (CHAR*)malloc(8000);  // Array to store final Post request requestData.
	memset(requestData, 0, sizeof(requestData));  // Clears the requestData array by setting all its elements to 0.
	LPSTR firstHalf = (LPSTR)"{\"message\":\""; // First Half of the POST request requestData
	LPSTR secondHalf = (LPSTR)"\", \"to_channel\":\"";  // Second half of the POST request requestData
	LPSTR lastPart = (LPSTR)"\"}";  // Ending of the POST request requestData

	lstrcatA(requestData, firstHalf);  // Concatenate the firstHalf to the requestData string
	lstrcatA(requestData, message);  // Concatenate the message to the requestData string
	lstrcatA(requestData, secondHalf);  // Concatenate the secondHalf to the requestData string
	lstrcatA(requestData, id);  // Concatenate the id to the requestData string
	lstrcatA(requestData, lastPart);  // Concatenate the lastPart to the requestData string

	return requestData;
}

