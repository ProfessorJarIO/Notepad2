// main.cpp: Contains code that initalizes and creates our application
#include "commheaders.h"
#include "fileFunctionality.h"
#include "fileOperations.h"

//#define _WIN32_WINNT 0x0600

const WCHAR szWindowClass[] = TEXT("DesktopApp");
WCHAR szTitle[] = TEXT("Notepad 2.0");

// Global handle to our executable instance
HINSTANCE hInstance;

// Window handle of dialog box
HWND hwndModelessDialog = NULL;

// handle to Find dialog box
HWND hFindDialog = NULL;

// Unique Message identifier for FINDMSGSTRING. We will use this unique ID to check for messages specifically for looking for our Find Dialog box
UINT uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);

// Common dialog box structure for our Find Dialog Box
FINDREPLACE fr;

// Buffer receiving string that holds what string we want to "Find" with our "Find dialog box"
WCHAR findWhatBuffer[80];

// Global handle to our font
HFONT hFont;

// We set the max number of characters in our EDIT control to be higher than what it originally was
// The default max was 32,767 characters. But to open big files, you need more.
// https://learn.microsoft.com/en-us/windows/win32/controls/em-setlimittext
// 
// Remember, we typically shouldn't open insanely large files inside of our application, even with regular Notepad. I believe 64 million characters should be more than enough.
// https://answers.microsoft.com/en-us/windows/forum/all/is-there-any-limit-of-size-of-windows-default/e2753aba-c23d-475c-87fe-c9da3bbc7ff7?tab=AllReplies#tabs
// 
// Another thing I should mention is by setting the max number of characters our EDIT contorl can hold, we are also changing the buffer for WM_UNDO/EM_UNDO buffer. So EM_SETTEXTLIIMT = buffer.
// 
// Can hold at MOST 8 million characters now. Change this to your liking.
DWORD MAX_NUM_CHARS = 1024 * 1024 * 500;

// We need to put this up here otherwise we get an undeclared identifier; This is a forward declaration
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ModelessDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ModalDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

void createInputBox(HWND hwnd)
{
    HWND hInput = CreateWindow(
        TEXT("EDIT"),  // Predefined class; Unicode assumed; HAS TO BE THE PREDEFINED CLASS OF EDIT
        NULL,      // Text, this can be set to NULL if you don't want predefined text
        WS_MAXIMIZE | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL,  // Styles 
        0,         // x position 
        0,         // y position 
        0,        // width
        0,        // height
        hwnd,     // Parent window
        (HMENU)TXT_INPUT,       // Put unique ID here to refer back to it if needed
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.


    SendMessage(hInput, EM_SETLIMITTEXT, MAX_NUM_CHARS, NULL);

    // We want to automatically set the focus towards my EDIT contorl just like regular Notepad, so we can start typing right away instead of having the user click, then type
    SetFocus(hInput);

}

// APIENTRY = WINAPI
// https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-winmain
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{

    // ------Define Window------: Give characteristics to our main window
    
    // Global variable
    hInstance = hInst;

    WNDCLASSEX wc; // Create a structure variable

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(189, 189, 189));
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    wc.lpszClassName = szWindowClass;
    wc.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));

    // ------Register Window------: By registering our class. Our parent window, when we create it, will inherit these characteristics
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, TEXT("Registering the class failed"), TEXT("Failed Register"), NULL);
        return 1;
    }

    // ------Create Window------: Create our window using our custom class for defining a window
    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW | WS_EX_ACCEPTFILES,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        600,
        500,
        NULL,
        NULL,
        hInst,
        NULL
    );

    // If we cannot create the parent window, alert user and return failcode message
    if (!hWnd) {
        MessageBox(NULL, TEXT("Creating the window failed"), TEXT("Failed Window"), NULL);
        return 1;
    }

    // ------Show Window------: Actually show our parent window
    
    // replace SW_SHOWDEFAULT with cmdshow from our WinMain function
    ShowWindow(hWnd, cmdshow);

    // ------Update Window------: Updates the client area of our parent window

    UpdateWindow(hWnd);

    // ------Handle Messages------: Handles messages sent to the parent window
    
    // Before, the first parameter for LoadAccelerators was hInstance, but I've learned that you can just use GetModuleHandle(NULL), which retrives the handle used to create the calling process (.exe file)/instance handle, which is what we want
    HACCEL haccel = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_ACCELERATOR1));

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        // You may have noticed I've gotten rid of !IsWindow(hwndGoto)l, the reason is because we already checked if the window exists in the WinProc, so no need to check it agian
        // Additionally, it's important that we have TranslateAccelerator first, before we check for IsDialogMessage, because if we switched them around, keyboard acceleration
        // would no longer work. 
        // 
        // The answer goes back to the return values of the two function. The Translate­Accelerator function returns a nonzero value if it recognized the message as an accelerator (and posted a WM_COMMAND message). The Is­Dialog­Message function returns a nonzero value if it recognized the message as a message for the dialog (and dispatched it). 
        // Now look at what happens if you have a message that is both a message for the dialog and an accelerator. For example, focus is on a button control in the dialog box and you press, say, Alt+F2.
        // Let’s say you call Is­Dialog­Message first. The Is­Dialog­Message function says, “Why yes, this message is for the dialog box, so I dispatched it to the button control. Mission accomplished. I’m so awesome!” The Is­Dialog­Message function returns a non-zero value, and the Translate­Accelerator never gets a chance to run
        // On the other hand, if you call Translate­Accelerator first, then the Translate­Accelerator function sees the accelerator key and posts the WM_COMMAND function to the dialog window, then it returns a non-zero value to say “Why yes, this message is an accelerator, so I posted a WM_COMMAND message. Mission accomplished. I’m so awesome!” The Translate­Accelerator function returns a non-zero value, and the Is­Dialog­Message never gets a chance to run.
        // The question of which one to call first is therefore a matter of priority. If the user presses the accelerator key while focus is on a control in the dialog box, which is more important to you: The fact that it is an accelerator, or the fact that it is a message that targets the dialog box? Whichever is more important to you goes first.
        // But wait, that’s not the end of the story. Note that the above code calls Translate­Accelerator unconditionally, which means that the accelerator keys are active even if focus is not on the dialog box at all. For example, focus may be on another window on the same thread (say, the owner of the modeless dialog box). You probably don’t want the modeless dialog box stealing accelerators from the owner. To avoid this problem, you need to translate accelerators for your dialog box only if the focus is somewhere in your dialog box.
        // https://devblogs.microsoft.com/oldnewthing/20160818-00/?p=94115
        // 
        // So if DON'T have a message for a keyboard accleration, AND we DON'T have a DialogMessage to process, we translate and dispatch messages normally
        // If the TranslateAccelerator succeeds, we don't want to process anymore normal messages, that's why we used the NOT operator with the AND operator, so we don't continue on
        // If IsDialogMessage has processed a dialog message, we don't want to process anymore normal messages, so we use NOT operator and AND operator, so we don't continue on
        // Like I said above, if we don't a keyboard acceleration message to process, and IsDialogMessage didn't process any dialog messages, which both functions are set to 0, or FALSE, the NOT statement switches it to TRUE, so we can process normal messages
        // 
        // Another way to think of it:

        /*
        if (!TranslateAccelerator(hwnd, haccel, &msg))
        {
            if (!IsDialogMessage(hwndGoto, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        */

        // TLDR: You need to check for TranslateMessage first before IsDialogMessage
        if (!TranslateAccelerator(hWnd, haccel, &msg) && !IsDialogMessage(hwndModelessDialog, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

        }


    }
    // Our return code status code sent by WM_QUIT in wParam
    return (int)msg.wParam;


}


// https://learn.microsoft.com/en-us/windows/win32/winmsg/using-window-procedures#designing-a-window-procedure
LRESULT CALLBACK MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{

    switch (uMsg)
    {
    case WM_DESTROY:
        DeleteObject((HBRUSH)GetClassLongPtr(hwnd, GCLP_HBRBACKGROUND)); // Release the window's background brush
        DeleteObject(hFont); // Delete the handle to the custom font we created
        PostQuitMessage(0);
        return 0;

    case WM_CREATE:
        // Noticed how we had to encapsulate each functions between curly brackets {}, otherwise we will get Compiler Error C2361
    {
        createInputBox(hwnd);

#if 1
        // https://cplusplus.com/forum/windows/27369/
        // I still need to do more research on Creating fonts
        hFont = CreateFont(18, 8, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, TEXT("Consolas"));
        SendMessage(GetDlgItem(hwnd, TXT_INPUT),             // Handle of edit control
            WM_SETFONT,         // Message to change the font
            (WPARAM)hFont,     // handle of the font
            TRUE // Redraw text
        );
#endif

        //SetEditText(GetDlgItem(hwnd, TXT_INPUT), _T("Hello"));
        //SetEditText(GetDlgItem(hwnd, TXT_INPUT), TEXT("Hello"));

        break;
    }

    case WM_SIZE:
        // Look at WM_SIZE Documentation: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-size
        // Updates the size of our EDIT control
        // It specifically states:
        // The low-order word of lParam specifies the new width of the client area.
        // The high-order word of lParam specifies the new height of the client area.
        SetWindowPos(GetDlgItem(hwnd, TXT_INPUT), NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_SHOWWINDOW);
        break;

    case WM_DROPFILES:
    {

        DWORD mCode;                                                           // MessageBox button code status
        DWORD textLength = GetWindowTextLength(GetDlgItem(hwnd, TXT_INPUT));   // Holds the number of characters in our EDIT control. We will use this variable to check if it's greater than one.

        // If we are working with a file and we want to create a new window, we ask the user to save their file first
        if (IsWorkingWithFile)
        {

            mCode = MessageBox(hwnd, L"Do you want to save your file?", L"Info", MB_ICONINFORMATION | MB_YESNOCANCEL);
            if (mCode == IDYES)
            {
                BasicFileSave(hwnd);

                DragAndDrop(hwnd, wParam);

                // We set to null because we are no longer working with the file from the previous section
                //filename = FALSE;
            }
            else if (mCode == IDNO) {
                MessageBox(hwnd, L"All work done in the previous file will be unsaved", L"Info", MB_OK);
                DragAndDrop(hwnd, wParam);

                // We set to null because we are no longer working with the file from the previous section
                //filename = FALSE;
            }
            else if (mCode == IDCANCEL) {} // Do nothing

        }
        // If we not working with a file and we want to create a new window, if we have any characters in our EDIT control, we ask the user to save their work
        else if (!IsWorkingWithFile && textLength > 0) {
            mCode = MessageBox(hwnd, L"Do you want to save your file?", L"Info", MB_YESNOCANCEL);
            if (mCode == IDYES)
            {
                BasicFileSave(hwnd);

                DragAndDrop(hwnd, wParam);

                // We set filename to FALSE because BasicFileSave sets filename to TRUE, however, we want to create a new file, so we set it to FALSE
                //filename = FALSE;

            }
            else if (mCode == IDNO) {
                MessageBox(hwnd, L"All work done in the previous file will be unsaved", L"Info", MB_OK);
                DragAndDrop(hwnd, wParam);
            }
            else if (mCode == IDCANCEL) {} // Do nothing
        }
        // If we aren't working with a file and we don't have any text, call DragAndDrop
        else {
            DragAndDrop(hwnd, wParam);
        }

        break;
    }

    case WM_COMMAND:
    {
        // For our other controls
        switch (LOWORD(wParam))
        {
            // For our menu
        case ID_FILE_NEW:
            NewWindow(hwnd);
            break;
        case ID_FILE_OPEN:
            BasicFileOpen(hwnd);
            break;
        case ID_FILE_SAVE:
            BasicFileSave(hwnd);
            break;
            //case IDM_FILE_EXIT:
        case ID_FILE_EXIT:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case ID_EDIT_UNDO:
            // https://devblogs.microsoft.com/oldnewthing/20070911-00/?p=25183
            // EM_UNDO and WM_UNDO do the same thing. I just use EM_UNDO to let users know this is an EDIT control
            SendMessage(GetDlgItem(hwnd, TXT_INPUT), EM_UNDO, 0, 0);
            break;
        case ID_EDIT_CUT:
            SendMessage(GetDlgItem(hwnd, TXT_INPUT), WM_CUT, 0, 0);
            break;
        case ID_EDIT_COPY:
            SendMessage(GetDlgItem(hwnd, TXT_INPUT), WM_COPY, 0, 0);
            break;
        case ID_EDIT_PASTE:
            SendMessage(GetDlgItem(hwnd, TXT_INPUT), WM_PASTE, 0, 0);
            break;

        case ID_EDIT_SELECTALL:
            SendMessage(GetDlgItem(hwnd, TXT_INPUT), EM_SETSEL, (WPARAM)0, (LPARAM)-1);
            break;
        case ID_HELP_ABOUT:
            // The first parameter HAS to be GetModuleHandle(NULL) if you want picture controls to even work. Holy s**t that is stupid
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG3), hwnd, (DLGPROC)ModalDlgProc);
            break;

            //case IDM_FILE_READINPUT:
        case ID_FILE_READINPUT:
            // We need the curly brackets to prevent Compiler Error C2360
        {
            DWORD bytesToAlloc = GetWindowTextLength(GetDlgItem(hwnd, TXT_INPUT));

            if (bytesToAlloc > 0)
            {
                WCHAR* buffer = new WCHAR[bytesToAlloc + sizeof(WCHAR)];

                GetDlgItemText(hwnd, TXT_INPUT, buffer, bytesToAlloc + sizeof(WCHAR));

                MessageBox(hwnd, buffer, TEXT("Reading Data..."), MB_OK);

                delete[] buffer;
            }
            else {
                MessageBox(hwnd, L"No data", TEXT("Reading Data..."), MB_OK);
            }

        }
        // This is just the short way of doing what we did above
        //GetDlgItemTextA(hwnd, TXT_INPUT, getText, sizeof(getText));
        break;

        //case IDM_FILE_OPENMODELESSDIALOG:
        case ID_FILE_OPENMODELESSDIALOG:
            //DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, (DLGPROC)DlgProc);
            // If there is no existing modeless dialog window, create the modeless dialog box
            if (!IsWindow(hwndModelessDialog)) {
                hwndModelessDialog = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hwnd, (DLGPROC)ModelessDlgProc);
                ShowWindow(hwndModelessDialog, SW_SHOW);
            }
            break;

            // case IDM_FILE_OPENMODALDIALOG:
        case ID_FILE_OPENMODALDIALOG:
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hwnd, (DLGPROC)ModalDlgProc);
            break;

            //case IDM_FILE_FIND:
        case ID_EDIT_FIND:
        {
            // Initialize our Find dialog box
            ZeroMemory(&fr, sizeof(fr));
            fr.lStructSize = sizeof(fr);
            fr.hwndOwner = hwnd;
            fr.lpstrFindWhat = findWhatBuffer;
            fr.wFindWhatLen = sizeof(findWhatBuffer);
            fr.Flags = 0;

            hFindDialog = FindText(&fr);

            break;
        }

        default:
            break;
        }

    }

    default:

        // Handle Find/Replace stuff; The reason why we check the uMsg specifically for our Find/Replace dialog box is because if we checked it with our switch/case statement, we would get an error saying how it's not a constant expression. If/Else statements however circumvent this error.
        // If we got a message from our Find dialog box
        if (uMsg == uFindReplaceMsg) {
            // We want to check the message/flags from find dialog box
            // Check out the different flags you can look for: https://learn.microsoft.com/en-us/windows/win32/dlgbox/findmsgstring
            // The lParam parameter of the FINDMSGSTRING message contains a pointer to a FINDREPLACE structure. The Flags member of this structure indicates the event that caused the message. Other members of the structure indicate the user's input.
            LPFINDREPLACE pFind = (LPFINDREPLACE)lParam;

            // If the FR_DIALOGTERM flag is set,
            // invalidate the handle identifying the dialog box.
            if (pFind->Flags & FR_DIALOGTERM)
            {
                hFindDialog = NULL;
                return 0;
                //MessageBox(hwnd, TEXT("The dialog box is closing. After the owner window processes this message, a handle to the dialog box is no longer valid."), TEXT("Info"), MB_OK);

            }
            // If "Find Next" button is clicked on the Find Dialog
            else if (pFind->Flags & FR_FINDNEXT) {

                // We search for whatever keyword we are looking for in our EDIT control
                INT find = SearchFile(hwnd, pFind->lpstrFindWhat,
                    (BOOL)(pFind->Flags & FR_DOWN),
                    (BOOL)(pFind->Flags & FR_MATCHCASE),
                    (BOOL)(pFind->Flags & FR_WHOLEWORD));

                // Get the handle to our EDIT control
                HWND hInputBox = GetDlgItem(hwnd, TXT_INPUT);

                // If we don't find a match
                if (find == -1)
                {
                    SendMessage(hInputBox, EM_SETSEL, (WPARAM)-1, (LPARAM)0);
                }
                // If we do find a match...
                else
                {
                    // Highlight the portion where the match was found
                    INT upTo = (find + wcslen(pFind->lpstrFindWhat));
                    SendMessage(hInputBox, EM_SETSEL, (WPARAM)find, (LPARAM)upTo);

                    // Match found! Adjust scroll position
                    // Also noticed how before, I used PostMessage above, but now I've used SendMessage. The reason is becaues there was a bug where it wouldn't always scroll the selected text into view, especially when it's near at the edge of the client area.
                    // I believe this is because of PostMessage being asynchronous. So we will wait until the text is highlighted, THEN we will move the scrollbar accordingly. How does the scrollbar move?
                    // Well, the message EM_SCROLLCARET basically moves the scrollbar until the caret is in view in an edit control
                    // If you don't know what a caret is, a caret is a indicator displayed on the screen to indicate where text input will be inserted. When you can type, you will usually see a line that's most likely blinking, that's the caret, it's where text will be inserted when you type
                    // So because when we select text, we are also moving the caret, by sending a message to move the scrollbar to see the caret will mean that we will now see the selected text, which is what we want.
                    // https://learn.microsoft.com/en-us/windows/win32/controls/em-scrollcaret
                    SendMessage(GetDlgItem(hwnd, TXT_INPUT), EM_SCROLLCARET, 0, 0);
                }
            }
            return 0;
        }
        else {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

        }

    }
    return 0;
}

BOOL CALLBACK ModelessDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        // Handle dialog messages here
        // ...
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            DestroyWindow(hwndDlg);
            hwndModelessDialog = NULL;
            return TRUE;

        case IDCANCEL:
            DestroyWindow(hwndDlg);
            hwndModelessDialog = NULL;
            return TRUE;

            // This is my own custom button on the dialog
        case IDC_BUTTON2:
            MessageBox(hwndDlg, TEXT("You clicked on the Modeless Funny Button"), TEXT("Information"), MB_OK);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CALLBACK ModalDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message) {
        // Handle dialog messages here
        // ...

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hwndDlg, wParam);
            return TRUE;

        case IDCANCEL:
            EndDialog(hwndDlg, wParam);
            return TRUE;

        }
    }
    return FALSE;
}