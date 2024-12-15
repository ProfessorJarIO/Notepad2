#pragma once

// Declare any variables

// Declare any functions
void ReverseString(const WCHAR* revTestData, WCHAR* outStr);
int SearchFile(HWND hwnd, const WCHAR* find, BOOL down, BOOL matchCase, BOOL matchWholeWord);
void SetEditControlTextInChunks(HWND hEdit, const std::string& text);