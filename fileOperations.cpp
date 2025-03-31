// fileOperations.cpp: Deals with creating, opening, writing, and reading files.

// TODO: Use memory mapping files for read/write operations for faster I/O operations; Additionally, find a way to use buffering/chunks for loading large files
#include "commheaders.h"
#include "memoryManagement.h"
#include "fileFunctionality.h"
#include "fileOperations.h"

// Checks to see if we are working with a file
BOOL IsWorkingWithFile;

// So it seems that WriteFile has some trouble putting unicode characters into files, not too sure why... That's why I created this function; DEPRECATED
#if 0
// https://stackoverflow.com/questions/7497958/write-a-unicode-cstring-to-a-file-using-writefile-api
void WriteUnicodeStringToFile(const CString& str, LPCWSTR FileName)
{
    HANDLE f = CreateFileW(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) return; //failed
    DWORD wr;
    unsigned char Header[2]; //unicode text file header

    // BOM Codes / Byte Order Marks
    // UTF-8 = EF BB BF
    // UTF-16 = FF FE
    Header[0] = 0xFF;
    Header[1] = 0xFE;

    //WriteFile(f, Header, 2, &wr, NULL);
    WriteFile(f, (LPCTSTR)str, str.GetLength() * 2, &wr, NULL);
    CloseHandle(f);
}
#endif

// UNICODE version of File Open - NOT RECOMMENDED FOR NOW; DEPRECATED
#if 0
// This code snippet demonstrates how to work with the common file dialog interface
void BasicFileOpen(HWND hwnd)
{
    OPENFILENAMEW ofn;       // common dialog box structure
    WCHAR szFile[260]; // buffer for file name
    HANDLE hf;              // file handle

    // Our filter list of filename extensions we want to search for. I put it in this format to make it easier for the user
    // Filter goes by the following format:
    // "NAME \0 EXTENSION-NAME-YOU-WANT-TO-LOOK-FOR.EXTENSION \0"
    const WCHAR filter[] =
        L"All (*.*)\0*.*\0"
        L"Text Documents (*.txt)\0*.txt\0"
        L"Word Documents (*.docx)\0*.docx\0"
        // Rich Text Format files aren't typically the best to open in a simple Notepad editor, even with the real Notepad
        L"Rich Documents (*.rtf)\0*.rtf\0"
        // You should NOT load images into text files, rather, this is just a test to see if we can look for multiple extensions
        L"Images (*.png, *.jpg, *.jpeg, *.webp, *.jiff, *.bmp)\0*.PNG;*.JPG;*.JPEG;*.WEBP;*.JIFF;*.BMP\0";

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = L'\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 2;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;


    // Display the Open dialog box. 
    if (GetOpenFileName(&ofn))
    {
        //MessageBox(hwnd, ofn.lpstrFile, L"Info", MB_OK);

        // We want to open the file only if it exists, and get a handle to it, so we can work with it.
        hf = CreateFileW(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        // Number of bytes that was read in our file. This variable will be changed/used by ReadFile
        DWORD bytesRead = 0;

        // Remember, each 1 char = 1 byte. So when we are getting the file size, we are also getting the number of characters in the program.
        // UPDATE: Now that we support Unicode characters, each character will be converted into 2 bytes to support Unicode.
        DWORD fileSize = GetFileSize(hf, NULL);
        // We create a buffer that's the size of our file size, plus 1 byte, in order to add the null character.
        // I need to do more research on memory allocation in Learncpp
        CHAR* buffer = new CHAR[fileSize + sizeof(CHAR)]; // Allocate space for null-terminated data

        WCHAR* buffer2 = new WCHAR[fileSize + sizeof(WCHAR)]; // Allocate space for null-terminated data

        // If we get a valid handle to our file
        if (hf != INVALID_HANDLE_VALUE)
        {
            // Read the file and all it's contents, NOT INCLUDING NULL CHARACTER
            ReadFile(hf, buffer, fileSize, &bytesRead, NULL);
            // Remember how we added an extra byte to our buffer variable? Well the contents from our file is JUST below  our buffer variable, by 1 byte, which is enough to put our null character in
            buffer[bytesRead] = '\0'; // Add null terminator because FileRead doesn't do this by default, If we don't do this, it will cause unexpected behavior, like showing weird unicode chars.

            // This will be able to support UTF-8 strings just like Notepad
            //int wchars_num = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)buffer, bytesRead, NULL, 0);
            MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, (LPCCH)buffer, -1, buffer2, fileSize + sizeof(WCHAR));

            MessageBoxA(hwnd, buffer, "Info...", 0);

            MessageBox(hwnd, buffer2, L"Info...", 0);

            // Add the contents from the file into our EDIT control
            SetWindowTextW(GetDlgItem(hwnd, TXT_INPUT), buffer2);

            // Add the PATH to the file we've opened to our filename global variable so we can Ctrl+S without having to open save dialog box
            IsWorkingWithFile = TRUE;
            //wcscpy(filename, ofn.lpstrFile);

            // Change the window title bar or window name to the file we've opened. This feature is like Notepad
            SetWindowTextW(hwnd, ofn.lpstrFile);

        }
        // I wonder if we should set our global variable "filename" to NULL if we do get an invalid handle?
        // filename = NULL;

        // We need to deallocate our variable to free up the memory we've used and no longer need
        delete[] buffer;
        delete[] buffer2;
        // Close the handle to our file
        CloseHandle(hf);
    }


}
#endif


// ANSI version of opening files - Memory Mapped Edition ; Uses less RAM based on my observations
// This code snippet demonstrates how to work with the common file dialog interface
void BasicFileOpen(HWND hwnd)
{
    OPENFILENAME ofn;       // common dialog box structure
    WCHAR szFile[260];      // buffer for file name
    HANDLE hFile;           // Handle to our file

    // Our filter list of filename extensions we want to search for. I put it in this format to make it easier for the user
    // Filter goes by the following format:
    // "NAME \0 EXTENSION-NAME-YOU-WANT-TO-LOOK-FOR.EXTENSION \0"
    const WCHAR filter[] =
        L"All (*.*)\0*.*\0"
        L"Text Documents (*.txt)\0*.txt\0"
        L"Word Documents (*.docx)\0*.docx\0"
        // Rich Text Format files aren't typically the best to open in a simple Notepad editor, even with the real Notepad
        L"Rich Documents (*.rtf)\0*.rtf\0"
        // You should NOT load images into text files, rather, this is just a test to see if we can look for multiple extensions
        L"Images (*.png, *.jpg, *.jpeg, *.webp, *.jiff, *.bmp)\0*.PNG;*.JPG;*.JPEG;*.WEBP;*.JIFF;*.BMP\0";

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = L'\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 2;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;


    // Display the Open dialog box. 
    if (GetOpenFileName(&ofn))
    {
        DWORD bytesRead; // Number of bytes that was read in our file. This variable will be changed/used by ReadFile
        DWORD fileSize; // File size in number of BYTES that we are working with; Each char = 1 byte. So when we are getting the file size, we are also getting the number of characters in the program.

        // We want to open the file only if it exists, and get a handle to it, so we can work with it.
        hFile = CreateFile(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        // If we get a valid handle to our file
        if (hFile != INVALID_HANDLE_VALUE)
        {
            // We want to use memory mapping functions instead of traditional I/O operations on a file
            HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, L"OpenFileMap");

            LPVOID fileAddress = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);

            // Read the contents
            //CHAR* pContents = static_cast<CHAR*>(fileAddress);
            const CHAR* pContents = (PCHAR)fileAddress;

#if 0
            std::string test = pContents;
            DWORD testLen = strlen(pContents);
            //MessageBoxA(NULL, std::to_string(testLen).c_str(), "testx", MB_OK);

            DWORD bufferAtTime = 1024;
            int index = 0;
            while (index < testLen)
            {
                //SetWindowTextA(GetDlgItem(hwnd, TXT_INPUT), test.substr(index, bufferAtTime));

                SendMessageA(GetDlgItem(hwnd, TXT_INPUT), EM_SETSEL, -1, -1); // Move cursor to end
                SendMessageA(GetDlgItem(hwnd, TXT_INPUT), EM_REPLACESEL, TRUE, test.substr(index, bufferAtTime));

                index += bufferAtTime;
                
            }
#endif

            // Use pContents as needed (e.g., print it, process it, etc.)
            //MessageBoxA(hwnd, pContents, "Caption", 0);

            // Clear/free any WM_UNDO/EM_UNDO buffer memory
            FreeUndoBufferMemory(GetDlgItem(hwnd, TXT_INPUT));

            // Add the contents from the file into our EDIT control
            //SetWindowTextA(GetDlgItem(hwnd, TXT_INPUT), pContents);
            //SetEditText(GetDlgItem(hwnd, TXT_INPUT), (TCHAR*)pContents);

#if 1
            // Crashes the program for some reason... - Probably because of no NULL terminator...
            //MessageBoxA(hwnd, std::to_string(strlen(pContents)).c_str(), "Info", 0);

            DWORD fileSize = GetFileSize(hFile, NULL);

            // If the file has no contents, we will simply use a empty string. Otherwise, use our other function.
            if (fileSize == 0)
            {
                SetWindowTextA(GetDlgItem(hwnd, TXT_INPUT), "");
            }
            else {
                SetEditControlTextInChunks(GetDlgItem(hwnd, TXT_INPUT), pContents);

            }

#endif

            // Add the PATH to the file we've opened to our filename global variable so we can Ctrl+S without having to open save dialog box
            IsWorkingWithFile = TRUE;

            // Change the window title bar or window name to the file we've opened. This feature is like Notepad
            SetWindowTextW(hwnd, ofn.lpstrFile);

            // Close the handle to our file/file mapping
            UnmapViewOfFile(fileAddress);
            CloseHandle(hFileMap);
            CloseHandle(hFile);
        }
        // I wonder if we should set our global variable "filename" to NULL if we do get an invalid handle?
        // Actually, lets not do that because we may be working with a previous file
        // filename = NULL;
    }


}

#if 0
// ANSI version of opening files
// This code snippet demonstrates how to work with the common file dialog interface
void BasicFileOpen(HWND hwnd)
{
    OPENFILENAME ofn;       // common dialog box structure
    WCHAR szFile[260];      // buffer for file name
    HANDLE hFile;           // Handle to our file

    // Our filter list of filename extensions we want to search for. I put it in this format to make it easier for the user
    // Filter goes by the following format:
    // "NAME \0 EXTENSION-NAME-YOU-WANT-TO-LOOK-FOR.EXTENSION \0"
    const WCHAR filter[] =
        L"All (*.*)\0*.*\0"
        L"Text Documents (*.txt)\0*.txt\0"
        L"Word Documents (*.docx)\0*.docx\0"
        // Rich Text Format files aren't typically the best to open in a simple Notepad editor, even with the real Notepad
        L"Rich Documents (*.rtf)\0*.rtf\0"
        // You should NOT load images into text files, rather, this is just a test to see if we can look for multiple extensions
        L"Images (*.png, *.jpg, *.jpeg, *.webp, *.jiff, *.bmp)\0*.PNG;*.JPG;*.JPEG;*.WEBP;*.JIFF;*.BMP\0";

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = L'\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 2;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;


    // Display the Open dialog box. 
    if (GetOpenFileName(&ofn))
    {
        DWORD bytesRead; // Number of bytes that was read in our file. This variable will be changed/used by ReadFile
        DWORD fileSize; // File size in number of BYTES that we are working with; Each char = 1 byte. So when we are getting the file size, we are also getting the number of characters in the program.

        // We want to open the file only if it exists, and get a handle to it, so we can work with it.
        hFile = CreateFile(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        fileSize = GetFileSize(hFile, NULL);

        // We create a buffer that's the size of our file size, plus 1 byte, in order to add the null character.
        CHAR* buffer = new CHAR[fileSize + sizeof(CHAR)];

        // If we get a valid handle to our file
        if (hFile != INVALID_HANDLE_VALUE)
        {
            // Read the file and all it's contents, NOT INCLUDING NULL CHARACTER; Contents from file will be put into "buffer" variable with number of bytes read put in the "bytesRead" variable
            ReadFile(hFile, buffer, fileSize, &bytesRead, NULL);

            // Remember how we added an extra byte to our buffer variable? Well the contents from our file is JUST below our buffer variable, by 1 byte, which is enough to put our null character in
            // Add null terminator because FileRead doesn't do this by default, If we don't do this, it will cause unexpected behavior, like showing weird unicode chars.
            buffer[bytesRead] = '\0';

            // Clear/free any WM_UNDO/EM_UNDO buffer memory
            FreeUndoBufferMemory(GetDlgItem(hwnd, TXT_INPUT));

            // Add the contents from the file into our EDIT control
            SetWindowTextA(GetDlgItem(hwnd, TXT_INPUT), buffer);

            // Add the PATH to the file we've opened to our filename global variable so we can Ctrl+S without having to open save dialog box
            IsWorkingWithFile = TRUE;

            // Change the window title bar or window name to the file we've opened. This feature is like Notepad
            SetWindowTextW(hwnd, ofn.lpstrFile);

            // Close the handle to our file
            CloseHandle(hFile);

        }
        // I wonder if we should set our global variable "filename" to NULL if we do get an invalid handle?
        // Actually, lets not do that because we may be working with a previous file
        // filename = NULL;

        // We need to deallocate our variable to free up the memory we've used and no longer need
        delete[] buffer;
    }


}
#endif

// Unicode saving, but doesn't work well if you try to open this file with the same program. Very buggy.; DEPRECATED
#if 0
void BasicFileSave(HWND hwnd)
{
    // If our filename variable has an actual PATH towards a file, we want to automatically save it without having to prompt for save dialog box. Otherwise, prompt for save dialog box
    if (!IsWorkingWithFile)
    {
        OPENFILENAMEW ofn;       // common dialog box structure
        WCHAR szFile[260]; // buffer for file name
        HANDLE hf;              // file handle

        // Our filter list of filename extensions we want to search for. I put it in this format to make it easier for the user
        // Filter goes by the following format:
        // "NAME \0 EXTENSION-NAME-YOU-WANT-TO-LOOK-FOR.EXTENSION \0"
        const WCHAR filter[] =
            L"All (*.*)\0*.*\0"
            L"Text Documents (*.txt)\0*.txt\0"
            L"Word Documents (*.docx)\0*.docx\0"
            // Rich Text Format files aren't typically the best to open in a simple Notepad editor, even with the real Notepad
            L"Rich Documents (*.rtf)\0*.rtf\0"
            // You should NOT load images into text files, rather, this is just a test to see if we can look for multiple extensions
            L"Images (*.png, *.jpg, *.jpeg, *.webp, *.jiff, *.bmp)\0*.png;*.jpg;*.jpeg;*.webp;*.jiff;*.bmp\0";

        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = L'\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.lpstrDefExt = L"txt"; // The default extension, if the user hasn't picked any from the Filter, will be TXT
        ofn.nFilterIndex = 2;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST;

        if (GetSaveFileName(&ofn))
        {
            // The handle to our EDIT control
            HWND editControl = GetDlgItem(hwnd, TXT_INPUT);
            // Get the length of the text from our EDIT control
            DWORD dwTextLength = GetWindowTextLength(editControl);
            //DWORD dwBufferSize = dwTextLength + 1;

            // Always ask for memory from the HEAP for dynamic allocation
            // Not exactly too sure why we need +sizeof(wchar), but I've noticed that without it, we would lose a character.
            CHAR* buffer = new CHAR[(dwTextLength * sizeof(WCHAR)) + sizeof(WCHAR)];// need to multiply by WCHAR because wchar is 2x that of char. We add a WCHAR because we would be missing the last character without it

            // Get text from our edit control, then put it into our buffer.
            GetWindowText(editControl, (LPWSTR)buffer, dwTextLength + sizeof(WCHAR)); // We add a WCHAR because we would be missing the last character without it

            // We need to do type conversion into LPWSTR, otherwise we would only get the first character
            CString str = (LPWSTR)buffer;

            //MessageBox(hwnd, (LPWSTR)buffer, L"Info", 0);
            //MessageBox(hwnd, str, L"Info", 0);

            // We want to save the file now by writing to it.
            WriteUnicodeStringToFile(str, ofn.lpstrFile);

            SetFocus(GetDlgItem(hwnd, TXT_INPUT));

            // Set the main window title to be that of the filename we saved
            SetWindowText(hwnd, ofn.lpstrFile);
            IsWorkingWithFile = TRUE;

            delete[] buffer;
        }
    }
    else {
        // All this code is taken directly from above, only except it doesn't show save dialog box, it just automatically saves it towards the file

        WCHAR pFilename[260];
        GetWindowText(hwnd, pFilename, sizeof(pFilename));

        MessageBox(hwnd, pFilename, L"Saving File", MB_OK);

        // The handle to our EDIT control
        HWND editControl = GetDlgItem(hwnd, TXT_INPUT);
        // Get the length of the text from our EDIT control
        DWORD dwTextLength = GetWindowTextLength(editControl);
        //DWORD dwBufferSize = dwTextLength + 1;

        // Always ask for memory from the HEAP for dynamic allocation
        // Not exactly too sure why we need +sizeof(wchar), but I've noticed that without it, we would lose a character.
        CHAR* buffer = new CHAR[(dwTextLength * sizeof(WCHAR)) + sizeof(WCHAR)];// need to multiply by WCHAR because wchar is 2x that of char. We add a WCHAR because we would be missing the last character without it

        // Get text from our edit control, then put it into our buffer.
        GetWindowText(editControl, (LPWSTR)buffer, dwTextLength + sizeof(WCHAR)); // We add a WCHAR because we would be missing the last character without it

        // We need to do type conversion into LPWSTR, otherwise we would only get the first character
        CString str = (LPWSTR)buffer;

        //MessageBox(hwnd, (LPWSTR)buffer, L"Info", 0);
        //MessageBox(hwnd, str, L"Info", 0);

        // We want to save the file now by writing to it.
        WriteUnicodeStringToFile(str, pFilename);

        SetFocus(GetDlgItem(hwnd, TXT_INPUT));

        delete[] buffer;
    }

}
#endif

void BasicFileSave(HWND hwnd)
{
    HANDLE hFile;                                           // Handle to our file
    WCHAR filePathName[260];                                // buffer for file name
    HWND editControl = GetDlgItem(hwnd, TXT_INPUT);         // The handle to our EDIT control
    DWORD textLength = GetWindowTextLength(editControl);    // Get the length of the text from our EDIT control. We will use this to calculate the buffer.
    DWORD bytesWritten;                                     // Number of bytes that was written to our file
    CHAR* buffer = new CHAR[textLength + sizeof(CHAR)];     // The buffer will hold the string from our EDIT control. Always ask for memory from the HEAP for dynamic allocation. For some reason, we have to use + sizeof(char)

    // We are getting the text from our EDIT control and putting it into the buffer
    // For some reason, we have to use + sizeof(char). Maybe this is because of NULL char?
    GetWindowTextA(editControl, buffer, textLength + sizeof(CHAR));

    // If we are NOT working with a file, prompt for save dialog box.
    if (!IsWorkingWithFile)
    {
        OPENFILENAME ofn; // Common dialog box structure used to initialized our Save Dialog box

        // Our filter list of filename extensions we want to search for. I put it in this format to make it easier for the user
        // Filter goes by the following format:
        // "NAME \0 EXTENSION-NAME-YOU-WANT-TO-LOOK-FOR.EXTENSION \0"
        const WCHAR filter[] =
            L"All (*.*)\0*.*\0"
            L"Text Documents (*.txt)\0*.txt\0"
            L"Word Documents (*.docx)\0*.docx\0"
            // Rich Text Format files aren't typically the best to open in a simple Notepad editor, even with the real Notepad
            L"Rich Documents (*.rtf)\0*.rtf\0"
            // You should NOT load images into text files, rather, this is just a test to see if we can look for multiple extensions
            L"Images (*.png, *.jpg, *.jpeg, *.webp, *.jiff, *.bmp)\0*.png;*.jpg;*.jpeg;*.webp;*.jiff;*.bmp\0";

        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = filePathName;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(filePathName);
        ofn.lpstrFilter = filter;
        ofn.lpstrDefExt = L"txt"; // The default extension, if the user hasn't picked any from the Filter, will be TXT
        ofn.nFilterIndex = 2;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST;

        // Open Save Dialog box
        if (GetSaveFileName(&ofn))
        {
            // We always want to create a new file. If the file already exists, everything is truncated (deleted), then, with our WriteFile, we will add data to the file
            // If the file doesn't exist, we simply create a new file, then our WriteFile will write data to the new file
            // The reason for this is because if we simply use OPEN_ALWAYS, the data from the file before we write to it will not have everything deleted, so some chunks
            // of data from the previous file may still exist when we save, which is not what we want. We just want to overwrite everything, then add our new data to it, even if its the same data.
            hFile = CreateFile(filePathName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            // If we get a valid handle
            if (hFile != INVALID_HANDLE_VALUE)
            {
                // Write the contents from our EDIT control into the file we like
                WriteFile(hFile, buffer, textLength, &bytesWritten, NULL);
                // Close the handle to our file
                CloseHandle(hFile);

                SetFocus(GetDlgItem(hwnd, TXT_INPUT));

                // Set the main window title to be that of the filename we saved
                SetWindowText(hwnd, filePathName);

                // We want to set "IsWorkingWithFile" to be TRUE because we are now working with a file.
                IsWorkingWithFile = TRUE;

            }
            else {
                MessageBox(hwnd, L"Invalid file handle", L"Info", MB_OK);
            }

        }

        // Deallocate memory so the system can use it again
        //MessageBox(hwnd, L"Do these get called?", L"info", 0);
        delete[] buffer;

    }
    // If we are working with an active file, we want to automatically save it without having to prompt for save dialog box.
    else {
        // All this code is taken directly from above, only except it doesn't show save dialog box, it just automatically saves it towards the file

        // We're getting the file name along with it's path so we know where to save the file
        GetWindowText(hwnd, filePathName, sizeof(filePathName));

        // Let the user know we are saving the file
        MessageBox(hwnd, filePathName, L"Saving File", MB_OK);

        // We always want to create a new file. If the file already exists, everything is truncated (deleted), then, with our WriteFile, we will add data to the file
        // If the file doesn't exist, we simply create a new file, then our WriteFile will write data to the new file
        // The reason for this is because if we simply use OPEN_ALWAYS, the data from the file before we write to it will not have everything deleted, so some chunks
        // of data from the previous file may still exist when we save, which is not what we want. We just want to overwrite everything, then add our new data to it, even if its the same data.
        hFile = CreateFile(filePathName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        // If we get a valid handle
        if (hFile != INVALID_HANDLE_VALUE)
        {
            // Write the contents from our EDIT control into the file we like
            WriteFile(hFile, buffer, textLength, &bytesWritten, NULL);
            // Close the handle to our file
            CloseHandle(hFile);

            // We want the caret to be set to the Input box so the user can continue typing after they save. If we don't do this, the user has to click on the input box after they save, which is a waste of time
            SetFocus(GetDlgItem(hwnd, TXT_INPUT));

        }
        else {
            MessageBox(hwnd, L"Invalid file handle", L"Info", MB_OK);
        }

        // Deallocate memory so the system can use it again
        delete[] buffer;
    }

}

void NewWindow(HWND hwnd)
{
    // If there is any text, we ask if the user wants to save it
    // We need to clear the input window and reset the window title bar to it's original state


    DWORD mCode;                                                           // MessageBox button code status
    DWORD textLength = GetWindowTextLength(GetDlgItem(hwnd, TXT_INPUT));   // Holds the number of characters in our EDIT control. We will use this variable to check if it's greater than one.

    // If we are working with a file and we want to create a new window, we ask the user to save their file first
    if (IsWorkingWithFile)
    {

        mCode = MessageBox(hwnd, L"Do you want to save your file?", L"Info", MB_ICONINFORMATION | MB_YESNOCANCEL);

        // If user clicks on "Yes" button
        if (mCode == IDYES)
        {
            // We want to save the file
            BasicFileSave(hwnd);

            // We clear any text and set the window title back to it's original state
            FreeUndoBufferMemory(GetDlgItem(hwnd, TXT_INPUT));
            SetWindowTextA(GetDlgItem(hwnd, TXT_INPUT), "");

            // Reset parent window title back to it's original title, which is "Notepad 2.0"
            SetWindowText(hwnd, szTitle);

            // Set the focus back to our INPUT box.
            SetFocus(GetDlgItem(hwnd, TXT_INPUT));

            // We set to FALSE because we are no longer working with the file from the previous session
            IsWorkingWithFile = FALSE;
        }
        // If user clicks on "No" we don't save the file, but we create a new session
        else if (mCode == IDNO) {

            MessageBox(hwnd, L"All work done in the previous file will be unsaved", L"Info", MB_OK);

            // We clear any text and set the window title back to it's original state
            FreeUndoBufferMemory(GetDlgItem(hwnd, TXT_INPUT));
            SetWindowTextA(GetDlgItem(hwnd, TXT_INPUT), "");

            // Reset parent window title back to it's original title, which is "Notepad 2.0"
            SetWindowText(hwnd, szTitle);

            // Set the focus back to our INPUT box.
            SetFocus(GetDlgItem(hwnd, TXT_INPUT));

            // We set to FALSE because we are no longer working with the file from the previous session
            IsWorkingWithFile = FALSE;
        }
        // If user clicks "Cancel", we don't save the file and we don't create a new session
        else if (mCode == IDCANCEL) {} // Do nothing

    }
    // If we are working with an Untitled file and we want to create a new window, if we have any characters in our EDIT control, we ask the user to save their work
    else if (!IsWorkingWithFile && textLength > 0) {
        mCode = MessageBox(hwnd, L"Do you want to save your file?", L"Info", MB_YESNOCANCEL);

        // If user clicks on "Yes" button
        if (mCode == IDYES)
        {
            // Save file
            BasicFileSave(hwnd);

            // We clear any text and set the window title back to it's original state
            FreeUndoBufferMemory(GetDlgItem(hwnd, TXT_INPUT));
            SetWindowTextA(GetDlgItem(hwnd, TXT_INPUT), "");

            // Reset parent window title back to it's original title, which is "Notepad 2.0"
            SetWindowText(hwnd, szTitle);

            // Set the focus back to our INPUT box.
            SetFocus(GetDlgItem(hwnd, TXT_INPUT));

            // We set filename to FALSE because BasicFileSave sets filename to TRUE, however, we want to create a new file, so we set it to FALSE
            IsWorkingWithFile = FALSE;

        }
        // If user clicks on "No" button
        else if (mCode == IDNO) {
            MessageBox(hwnd, L"All work done in the previous file will be unsaved", L"Info", MB_OK);

            // We clear any text and set the window title back to it's original state
            FreeUndoBufferMemory(GetDlgItem(hwnd, TXT_INPUT));
            SetWindowTextA(GetDlgItem(hwnd, TXT_INPUT), "");

            // Reset parent window title back to it's original title, which is "Notepad 2.0"
            SetWindowText(hwnd, szTitle);
            // Set the focus back to our INPUT box.
            SetFocus(GetDlgItem(hwnd, TXT_INPUT));

        }
        // If user clicks on "Cancel" button
        else if (mCode == IDCANCEL) {} // Do nothing
    }

}

// If a user wants to drag and drop files into our EDIT control. Put the contents from that file into our EDIT control
void DragAndDrop(HWND hwnd, WPARAM wParam)
{
    // https://learn.microsoft.com/en-us/windows/win32/shell/wm-dropfiles
    // WM-DROPFILES - Sent when the user drops a file on the window of an application that has registered itself as a recipient of dropped files.

    HANDLE hFile;                   // Handle to the file that's dragged into the EDIT control
    HDROP hDrop = (HDROP)wParam;    // A handle to an internal structure describing the dropped files. Pass this handle DragFinish, DragQueryFile, or DragQueryPoint to retrieve information about the dropped files.
    WCHAR filePathName[260];        // Buffer for file name
    DWORD bytesRead;                // Number of bytes that was read in our file. This variable will be changed/used by ReadFile
    DWORD fileSize;                 // Remember, each 1 char = 1 byte. So when we are getting the file size, we are also getting the number of characters in the program.

    // We always want to use the first file. Although a user can drag multiple files, we only want to process the first one
    // Also, The MAX_PATH constant is a legacy value from the Windows API that represents the maximum length (in characters) of a file path. It’s defined as 260 characters.  
    DragQueryFile(hDrop, 0, filePathName, MAX_PATH);

    // OK, so now that we got the filename, let's load that file into the application. We want to open the file only if it exists, and get a handle to it, so we can work with it.
    hFile = CreateFile(filePathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    fileSize = GetFileSize(hFile, NULL);

    // We create a buffer that's the size of our file size, plus 1 byte, in order to add the null character.
    //CHAR* buffer = new CHAR[fileSize + sizeof(CHAR)];

    // If we get a valid handle to our file
    if (hFile != INVALID_HANDLE_VALUE)
    {
        // We want to use memory mapping functions instead of traditional I/O operations on a file
        HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, L"OpenFileMap");

        LPVOID fileAddress = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);

        // Read the contents
        //CHAR* pContents = static_cast<CHAR*>(fileAddress);
        CHAR* pContents = (PCHAR)fileAddress;

        // Use pContents as needed (e.g., print it, process it, etc.)
            //MessageBoxA(hwnd, pContents, "Caption", 0);

            // Clear/free any WM_UNDO/EM_UNDO buffer memory
        FreeUndoBufferMemory(GetDlgItem(hwnd, TXT_INPUT));

        // Add the contents from the file into our EDIT control
        //SetWindowTextA(GetDlgItem(hwnd, TXT_INPUT), pContents);

        // If the file has no contents, we will simply use a empty string. Otherwise, use our other function.
        if (fileSize == 0)
        {
            SetWindowTextA(GetDlgItem(hwnd, TXT_INPUT), "");
        }
        else {
            SetEditControlTextInChunks(GetDlgItem(hwnd, TXT_INPUT), pContents);

        }

        // Set the caret focus back onto the EDIT control
        //SetFocus(GetDlgItem(hwnd, TXT_INPUT));

        // Add the PATH to the file we've opened to our filename global variable so we can Ctrl+S without having to open save dialog box
        IsWorkingWithFile = TRUE;

        // Change the window title bar or window name to the file we've opened.
        SetWindowText(hwnd, filePathName);

        // Close the handle to our file/file mapping
        UnmapViewOfFile(fileAddress);
        CloseHandle(hFileMap);
        CloseHandle(hFile);

    }

    // Close the handle to our file
    //CloseHandle(hFile);

    // Releases memory that the system allocated for use in transferring file names to the application.
    DragFinish(hDrop);
}
#if 0
// If a user wants to drag and drop files into our EDIT control. Put the contents from that file into our EDIT control
void DragAndDrop(HWND hwnd, WPARAM wParam)
{
    // https://learn.microsoft.com/en-us/windows/win32/shell/wm-dropfiles
    // WM-DROPFILES - Sent when the user drops a file on the window of an application that has registered itself as a recipient of dropped files.

    HANDLE hFile;                   // Handle to the file that's dragged into the EDIT control
    HDROP hDrop = (HDROP)wParam;    // A handle to an internal structure describing the dropped files. Pass this handle DragFinish, DragQueryFile, or DragQueryPoint to retrieve information about the dropped files.
    WCHAR filePathName[260];        // Buffer for file name
    DWORD bytesRead;                // Number of bytes that was read in our file. This variable will be changed/used by ReadFile
    DWORD fileSize;                 // Remember, each 1 char = 1 byte. So when we are getting the file size, we are also getting the number of characters in the program.

    // We always want to use the first file. Although a user can drag multiple files, we only want to process the first one
    // Also, The MAX_PATH constant is a legacy value from the Windows API that represents the maximum length (in characters) of a file path. It’s defined as 260 characters.  
    DragQueryFile(hDrop, 0, filePathName, MAX_PATH);

    // OK, so now that we got the filename, let's load that file into the application. We want to open the file only if it exists, and get a handle to it, so we can work with it.
    hFile = CreateFile(filePathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    fileSize = GetFileSize(hFile, NULL);

    // We create a buffer that's the size of our file size, plus 1 byte, in order to add the null character.
    CHAR* buffer = new CHAR[fileSize + sizeof(CHAR)];

    // If we get a valid handle to our file
    if (hFile != INVALID_HANDLE_VALUE)
    {
        // Read the file and all it's contents, NOT INCLUDING NULL CHARACTER
        ReadFile(hFile, buffer, fileSize, &bytesRead, NULL);

        // Remember how we added an extra byte to our buffer variable? Well the contents from our file is JUST below  our buffer variable, by 1 byte, which is enough to put our null character in. Add null terminator because FileRead doesn't do this by default, If we don't do this, it will cause unexpected behavior, like showing weird unicode chars.
        buffer[bytesRead] = '\0';

        // Clear/free any buffer memory from WM_UNDO/EM_UNDO
        FreeUndoBufferMemory(GetDlgItem(hwnd, TXT_INPUT));

        // Add the contents from the file into our EDIT control
        SetWindowTextA(GetDlgItem(hwnd, TXT_INPUT), buffer);

        // Set the caret focus back onto the EDIT control
        SetFocus(GetDlgItem(hwnd, TXT_INPUT));

        // Set IsWorkingWithFile to TRUE because we are now working with a file
        IsWorkingWithFile = TRUE;

        // Change the window title bar or window name to the file we've opened.
        SetWindowText(hwnd, filePathName);
    }
    // We need to deallocate our variable to free up the memory we've used and no longer need
    delete[] buffer;

    // Close the handle to our file
    CloseHandle(hFile);

    // Releases memory that the system allocated for use in transferring file names to the application.
    DragFinish(hDrop);
}
#endif