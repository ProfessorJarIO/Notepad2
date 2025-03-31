// FileOperations = Deals with opening and saving files
// FileFunctionality = Functions that work with open files to perform a certain action
#include "commheaders.h"
#include "fileFunctionality.h"


// This function reverses a WCHAR string and puts that reversed string into "outStr" buffer the user provides.
void ReverseString(const WCHAR* revTestData, WCHAR* outStr)
{
    DWORD numChar = wcslen(revTestData);

    INT i = 0;

    while (i < numChar)
    {
        outStr[i] = revTestData[numChar - 1 - i];
        //MessageBoxA(NULL, std::to_string(revData[i]).c_str(), "Info", 0);
        //MessageBox(NULL, (LPCWSTR)revData[i], L"Info", 0);
        i++;
    }
    outStr[i] = L'\0';
}

// This function looks for a match within a given string. Specifically, it looks for a match in our EDIT control. This function supports UNICODE characters.
int SearchFile(HWND hwnd, const WCHAR* find, BOOL down, BOOL matchCase, BOOL matchWholeWord)
{
    // Variable used for determining the index of where to start the search in the string. This is especially important if we want to look for other matches in our string.
    // We also set the variable to "static" so that it wouldn't be destroyed at the end of the function and retain it's value
    static DWORD findFrom = 0;

    // We will use this variable to see if the user has chosen "down" or "up" before. We do this in order to adjust "findFrom" when switching between "down" or "up".
    static BOOL wasDown = FALSE;

    //MessageBox(NULL, find, TEXT("Info"), MB_OK);

    // Chcek the length of the string we are looking for from the input box of the Find Dialog, NOT the string in the input box of our main program
    if (0 < wcslen(find))
    {
        //MessageBoxA(hwnd, std::to_string(findFrom).c_str(), "findFrom", 0);

        // Get the number of bytes in our EDIT control to know how much memory to allocate
        DWORD bytesToAlloc = GetWindowTextLength(GetDlgItem(hwnd, TXT_INPUT));

        // If we try to find text but our EDIT control is empty, we will get a HEAP error. So we need to exit if there's no text in our EDIT control.
        if (bytesToAlloc == 0)
        {
            MessageBox(hwnd, L"No text found in EDIT control", L"Info", 0);
            return -1;
        }

        if (matchCase)
        {
            MessageBox(hwnd, L"Sorry, the \"Match case\" feature isn't available yet", L"Info", 0);
        }

        if (matchWholeWord)
        {
            MessageBox(hwnd, L"Sorry, the \"Match Whole Word\" feature isn't available yet", L"Info", 0);
        }

        // Our Up button
        if (!down)
        {
            WCHAR* normalStrbuffer = new WCHAR[bytesToAlloc * sizeof(WCHAR)];

            GetDlgItemText(hwnd, TXT_INPUT, normalStrbuffer, bytesToAlloc * sizeof(WCHAR));

            // If the user clicked the down button before, but decided to click the "up" button...; down to up conversion for findFrom
            if (wasDown == TRUE && down == FALSE)
            {
                findFrom = wcslen(normalStrbuffer) - findFrom + wcslen(find);

                wasDown = FALSE;
            }


            // A buffer to hold our reversed string
            WCHAR* reverseStrbuffer = new WCHAR[bytesToAlloc * sizeof(WCHAR)];
            ReverseString(normalStrbuffer, reverseStrbuffer);

            //MessageBox(hwnd, normalStrbuffer, L"Info...", 0);
            //MessageBox(hwnd, reverseStrbuffer, L"Info...", 0);

            // The buffer will be exactly the same as what we gave the "Find Dialog" buffer input box at the beginning of the program. No need to dynamically allocate memory
            WCHAR revFindBuffer[sizeof(findWhatBuffer)];
            ReverseString(find, revFindBuffer);


            //MessageBox(hwnd, revFindBuffer, L"Info...", 0);

            const WCHAR* start = &reverseStrbuffer[findFrom];
            const WCHAR* pos = wcsstr(start, revFindBuffer);

            if (NULL != pos)
            {
                // The index where match was found in the reversed string
                DWORD foundAtRevStr = (pos - reverseStrbuffer); // It NEEDS to be pos - revTestdata, if we were to use pos - testdata, the value wouldn't be right.
                //MessageBoxA(NULL, std::to_string(foundAtRevStr).c_str(), "Info", 0);
                // Find the index where match was found in non-reversed string
                DWORD foundAtNormStr = wcslen(normalStrbuffer) - foundAtRevStr - wcslen(find);
                //MessageBoxA(NULL, std::to_string(foundAtNormStr).c_str(), "Info", 0);
                //MessageBoxA(NULL, std::to_string(foundAt).c_str(), "Info", 0);
                findFrom = foundAtRevStr + wcslen(find);
                //MessageBoxA(NULL, std::to_string(findFrom).c_str(), "Info", 0);

                // Free up memory
                delete[] normalStrbuffer;
                delete[] reverseStrbuffer;

                return foundAtNormStr;
            }

            if (findFrom != 0 && pos == NULL)
            {
                start = &reverseStrbuffer[0];
                pos = wcsstr(start, revFindBuffer);

                // If a match was found <- grabbed from above
                if (NULL != pos)
                {
                    // The index where match was found in the reversed string
                    DWORD foundAtRevStr = (pos - reverseStrbuffer); // It NEEDS to be pos - revTestdata, if we were to use pos - testdata, the value wouldn't be right.
                    //MessageBoxA(NULL, std::to_string(foundAtRevStr).c_str(), "Info", 0);
                    // Find the index where match was found in non-reversed string
                    DWORD foundAtNormStr = wcslen(normalStrbuffer) - foundAtRevStr - wcslen(find);
                    //MessageBoxA(NULL, std::to_string(foundAtNormStr).c_str(), "Info", 0);
                    //MessageBoxA(NULL, std::to_string(foundAt).c_str(), "Info", 0);
                    // set "findFrom" variable to the index after the match to look for other matches in the string
                    findFrom = foundAtRevStr + wcslen(find);
                    //MessageBoxA(NULL, std::to_string(findFrom).c_str(), "Info", 0);

                    // Free up memory
                    delete[] normalStrbuffer;
                    delete[] reverseStrbuffer;
                    return foundAtNormStr;
                }

            }
            // If absolutely no match was found, even after we loop through the string again, free up memory
            //MessageBox(hwnd, L"Do these get called?", L"info", 0);
            delete[] normalStrbuffer;
            delete[] reverseStrbuffer;

            // User clicked "Down" button
        }
        else {
            // Create a buffer which will hold the text in our EDIT control
            WCHAR* upBuffer = new WCHAR[bytesToAlloc * sizeof(WCHAR)];

            // Get the text from our EDIT control and putting it into our buffer
            GetDlgItemText(hwnd, TXT_INPUT, upBuffer, bytesToAlloc * sizeof(WCHAR));

            // If the user clicked the "up" button before, but now decided to click the "down" button; up to down conversion for findFrom
            if (wasDown == FALSE && down == TRUE)
            {
                findFrom = wcslen(upBuffer) - findFrom + wcslen(find);

                wasDown = TRUE;
            }

            // "start" holds the string from pGlobal starting from index "findFrom"
            const WCHAR* start = &upBuffer[findFrom];
            // "pos" holds the string starting at where a match was found. Typically, it would hold the first character, but we are using a pointer, so the rest of the string is stored.
            const WCHAR* pos = wcsstr(start, find);

            // If a match was found
            if (NULL != pos)
            {
                // Get the index of where the match was found
                DWORD foundAt = (pos - upBuffer);
                // set "findFrom" variable to the index after the match to look for other matches in the string
                findFrom = (foundAt + wcslen(find));
                // free up memory
                delete[] upBuffer;
                // return the value of the index of where the match was found
                return foundAt;
            }

            // If we are at or near the end of the string, we want to loop back up to that point, so we can check for matches
            /*
            * We are looking for "l", but we are at the end of the string, but because the program is designed to cut the string from the match onwards, we need to loop back
            * to get the rest of the string
            *
            * "hello world"
            *           ^
            */
            if (findFrom != 0 && pos == NULL)
            {
                // start of string
                start = &upBuffer[0];
                // We start the process of finding a match
                pos = wcsstr(start, find);


                // If a match was found <- grabbed from above
                if (NULL != pos)
                {
                    // Get the index of where the match was found
                    DWORD foundAt = (pos - upBuffer);
                    // set "findFrom" variable to the index after the match to look for other matches in the string
                    findFrom = (foundAt + wcslen(find));
                    // free up memory
                    delete[] upBuffer;
                    // return the value of the index of where the match was found
                    return foundAt;
                }

            }
            // If absolutely no match was found, even after we loop through the string again, free up memory
            //MessageBox(hwnd, L"Does this get called?", L"info", 0);
            delete[] upBuffer;
        }

    }
    MessageBox(hwnd, L"No match found", L"Info", 0);
    // If the whole string is searched and no match was found, we want to set the index back to 0, so the whole string can be observed again
    findFrom = 0;
    // Because no match was found, we return -1 to denote that a match wasn't found
    return -1;
}


#if 1
// Function to set text in chunks - This function will use less memory but with a little more CPU power when loading text, rather than loading all of it at once. It doesn't really speed anything up, but I heard that this was recommended.
void SetEditControlTextInChunks(HWND hEdit, const std::string& text) {
//void SetEditControlTextInChunks(HWND hEdit, CHAR* text) {
    size_t textLength = text.length();
    DWORD divideBy = 0;

    if (textLength <= divideBy || textLength <= 1000000)
    {
        SetWindowTextA(hEdit, text.c_str());
        return;
    }
    else if (textLength <= 10000000)
    {
        divideBy = 5;
    }
    else if (textLength <= 100000000)
    {
        divideBy = 10;
    }
    // For files larger than 100 million bytes
    else 
    {
        divideBy = 15;
    }

    // Debugging purposes for if files are less than 1 MB
    //MessageBox(hEdit, L"Hi", L"Hi", 0);

    const size_t chunkSize = textLength / divideBy; // Adjust chunk size as needed

    //MessageBoxA(hEdit, std::to_string(chunkSize).c_str(), "Info", 0);

    //size_t textLength = strlen(text);
    size_t offset = 0;

    //std::string text2 = *text;

    //MessageBoxA(hEdit, std::to_string(textLength).c_str(), "Info", 0);

    // temporarily disable drawing on EDIT
    SendMessage(hEdit, WM_SETREDRAW, FALSE, 0);

    while (offset < textLength) {
        //size_t length = min(chunkSize, textLength - offset);
        std::string chunk = text.substr(offset, chunkSize);
        SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM)chunk.c_str());
        //SetWindowTextA(hEdit, chunk.c_str());
        offset += chunkSize;
        
        // Set the scroll bar back to it's topmost and leftmost positions. If we don't do this, we won't be able to see the top of our file as new text is being added
        SendMessage(hEdit, WM_VSCROLL, SB_TOP, NULL);
        SendMessage(hEdit, WM_HSCROLL, SB_LEFT, NULL);
        //MessageBoxA(hEdit, std::to_string(offset).c_str(), "Info", 0);
    }
    // re-enable drawing on EDIT
    SendMessage(hEdit, WM_SETREDRAW, TRUE, 0);

    SetFocus(hEdit);
    SendMessage(hEdit, EM_SETSEL, 0, 0);
}
#endif