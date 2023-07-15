#include <windows.h>
#include <stdio.h>
#include "SendRecv.h"

// Function: ReplaceParameters
// Replaces parameters in a message and generates a Post request data.
// Parameters:
// - message: Pointer to the message string.
// - id: Pointer to the identifier string.
// Returns:
// - LPSTR: Pointer to the dynamically allocated Post request data string.
//          The caller is responsible for freeing the memory.
LPSTR ReplaceParameters(const char* message, const char* id) {
	CHAR* requestData = (CHAR*)malloc(8000);            // Array to store final Post request requestData.
	memset(requestData, 0, sizeof(requestData));        // Clears the requestData array by setting all its elements to 0.
	LPSTR firstHalf = (LPSTR)"{\"message\":\"";         // First Half of the POST request requestData
	LPSTR secondHalf = (LPSTR)"\", \"to_channel\":\"";  // Second half of the POST request requestData
	LPSTR lastPart = (LPSTR)"\"}";                      // Ending of the POST request requestData

	lstrcatA(requestData, firstHalf);       // Concatenate the firstHalf to the requestData string
	lstrcatA(requestData, message);         // Concatenate the message to the requestData string
	lstrcatA(requestData, secondHalf);      // Concatenate the secondHalf to the requestData string
	lstrcatA(requestData, id);              // Concatenate the id to the requestData string
	lstrcatA(requestData, lastPart);        // Concatenate the lastPart to the requestData string

	return requestData;
}

// Function: ReplaceChildParameters
// Replaces parameters in a message and generates a Post request data for a child message.
// Parameters:
// - message: Pointer to the message string.
// - id: Pointer to the identifier string.
// - ParentMsgID: Pointer to the parent message ID string.
// Returns:
// - LPSTR: Pointer to the dynamically allocated Post request data string.
//          The caller is responsible for freeing the memory.
LPSTR ReplaceChildParameters(const char* message, const char* id, const char* ParentMsgID) {
	CHAR* requestData = (CHAR*)malloc(8000);                        // Array to store final Post request requestData.
	memset(requestData, 0, sizeof(requestData));                    // Clears the requestData array by setting all its elements to 0.
	LPSTR firstHalf = (LPSTR)"{\"message\":\"";                     // First Half of the POST request requestData
	LPSTR secondHalf = (LPSTR)"\", \"to_channel\":\"";              // Second half of the POST request requestData
	LPSTR ThirdHalf = (LPSTR)"\", \"reply_main_message_id\":\"";    // Third Half of the POST request requestData
	LPSTR lastPart = (LPSTR)"\"}";                                  // Ending of the POST request requestData

	lstrcatA(requestData, firstHalf);       // Concatenate the firstHalf to the requestData string
	lstrcatA(requestData, message);         // Concatenate the message to the requestData string
	lstrcatA(requestData, secondHalf);      // Concatenate the secondHalf to the requestData string
	lstrcatA(requestData, id);              // Concatenate the id to the requestData string
	lstrcatA(requestData, ThirdHalf);       // Concatenate the Third Half to the requestData string
	lstrcatA(requestData, ParentMsgID);     // Concatenate the Parent Message ID to the requestData string
	lstrcatA(requestData, lastPart);        // Concatenate the lastPart to the requestData string

	return requestData;
}