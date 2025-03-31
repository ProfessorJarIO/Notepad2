#include "commheaders.h"
#include "memoryManagement.h"

// This function frees up WM_UNDO/EM_UNDO buffer memory. If we don't do this, memory won't be freed until user types in a character, and deletes it. Pretty strange...
void FreeUndoBufferMemory(HWND hEdit) {
    // Step 1: Retrieve the handle
    HANDLE hUndoBuffer = (HANDLE)SendMessage(hEdit, EM_GETHANDLE, 0, 0);

    if (hUndoBuffer) {
        // Step 2: Free the buffer
        LocalFree(hUndoBuffer);
        SendMessage(hEdit, EM_SETHANDLE, 0, 0); // Set handle to NULL
    }

    // Step 3: Allocate new buffer
    // Because we allocated 0 bytes, that means the EDIT control falls back to it's internal buffer. Usually, this would be 32k characters/bytes. But because we changed this to 64 million characters when we adjusted EM_SETLIMITTEXT, the buffer will be set to 64 million bytes..
    // HANDLE hNewBuffer = LocalAlloc(LMEM_FIXED, 0); 
    // Just to be safe however, I will set it to be equal to that of MAX_NUM_CHARS, which is the same variable we used to set EM_SETLIMITTEXT. If we have any issues with the above, commment it out, then uncomment the line below this comment.
    HANDLE hNewBuffer = LocalAlloc(LMEM_FIXED, MAX_NUM_CHARS);

    if (hNewBuffer) {
        // Step 4: Set the new handle
        // Just FYI, the EDIT UNDO buffer is automatically reset whenever we send EM_SETHANDLE
        SendMessage(hEdit, EM_SETHANDLE, (WPARAM)hNewBuffer, 0);
    }
}