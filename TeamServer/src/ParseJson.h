#pragma once
#include <string>
#include <stdio.h>

// Function: replaceBackslashes
// Replaces backslashes in a JSON string with double backslashes.
// Parameters:
// - jsonString: Pointer to the JSON string to be processed.
// Returns:
// - char*: Pointer to the newly allocated string with replaced backslashes.
//          The caller is responsible for freeing the memory.
//          Returns NULL if memory allocation fails.
char* replaceBackslashes(const char* jsonString) {
    size_t length = strlen(jsonString);
    char* newString = (char*)malloc(2 * length + 1); // Allocate memory for the new string
    if (newString == NULL) {
        return NULL; // Memory allocation failed
    }

    size_t j = 0;
    for (size_t i = 0; i < length; i++) {
        if (jsonString[i] == '\\') {
            newString[j++] = '\\';
            newString[j++] = '\\';
        }
        else {
            newString[j++] = jsonString[i];
        }
    }
    newString[j] = '\0'; // Null-terminate the new string

    return newString;
}
